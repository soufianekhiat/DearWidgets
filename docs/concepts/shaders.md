# Shader Cross-Compilation

DearWidgets custom shaders are authored in HLSL and cross-compiled to GLSL, MSL, WGSL, and WGPU targets using [Slang](https://shader-slang.com/).

---

## Directory Layout

```
workingdir/
+-- bin/slang/bin/slangc.exe        # Bundled Slang compiler
+-- shaders/
|   +-- hlsl_src/                   # Authoritative HLSL source files
|   |   +-- template.hlsl           # Authoring template (not compiled directly)
|   |   +-- slug.hlsl               # Slug monochrome glyph shader
|   |   +-- slug_color.hlsl         # Slug COLR v0 color-layer shader
|   |   +-- slug_gradient.hlsl      # Slug COLR v1 gradient shader
|   |   +-- slug_debug.hlsl         # Slug debug visualization shader
|   |   +-- markers.hlsl            # GPU shape markers
|   |   +-- lines.hlsl              # GPU dashed polyline shader
|   |   +-- lines_copy.hlsl         # Dashed polyline copy pass
|   +-- glsl/                       # Generated GLSL outputs
|   +-- msl/                        # Generated MSL outputs (Metal)
|   +-- wgsl/                       # Generated WGSL outputs (WebGPU via Naga)
|   +-- wgpu/                       # Generated WGPU outputs
+-- generate_shaders_all.bat        # Compile all shaders for all targets
+-- generate_glsl.bat               # Compile pixel shaders to GLSL only
+-- generate_msl.bat                # Compile markers pixel shader to MSL only
+-- generate_wgsl.bat               # Compile markers pixel shader to WGSL only
+-- generate_wgpu.bat               # Compile markers pixel shader to WGPU only
```

---

## HLSL Authoring Convention

All shaders follow the template layout from `hlsl_src/template.hlsl`:

```hlsl
cbuffer PS_CONSTANT_BUFFER { /* per-draw constants */ };
cbuffer VB_CONSTANT_BUFFER { /* vertex buffer constants */ };

struct VS_INPUT { /* vertex attributes */ };
struct PS_INPUT  { /* interpolants */ };

PS_INPUT main_vs(VS_INPUT input) { ... }   // vertex entry point
float4   main_ps(PS_INPUT input) : SV_Target { ... }   // pixel entry point
```

**Entry point names are fixed:**
- Vertex shader: `main_vs`
- Pixel shader: `main_ps`

**Profile:** `sm_5_1` (Shader Model 5.1) is used for all targets.

---

## Compilation Command

The pattern used by `generate_shaders_all.bat`:

```bat
slangc.exe -lang hlsl -profile sm_5_1 <source.hlsl> \
    -stage vertex -entry main_vs \
    [-target metal] \
    -o shaders/<target>/<name>_vs.<ext>

slangc.exe -lang hlsl -profile sm_5_1 <source.hlsl> \
    -stage pixel -entry main_ps \
    [-target metal] \
    -o shaders/<target>/<name>_ps.<ext>
```

The `-target metal` flag is required for MSL only; other targets are inferred from the output file extension.

---

## Output Naming

For `generate_shaders_all.bat` (full compilation):

| Output file | Description |
|---|---|
| `glsl/<name>_vs.glsl` | GLSL vertex shader |
| `glsl/<name>_ps.glsl` | GLSL pixel shader |
| `msl/<name>_vs.msl` | MSL vertex shader (Metal) |
| `msl/<name>_ps.msl` | MSL pixel shader (Metal) |
| `wgsl/<name>_vs.wgsl` | WGSL vertex shader |
| `wgsl/<name>_ps.wgsl` | WGSL pixel shader |
| `wgpu/<name>_vs.wgpu` | WGPU vertex shader |
| `wgpu/<name>_ps.wgpu` | WGPU pixel shader |

The `<name>` matches the HLSL source filename without extension (e.g., `slug.hlsl` -> `slug_vs.glsl`, `slug_ps.glsl`).

---

## Per-Target Scripts

The individual scripts are convenience shortcuts for iterating on a single target:

| Script | Target | Scope |
|---|---|---|
| `generate_glsl.bat` | GLSL | Pixel shaders only, all `hlsl_src/` files |
| `generate_msl.bat` | MSL (Metal) | `markers.hlsl` pixel shader only |
| `generate_wgsl.bat` | WGSL | `markers.hlsl` pixel shader only |
| `generate_wgpu.bat` | WGPU | `markers.hlsl` pixel shader only |

> **Note:** The per-target scripts are currently scoped to pixel shaders or single shaders only. Use `generate_shaders_all.bat` for a complete rebuild of all shaders for all targets.

---

## Adding a New Shader

1. Create `workingdir/shaders/hlsl_src/<name>.hlsl` following the template conventions (entry points `main_vs` / `main_ps`).
2. Run `generate_shaders_all.bat` from `workingdir/` to produce all target outputs.
3. Register the shader in the ImPlatform backend using `ImPlatform_CreateShader` with the appropriate compiled source.
4. Use `ImPlatform_BeginCustomShader` / `ImPlatform_EndCustomShader` in your draw code to activate it.
