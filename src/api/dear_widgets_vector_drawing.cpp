// dear_widgets_vector_drawing.cpp
// Vector Drawing Tool widget -- world-space bezier path authoring canvas.
// Extracted from dear_widgets.cpp as its own translation unit; compiled
// standalone and resolved via the linker (not unity-build #include).

#include "dear_widgets.h"
#include "dear_widgets_internal.h"
#include "imgui_internal.h"

namespace ImWidgets {

    // Local copies of two trivial helpers that still live as file-statics
    // inside dear_widgets.cpp. Duplicating them here keeps this TU independent
    // without the overhead of promoting them to external-linkage symbols.
    static inline float DWE_Len2(const ImVec2& a) { return ImSqrt(a.x * a.x + a.y * a.y); }
    static ImVec2 DWE_CubicBezier(const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float t)
    {
        float u = 1.0f - t;
        float uu = u * u;
        float uuu = uu * u;
        float tt = t * t;
        float ttt = tt * t;
        return ImVec2(uuu * p0.x + 3.0f * uu * t * p1.x + 3.0f * u * tt * p2.x + ttt * p3.x,
                      uuu * p0.y + 3.0f * uu * t * p1.y + 3.0f * u * tt * p2.y + ttt * p3.y);
    }

    //==================================================================
    // W2. Vector Drawing Tool (canvas with zoom/pan + bezier authoring)
    //==================================================================
    // Convert world coords <-> screen coords using (PanOffset, Zoom).
    // World (0,0) maps to (canvas_min + PanOffset). Zoom scales from world units.
    static inline ImVec2 DWE_VDTW2S(const ImVectorDrawingData& d, ImVec2 canvas_min, ImVec2 w)
    {
        return ImVec2(canvas_min.x + d.PanOffset.x + w.x * d.Zoom,
                      canvas_min.y + d.PanOffset.y + w.y * d.Zoom);
    }
    static inline ImVec2 DWE_VDTS2W(const ImVectorDrawingData& d, ImVec2 canvas_min, ImVec2 s)
    {
        return ImVec2((s.x - canvas_min.x - d.PanOffset.x) / d.Zoom,
                      (s.y - canvas_min.y - d.PanOffset.y) / d.Zoom);
    }

    // Flatten one path into a cubic Bezier control-point array [p0, p1, p2, p3, p4, p5, p6, ...]
    // suitable for DrawStrokedBezierPath or a plain polyline (via evaluation).
    static void DWE_VDTFlattenBezierCP(const ImVectorDrawingPath& path, ImVector<ImVec2>& cp)
    {
        cp.resize(0);
        int n = path.Nodes.Size;
        if (n < 2) return;
        int end = path.Closed ? n : n - 1;
        for (int i = 0; i < end; ++i)
        {
            const ImVectorDrawingNode& a = path.Nodes[i];
            const ImVectorDrawingNode& b = path.Nodes[(i + 1) % n];
            ImVec2 P0 = a.Anchor;
            ImVec2 P1(a.Anchor.x + a.OutTangent.x, a.Anchor.y + a.OutTangent.y);
            ImVec2 P2(b.Anchor.x + b.InTangent.x,  b.Anchor.y + b.InTangent.y);
            ImVec2 P3 = b.Anchor;
            if (cp.empty()) cp.push_back(P0);
            cp.push_back(P1);
            cp.push_back(P2);
            cp.push_back(P3);
        }
    }

    static void DWE_VDTFlattenPolyline(const ImVectorDrawingPath& path, int samples_per_seg,
                                       ImVector<ImVec2>& out)
    {
        out.resize(0);
        int n = path.Nodes.Size;
        if (n < 2) return;
        int end = path.Closed ? n : n - 1;
        for (int i = 0; i < end; ++i)
        {
            const ImVectorDrawingNode& a = path.Nodes[i];
            const ImVectorDrawingNode& b = path.Nodes[(i + 1) % n];
            ImVec2 P0 = a.Anchor;
            ImVec2 P1(a.Anchor.x + a.OutTangent.x, a.Anchor.y + a.OutTangent.y);
            ImVec2 P2(b.Anchor.x + b.InTangent.x,  b.Anchor.y + b.InTangent.y);
            ImVec2 P3 = b.Anchor;
            if (out.empty()) out.push_back(P0);
            for (int k = 1; k <= samples_per_seg; ++k)
            {
                float t = (float)k / (float)samples_per_seg;
                out.push_back(DWE_CubicBezier(P0, P1, P2, P3, t));
            }
        }
    }

