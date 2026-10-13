#!/usr/bin/env python3
"""
Rewrite Slang's GLSL output into desktop-OpenGL-compatible GLSL.

Slang targets Vulkan-GLSL, where a texture and its sampler are separate objects:

    #extension GL_EXT_samplerless_texture_functions : require
    layout(binding = 2) uniform texture2D fillTexture_0;
    layout(binding = 0) uniform sampler   fillSampler_0;
    ...
    texture(sampler2D(fillTexture_0, fillSampler_0), uv)

Desktop OpenGL GLSL has no `texture2D`/`texture3D` *type* and no bare `sampler`,
so those declarations are a syntax error and the whole shader fails to compile.
slangc (2025.4) has no flag to emit combined samplers, so we fold them here:

    layout(binding = 2) uniform sampler2D fillTexture_0;
    ...
    texture(fillTexture_0, uv)

Texture bindings are then renumbered to the D3D slot of the matching
hlsl_src texture (explicit register(tN), else declaration order) -- the slot the
C++ passes to ImPlatform_SetShaderTexture or, for slot 0, the texture ImGui binds
for the draw command. Slang numbers textures, samplers and uniform blocks in one
sequence, so without this `texture1` landed on unit 5.
`texelFetch` works on a combined sampler in core GLSL, which is why the
samplerless extension can simply be dropped.

Idempotent: running it on an already-converted file is a no-op.

Usage:  python glsl_combine_samplers.py [shaders/glsl]
"""

import os
import re
import sys

TEX_DECL = re.compile(
    r'^layout\(binding = (\d+)\)\s*\n'
    r'uniform texture(1D|2D|3D|Cube|2DArray)\s+([A-Za-z_][A-Za-z0-9_]*)\s*;',
    re.M)

SMP_DECL = re.compile(
    r'^layout\(binding = \d+\)\s*\n'
    r'uniform sampler\s+([A-Za-z_][A-Za-z0-9_]*)\s*;\s*\n',
    re.M)

EXT_LINE = re.compile(
    r'^#extension GL_EXT_samplerless_texture_functions\s*:\s*require\s*\n', re.M)

# sampler2D(tex_0,smp_0)  ->  tex_0     (Slang emits no space after the comma)
COMBINE_CALL = re.compile(
    r'\bsampler(?:1D|2D|3D|Cube|2DArray)\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*,'
    r'\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)')


def convert(text):
    changed = False

    # 1. the samplerless extension is unnecessary once samplers are combined
    text, n = EXT_LINE.subn('', text)
    changed |= n > 0

    # 2. `uniform texture2D X;` -> `uniform sampler2D X;` (binding preserved)
    def tex_sub(m):
        return 'layout(binding = %s)\nuniform sampler%s %s;' % (
            m.group(1), m.group(2), m.group(3))

    text, n = TEX_DECL.subn(tex_sub, text)
    changed |= n > 0

    # 3. fold `sampler2D(tex, smp)` down to the combined object
    text, n = COMBINE_CALL.subn(lambda m: m.group(1), text)
    changed |= n > 0

    # 4. drop the now-unreferenced standalone sampler declarations
    #    (only when nothing else still names them)
    #    Rescan after each removal: match offsets go stale once text shrinks.
    removed = True
    while removed:
        removed = False
        for m in SMP_DECL.finditer(text):
            body = text[:m.start()] + text[m.end():]
            if not re.search(r'\b' + re.escape(m.group(1)) + r'\b', body):
                text, changed, removed = body, True, True
                break

    return text, changed


HLSL_TEX = re.compile(
    r'^\s*Texture(?:1D|2D|3D|Cube|2DArray)(?:<[^>]*>)?\s+([A-Za-z_]\w*)\s*(?::\s*register\(\s*t(\d+)\s*\))?\s*;',
    re.M)
GLSL_SMP = re.compile(
    r'^layout\(binding = (\d+)\)(\s*\n\s*uniform sampler(?:1D|2D|3D|Cube|2DArray)\s+)([A-Za-z_]\w*?)_0\s*;',
    re.M)


def d3d_texture_slots(hlsl_path):
    """name -> D3D t-register: explicit register(tN), else declaration order."""
    try:
        with open(hlsl_path, 'r', encoding='utf-8', errors='replace') as f:
            src = f.read()
    except OSError:
        return {}
    slots, nxt, used = {}, 0, set()
    decls = HLSL_TEX.findall(src)
    for name, reg in decls:
        if reg:
            slots[name] = int(reg)
            used.add(int(reg))
    for name, reg in decls:
        if not reg:
            while nxt in used:
                nxt += 1
            slots[name] = nxt
            used.add(nxt)
    return slots


def remap_bindings(text, slots):
    """Texture units follow the D3D slots the C++ binds (Slang numbers samplers,
    textures and uniform blocks in one sequence, so `texture1` came out at 5)."""
    def sub(m):
        slot = slots.get(m.group(3))
        if slot is None:
            return m.group(0)
        return 'layout(binding = %d)%s%s_0;' % (slot, m.group(2), m.group(3))
    out = GLSL_SMP.sub(sub, text)
    return out, out != text


def hlsl_for(glsl_name, hlsl_dir):
    base = re.sub(r'_(ps|vs)\.glsl$', '', glsl_name)
    while base:
        cand = os.path.join(hlsl_dir, base + '.hlsl')
        if os.path.isfile(cand):
            return cand
        if '_' not in base:
            break
        base = base.rsplit('_', 1)[0]   # slug_fill -> slug
    return None


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        os.path.dirname(os.path.abspath(__file__)), 'shaders', 'glsl')
    hlsl_dir = os.path.join(os.path.dirname(os.path.abspath(root)), 'hlsl_src')
    if not os.path.isdir(root):
        print('not a directory: %s' % root)
        return 1

    touched = 0
    for name in sorted(os.listdir(root)):
        if not name.endswith('.glsl'):
            continue
        path = os.path.join(root, name)
        with open(path, 'r', encoding='utf-8', errors='replace', newline='') as f:
            src = f.read()
        out, changed = convert(src)
        hlsl = hlsl_for(name, hlsl_dir)
        if hlsl:
            out, remapped = remap_bindings(out, d3d_texture_slots(hlsl))
            changed |= remapped
        if changed:
            with open(path, 'w', encoding='utf-8', newline='') as f:
                f.write(out)
            print('  combined samplers: %s' % name)
            touched += 1

    print('glsl_combine_samplers: %d file(s) updated' % touched)
    return 0


if __name__ == '__main__':
    sys.exit(main())