    bool VectorDrawingTool(const char* label, ImVectorDrawingData& data, ImVec2 size)
    {
        ImGuiWindow* win = ImGui::GetCurrentWindow();
        if (win->SkipItems) return false;
        if (size.x <= 0.0f) size.x = ImGui::GetContentRegionAvail().x;
        if (size.y <= 0.0f) size.y = 360.0f;
        ImGui::PushID(label);
        ImVec2 canvas_min = ImGui::GetCursorScreenPos();
        ImVec2 canvas_max(canvas_min.x + size.x, canvas_min.y + size.y);
        ImGui::InvisibleButton("##vdt_canvas", size, ImGuiButtonFlags_MouseButtonLeft
                                                     | ImGuiButtonFlags_MouseButtonRight
                                                     | ImGuiButtonFlags_MouseButtonMiddle);
        bool hovered = ImGui::IsItemHovered();
        bool active = ImGui::IsItemActive();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->PushClipRect(canvas_min, canvas_max, true);
        dl->AddRectFilled(canvas_min, canvas_max, IM_COL32(18, 22, 28, 255));

        // Grid (world-space at fixed world spacing).
        {
            float grid_w = GetStyle().VectorDrawing_GridSpacing;
            float grid_px = grid_w * data.Zoom;
            if (grid_px >= 4.0f)
            {
                float x0 = canvas_min.x + ImFmod(data.PanOffset.x, grid_px);
                float y0 = canvas_min.y + ImFmod(data.PanOffset.y, grid_px);
                for (float x = x0; x < canvas_max.x; x += grid_px)
                    dl->AddLine(ImVec2(x, canvas_min.y), ImVec2(x, canvas_max.y), IM_COL32(255, 255, 255, 15));
                for (float y = y0; y < canvas_max.y; y += grid_px)
                    dl->AddLine(ImVec2(canvas_min.x, y), ImVec2(canvas_max.x, y), IM_COL32(255, 255, 255, 15));
            }
            // World origin crosshair
            ImVec2 o = DWE_VDTW2S(data, canvas_min, ImVec2(0, 0));
            dl->AddLine(ImVec2(o.x - 10, o.y), ImVec2(o.x + 10, o.y), IM_COL32(200, 80, 80, 200));
            dl->AddLine(ImVec2(o.x, o.y - 10), ImVec2(o.x, o.y + 10), IM_COL32(80, 180, 80, 200));
        }

        bool changed = false;
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mouse = io.MousePos;

        // ---- Pan / zoom ----
        if (hovered && ImGui::IsMouseDown(ImGuiMouseButton_Middle))
        {
            ImVec2 d = io.MouseDelta;
            data.PanOffset.x += d.x;
            data.PanOffset.y += d.y;
        }
        // Shift + left drag pans too (convenience when no middle button).
        if (hovered && io.KeyShift && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            ImVec2 d = io.MouseDelta;
            data.PanOffset.x += d.x;
            data.PanOffset.y += d.y;
        }
        if (hovered && io.MouseWheel != 0.0f)
        {
            float old_zoom = data.Zoom;
            float factor = ImPow(1.1f, io.MouseWheel);
            data.Zoom = ImClamp(old_zoom * factor, 0.05f, 20.0f);
            // Zoom around mouse: keep the world-point under the cursor stationary.
            ImVec2 wbefore = DWE_VDTS2W(data, canvas_min, mouse);
            // After zoom change, compute where wbefore lands; compensate via PanOffset.
            ImVec2 safter  = ImVec2(canvas_min.x + data.PanOffset.x + wbefore.x * data.Zoom,
                                    canvas_min.y + data.PanOffset.y + wbefore.y * data.Zoom);
            data.PanOffset.x += (mouse.x - safter.x);
            data.PanOffset.y += (mouse.y - safter.y);
        }

        // ---- Hit test existing nodes (anchors and handles) ----
        int hit_path = -1, hit_node = -1, hit_handle = 0;
        const float hitR = 7.0f;
        for (int pi = 0; pi < data.Paths.Size && hovered; ++pi)
        {
            ImVectorDrawingPath& p = data.Paths[pi];
            for (int ni = 0; ni < p.Nodes.Size; ++ni)
            {
                ImVec2 ap = DWE_VDTW2S(data, canvas_min, p.Nodes[ni].Anchor);
                ImVec2 inp = DWE_VDTW2S(data, canvas_min,
                                        ImVec2(p.Nodes[ni].Anchor.x + p.Nodes[ni].InTangent.x,
                                               p.Nodes[ni].Anchor.y + p.Nodes[ni].InTangent.y));
                ImVec2 outp = DWE_VDTW2S(data, canvas_min,
                                         ImVec2(p.Nodes[ni].Anchor.x + p.Nodes[ni].OutTangent.x,
                                                p.Nodes[ni].Anchor.y + p.Nodes[ni].OutTangent.y));
                if (DWE_Len2(ImVec2(mouse.x - ap.x, mouse.y - ap.y)) < hitR) { hit_path = pi; hit_node = ni; hit_handle = 0; }
                else if ((p.Nodes[ni].InTangent.x != 0.0f || p.Nodes[ni].InTangent.y != 0.0f)
                         && DWE_Len2(ImVec2(mouse.x - inp.x, mouse.y - inp.y)) < hitR)
                    { hit_path = pi; hit_node = ni; hit_handle = 1; }
                else if ((p.Nodes[ni].OutTangent.x != 0.0f || p.Nodes[ni].OutTangent.y != 0.0f)
                         && DWE_Len2(ImVec2(mouse.x - outp.x, mouse.y - outp.y)) < hitR)
                    { hit_path = pi; hit_node = ni; hit_handle = 2; }
            }
        }

        // ---- Click handling (only when NOT panning) ----
        bool panning = io.KeyShift || ImGui::IsMouseDown(ImGuiMouseButton_Middle);

        // Hover feedback: tell the user whether a click adds a new anchor or
        // grabs an existing anchor/handle. Cursor + tooltip + a ghost ring at
        // mouse position when over empty canvas. The actual highlight halo on
        // hovered anchor/handle is drawn during the render loop below.
        if (hovered && !panning)
        {
            if (hit_path >= 0)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                if (hit_handle == 0)      ImGui::SetTooltip("Click to select / drag to move anchor");
                else if (hit_handle == 1) ImGui::SetTooltip("Drag to adjust In tangent");
                else                      ImGui::SetTooltip("Drag to adjust Out tangent");
            }
            else
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                ImGui::SetTooltip(data.ActivePath >= 0
                    ? "Click to add anchor (right-click: finish path)"
                    : "Click to start a new path");
            }
        }

        if (hovered && !panning && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            ImVec2 w = DWE_VDTS2W(data, canvas_min, mouse);
            if (hit_path >= 0)
            {
                // Authoring close-check: if clicking first node of active path, close it.
                if (data.ActivePath >= 0 && hit_path == data.ActivePath && hit_node == 0
                    && data.Paths[hit_path].Nodes.Size >= 3)
                {
                    // Derive InTangent for the first node so the closing segment
                    // inherits curvature instead of going straight. Only set when
                    // InTangent is zero (user never dragged it) to preserve
                    // intentional tangent-less (straight) closing segments.
                    ImVectorDrawingPath& cp = data.Paths[hit_path];
                    ImVectorDrawingNode& n0 = cp.Nodes[0];
                    if (n0.InTangent.x == 0.0f && n0.InTangent.y == 0.0f)
                    {
                        if (!n0.Broken && (n0.OutTangent.x != 0.0f || n0.OutTangent.y != 0.0f))
                            n0.InTangent = ImVec2(-n0.OutTangent.x, -n0.OutTangent.y);
                        else
                        {
                            const ImVectorDrawingNode& nL = cp.Nodes[cp.Nodes.Size - 1];
                            if (nL.OutTangent.x != 0.0f || nL.OutTangent.y != 0.0f)
                                n0.InTangent = ImVec2(-nL.OutTangent.x, -nL.OutTangent.y);
                        }
                    }
                    cp.Closed = true;
                    data.ActivePath = -1;
                    data.SelectedPath = hit_path;
                    data.SelectedNode = 0;
                    data.SelectedHandle = 0;
                    changed = true;
                }
                else
                {
                    // Select existing. Shift/Ctrl toggles multi-selection within the same path.
                    bool shift = ImGui::GetIO().KeyShift;
                    bool ctrl  = ImGui::GetIO().KeyCtrl;
                    if (data.SelectedPath != hit_path) { data.SelectedNodes.clear(); }
                    data.SelectedPath = hit_path;
                    data.SelectedNode = hit_node;
                    data.SelectedHandle = hit_handle;
                    if (shift || ctrl)
                    {
                        bool in = false;
                        for (int k = 0; k < data.SelectedNodes.Size; ++k)
                            if (data.SelectedNodes[k] == hit_node) { in = true; data.SelectedNodes.erase(data.SelectedNodes.Data + k); break; }
                        if (!in) data.SelectedNodes.push_back(hit_node);
                    }
                    else
                    {
                        data.SelectedNodes.clear();
                        data.SelectedNodes.push_back(hit_node);
                    }
                }
            }
            else
            {
                // Add new anchor.
                if (data.ActivePath < 0)
                {
                    ImVectorDrawingPath np;
                    data.Paths.push_back(np);
                    data.ActivePath = data.Paths.Size - 1;
                }
                ImVectorDrawingPath& p = data.Paths[data.ActivePath];
                p.Nodes.push_back(ImVectorDrawingNode(w));
                data.SelectedPath = data.ActivePath;
                data.SelectedNode = p.Nodes.Size - 1;
                data.SelectedHandle = 2; // drag to pull out tangent
                changed = true;
            }
        }

        // Drag-pull tangent on newly-placed anchor: while left-down after placement,
        // set Out/In tangents based on drag.
        if (active && !panning && data.SelectedPath >= 0 && data.SelectedNode >= 0
            && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            ImVectorDrawingPath& p = data.Paths[data.SelectedPath];
            if (data.SelectedNode < p.Nodes.Size)
            {
                ImVectorDrawingNode& n = p.Nodes[data.SelectedNode];
                ImVec2 w = DWE_VDTS2W(data, canvas_min, mouse);
                if (data.SelectedHandle == 0)
                {
                    // Move anchor. If multi-selected, translate the whole group by the same delta.
                    ImVec2 delta(w.x - n.Anchor.x, w.y - n.Anchor.y);
                    if (data.SelectedNodes.Size > 1)
                    {
                        for (int k = 0; k < data.SelectedNodes.Size; ++k)
                        {
                            int idx = data.SelectedNodes[k];
                            if (idx < 0 || idx >= p.Nodes.Size) continue;
                            p.Nodes[idx].Anchor.x += delta.x;
                            p.Nodes[idx].Anchor.y += delta.y;
                        }
                    }
                    else
                    {
                        n.Anchor = w;
                    }
                }
                else if (data.SelectedHandle == 1)
                {
                    n.InTangent = ImVec2(w.x - n.Anchor.x, w.y - n.Anchor.y);
                    if (!n.Broken) n.OutTangent = ImVec2(-n.InTangent.x, -n.InTangent.y);
                }
                else // 2 = out
                {
                    n.OutTangent = ImVec2(w.x - n.Anchor.x, w.y - n.Anchor.y);
                    if (!n.Broken) n.InTangent = ImVec2(-n.OutTangent.x, -n.OutTangent.y);
                }
                changed = true;
            }
        }

        // Right-click: if a path is active, finish it; otherwise open context menu.
        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            if (data.ActivePath >= 0)
            {
                data.ActivePath = -1;
            }
            else
            {
                if (hit_path >= 0) { data.SelectedPath = hit_path; data.SelectedNode = hit_node; }
                ImGui::OpenPopup("##vdt_ctx");
            }
        }
        if (ImGui::BeginPopup("##vdt_ctx"))
        {
            if (data.SelectedPath >= 0 && data.SelectedPath < data.Paths.Size)
            {
                ImVectorDrawingPath& p = data.Paths[data.SelectedPath];
                if (data.SelectedNode >= 0 && data.SelectedNode < p.Nodes.Size)
                {
                    ImVectorDrawingNode& n = p.Nodes[data.SelectedNode];
                    if (ImGui::MenuItem("Node: Make corner (tangents=0)"))
                    {
                        n.InTangent = ImVec2(0, 0); n.OutTangent = ImVec2(0, 0);
                        changed = true;
                    }
                    if (ImGui::MenuItem("Node: Make smooth (mirror tangents)"))
                    {
                        n.OutTangent = ImVec2(-n.InTangent.x, -n.InTangent.y);
                        n.Broken = false;
                        changed = true;
                    }
                    if (ImGui::MenuItem("Node: Break tangents", NULL, n.Broken))
                    {
                        n.Broken = !n.Broken; changed = true;
                    }
                    if (ImGui::MenuItem("Node: Delete"))
                    {
                        p.Nodes.erase(p.Nodes.Data + data.SelectedNode);
                        if (p.Nodes.empty())
                            data.Paths.erase(data.Paths.Data + data.SelectedPath), data.SelectedPath = -1;
                        data.SelectedNode = -1;
                        changed = true;
                    }
                    ImGui::Separator();
                }
                if (ImGui::MenuItem("Path: Reverse direction"))
                {
                    for (int a = 0, b = p.Nodes.Size - 1; a < b; ++a, --b)
                    {
                        ImVectorDrawingNode tmp = p.Nodes[a];
                        p.Nodes[a] = p.Nodes[b];
                        p.Nodes[b] = tmp;
                    }
                    for (int i = 0; i < p.Nodes.Size; ++i)
                    {
                        ImVec2 t = p.Nodes[i].InTangent;
                        p.Nodes[i].InTangent = p.Nodes[i].OutTangent;
                        p.Nodes[i].OutTangent = t;
                    }
                    changed = true;
                }
                if (ImGui::MenuItem("Path: Toggle closed", NULL, p.Closed))
                {
                    p.Closed = !p.Closed; changed = true;
                }
                if (ImGui::MenuItem("Path: Duplicate"))
                {
                    ImVectorDrawingPath clone = p;
                    for (int i = 0; i < clone.Nodes.Size; ++i)
                    {
                        clone.Nodes[i].Anchor.x += 20.0f;
                        clone.Nodes[i].Anchor.y += 20.0f;
                    }
                    data.Paths.push_back(clone);
                    changed = true;
                }
                if (ImGui::MenuItem("Path: Delete"))
                {
                    data.Paths.erase(data.Paths.Data + data.SelectedPath);
                    data.SelectedPath = -1; data.SelectedNode = -1;
                    changed = true;
                }
                ImGui::Separator();
            }
            if (ImGui::MenuItem("View: Reset (pan=0, zoom=1)"))
            {
                data.PanOffset = ImVec2(0, 0); data.Zoom = 1.0f;
            }
            ImGui::EndPopup();
        }

        // Delete selected node(s). If multi-selected, deletes all at once.
        if (hovered && ImGui::IsKeyPressed(ImGuiKey_Delete)
            && data.SelectedPath >= 0 && data.SelectedNode >= 0)
        {
            ImVectorDrawingPath& p = data.Paths[data.SelectedPath];
            // Collect indices (descending).
            ImVector<int> to_del;
            if (!data.SelectedNodes.empty()) to_del = data.SelectedNodes;
            else to_del.push_back(data.SelectedNode);
            for (int i = 1; i < to_del.Size; ++i)
            {
                int k = to_del[i]; int j = i - 1;
                while (j >= 0 && to_del[j] < k) { to_del[j + 1] = to_del[j]; --j; }
                to_del[j + 1] = k;
            }
            for (int i = 0; i < to_del.Size; ++i)
            {
                int idx = to_del[i];
                if (idx >= 0 && idx < p.Nodes.Size) p.Nodes.erase(p.Nodes.Data + idx);
            }
            if (p.Nodes.empty())
            {
                data.Paths.erase(data.Paths.Data + data.SelectedPath);
                if (data.ActivePath == data.SelectedPath) data.ActivePath = -1;
                data.SelectedPath = -1;
            }
            data.SelectedNode = -1;
            data.SelectedNodes.clear();
            if (!to_del.empty()) changed = true;
        }

        // ---- Render paths ----
        for (int pi = 0; pi < data.Paths.Size; ++pi)
        {
            const ImVectorDrawingPath& p = data.Paths[pi];
            // Path drawing requires at least 2 nodes, but handles are drawn even
            // for a single-node path so the first click is immediately visible.
            if (p.Nodes.Size >= 2)
            {
                ImVector<ImVec2> cp_world; DWE_VDTFlattenBezierCP(p, cp_world);
                ImVector<ImVec2> cp_screen; cp_screen.resize(cp_world.Size);
                for (int i = 0; i < cp_world.Size; ++i)
                    cp_screen[i] = DWE_VDTW2S(data, canvas_min, cp_world[i]);
                float th = p.Thickness;
                switch (p.Style)
                {
                case ImVectorDrawingStyle_Polyline:
                    {
                        ImVector<ImVec2> pl; DWE_VDTFlattenPolyline(p, 16, pl);
                        for (int i = 0; i < pl.Size; ++i) pl[i] = DWE_VDTW2S(data, canvas_min, pl[i]);
                        if (pl.Size >= 2)
                            dl->AddPolyline(pl.Data, pl.Size, p.Color, p.Closed ? ImDrawFlags_Closed : 0, th);
                    }
                    break;
                case ImVectorDrawingStyle_PolylineAA:
                    {
                        ImVector<ImVec2> pl; DWE_VDTFlattenPolyline(p, 16, pl);
                        for (int i = 0; i < pl.Size; ++i) pl[i] = DWE_VDTW2S(data, canvas_min, pl[i]);
                        if (pl.Size >= 2)
                            DrawPolylineAA(dl, pl.Data, pl.Size, p.Color, th, p.Closed,
                                           (ImWidgetsCap)p.Cap, (ImWidgetsJoin)p.Join);
                    }
                    break;
                case ImVectorDrawingStyle_StrokedBezier:
                    if (cp_screen.Size >= 4)
                        DrawStrokedBezierPath(dl, cp_screen.Data, cp_screen.Size, p.Color, th,
                                              (ImWidgetsCap)p.Cap, (ImWidgetsJoin)p.Join, 4.0f, 0.25f, p.Closed);
                    break;
                case ImVectorDrawingStyle_StrokedDashedBezier:
                    if (cp_screen.Size >= 4)
                    {
                        float dashes[2] = { p.DashLen, p.GapLen };
                        DrawStrokedDashedBezierPath(dl, cp_screen.Data, cp_screen.Size, p.Color, th,
                                                    dashes, 2, 0.0f,
                                                    (ImWidgetsCap)p.Cap, (ImWidgetsJoin)p.Join, 4.0f, 0.25f, p.Closed);
                    }
                    break;
                case ImVectorDrawingStyle_DashedPolyline:
                    {
                        ImVector<ImVec2> pl; DWE_VDTFlattenPolyline(p, 16, pl);
                        for (int i = 0; i < pl.Size; ++i) pl[i] = DWE_VDTW2S(data, canvas_min, pl[i]);
                        if (pl.Size >= 2)
                            DrawDashedPolylineAA(dl, pl.Data, pl.Size, p.Color, th,
                                                 p.DashLen, p.GapLen, 0.0f, p.Closed,
                                                 (ImWidgetsCap)p.Cap, (ImWidgetsJoin)p.Join);
                    }
                    break;
                default: break;
                }
            }

            // Handles (only for selected path, plus active authoring path).
            // Runs even for single-node paths so the first placed anchor is visible.
            bool show_handles = (pi == data.SelectedPath) || (pi == data.ActivePath);
            if (show_handles)
            {
                for (int ni = 0; ni < p.Nodes.Size; ++ni)
                {
                    ImVec2 ap = DWE_VDTW2S(data, canvas_min, p.Nodes[ni].Anchor);
                    ImVec2 inp = DWE_VDTW2S(data, canvas_min,
                                            ImVec2(p.Nodes[ni].Anchor.x + p.Nodes[ni].InTangent.x,
                                                   p.Nodes[ni].Anchor.y + p.Nodes[ni].InTangent.y));
                    ImVec2 outp = DWE_VDTW2S(data, canvas_min,
                                             ImVec2(p.Nodes[ni].Anchor.x + p.Nodes[ni].OutTangent.x,
                                                    p.Nodes[ni].Anchor.y + p.Nodes[ni].OutTangent.y));
                    ImWidgetsStyle& vdt_style = GetStyle();
                    float vdt_ar = ImPlatform_LpToPx(vdt_style.VectorDrawing_AnchorRadius);
                    float vdt_tr = ImPlatform_LpToPx(vdt_style.VectorDrawing_TangentRadius);
                    if (p.Nodes[ni].InTangent.x != 0.0f || p.Nodes[ni].InTangent.y != 0.0f)
                    {
                        dl->AddLine(ap, inp, IM_COL32(255, 200, 80, 180), 1.0f);
                        dl->AddCircleFilled(inp, vdt_tr, IM_COL32(255, 200, 80, 220));
                    }
                    if (p.Nodes[ni].OutTangent.x != 0.0f || p.Nodes[ni].OutTangent.y != 0.0f)
                    {
                        dl->AddLine(ap, outp, IM_COL32(255, 200, 80, 180), 1.0f);
                        dl->AddCircleFilled(outp, vdt_tr, IM_COL32(255, 200, 80, 220));
                    }
                    bool sel = (pi == data.SelectedPath && ni == data.SelectedNode);
                    if (!sel && pi == data.SelectedPath) {
                        for (int k = 0; k < data.SelectedNodes.Size; ++k)
                            if (data.SelectedNodes[k] == ni) { sel = true; break; }
                    }
                    float ar = sel ? vdt_ar * 1.25f : vdt_ar;
                    dl->AddCircleFilled(ap, ar,
                                        sel ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 255, 255, 255));
                    dl->AddCircle(ap, ar, IM_COL32(0, 0, 0, 255), 16, 1.0f);
                }
                // First-node ring indicator when closing is possible.
                if (pi == data.ActivePath && p.Nodes.Size >= 3)
                {
                    ImVec2 ap0 = DWE_VDTW2S(data, canvas_min, p.Nodes[0].Anchor);
                    dl->AddCircle(ap0, 9.0f, IM_COL32(120, 220, 255, 200), 24, 1.5f);
                }
            }
        }

        // Hover feedback overlay: highlight the anchor/handle under the cursor,
        // or draw a ghost ring at the mouse when the canvas is empty so the
        // user can see exactly where a click would place a new anchor.
        if (hovered && !panning)
        {
            float vdt_ar = ImPlatform_LpToPx(GetStyle().VectorDrawing_AnchorRadius);
            float vdt_tr = ImPlatform_LpToPx(GetStyle().VectorDrawing_TangentRadius);
            if (hit_path >= 0 && hit_node >= 0
                && hit_path < data.Paths.Size
                && hit_node < data.Paths[hit_path].Nodes.Size)
            {
                const ImVectorDrawingNode& hn = data.Paths[hit_path].Nodes[hit_node];
                ImVec2 ap = DWE_VDTW2S(data, canvas_min, hn.Anchor);
                if (hit_handle == 0)
                {
                    dl->AddCircle(ap, vdt_ar + 3.0f, IM_COL32(120, 220, 255, 230), 24, 2.0f);
                }
                else if (hit_handle == 1)
                {
                    ImVec2 inp = DWE_VDTW2S(data, canvas_min,
                        ImVec2(hn.Anchor.x + hn.InTangent.x, hn.Anchor.y + hn.InTangent.y));
                    dl->AddCircle(inp, vdt_tr + 3.0f, IM_COL32(120, 220, 255, 230), 16, 2.0f);
                }
                else
                {
                    ImVec2 outp = DWE_VDTW2S(data, canvas_min,
                        ImVec2(hn.Anchor.x + hn.OutTangent.x, hn.Anchor.y + hn.OutTangent.y));
                    dl->AddCircle(outp, vdt_tr + 3.0f, IM_COL32(120, 220, 255, 230), 16, 2.0f);
                }
            }
            else
            {
                // Ghost ring at the mouse to show "click here adds an anchor".
                dl->AddCircle(mouse, vdt_ar, IM_COL32(255, 255, 255, 140), 16, 1.0f);
                dl->AddLine(ImVec2(mouse.x - 5, mouse.y), ImVec2(mouse.x + 5, mouse.y),
                            IM_COL32(255, 255, 255, 200), 1.0f);
                dl->AddLine(ImVec2(mouse.x, mouse.y - 5), ImVec2(mouse.x, mouse.y + 5),
                            IM_COL32(255, 255, 255, 200), 1.0f);
            }
        }

        // Status overlay.
        {
            char status[128];
            ImFormatString(status, sizeof(status),
                           "zoom=%.2fx  pan=(%.0f,%.0f)  paths=%d  %s",
                           (double)data.Zoom, (double)data.PanOffset.x, (double)data.PanOffset.y,
                           data.Paths.Size, data.ActivePath >= 0 ? "drawing" : "idle");
            dl->AddText(ImVec2(canvas_min.x + 6, canvas_min.y + 4),
                        IM_COL32(200, 200, 200, 200), status);
        }
        dl->AddRect(canvas_min, canvas_max, IM_COL32(255, 255, 255, 60));
        dl->PopClipRect();

        // Expand-to-window button in the top-right of the canvas -- mirrors the
        // pattern used by Histogram / ToneCurve / ImageViewer. When inside an
        // already-expanded window, WidgetExpandButton returns NULL and the
        // block is a no-op (no nested expand).
        ImGuiID vdt_id = ImGui::GetID("##vdt_expand");
        ImRect canvas_bb(canvas_min, canvas_max);
        bool* pExpanded = WidgetExpandButton(vdt_id, canvas_bb);
        if (pExpanded && *pExpanded)
        {
            if (BeginExpandedWindow(label, vdt_id, pExpanded, ImPlatform_LpToPx(ImVec2(1200, 720))))
            {
                // Two-pane modal layout (no scrolling): canvas on the left,
                // per-path options on the right. Left/right widths sum to the
                // full content region so nothing clips or scrolls.
                ImVec2 avail = ImGui::GetContentRegionAvail();
                float panel_w = ImMax(ImPlatform_LpToPx(260.0f), avail.x * 0.25f);
                float spacing = ImGui::GetStyle().ItemSpacing.x;
                float canvas_w = ImMax(ImPlatform_LpToPx(200.0f), avail.x - panel_w - spacing);

                ImGui::BeginChild("##vdt_modal_canvas", ImVec2(canvas_w, avail.y), false,
                                  ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                VectorDrawingTool("##vdt_modal", data, ImGui::GetContentRegionAvail());
                ImGui::EndChild();

                ImGui::SameLine();

                ImGui::BeginChild("##vdt_modal_opts", ImVec2(panel_w, avail.y), true,
                                  ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                // -- Path list --
                ImGui::TextUnformatted("Paths");
                ImGui::Separator();
                for (int pi = 0; pi < data.Paths.Size; ++pi)
                {
                    char row[96];
                    ImFormatString(row, sizeof(row), "Path %d (%d nodes%s)###p_%d",
                                   pi, data.Paths[pi].Nodes.Size,
                                   data.Paths[pi].Closed ? ", closed" : "", pi);
                    if (ImGui::Selectable(row, data.SelectedPath == pi))
                    {
                        data.SelectedPath = pi;
                        data.SelectedNode = -1;
                        data.SelectedNodes.clear();
                    }
                }
                if (ImGui::Button("+ New path"))
                {
                    ImVectorDrawingPath np;
                    data.Paths.push_back(np);
                    data.SelectedPath = data.Paths.Size - 1;
                    data.ActivePath = data.SelectedPath;
                    data.SelectedNode = -1;
                    data.SelectedNodes.clear();
                }
                ImGui::SameLine();
                if (ImGui::Button("Delete path")
                    && data.SelectedPath >= 0 && data.SelectedPath < data.Paths.Size)
                {
                    data.Paths.erase(data.Paths.Data + data.SelectedPath);
                    if (data.ActivePath == data.SelectedPath) data.ActivePath = -1;
                    data.SelectedPath = -1;
                    data.SelectedNode = -1;
                    data.SelectedNodes.clear();
                }
                ImGui::Separator();

                // -- Current path options --
                if (data.SelectedPath >= 0 && data.SelectedPath < data.Paths.Size)
                {
                    ImVectorDrawingPath& p = data.Paths[data.SelectedPath];
                    const char* style_names[] = {
                        "Polyline", "PolylineAA", "StrokedBezier",
                        "StrokedDashedBezier", "DashedPolyline"
                    };
                    int s = (int)p.Style;
                    if (ImGui::Combo("Style", &s, style_names, IM_ARRAYSIZE(style_names)))
                        p.Style = (ImVectorDrawingStyle)s;
                    ImGui::SliderFloat("Thickness", &p.Thickness, 0.5f, 16.0f);
                    ImVec4 col = ImGui::ColorConvertU32ToFloat4(p.Color);
                    if (ImGui::ColorEdit4("Color", &col.x, ImGuiColorEditFlags_NoInputs))
                        p.Color = ImGui::ColorConvertFloat4ToU32(col);
                    // ImWidgetsCap_: None(0) / Butt(1) / Square(2) / Round(3) / TriangleOut(4) / TriangleIn(5)
                    const char* cap_names[] = { "None", "Butt", "Square", "Round", "TriangleOut", "TriangleIn" };
                    ImGui::Combo("Cap", &p.Cap, cap_names, IM_ARRAYSIZE(cap_names));
                    // ImWidgetsJoin_: Round(0) / Mitter(1) / Bevel(2)
                    const char* join_names[] = { "Round", "Mitter", "Bevel" };
                    ImGui::Combo("Join", &p.Join, join_names, IM_ARRAYSIZE(join_names));
                    if (p.Style == ImVectorDrawingStyle_StrokedDashedBezier
                        || p.Style == ImVectorDrawingStyle_DashedPolyline)
                    {
                        ImGui::SliderFloat("Dash", &p.DashLen, 1.0f, 40.0f);
                        ImGui::SliderFloat("Gap",  &p.GapLen,  1.0f, 40.0f);
                    }
                    ImGui::Checkbox("Closed", &p.Closed);
                }
                else
                {
                    ImGui::TextDisabled("Select a path from the list\nor click in the canvas to\nstart drawing a new one.");
                }
                ImGui::EndChild();
            }
            EndExpandedWindow();
        }

        // Tangent-edit side panel (shown when a node is selected).
        if (data.SelectedPath >= 0 && data.SelectedPath < data.Paths.Size
            && data.SelectedNode >= 0
            && data.SelectedNode < data.Paths[data.SelectedPath].Nodes.Size)
        {
            ImVectorDrawingNode& n = data.Paths[data.SelectedPath].Nodes[data.SelectedNode];
            if (ImGui::CollapsingHeader("Selected node", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (PrecisionFloat("Anchor X", &n.Anchor.x, 1.0f, -1e6f, 1e6f, "%.2f")) changed = true;
                if (PrecisionFloat("Anchor Y", &n.Anchor.y, 1.0f, -1e6f, 1e6f, "%.2f")) changed = true;
                if (PrecisionFloat("In.x",  &n.InTangent.x, 1.0f, -1e4f, 1e4f, "%.2f")) changed = true;
                if (PrecisionFloat("In.y",  &n.InTangent.y, 1.0f, -1e4f, 1e4f, "%.2f")) changed = true;
                if (PrecisionFloat("Out.x", &n.OutTangent.x, 1.0f, -1e4f, 1e4f, "%.2f")) changed = true;
                if (PrecisionFloat("Out.y", &n.OutTangent.y, 1.0f, -1e4f, 1e4f, "%.2f")) changed = true;
                if (ImGui::Checkbox("Broken tangents", &n.Broken)) changed = true;
                if (!n.Broken && ImGui::Button("Mirror: Out = -In"))
                {
                    n.OutTangent = ImVec2(-n.InTangent.x, -n.InTangent.y);
                    changed = true;
                }
            }
        }
        ImGui::PopID();
        return changed;
    }

} // namespace ImWidgets
