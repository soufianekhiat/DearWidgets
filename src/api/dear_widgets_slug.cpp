// dear_widgets_slug.cpp — Slug GPU font rendering: color fonts, gradients, ligatures, atlas baking.
// This file is #included from dear_widgets.cpp — do NOT compile separately.
#ifdef _DEAR_WIDGETS_SLUG_INCLUDED
#include "dear_widgets_slug.h"

// stb_rect_pack and stb_truetype are included from dear_widgets.cpp (before namespace)

#ifndef IM_SUPPORT_LIGATURE
#define IM_SUPPORT_LIGATURE 1
#endif
#if IM_SUPPORT_LIGATURE
#define KB_TEXT_SHAPE_STATIC
#define KB_TEXT_SHAPE_IMPLEMENTATION
#include "kb_text_shape.h"
#endif


#if IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER

	// ---- Constants ----------------------------------------------------------
	#define SLUG_TEX_WIDTH    4096
	#define SLUG_LOG_TEX_W    12       // log2(SLUG_TEX_WIDTH)
	#define SLUG_BANDS_X      8        // vertical band count (partition x-axis)
	#define SLUG_BANDS_Y      8        // horizontal band count (partition y-axis)
	#define SLUG_TEX_INIT_H   16       // initial texture height (rows)

	// ---- Data Structures ----------------------------------------------------

	// One quadratic Bezier curve in em-space (TTF coordinate system, Y-up)
	struct SlugCurve
	{
		float p1x, p1y;   // start point
		float p2x, p2y;   // control point
		float p3x, p3y;   // end point
	};

	// Approximate one cubic Bezier as quadratic segments and push to curve list.
	// Splits the cubic at midpoints recursively (depth=number of subdivisions, 2 → 4 quads).
	static void SlugCubicToQuads(ImVector<SlugCurve>& curves,
		float p0x, float p0y, float p1x, float p1y,
		float p2x, float p2y, float p3x, float p3y, int depth = 2)
	{
		if (depth <= 0)
		{
			// Approximate this cubic segment as a single quadratic.
			// Control point = intersection of tangent lines at endpoints:
			//   Q = (3*P1 - P0 + 3*P2 - P3) / 4  (midpoint of the two inner control points,
			//   adjusted — this is the standard cubic→quadratic approximation for a single segment)
			// For a pre-subdivided cubic, using the mid-control-point gives good results:
			float qx = (3.0f * p1x - p0x + 3.0f * p2x - p3x) * 0.25f;
			float qy = (3.0f * p1y - p0y + 3.0f * p2y - p3y) * 0.25f;
			SlugCurve cv = {};
			cv.p1x = p0x; cv.p1y = p0y;
			cv.p2x = qx;  cv.p2y = qy;
			cv.p3x = p3x; cv.p3y = p3y;
			curves.push_back(cv);
			return;
		}
		// De Casteljau split at t=0.5
		float m01x = (p0x + p1x) * 0.5f, m01y = (p0y + p1y) * 0.5f;
		float m12x = (p1x + p2x) * 0.5f, m12y = (p1y + p2y) * 0.5f;
		float m23x = (p2x + p3x) * 0.5f, m23y = (p2y + p3y) * 0.5f;
		float m012x = (m01x + m12x) * 0.5f, m012y = (m01y + m12y) * 0.5f;
		float m123x = (m12x + m23x) * 0.5f, m123y = (m12y + m23y) * 0.5f;
		float mx = (m012x + m123x) * 0.5f, my = (m012y + m123y) * 0.5f;
		SlugCubicToQuads(curves, p0x, p0y, m01x, m01y, m012x, m012y, mx, my, depth - 1);
		SlugCubicToQuads(curves, mx, my, m123x, m123y, m23x, m23y, p3x, p3y, depth - 1);
	}

	// Cached per-glyph rendering data
	struct SlugGlyphEntry
	{
		ImWchar codepoint;
		// Location of this glyph's header block in the band texture
		int  bandTexX, bandTexY;    // = glyphLoc passed to vertex
		// Band grid parameters
		int  bandMaxX, bandMaxY;    // max band indices (= SLUG_BANDS_X/Y - 1)
		float bandScaleX, bandScaleY;
		float bandOffsetX, bandOffsetY;
		// Glyph metrics in em-space (TTF Y-up)
		float advanceEm;
		float minXEm, minYEm, maxXEm, maxYEm;  // em-space bounding box
		// COLR v0 color layer support (-1 / 0 = not a color glyph)
		int  colorLayerStart;  // index into SlugFontCache::colorLayers
		int  colorLayerCount;  // 0 = monochrome glyph
	};

	// One color layer: a built layer glyph entry + its fill (solid color or gradient)
	struct SlugColorLayer
	{
		int   glyphEntryIdx;  // index in SlugFontCache::glyphs (already-built outline)
		ImU32 color;          // solid RGBA; IM_COL32(0,0,0,0) = use caller's text color (CPAL 0xFFFF)
		// Linear gradient (if hasGradient is true, color field is ignored)
		bool  hasGradient;
		ImU32 gradColor0, gradColor1;  // gradient stop colors
		float gradDirX, gradDirY;      // gradient direction in em-space (normalized)
		float gradScale, gradBias;     // t = dot(emCoord, dir) * scale + bias
		// COLR v1 PaintTranslate offset (em-space)
		float translateX, translateY;
	};

	// Per-font cache: holds curve control points, band acceleration structure, and glyph metrics
	struct SlugFontCache
	{
		ImFont*        imguiFont;    // key into atlas cache
		stbtt_fontinfo stbFont;      // stb_truetype handle (points into ImGui's font data)
		float          emScale;      // stbtt_ScaleForMappingEmToPixels(&stbFont, 1.0f) = 1/unitsPerEm

		ImVector<SlugGlyphEntry> glyphs;       // built on demand
		ImVector<SlugColorLayer> colorLayers;  // COLR v0 layer list (referenced by SlugGlyphEntry)

		// Color table offsets from the start of the font file data (0 = table not present)
		uint32_t colrTableOffset;
		uint32_t cpalTableOffset;
		uint32_t svgTableOffset;

#if IM_SUPPORT_LIGATURE
		kbts_shape_context* shapeCtx;  // text shaping context (RTL, ligatures, etc.)
		kbts_font*          shapeFont; // font pushed to the shaping context
#endif

		// Curve texture: RGBA32F, width = SLUG_TEX_WIDTH
		//   Each Bezier curve occupies 2 consecutive texels on the same row:
		//     texel 0: (p1.x, p1.y, p2.x, p2.y)
		//     texel 1: (p3.x, p3.y, 0, 0)
		ImVector<float> curveTex;     // flat RGBA32F pixel data
		int curveCursor;              // next available texel index (1-D, wraps to next row)
		int curveTexHeight;           // current allocated height

		// Band texture: RGBA32F, integers stored as exact floats
		//   Band header: (count, offset, 0, 0)
		//   Curve ref:   (curveTexX, curveTexY, 0, 0)
		ImVector<float> bandTex;
		int bandCursor;
		int bandTexHeight;

		ImTextureID curveTexture;     // GPU handle (NULL until first upload)
		ImTextureID bandTexture;
		bool        dirty;            // needs GPU re-upload
	};

	// Top-level slug state stored in ImWidgetsContext
	struct ImWidgetsSlugState
	{
		ImVector<SlugFontCache*> atlases;
	};

	// ---- Texture helpers ----------------------------------------------------

	// Write one RGBA32F texel at absolute (x, y) in the curve texture
	static void SlugCurveWrite4f(SlugFontCache* a, int x, int y,
	                              float r, float g, float b, float f)
	{
		int needed = y + 1;
		while (needed > a->curveTexHeight)
		{
			a->curveTexHeight *= 2;
			a->curveTex.resize(SLUG_TEX_WIDTH * a->curveTexHeight * 4, 0.0f);
		}
		int idx = (y * SLUG_TEX_WIDTH + x) * 4;
		a->curveTex[idx + 0] = r;
		a->curveTex[idx + 1] = g;
		a->curveTex[idx + 2] = b;
		a->curveTex[idx + 3] = f;
	}

	// Allocate space for one curve (2 texels on the same row).
	// Returns the position (cx, cy) of the first texel.
	static void SlugAllocOneCurve(SlugFontCache* a, int* cx, int* cy)
	{
		// Each pair of texels must lie in the same row (shader reads [x] and [x+1])
		int x = a->curveCursor % SLUG_TEX_WIDTH;
		int y = a->curveCursor / SLUG_TEX_WIDTH;
		if (x + 2 > SLUG_TEX_WIDTH) { x = 0; y++; a->curveCursor = y * SLUG_TEX_WIDTH; }
		*cx = x;
		*cy = y;
		a->curveCursor += 2;
	}

	// Write one RGBA32F texel at absolute (x, y) in the band texture
	static void SlugBandWrite2f(SlugFontCache* a, int x, int y, float v0, float v1)
	{
		int needed = y + 1;
		while (needed > a->bandTexHeight)
		{
			a->bandTexHeight *= 2;
			a->bandTex.resize(SLUG_TEX_WIDTH * a->bandTexHeight * 4, 0.0f);
		}
		int idx = (y * SLUG_TEX_WIDTH + x) * 4;
		a->bandTex[idx + 0] = v0;
		a->bandTex[idx + 1] = v1;
		a->bandTex[idx + 2] = 0.0f;
		a->bandTex[idx + 3] = 0.0f;
	}

	// Resolve (glyphLoc + offset) with row wrapping (mirrors CalcBandLoc in the shader)
	static void SlugBandCalcLoc(int glx, int gly, int offset, int* outX, int* outY)
	{
		int x = glx + offset;
		int y = gly + x / SLUG_TEX_WIDTH;
		x     = x % SLUG_TEX_WIDTH;
		*outX = x;
		*outY = y;
	}

	// ---- COLR v0 / CPAL helpers ---------------------------------------------

	static inline uint16_t SlugTTU16(const uint8_t* p) { return (uint16_t)((p[0]<<8)|p[1]); }
	static inline uint32_t SlugTTU32(const uint8_t* p) { return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3]; }

	// Search the TrueType table directory for a 4-char tag; returns byte offset from data[0], 0 if not found.
	static uint32_t SlugFindTable(const uint8_t* data, uint32_t fontStart, const char* tag)
	{
		int numTables = (int)SlugTTU16(data + fontStart + 4);
		uint32_t tableDir = fontStart + 12;
		for (int i = 0; i < numTables; i++)
		{
			const uint8_t* e = data + tableDir + 16 * i;
			if (e[0]==tag[0] && e[1]==tag[1] && e[2]==tag[2] && e[3]==tag[3])
				return SlugTTU32(e + 8);
		}
		return 0;
	}

	// For a given glyph index, fill outGlyphIDs/outColors with COLR v0 layer data.
	// outColors entry = 0 (IM_COL32(0,0,0,0)) means "use foreground text color" (paletteIndex 0xFFFF).
	// Returns number of layers, or 0 if not a COLR glyph.
	static int SlugGetColrLayers(SlugFontCache* atlas, int glyphID,
	                             ImVector<int>& outGlyphIDs, ImVector<ImU32>& outColors)
	{
		if (!atlas->colrTableOffset || !atlas->cpalTableOffset) return 0;
		const uint8_t* data = (const uint8_t*)atlas->stbFont.data;
		const uint8_t* colr = data + atlas->colrTableOffset;

		// COLR v0 header: uint16 version, uint16 numBaseGlyphs, uint32 offsetBase, uint32 offsetLayer, uint16 numLayerRecords
		int      numBase    = (int)SlugTTU16(colr + 2);
		uint32_t offBase    = SlugTTU32(colr + 4);
		uint32_t offLayer   = SlugTTU32(colr + 8);

		// Binary search in BaseGlyphRecord array (sorted ascending by GlyphID per spec)
		// Each record: uint16 GlyphID, uint16 FirstLayerIndex, uint16 NumLayers = 6 bytes
		const uint8_t* baseArr = colr + offBase;
		int lo = 0, hi = numBase - 1, found = -1;
		while (lo <= hi)
		{
			int mid = (lo + hi) / 2;
			int gid = (int)SlugTTU16(baseArr + mid * 6);
			if      (gid == glyphID) { found = mid; break; }
			else if (gid  < glyphID)   lo = mid + 1;
			else                       hi = mid - 1;
		}
		if (found < 0) return 0;

		int firstLayer = (int)SlugTTU16(baseArr + found * 6 + 2);
		int numLayers  = (int)SlugTTU16(baseArr + found * 6 + 4);

		// CPAL v0 header: uint16 version, uint16 numPaletteEntries, uint16 numPalettes,
		//                 uint16 numColorRecords, uint32 offsetFirstColorRecord
		const uint8_t* cpal       = data + atlas->cpalTableOffset;
		uint32_t       offColors  = SlugTTU32(cpal + 8);

		// LayerRecord: uint16 GlyphID, uint16 PaletteIndex = 4 bytes each
		const uint8_t* layerArr = colr + offLayer;
		for (int i = 0; i < numLayers; i++)
		{
			const uint8_t* lr           = layerArr + (firstLayer + i) * 4;
			int            layerGlyphID = (int)SlugTTU16(lr);
			int            palIdx       = (int)SlugTTU16(lr + 2);

			ImU32 color;
			if (palIdx == 0xFFFF)
			{
				color = IM_COL32(0, 0, 0, 0);  // sentinel: use caller's text color
			}
			else
			{
				// CPAL stores colors as B, G, R, A
				const uint8_t* cr = cpal + offColors + palIdx * 4;
				color = IM_COL32(cr[2], cr[1], cr[0], cr[3]);
			}
			outGlyphIDs.push_back(layerGlyphID);
			outColors.push_back(color);
		}
		return numLayers;
	}

	// COLR v1 layer extraction: PaintColrLayers → PaintGlyph → PaintSolid / PaintLinearGradient.
	// Returns layer count or 0. Fills outLayers with glyph IDs and fill info (solid or gradient).
	static int SlugGetColrV1Layers(SlugFontCache* atlas, int glyphID,
	                               ImVector<int>& outGlyphIDs, ImVector<SlugColorLayer>& outLayers)
	{
		if (!atlas->colrTableOffset || !atlas->cpalTableOffset) return 0;
		const uint8_t* data = (const uint8_t*)atlas->stbFont.data;
		const uint8_t* colr = data + atlas->colrTableOffset;

		if (SlugTTU16(colr) < 1) return 0;  // need COLR v1+

		uint32_t offBGL = SlugTTU32(colr + 14);  // offsetBaseGlyphList
		uint32_t offLL  = SlugTTU32(colr + 18);  // offsetLayerList
		if (offBGL == 0 || offLL == 0) return 0;

		const uint8_t* bgl = colr + offBGL;
		const uint8_t* ll  = colr + offLL;
		uint32_t numBGL = SlugTTU32(bgl);
		uint32_t numLL  = SlugTTU32(ll);

		int lo = 0, hi = (int)numBGL - 1, found = -1;
		while (lo <= hi)
		{
			int mid = (lo + hi) / 2;
			int gid = (int)SlugTTU16(bgl + 4 + mid * 6);
			if      (gid == glyphID) { found = mid; break; }
			else if (gid  < glyphID)   lo = mid + 1;
			else                       hi = mid - 1;
		}
		if (found < 0) return 0;

		uint32_t paintOff = SlugTTU32(bgl + 4 + found * 6 + 2);
		const uint8_t* p0 = bgl + paintOff;

		// Determine top-level layers: PaintColrLayers (1), or single-layer paint nodes (10, 11, 14, etc.)
		uint8_t  numLayers = 0;
		uint32_t firstLayerIdx = 0;
		bool     topIsSinglePaint = false;

		if (p0[0] == 1) { // PaintColrLayers
			numLayers     = p0[1];
			firstLayerIdx = SlugTTU32(p0 + 2);
		} else {
			// Top-level is a single paint node (PaintGlyph, PaintColrGlyph, PaintTranslate, etc.)
			// We'll handle it as a single-layer glyph below
			topIsSinglePaint = true;
			numLayers = 1;
		}

		const uint8_t* cpal      = data + atlas->cpalTableOffset;
		uint32_t       offColors = SlugTTU32(cpal + 8);
		const float sc = atlas->emScale;

		auto GetPalColor = [&](int palIdx, float alpha) -> ImU32 {
			if (palIdx == 0xFFFF) return IM_COL32(0, 0, 0, 0);
			const uint8_t* cr = cpal + offColors + palIdx * 4;
			uint8_t a = (uint8_t)(alpha * cr[3]);
			return IM_COL32(cr[2], cr[1], cr[0], a);
		};
		auto F2D14 = [](const uint8_t* p) -> float {
			int16_t v = (int16_t)((p[0] << 8) | p[1]);
			return (float)v / 16384.0f;
		};
		auto FWORD = [](const uint8_t* p) -> int16_t {
			return (int16_t)((p[0] << 8) | p[1]);
		};

		// Helper: resolve a paint node to (glyphID, fill paint pointer).
		// Resolves paint tree nodes, accumulating translation offsets from PaintTranslate.
		// For PaintColrGlyph (fmt 11), recursively looks up the referenced glyph's paint tree.
		struct PaintResolved { int glyphID; const uint8_t* fillPaint; float translateX; float translateY; };
		ImVector<PaintResolved> resolvedPaints;

		auto ResolvePaint = [&](const uint8_t* paint, int recurseDepth, float txAcc, float tyAcc, auto& self) -> void {
			if (recurseDepth > 4) return;
			// Unwrap wrappers, accumulating translation offsets
			const uint8_t* p2 = paint;
			for (int d2 = 0; d2 < 8; d2++) {
				uint8_t f = p2[0];
				if (f == 14) { // PaintTranslate: fmt(1) + paintOffset(3) + dx(2) + dy(2) = 8 bytes
					uint32_t off2 = ((uint32_t)p2[1] << 16) | ((uint32_t)p2[2] << 8) | p2[3];
					int16_t dx = (int16_t)SlugTTU16(p2 + 4);
					int16_t dy = (int16_t)SlugTTU16(p2 + 6);
					txAcc += (float)dx * sc;
					tyAcc += (float)dy * sc;
					p2 = p2 + off2;
				} else if (f == 15) { // PaintVarTranslate: fmt(1) + paintOffset(3) + dx(2) + dy(2) + varIdxBase(4) = 12
					uint32_t off2 = ((uint32_t)p2[1] << 16) | ((uint32_t)p2[2] << 8) | p2[3];
					int16_t dx = (int16_t)SlugTTU16(p2 + 4);
					int16_t dy = (int16_t)SlugTTU16(p2 + 6);
					txAcc += (float)dx * sc;
					tyAcc += (float)dy * sc;
					p2 = p2 + off2;
				} else if (f >= 12 && f <= 21) { // Other transforms (Scale, Rotate, Transform) — skip for now
					uint32_t off2 = ((uint32_t)p2[1] << 16) | ((uint32_t)p2[2] << 8) | p2[3];
					p2 = p2 + off2;
				} else break;
			}
			if (p2[0] == 10) { // PaintGlyph
				uint32_t fOff = ((uint32_t)p2[1] << 16) | ((uint32_t)p2[2] << 8) | p2[3];
				PaintResolved pr;
				pr.glyphID = (int)SlugTTU16(p2 + 4);
				pr.fillPaint = p2 + fOff;
				pr.translateX = txAcc;
				pr.translateY = tyAcc;
				resolvedPaints.push_back(pr);
			}
			else if (p2[0] == 11) { // PaintColrGlyph — recurse into referenced glyph's paint tree
				int refGlyphID = (int)SlugTTU16(p2 + 1);
				// Binary search BaseGlyphList for refGlyphID
				int lo2 = 0, hi2 = (int)numBGL - 1, found2 = -1;
				while (lo2 <= hi2) {
					int mid2 = (lo2 + hi2) / 2;
					int gid2 = (int)SlugTTU16(bgl + 4 + mid2 * 6);
					if (gid2 == refGlyphID) { found2 = mid2; break; }
					else if (gid2 < refGlyphID) lo2 = mid2 + 1;
					else hi2 = mid2 - 1;
				}
				if (found2 >= 0) {
					uint32_t refPaintOff = SlugTTU32(bgl + 4 + found2 * 6 + 2);
					const uint8_t* refPaint = bgl + refPaintOff;
					if (refPaint[0] == 1) { // PaintColrLayers
						uint8_t refNL = refPaint[1];
						uint32_t refFirstIdx = SlugTTU32(refPaint + 2);
						for (int ri = 0; ri < (int)refNL; ri++) {
							uint32_t rloff = SlugTTU32(ll + 4 + (refFirstIdx + ri) * 4);
							self(ll + rloff, recurseDepth + 1, txAcc, tyAcc, self);
						}
					} else {
						self(refPaint, recurseDepth + 1, txAcc, tyAcc, self);
					}
				}
			}
			else if (p2[0] == 1) { // PaintColrLayers (nested)
				uint8_t nL2 = p2[1];
				uint32_t firstIdx2 = SlugTTU32(p2 + 2);
				for (int ri = 0; ri < (int)nL2; ri++) {
					uint32_t rloff = SlugTTU32(ll + 4 + (firstIdx2 + ri) * 4);
					self(ll + rloff, recurseDepth + 1, txAcc, tyAcc, self);
				}
			}
		};

		for (int li = 0; li < (int)numLayers; li++)
		{
			resolvedPaints.clear();
			if (topIsSinglePaint) {
				ResolvePaint(p0, 0, 0.0f, 0.0f, ResolvePaint);
			} else {
				uint32_t layerIdx = firstLayerIdx + (uint32_t)li;
				if (layerIdx >= numLL) break;
				uint32_t loff = SlugTTU32(ll + 4 + layerIdx * 4);
				ResolvePaint(ll + loff, 0, 0.0f, 0.0f, ResolvePaint);
			}

			for (int ri = 0; ri < resolvedPaints.Size; ri++)
			{
			int layerGlyphID = resolvedPaints[ri].glyphID;
			const uint8_t* fp = resolvedPaints[ri].fillPaint;
			float layerTX = resolvedPaints[ri].translateX;
			float layerTY = resolvedPaints[ri].translateY;

			SlugColorLayer cl = {};
			cl.translateX = layerTX;
			cl.translateY = layerTY;
			switch (fp[0])
			{
			case 2:  // PaintSolid
			case 3:  // PaintVarSolid
			{
				int palIdx = (int)SlugTTU16(fp + 1);
				float alpha = F2D14(fp + 3);
				cl.color = GetPalColor(palIdx, alpha);
				break;
			}
			case 4:  // PaintLinearGradient
			case 5:  // PaintVarLinearGradient
			{
				uint32_t clOff = ((uint32_t)fp[1] << 16) | ((uint32_t)fp[2] << 8) | fp[3];
				float gx0 = (float)FWORD(fp + 4) * sc, gy0 = (float)FWORD(fp + 6) * sc;
				float gx1 = (float)FWORD(fp + 8) * sc, gy1 = (float)FWORD(fp + 10) * sc;
				const uint8_t* clp = fp + clOff;
				uint16_t numStops = SlugTTU16(clp + 1);
				if (numStops < 2) { if (numStops == 1) { cl.color = GetPalColor((int)SlugTTU16(clp+3+2), F2D14(clp+3+4)); } break; }
				int pal0 = (int)SlugTTU16(clp + 3 + 2); float alp0 = F2D14(clp + 3 + 4);
				const uint8_t* lastStop = clp + 3 + (numStops - 1) * 6;
				int pal1 = (int)SlugTTU16(lastStop + 2); float alp1 = F2D14(lastStop + 4);
				cl.color = GetPalColor(pal0, alp0);  // solid fallback (always set)
				cl.hasGradient = true;
				cl.gradColor0 = cl.color;
				cl.gradColor1 = GetPalColor(pal1, alp1);
				float dx = gx1 - gx0, dy = gy1 - gy0;
				float len2 = dx * dx + dy * dy;
				if (len2 < 1e-10f) { cl.hasGradient = false; break; }
				cl.gradDirX = dx / len2; cl.gradDirY = dy / len2;
				float t0 = F2D14(clp + 3), t1 = F2D14(lastStop);
				float tRange = (t1 - t0 > 1e-6f) ? (t1 - t0) : 1.0f;
				cl.gradScale = 1.0f / tRange;
				cl.gradBias = -(cl.gradDirX * gx0 + cl.gradDirY * gy0) / tRange - t0 / tRange;
				break;
			}
			default:
				continue;
			}

			outGlyphIDs.push_back(layerGlyphID);
			outLayers.push_back(cl);
			} // end resolvedPaints loop
		}
		return outGlyphIDs.Size;
	}

	// ---- SVG layer extraction ------------------------------------------------

	// Forward declaration needed by SlugGetSVGLayers
	static bool SlugBuildGlyphFromCurves(SlugFontCache* atlas, ImWchar cp, float advEm,
	                                      float minX, float minY, float maxX, float maxY,
	                                      ImVector<SlugCurve>& curves, SlugGlyphEntry* outEntry);

	static inline bool ImIsSpace(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }

	// Parse an SVG hex colour value ("#RGB" or "#RRGGBB") into IM_COL32 RGBA.
	// Returns 0 (transparent) for "none" or unparseable values.
	static ImU32 SlugParseSVGColor(const char* s, int len)
	{
		if (len <= 0 || !s) return 0;
		// Skip leading whitespace
		while (len > 0 && ImIsSpace(*s)) { s++; len--; }
		if (len <= 0) return 0;
		if (s[0] == '#')
		{
			s++; len--;
			// Count hex digits
			int nd = 0;
			while (nd < len && ((s[nd] >= '0' && s[nd] <= '9') ||
			                    (s[nd] >= 'a' && s[nd] <= 'f') ||
			                    (s[nd] >= 'A' && s[nd] <= 'F'))) nd++;
			auto hexdig = [](char c) -> int {
				if (c >= '0' && c <= '9') return c - '0';
				if (c >= 'a' && c <= 'f') return c - 'a' + 10;
				if (c >= 'A' && c <= 'F') return c - 'A' + 10;
				return 0;
			};
			if (nd == 3) {
				int r = hexdig(s[0]) * 17;
				int g = hexdig(s[1]) * 17;
				int b = hexdig(s[2]) * 17;
				return IM_COL32(r, g, b, 255);
			}
			if (nd >= 6) {
				int r = hexdig(s[0]) * 16 + hexdig(s[1]);
				int g = hexdig(s[2]) * 16 + hexdig(s[3]);
				int b = hexdig(s[4]) * 16 + hexdig(s[5]);
				return IM_COL32(r, g, b, 255);
			}
		}
		// "none" → transparent (caller should skip)
		if (len >= 4 && s[0]=='n' && s[1]=='o' && s[2]=='n' && s[3]=='e') return 0;
		return 0;
	}

	// Skip whitespace and optional comma; return pointer advanced past it.
	static const char* SlugSVGSkipWS(const char* p)
	{
		while (ImIsSpace(*p) || *p == ',') p++;
		return p;
	}

	// Parse one float from *p, advance *p past it.
	static float SlugSVGParseFloat(const char** p)
	{
		*p = SlugSVGSkipWS(*p);
		char* end;
		float v = strtof(*p, &end);
		*p = end;
		return v;
	}

	// Parse SVG path `d` attribute into Slug curves.
	// sc = emScale (font units → em), negateY = true to flip Y axis.
	// dEnd is one-past-end; if NULL the string is assumed null-terminated.
	static void SlugParseSVGPath(const char* d, const char* dEnd, ImVector<SlugCurve>& curves, float sc, bool negateY)
	{
		const float ys = negateY ? -1.0f : 1.0f;
		float cx = 0, cy = 0;   // current point
		float sx = 0, sy = 0;   // subpath start (for Z)
		char  cmd = 0;
		bool  subpathActive = false;  // true after M, false before any M or after explicit Z
		float lcpx = 0, lcpy = 0;    // last cubic control point (for S/s continuity)
		bool  lastWasCubic = false;
		float lqcpx = 0, lqcpy = 0;  // last quadratic control point (for T/t continuity)
		bool  lastWasQuad = false;
		const char* p = d;
		// Per SVG spec: for fill, each subpath is treated as closed (implicit Z at end / before new M)
		auto ImplicitClose = [&]() {
			if (subpathActive && (cx != sx || cy != sy)) {
				SlugCurve cv = {};
				cv.p1x = cx; cv.p1y = cy;
				cv.p2x = (cx + sx) * 0.5f; cv.p2y = (cy + sy) * 0.5f;
				cv.p3x = sx; cv.p3y = sy;
				curves.push_back(cv);
				cx = sx; cy = sy;
			}
		};
		while ((!dEnd && *p) || (dEnd && p < dEnd))
		{
			p = SlugSVGSkipWS(p);
			if (!*p || (dEnd && p >= dEnd)) break;
			char c = *p;
			if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) { cmd = c; p++; }
			// else implicit repeat of last cmd

			if (cmd == 'M' || cmd == 'm')
			{
				// Implicitly close any previous unclosed subpath before starting a new one
				ImplicitClose();
				float x = SlugSVGParseFloat(&p) * sc;
				float y = SlugSVGParseFloat(&p) * sc * ys;
				if (cmd == 'm') { x += cx; y += cy; }
				cx = sx = x; cy = sy = y;
				subpathActive = true;
				// subsequent coords are implicit L/l
				cmd = (cmd == 'M') ? 'L' : 'l';
				continue;
			}
			if (cmd == 'Z' || cmd == 'z')
			{
				// Close path: line back to subpath start if not already there
				if (cx != sx || cy != sy)
				{
					SlugCurve cv = {};
					cv.p1x = cx; cv.p1y = cy;
					cv.p2x = (cx + sx) * 0.5f; cv.p2y = (cy + sy) * 0.5f;
					cv.p3x = sx; cv.p3y = sy;
					curves.push_back(cv);
				}
				cx = sx; cy = sy;
				subpathActive = false;
				lastWasCubic = false; lastWasQuad = false;
				cmd = 0;
				continue;
			}
			if (cmd == 'L' || cmd == 'l')
			{
				float x = SlugSVGParseFloat(&p) * sc;
				float y = SlugSVGParseFloat(&p) * sc * ys;
				if (cmd == 'l') { x += cx; y += cy; }
				if (x != cx || y != cy) { // skip zero-length
					SlugCurve cv = {};
					cv.p1x = cx; cv.p1y = cy;
					cv.p2x = (cx + x) * 0.5f; cv.p2y = (cy + y) * 0.5f;
					cv.p3x = x;  cv.p3y = y;
					curves.push_back(cv);
				}
				cx = x; cy = y;
				continue;
			}
			if (cmd == 'H' || cmd == 'h')
			{
				float x = SlugSVGParseFloat(&p) * sc;
				if (cmd == 'h') x += cx;
				if (x != cx) { // skip zero-length
					SlugCurve cv = {};
					cv.p1x = cx;  cv.p1y = cy;
					cv.p2x = (cx + x) * 0.5f; cv.p2y = cy;
					cv.p3x = x;   cv.p3y = cy;
					curves.push_back(cv);
				}
				cx = x;
				continue;
			}
			if (cmd == 'V' || cmd == 'v')
			{
				float y = SlugSVGParseFloat(&p) * sc * ys;
				if (cmd == 'v') y += cy;
				if (y != cy) { // skip zero-length
					SlugCurve cv = {};
					cv.p1x = cx; cv.p1y = cy;
					cv.p2x = cx; cv.p2y = (cy + y) * 0.5f;
					cv.p3x = cx; cv.p3y = y;
					curves.push_back(cv);
				}
				cy = y;
				continue;
			}
			if (cmd == 'Q' || cmd == 'q')
			{
				float x1 = SlugSVGParseFloat(&p) * sc, y1 = SlugSVGParseFloat(&p) * sc * ys;
				float x  = SlugSVGParseFloat(&p) * sc, y  = SlugSVGParseFloat(&p) * sc * ys;
				if (cmd == 'q') { x1 += cx; y1 += cy; x += cx; y += cy; }
				SlugCurve cv = {};
				cv.p1x = cx; cv.p1y = cy;
				cv.p2x = x1; cv.p2y = y1;
				cv.p3x = x;  cv.p3y = y;
				curves.push_back(cv);
				lqcpx = x1; lqcpy = y1;  // last control point for T continuity
				lastWasQuad = true; lastWasCubic = false;
				cx = x; cy = y;
				continue;
			}
			if (cmd == 'C' || cmd == 'c')
			{
				float x1 = SlugSVGParseFloat(&p) * sc, y1 = SlugSVGParseFloat(&p) * sc * ys;
				float x2 = SlugSVGParseFloat(&p) * sc, y2 = SlugSVGParseFloat(&p) * sc * ys;
				float x  = SlugSVGParseFloat(&p) * sc, y  = SlugSVGParseFloat(&p) * sc * ys;
				if (cmd == 'c') { x1 += cx; y1 += cy; x2 += cx; y2 += cy; x += cx; y += cy; }
				SlugCubicToQuads(curves, cx, cy, x1, y1, x2, y2, x, y);
				lcpx = x2; lcpy = y2;
				lastWasCubic = true; lastWasQuad = false;
				cx = x; cy = y;
				continue;
			}
			if (cmd == 'S' || cmd == 's')
			{
				float x2 = SlugSVGParseFloat(&p) * sc, y2 = SlugSVGParseFloat(&p) * sc * ys;
				float x  = SlugSVGParseFloat(&p) * sc, y  = SlugSVGParseFloat(&p) * sc * ys;
				if (cmd == 's') { x2 += cx; y2 += cy; x += cx; y += cy; }
				float x1 = lastWasCubic ? (2*cx - lcpx) : cx;
				float y1 = lastWasCubic ? (2*cy - lcpy) : cy;
				SlugCubicToQuads(curves, cx, cy, x1, y1, x2, y2, x, y);
				lcpx = x2; lcpy = y2;
				lastWasCubic = true; lastWasQuad = false;
				cx = x; cy = y;
				continue;
			}
			if (cmd == 'T' || cmd == 't')
			{
				float x = SlugSVGParseFloat(&p) * sc, y = SlugSVGParseFloat(&p) * sc * ys;
				if (cmd == 't') { x += cx; y += cy; }
				float x1 = lastWasQuad ? (2*cx - lqcpx) : cx;
				float y1 = lastWasQuad ? (2*cy - lqcpy) : cy;
				SlugCurve cv = {};
				cv.p1x = cx; cv.p1y = cy;
				cv.p2x = x1; cv.p2y = y1;
				cv.p3x = x;  cv.p3y = y;
				curves.push_back(cv);
				lqcpx = x1; lqcpy = y1;
				lastWasQuad = true; lastWasCubic = false;
				cx = x; cy = y;
				continue;
			}
			// Unknown command: skip to next
			p++;
		}
		// Implicit close of the final subpath (SVG fill treats all subpaths as closed)
		ImplicitClose();
	}

	// Scan for attribute `name` in the XML text [s, end).
	// Returns pointer to char after '=', NULL if not found.
	static const char* SlugSVGFindAttr(const char* s, const char* end, const char* name)
	{
		const char* sStart = s;
		int nl = (int)strlen(name);
		while (s + nl + 1 < end)
		{
			if (s[0] == '>')  return NULL;
			if (strncmp(s, name, nl) == 0 && (s[nl] == '=' || ImIsSpace(s[nl])))
			{
				// Word boundary: must be preceded by whitespace or be at start of element
				bool atBoundary = (s == sStart) || ImIsSpace(*(s-1));
				if (atBoundary)
				{
					const char* q = s + nl;
					while (ImIsSpace(*q)) q++;
					if (*q == '=') return q + 1;
				}
			}
			s++;
		}
		return NULL;
	}

	// Read a quoted attribute value starting at `p` (points just after '=').
	// Returns pointer to value start; writes length into *outLen. NULL if malformed.
	static const char* SlugSVGReadQuoted(const char* p, int* outLen)
	{
		while (ImIsSpace(*p)) p++;
		char q = *p;
		if (q != '"' && q != '\'') return NULL;
		p++;
		const char* start = p;
		while (*p && *p != q) p++;
		*outLen = (int)(p - start);
		return start;
	}

	// Parse fill colour from a `<path` element's attributes.
	// Checks both fill="..." and style="...fill:...".
	// outExplicit = true  → element has an explicit fill (even if "none"/transparent)
	// outExplicit = false → no fill attribute found (caller should use inherited fill)
	// Returns IM_COL32 colour or 0 for transparent / none.
	static ImU32 SlugSVGPathFill(const char* elem, const char* elemEnd, bool* outExplicit = NULL)
	{
		// Try fill="..."
		const char* p = SlugSVGFindAttr(elem, elemEnd, "fill");
		if (p)
		{
			int len = 0;
			const char* val = SlugSVGReadQuoted(p, &len);
			if (val) { if (outExplicit) *outExplicit = true; return SlugParseSVGColor(val, len); }
		}
		// Try style="...fill:#rrggbb..."
		p = SlugSVGFindAttr(elem, elemEnd, "style");
		if (p)
		{
			int len = 0;
			const char* style = SlugSVGReadQuoted(p, &len);
			if (style)
			{
				const char* styleEnd = style + len;
				// Scan for fill: and opacity: properties
				ImU32 fillColor = 0; bool foundFill = false;
				float opacity = 1.0f;
				const char* fp = style;
				while (fp < styleEnd)
				{
					if (fp + 5 <= styleEnd && strncmp(fp, "fill:", 5) == 0)
					{
						fp += 5; while (ImIsSpace(*fp)) fp++;
						int vlen = 0; const char* vp = fp;
						while (vp + vlen < styleEnd && fp[vlen] != ';') vlen++;
						fillColor = SlugParseSVGColor(vp, vlen); foundFill = true;
						fp += vlen; continue;
					}
					if (fp + 8 <= styleEnd && strncmp(fp, "opacity:", 8) == 0)
					{
						fp += 8; while (ImIsSpace(*fp)) fp++;
						opacity = (float)atof(fp);
						while (fp < styleEnd && *fp != ';') fp++;
						continue;
					}
					fp++;
				}
				if (foundFill)
				{
					if (outExplicit) *outExplicit = true;
					if (opacity < 0.9961f) // apply opacity to alpha channel
					{
						uint8_t a = (uint8_t)((fillColor >> IM_COL32_A_SHIFT) & 0xFF);
						a = (uint8_t)(a * opacity + 0.5f);
						fillColor = (fillColor & ~((ImU32)0xFF << IM_COL32_A_SHIFT)) | ((ImU32)a << IM_COL32_A_SHIFT);
					}
					return fillColor;
				}
			}
		}
		if (outExplicit) *outExplicit = false;
		return 0;
	}

	// Parse the SVG table, find glyph glyphID, extract <path> elements grouped by fill colour,
	// build one SlugGlyphEntry per colour group, and populate atlas->colorLayers.
	// Returns false if no SVG data found for this glyph.
	static bool SlugGetSVGLayers(SlugFontCache* atlas, int glyphID, ImWchar cp, SlugGlyphEntry* outEntry)
	{
		if (!atlas->svgTableOffset) return false;
		const uint8_t* data = (const uint8_t*)atlas->stbFont.data;
		const uint8_t* svg  = data + atlas->svgTableOffset;

		// SVG table header: uint16 version, Offset32 svgDocListOffset, uint32 reserved
		uint32_t dlRel = SlugTTU32(svg + 2);
		const uint8_t* dl = svg + dlRel;  // SVGDocumentList

		// numEntries is uint16 (older OpenType spec, confirmed by inspection)
		uint16_t numEntries = SlugTTU16(dl);

		// Find entry covering glyphID (linear scan; entries are usually sorted)
		const uint8_t* docData = NULL;
		uint32_t       docLen  = 0;
		for (int i = 0; i < (int)numEntries; i++)
		{
			const uint8_t* rec = dl + 2 + i * 12;
			uint16_t sg  = SlugTTU16(rec);
			uint16_t eg  = SlugTTU16(rec + 2);
			uint32_t off = SlugTTU32(rec + 4);
			uint32_t len = SlugTTU32(rec + 8);
			if ((int)glyphID >= sg && (int)glyphID <= eg)
			{
				docData = dl + off;
				docLen  = len;
				break;
			}
		}
		if (!docData || docLen == 0) return false;

		// Gzip check: we only handle uncompressed UTF-8
		if (docData[0] == 0x1F && docData[1] == 0x8B) return false;

		const char* svgText    = (const char*)docData;
		const char* svgTextEnd = svgText + (int)docLen;

		// Get horizontal advance for this glyph
		int adv, lsb;
		stbtt_GetGlyphHMetrics(&atlas->stbFont, glyphID, &adv, &lsb);
		float advEm = (float)adv * atlas->emScale;

		// SVG coordinates are in font units with Y-down convention (Y negative = above baseline).
		// Negate Y to convert to Slug's Y-up em-space.
		float sc = atlas->emScale;

		// Collect per-colour curve groups (max 64 unique colours per glyph)
		static const int MAX_SVG_GROUPS = 256;
		ImU32             groupColor[MAX_SVG_GROUPS];
		ImVector<SlugCurve> groupCurves[MAX_SVG_GROUPS];
		float             groupMinX[MAX_SVG_GROUPS], groupMinY[MAX_SVG_GROUPS];
		float             groupMaxX[MAX_SVG_GROUPS], groupMaxY[MAX_SVG_GROUPS];
		int               numGroups = 0;

		// Fill-color inheritance stack: paths inside <g fill="#..."> inherit its fill
		ImU32 fillStack[32];
		int   fillStackTop = 0;
		fillStack[0] = 0;  // default: no inherited fill

		// Transform accumulation stack for translate() in <g> elements
		float txStack[32], tyStack[32];
		int   txStackTop  = 0;
		txStack[0] = tyStack[0] = 0.0f;

		// Scan for <path, <g, </g> elements
		const char* p = svgText;
		while (p < svgTextEnd)
		{
			if (*p != '<') { p++; continue; }

			// </g> or </G> → pop fill stack
			if (p + 3 < svgTextEnd && p[1] == '/' && (p[2] == 'g' || p[2] == 'G') &&
			    (p[3] == '>' || ImIsSpace(p[3])))
			{
				if (fillStackTop > 0) fillStackTop--;
				if (txStackTop  > 0) txStackTop--;
				p += 3; continue;
			}

			// <g ...> or <G ...> → parse fill attribute, push to stack
			if (p + 2 < svgTextEnd && (p[1] == 'g' || p[1] == 'G') &&
			    (p[2] == '>' || ImIsSpace(p[2])))
			{
				const char* gEnd = p + 2;
				while (gEnd < svgTextEnd && *gEnd != '>') gEnd++;
				// Find fill attribute on this <g>
				ImU32 gFill = fillStack[fillStackTop];  // default: inherit parent
				const char* gfp = SlugSVGFindAttr(p + 1, gEnd, "fill");
				if (gfp) {
					int flen = 0; const char* fval = SlugSVGReadQuoted(gfp, &flen);
					if (fval) gFill = SlugParseSVGColor(fval, flen);
				}
				if (fillStackTop < 31) fillStack[++fillStackTop] = gFill;
				// Parse transform="translate(tx, ty)" if present (accumulate with parent)
				float gtx = txStack[txStackTop], gty = tyStack[txStackTop];
				const char* gtp = SlugSVGFindAttr(p + 1, gEnd, "transform");
				if (gtp) {
					int gtlen = 0; const char* gtval = SlugSVGReadQuoted(gtp, &gtlen);
					if (gtval) {
						const char* tv = gtval, *te = gtval + gtlen;
						while (tv + 9 <= te && strncmp(tv, "translate", 9) != 0) tv++;
						if (tv + 9 <= te) {
							tv += 9; while (tv < te && *tv != '(') tv++;
							if (tv < te) {
								tv++; // skip '('
								gtx += SlugSVGParseFloat(&tv);
								gty += SlugSVGParseFloat(&tv);
							}
						}
					}
				}
				if (txStackTop < 31) { txStack[++txStackTop] = gtx; tyStack[txStackTop] = gty; }
				p = gEnd + 1; continue;
			}

			// <path ...> or <PATH ...>
			if (p + 5 >= svgTextEnd ||
			    !(strncmp(p+1, "path", 4) == 0 || strncmp(p+1, "PATH", 4) == 0) ||
			    !ImIsSpace(p[5]))
			{ p++; continue; }

			const char* elemStart = p + 1;  // skip '<'
			// Find end of element '>' or '/>'
			const char* elemEnd = elemStart;
			while (elemEnd < svgTextEnd && *elemEnd != '>') elemEnd++;
			if (elemEnd >= svgTextEnd) break;

			// Extract fill colour: explicit attribute takes priority; fall back to inherited group fill
			bool hasExplicit = false;
			ImU32 color = SlugSVGPathFill(elemStart, elemEnd, &hasExplicit);
			if (!hasExplicit) color = fillStack[fillStackTop];
			// color == 0 → transparent / none → skip
			if (color != 0)
			{
				// Extract d="..." attribute
				const char* dp = SlugSVGFindAttr(elemStart, elemEnd, "d");
				if (dp)
				{
					int dlen = 0;
					const char* dval = SlugSVGReadQuoted(dp, &dlen);
					if (dval && dlen > 0)
					{
						// Each <path> is its own layer (SVG painter model: independent fill per path)
						int gi2 = -1;
						if (numGroups < MAX_SVG_GROUPS)
						{
							gi2 = numGroups++;
							groupColor[gi2] = color;
							groupMinX[gi2] = groupMinY[gi2] =  1e30f;
							groupMaxX[gi2] = groupMaxY[gi2] = -1e30f;
						}
						if (gi2 >= 0)
						{
							// Parse path directly (no null-termination needed)
							int curvesBefore = groupCurves[gi2].Size;
							SlugParseSVGPath(dval, dval + dlen, groupCurves[gi2], sc, /*negateY=*/true);
							// Apply accumulated translate transform (font units → em-space, Y negated for Slug Y-up)
							{
								float ttx = txStack[txStackTop] * sc;
								float tty = tyStack[txStackTop] * sc * (-1.0f);
								if (ttx != 0.0f || tty != 0.0f) {
									for (int tk = curvesBefore; tk < groupCurves[gi2].Size; tk++) {
										SlugCurve& cv = groupCurves[gi2][tk];
										cv.p1x += ttx; cv.p1y += tty;
										cv.p2x += ttx; cv.p2y += tty;
										cv.p3x += ttx; cv.p3y += tty;
									}
								}
							}
							// Update group bounding box from newly added curves
							for (int k = curvesBefore; k < groupCurves[gi2].Size; k++)
							{
								const SlugCurve& cv = groupCurves[gi2][k];
								float x0 = ImMin(ImMin(cv.p1x, cv.p2x), cv.p3x);
								float x1 = ImMax(ImMax(cv.p1x, cv.p2x), cv.p3x);
								float y0 = ImMin(ImMin(cv.p1y, cv.p2y), cv.p3y);
								float y1 = ImMax(ImMax(cv.p1y, cv.p2y), cv.p3y);
								groupMinX[gi2] = ImMin(groupMinX[gi2], x0);
								groupMaxX[gi2] = ImMax(groupMaxX[gi2], x1);
								groupMinY[gi2] = ImMin(groupMinY[gi2], y0);
								groupMaxY[gi2] = ImMax(groupMaxY[gi2], y1);
							}
						}
					}
				}
			}
			p = elemEnd + 1;
		}

		if (numGroups == 0) return false;

		// Compute union bounding box across all groups
		float uMinX =  1e30f, uMinY =  1e30f;
		float uMaxX = -1e30f, uMaxY = -1e30f;
		for (int k = 0; k < numGroups; k++)
		{
			if (groupCurves[k].Size == 0) continue;
			uMinX = ImMin(uMinX, groupMinX[k]); uMaxX = ImMax(uMaxX, groupMaxX[k]);
			uMinY = ImMin(uMinY, groupMinY[k]); uMaxY = ImMax(uMaxY, groupMaxY[k]);
		}
		// Pad bounds
		const float pad = 0.01f;
		uMinX -= pad; uMinY -= pad; uMaxX += pad; uMaxY += pad;

		// Build parent glyph entry
		SlugGlyphEntry e = {};
		e.codepoint       = cp;
		e.advanceEm       = advEm;
		e.colorLayerStart = (int)atlas->colorLayers.Size;
		e.colorLayerCount = 0;
		e.minXEm = uMinX; e.minYEm = uMinY;
		e.maxXEm = uMaxX; e.maxYEm = uMaxY;

		for (int k = 0; k < numGroups; k++)
		{
			if (groupCurves[k].Size == 0) continue;
			// Skip degenerate layers whose bbox is too small to be visible
			float gw = groupMaxX[k] - groupMinX[k], gh = groupMaxY[k] - groupMinY[k];
			if (gw < 1e-4f || gh < 1e-4f) continue;
			float minX = groupMinX[k] - pad, maxX = groupMaxX[k] + pad;
			float minY = groupMinY[k] - pad, maxY = groupMaxY[k] + pad;
			SlugGlyphEntry layerEntry = {};
			layerEntry.colorLayerStart = -1;
			if (!SlugBuildGlyphFromCurves(atlas, 0, advEm, minX, minY, maxX, maxY, groupCurves[k], &layerEntry))
				continue;
			SlugColorLayer cl = {};
			cl.glyphEntryIdx = (int)atlas->glyphs.Size - 1; // SlugBuildGlyphFromCurves already pushed
			cl.color         = groupColor[k];
			atlas->colorLayers.push_back(cl);
			e.colorLayerCount++;
		}

		if (e.colorLayerCount == 0) return false;

		atlas->glyphs.push_back(e);
		*outEntry = e;
		return true;
	}

	// ---- Glyph builder ------------------------------------------------------

	// Build a glyph from a glyph index (gi) directly; cp is stored in the entry (0 for layer glyphs).
	// skipColr = true when building a layer's outline — prevents recursive COLR lookup.
	static bool SlugBuildGlyphByIndex(SlugFontCache* atlas, int gi, ImWchar cp, SlugGlyphEntry* outEntry, bool skipColr = false);

	static bool SlugBuildGlyph(SlugFontCache* atlas, ImWchar cp, SlugGlyphEntry* outEntry)
	{
		int gi = stbtt_FindGlyphIndex(&atlas->stbFont, (int)cp);
		if (gi == 0) return false;
		return SlugBuildGlyphByIndex(atlas, gi, cp, outEntry);
	}

	static bool SlugBuildGlyphByIndex(SlugFontCache* atlas, int gi, ImWchar cp, SlugGlyphEntry* outEntry, bool skipColr)
	{
		// ---- COLR: check for color layers (v0 first, fall back to v1) ----
		// skipColr is set for recursive layer builds.
		if (!skipColr)
		{
		ImVector<int>   layerGlyphIDs;
		ImVector<ImU32> layerColors;
		ImVector<SlugColorLayer> v1Layers;  // COLR v1 layers with gradient info
		int nColrLayers = SlugGetColrLayers(atlas, gi, layerGlyphIDs, layerColors);
		bool isV1 = false;
		if (nColrLayers == 0) {
			nColrLayers = SlugGetColrV1Layers(atlas, gi, layerGlyphIDs, v1Layers);
			isV1 = (nColrLayers > 0);
		}
		// SVG fallback: if no COLR layers found, try SVG table
		if (nColrLayers == 0 && atlas->svgTableOffset)
		{
			if (SlugGetSVGLayers(atlas, gi, cp, outEntry))
				return true;
		}
		if (nColrLayers > 0)
		{
			int adv2, lsb2;
			stbtt_GetGlyphHMetrics(&atlas->stbFont, gi, &adv2, &lsb2);

			SlugGlyphEntry e = {};
			e.codepoint        = cp;
			e.advanceEm        = (float)adv2 * atlas->emScale;
			e.colorLayerStart  = (int)atlas->colorLayers.Size;
			e.colorLayerCount  = 0;
			bool firstLayer = true;
			for (int i = 0; i < nColrLayers; i++)
			{
				SlugGlyphEntry layerEntry = {};
				layerEntry.colorLayerStart = -1;
				if (!SlugBuildGlyphByIndex(atlas, layerGlyphIDs[i], 0, &layerEntry, /*skipColr=*/true))
					continue;
				// Skip layers with empty bounding box (no actual curves)
				if ((layerEntry.maxXEm - layerEntry.minXEm) < 1e-5f || (layerEntry.maxYEm - layerEntry.minYEm) < 1e-5f)
					continue;

				SlugColorLayer cl = isV1 ? v1Layers[i] : SlugColorLayer{};
				cl.glyphEntryIdx = (int)atlas->glyphs.Size;
				if (!isV1) cl.color = layerColors[i];
				atlas->colorLayers.push_back(cl);
				atlas->glyphs.push_back(layerEntry);
				e.colorLayerCount++;

				// Include translation offset in base glyph bbox
				float lTX = cl.translateX, lTY = cl.translateY;
				if (firstLayer) {
					e.minXEm = layerEntry.minXEm + lTX; e.maxXEm = layerEntry.maxXEm + lTX;
					e.minYEm = layerEntry.minYEm + lTY; e.maxYEm = layerEntry.maxYEm + lTY;
					firstLayer = false;
				} else {
					e.minXEm = ImMin(e.minXEm, layerEntry.minXEm + lTX);
					e.maxXEm = ImMax(e.maxXEm, layerEntry.maxXEm + lTX);
					e.minYEm = ImMin(e.minYEm, layerEntry.minYEm + lTY);
					e.maxYEm = ImMax(e.maxYEm, layerEntry.maxYEm + lTY);
				}
			}
			if (e.colorLayerCount > 0)
			{
				// Validate: check if the COLR glyph has a reasonable bounding box.
				// For shaped glyphs (cp >= 0x100000), compare against the outline bbox.
				// If COLR bbox is empty but outline has curves, discard COLR and use outline.
				bool colrValid = (e.maxXEm - e.minXEm) > 1e-5f && (e.maxYEm - e.minYEm) > 1e-5f;
				if (!colrValid && cp >= 0x100000) {
					// Discard the COLR layers we just pushed
					atlas->colorLayers.resize(e.colorLayerStart);
					atlas->glyphs.resize(atlas->glyphs.Size - e.colorLayerCount);
					// Fall through to monochrome outline
				} else {
					atlas->glyphs.push_back(e);
					*outEntry = e;
					return true;
				}
			}
			// All layers failed or invalid — fall through to monochrome outline
		}
		}  // end if (!skipColr)

		// ---- normal monochrome outline path ----

		// ---- metrics ----
		int adv, lsb;
		stbtt_GetGlyphHMetrics(&atlas->stbFont, gi, &adv, &lsb);

		int bx0, by0, bx1, by1;
		if (!stbtt_GetGlyphBox(&atlas->stbFont, gi, &bx0, &by0, &bx1, &by1))
		{
			// Empty glyph (space etc.)
			SlugGlyphEntry e = {};
			e.codepoint  = cp;
			e.advanceEm  = (float)adv * atlas->emScale;
			atlas->glyphs.push_back(e);
			*outEntry = e;
			return true;
		}

		const float sc = atlas->emScale;

		// ---- extract Bezier outlines via stb_truetype ----
		stbtt_vertex* verts = NULL;
		int nVerts = stbtt_GetGlyphShape(&atlas->stbFont, gi, &verts);

		ImVector<SlugCurve> curves;
		float curX = 0.0f, curY = 0.0f;

		for (int i = 0; i < nVerts; i++)
		{
			stbtt_vertex& v = verts[i];
			float vx  = v.x  * sc,  vy  = v.y  * sc;
			float vcx = v.cx * sc,   vcy = v.cy * sc;

			switch (v.type)
			{
			case STBTT_vmove:
				curX = vx; curY = vy;
				break;
			case STBTT_vline:
			{
				// Skip zero-length lines (moveto didn't move)
				if (curX == vx && curY == vy) break;
				// Line: degenerate quadratic (midpoint as control point)
				SlugCurve c = {};
				c.p1x = curX; c.p1y = curY;
				c.p2x = (curX + vx) * 0.5f; c.p2y = (curY + vy) * 0.5f;
				c.p3x = vx;   c.p3y = vy;
				curves.push_back(c);
				curX = vx; curY = vy;
				break;
			}
			case STBTT_vcurve:
			{
				SlugCurve c = {};
				c.p1x = curX; c.p1y = curY;
				c.p2x = vcx;  c.p2y = vcy;
				c.p3x = vx;   c.p3y = vy;
				curves.push_back(c);
				curX = vx; curY = vy;
				break;
			}
			case STBTT_vcubic:
			{
				// Approximate cubic as quadratics (subdivide 2 levels → 4 quads per cubic)
				float cx0 = v.cx  * sc, cy0 = v.cy  * sc;
				float cx1 = v.cx1 * sc, cy1 = v.cy1 * sc;
				SlugCubicToQuads(curves, curX, curY, cx0, cy0, cx1, cy1, vx, vy);
				curX = vx; curY = vy;
				break;
			}
			}
		}
		stbtt_FreeShape(&atlas->stbFont, verts);

		// Compute bounding box from actual extracted curves (not stbtt_GetGlyphBox)
		// to guarantee the bbox matches the curves exactly
		const float pad = 0.01f;
		float minX =  1e30f, minY =  1e30f;
		float maxX = -1e30f, maxY = -1e30f;
		for (int i = 0; i < curves.Size; i++)
		{
			const SlugCurve& c = curves[i];
			minX = ImMin(minX, ImMin(ImMin(c.p1x, c.p2x), c.p3x));
			maxX = ImMax(maxX, ImMax(ImMax(c.p1x, c.p2x), c.p3x));
			minY = ImMin(minY, ImMin(ImMin(c.p1y, c.p2y), c.p3y));
			maxY = ImMax(maxY, ImMax(ImMax(c.p1y, c.p2y), c.p3y));
		}
		if (curves.Size == 0) { minX = minY = maxX = maxY = 0.0f; }
		minX -= pad; minY -= pad; maxX += pad; maxY += pad;

		return SlugBuildGlyphFromCurves(atlas, cp, (float)adv * sc, minX, minY, maxX, maxY, curves, outEntry);
	}

	static bool SlugBuildGlyphFromCurves(SlugFontCache* atlas, ImWchar cp, float advEm,
	                                      float minX, float minY, float maxX, float maxY,
	                                      ImVector<SlugCurve>& curves, SlugGlyphEntry* outEntry)
	{
		int nc = curves.Size;

		// ---- handle empty glyph ----
		if (nc == 0)
		{
			SlugGlyphEntry e = {};
			e.codepoint  = cp;
			e.advanceEm  = advEm;
			e.minXEm = minX; e.minYEm = minY;
			e.maxXEm = maxX; e.maxYEm = maxY;
			atlas->glyphs.push_back(e);
			*outEntry = e;
			return true;
		}

		float glyphW = maxX - minX;
		float glyphH = maxY - minY;
		if (glyphW < 1e-5f) glyphW = 1e-5f;
		if (glyphH < 1e-5f) glyphH = 1e-5f;

		// ---- write each curve into the curve texture ----
		// curveLoc[i] = texel position (x, y) of curve i in the curve texture
		ImVector<int> curveLocX, curveLocY;
		curveLocX.resize(nc);
		curveLocY.resize(nc);

		for (int i = 0; i < nc; i++)
		{
			int cx, cy;
			SlugAllocOneCurve(atlas, &cx, &cy);
			curveLocX[i] = cx;
			curveLocY[i] = cy;
			const SlugCurve& c = curves[i];
			SlugCurveWrite4f(atlas, cx,   cy, c.p1x, c.p1y, c.p2x, c.p2y);
			SlugCurveWrite4f(atlas, cx+1, cy, c.p3x, c.p3y, 0.0f, 0.0f);
		}

		// ---- band assignment ----
		// Horizontal bands partition the Y-axis; curves are sorted by descending max-X.
		// Vertical bands partition the X-axis;   curves are sorted by descending max-Y.

		// Adaptive band count: scale with sqrt(curve_count) weighted by glyph aspect ratio.
		// More bands → finer acceleration, fewer wasted ray tests per pixel.
		// NBY capped at 255 (packed into 8 bits in glyph.w); NBX capped at 64 for safety.
		int NBX, NBY;
		if (nc == 0)
		{
			NBX = 1; NBY = 1;
		}
		else
		{
			float sqrtC  = sqrtf((float)nc);
			float aspect = (glyphH > 1e-6f) ? (glyphW / glyphH) : 1.0f;
			float sqA    = sqrtf(aspect);
			NBX = ImClamp((int)ceilf(sqrtC * sqA),       4, 64);
			NBY = ImClamp((int)ceilf(sqrtC / sqA),       4, 64);
		}

		const float bsx = (float)NBX / glyphW;            // em → band-x scale
		const float bsy = (float)NBY / glyphH;            // em → band-y scale
		const float box = -minX * bsx;                    // em → band-x offset
		const float boy = -minY * bsy;                    // em → band-y offset

		// Per-band curve lists (heap-allocated, size determined adaptively)
		ImVector<int>* hBand = new ImVector<int>[NBY];    // horizontal bands (indexed by y-band)
		ImVector<int>* vBand = new ImVector<int>[NBX];    // vertical bands  (indexed by x-band)

		for (int i = 0; i < nc; i++)
		{
			const SlugCurve& c = curves[i];
			float cMinX = ImMin(ImMin(c.p1x, c.p2x), c.p3x);
			float cMaxX = ImMax(ImMax(c.p1x, c.p2x), c.p3x);
			float cMinY = ImMin(ImMin(c.p1y, c.p2y), c.p3y);
			float cMaxY = ImMax(ImMax(c.p1y, c.p2y), c.p3y);

			// Horizontal bands: which y-strips does this curve's y-extent overlap?
			int hyMin = (int)(( cMinY - minY ) * bsy);
			int hyMax = (int)ceilf(( cMaxY - minY ) * bsy);
			hyMin = ImClamp(hyMin, 0, NBY - 1);
			hyMax = ImClamp(hyMax, 0, NBY - 1);
			for (int b = hyMin; b <= hyMax; b++)
				hBand[b].push_back(i);

			// Vertical bands: which x-strips does this curve's x-extent overlap?
			int vxMin = (int)(( cMinX - minX ) * bsx);
			int vxMax = (int)ceilf(( cMaxX - minX ) * bsx);
			vxMin = ImClamp(vxMin, 0, NBX - 1);
			vxMax = ImClamp(vxMax, 0, NBX - 1);
			for (int b = vxMin; b <= vxMax; b++)
				vBand[b].push_back(i);
		}

		// Sort horizontal bands by descending max-X (for early-exit in the PS).
		// Sort key must match the shader's early-exit check: max over all control points.
		auto CurveMaxX = [&](int idx) -> float {
			const SlugCurve& c = curves[idx];
			return ImMax(ImMax(c.p1x, c.p2x), c.p3x);
		};
		auto CurveMaxY = [&](int idx) -> float {
			const SlugCurve& c = curves[idx];
			return ImMax(ImMax(c.p1y, c.p2y), c.p3y);
		};

		for (int b = 0; b < NBY; b++)
		{
			ImVector<int>& lst = hBand[b];
			for (int i = 1; i < lst.Size; i++)
			{
				int   key     = lst[i];
				float keyMaxX = CurveMaxX(key);
				int j = i - 1;
				while (j >= 0 && CurveMaxX(lst[j]) < keyMaxX)
				{
					lst[j + 1] = lst[j];
					j--;
				}
				lst[j + 1] = key;
			}
		}

		// Sort vertical bands by descending max-Y
		for (int b = 0; b < NBX; b++)
		{
			ImVector<int>& lst = vBand[b];
			for (int i = 1; i < lst.Size; i++)
			{
				int   key     = lst[i];
				float keyMaxY = CurveMaxY(key);
				int j = i - 1;
				while (j >= 0 && CurveMaxY(lst[j]) < keyMaxY)
				{
					lst[j + 1] = lst[j];
					j--;
				}
				lst[j + 1] = key;
			}
		}

		// Count total curve references for band texture allocation
		int totalCurveRefs = 0;
		for (int b = 0; b < NBY; b++) totalCurveRefs += hBand[b].Size;
		for (int b = 0; b < NBX; b++) totalCurveRefs += vBand[b].Size;

		// ---- allocate glyph block in band texture ----
		// The header block must fit in one row (direct indexed by the shader without wrapping).
		// Header layout: NBY horiz headers, then NBX vert headers = NBY + NBX total.
		int totalHeaders = NBY + NBX;

		// Ensure headers fit in current row
		int bx = atlas->bandCursor % SLUG_TEX_WIDTH;
		int by = atlas->bandCursor / SLUG_TEX_WIDTH;
		if (bx + totalHeaders > SLUG_TEX_WIDTH)
		{
			// Move to start of next row
			atlas->bandCursor = (by + 1) * SLUG_TEX_WIDTH;
			bx = 0;
			by++;
		}
		int glyphLocX = bx;
		int glyphLocY = by;

		// Reserve: headers + curve ref list (curve refs can wrap via CalcBandLoc)
		atlas->bandCursor += totalHeaders + totalCurveRefs;

		// ---- write band headers and curve ref lists ----
		// offset is relative to glyphLoc, using CalcBandLoc semantics
		int nextOffset = totalHeaders;  // first curve list starts after all headers

		for (int b = 0; b < NBY; b++)
		{
			// Write horizontal band header at (glyphLocX + b, glyphLocY)
			SlugBandWrite2f(atlas, glyphLocX + b, glyphLocY,
			                (float)hBand[b].Size, (float)nextOffset);

			// Write curve references at CalcBandLoc(glyphLoc, nextOffset + i)
			for (int i = 0; i < hBand[b].Size; i++)
			{
				int ci = hBand[b][i];
				int ax, ay;
				SlugBandCalcLoc(glyphLocX, glyphLocY, nextOffset + i, &ax, &ay);
				SlugBandWrite2f(atlas, ax, ay, (float)curveLocX[ci], (float)curveLocY[ci]);
			}
			nextOffset += hBand[b].Size;
		}

		for (int b = 0; b < NBX; b++)
		{
			// Write vertical band header at (glyphLocX + NBY + b, glyphLocY)
			SlugBandWrite2f(atlas, glyphLocX + NBY + b, glyphLocY,
			                (float)vBand[b].Size, (float)nextOffset);

			for (int i = 0; i < vBand[b].Size; i++)
			{
				int ci = vBand[b][i];
				int ax, ay;
				SlugBandCalcLoc(glyphLocX, glyphLocY, nextOffset + i, &ax, &ay);
				SlugBandWrite2f(atlas, ax, ay, (float)curveLocX[ci], (float)curveLocY[ci]);
			}
			nextOffset += vBand[b].Size;
		}

		// ---- fill in glyph entry ----
		SlugGlyphEntry e = {};
		e.codepoint    = cp;
		e.bandTexX     = glyphLocX;
		e.bandTexY     = glyphLocY;
		e.bandMaxX     = NBX - 1;
		e.bandMaxY     = NBY - 1;
		e.bandScaleX   = bsx;
		e.bandScaleY   = bsy;
		e.bandOffsetX  = box;
		e.bandOffsetY  = boy;
		e.advanceEm    = advEm;
		e.minXEm = minX; e.minYEm = minY;
		e.maxXEm = maxX; e.maxYEm = maxY;

		atlas->glyphs.push_back(e);
		atlas->dirty = true;
		*outEntry = e;

		delete[] hBand;
		delete[] vBand;
		return true;
	}

	// ---- Atlas management ---------------------------------------------------

	static SlugGlyphEntry* SlugFindGlyph(SlugFontCache* atlas, ImWchar cp)
	{
		for (int i = 0; i < atlas->glyphs.Size; i++)
			if (atlas->glyphs[i].codepoint == cp)
				return &atlas->glyphs[i];
		return NULL;
	}

#if IM_SUPPORT_LIGATURE
#ifdef _WIN32
	// Separate function so __try can be used (not allowed in functions with C++ object unwinding)
	static kbts_font* SlugTryShapePushFont(kbts_shape_context* ctx, void* data, int size)
	{
		kbts_font* f = NULL;
		__try { f = kbts_ShapePushFontFromMemory(ctx, data, size, 0); }
		__except(1) { f = NULL; }
		return f;
	}
#endif
	// Validate GSUB ligature subtables — reject fonts with malformed ComponentCount
	// that would crash kbts_PlaceBlob via out-of-bounds array access.
	static void SlugInitShaping(SlugFontCache* atlas, void* fontData, int fontDataSize)
	{
		// Only init shaping if font has a GSUB table
		const uint8_t* raw = (const uint8_t*)fontData;
		uint32_t fontStart = (uint32_t)atlas->stbFont.fontstart;
		if (SlugFindTable(raw, fontStart, "GSUB") == 0)
			return; // No GSUB → no shaping needed

		kbts_shape_context* ctx = kbts_CreateShapeContext(0, 0);
#ifdef _WIN32
		kbts_font* f = SlugTryShapePushFont(ctx, fontData, fontDataSize);
#else
		kbts_font* f = kbts_ShapePushFontFromMemory(ctx, fontData, fontDataSize, 0);
#endif
		if (f && f->Error == 0) {
			// Default features: liga + calt cover standard ligatures and contextual alternates.
			// ss01-ss10 (stylistic sets) are NOT pushed globally — they're per-font optional
			// features that can break some fonts (e.g. Monblock's connected forms).
			// Use kbts_ShapePushFeature() per-font if you need specific stylistic sets
			// (e.g. FiraCode's =~ ligature requires ss07).
			kbts_ShapePushFeature(ctx, KBTS_FEATURE_TAG_liga, 1);
			kbts_ShapePushFeature(ctx, KBTS_FEATURE_TAG_calt, 1);
			atlas->shapeCtx  = ctx;
			atlas->shapeFont = f;
		} else {
			kbts_DestroyShapeContext(ctx);
		}
	}
#endif

	static SlugFontCache* SlugGetOrCreateAtlas(ImWidgetsSlugState* state, ImFont* font)
	{
		// Find existing atlas for this font
		for (int i = 0; i < state->atlases.Size; i++)
			if (state->atlases[i]->imguiFont == font)
				return state->atlases[i];

		// Create new atlas — find font config by matching DstFont pointer
		if (!font)
			return NULL;

		ImFontAtlas* fontAtlas = ImGui::GetIO().Fonts;
		ImFontConfig* cfg = NULL;
		for (int i = 0; i < fontAtlas->Sources.Size; i++)
		{
			if (fontAtlas->Sources[i].DstFont == font)
			{
				cfg = &fontAtlas->Sources[i];
				break;
			}
		}
		if (!cfg || !cfg->FontData || cfg->FontDataSize == 0)
			return NULL;

		SlugFontCache* atlas = IM_NEW(SlugFontCache);
		atlas->imguiFont     = font;
		atlas->curveCursor   = 0;
		atlas->curveTexHeight = SLUG_TEX_INIT_H;
		atlas->curveTex.resize(SLUG_TEX_WIDTH * SLUG_TEX_INIT_H * 4, 0.0f);
		atlas->bandCursor    = 0;
		atlas->bandTexHeight = SLUG_TEX_INIT_H;
		atlas->bandTex.resize(SLUG_TEX_WIDTH * SLUG_TEX_INIT_H * 4, 0.0f);
		atlas->curveTexture  = NULL;
		atlas->bandTexture   = NULL;
		atlas->dirty         = true;

		int offset = stbtt_GetFontOffsetForIndex((unsigned char*)cfg->FontData, 0);
		if (!stbtt_InitFont(&atlas->stbFont, (unsigned char*)cfg->FontData, offset))
		{
			IM_DELETE(atlas);
			return NULL;
		}

		// stbtt_ScaleForMappingEmToPixels(info, 1.0f) = 1.0f / unitsPerEm
		atlas->emScale = stbtt_ScaleForMappingEmToPixels(&atlas->stbFont, 1.0f);

		// Cache COLR/CPAL table offsets for color font support
		const uint8_t* rawData = (const uint8_t*)atlas->stbFont.data;
		uint32_t       fontStart = (uint32_t)atlas->stbFont.fontstart;
		atlas->colrTableOffset = SlugFindTable(rawData, fontStart, "COLR");
		atlas->cpalTableOffset = SlugFindTable(rawData, fontStart, "CPAL");
		atlas->svgTableOffset  = SlugFindTable(rawData, fontStart, "SVG ");

#if IM_SUPPORT_LIGATURE
		atlas->shapeCtx  = NULL;
		atlas->shapeFont = NULL;
		SlugInitShaping(atlas, cfg->FontData, cfg->FontDataSize);
#endif

		state->atlases.push_back(atlas);
		return atlas;
	}

	static void SlugUploadTextures(SlugFontCache* atlas)
	{
		if (!atlas->dirty) return;

		// Curve texture
		if (atlas->curveTexture != NULL)
			ImPlatform_DestroyTexture(atlas->curveTexture);
		{
			ImPlatform_TextureDesc desc = {};
			desc.width      = SLUG_TEX_WIDTH;
			desc.height     = (unsigned int)atlas->curveTexHeight;
			desc.format     = ImPlatform_PixelFormat_RGBA32F;
			desc.min_filter = ImPlatform_TextureFilter_Nearest;
			desc.mag_filter = ImPlatform_TextureFilter_Nearest;
			desc.wrap_u     = ImPlatform_TextureWrap_Clamp;
			desc.wrap_v     = ImPlatform_TextureWrap_Clamp;
			atlas->curveTexture = ImPlatform_CreateTexture(atlas->curveTex.Data, &desc);
		}

		// Band texture
		if (atlas->bandTexture != NULL)
			ImPlatform_DestroyTexture(atlas->bandTexture);
		{
			ImPlatform_TextureDesc desc = {};
			desc.width      = SLUG_TEX_WIDTH;
			desc.height     = (unsigned int)atlas->bandTexHeight;
			desc.format     = ImPlatform_PixelFormat_RGBA32F;
			desc.min_filter = ImPlatform_TextureFilter_Nearest;
			desc.mag_filter = ImPlatform_TextureFilter_Nearest;
			desc.wrap_u     = ImPlatform_TextureWrap_Clamp;
			desc.wrap_v     = ImPlatform_TextureWrap_Clamp;
			atlas->bandTexture = ImPlatform_CreateTexture(atlas->bandTex.Data, &desc);
		}

		atlas->dirty = false;
	}

	static void SlugDestroyAtlas(SlugFontCache* atlas)
	{
		if (atlas->curveTexture) ImPlatform_DestroyTexture(atlas->curveTexture);
		if (atlas->bandTexture)  ImPlatform_DestroyTexture(atlas->bandTexture);
#if IM_SUPPORT_LIGATURE
		if (atlas->shapeCtx)     kbts_DestroyShapeContext(atlas->shapeCtx);
#endif
		IM_DELETE(atlas);
	}

	// ---- Slug draw callbacks ------------------------------------------------
	//
	// DX11 path (reference implementation, single draw call):
	//   Custom vertex buffer with SlugVertex format (56 bytes):
	//     POSITION  float2 - screen pos, CPU-dilated by 0.5 px
	//     TEXCOORD0 float4 - em UV (xy, dilated) + packed glyph data (zw, bit-cast)
	//     TEXCOORD1 float4 - band transform (scaleX, scaleY, offsetX, offsetY)
	//     COLOR0    float4 - RGBA vertex color
	//
	//   tex.z bits 0-15 = bandTexX, bits 16-31 = bandTexY
	//   tex.w bits 0-7  = bandMaxX, bits 16-23 = bandMaxY, bit 28 = E (even-odd flag)
	//
	//   All glyphs in one DrawText call share one VB/IB → one draw call.
	//   Per-glyph data is in the vertex stream, decoded in VS by SlugUnpack,
	//   passed as nointerpolation int4 glyph and float4 banding to PS.
	//
	// All backends use this single-draw-call VB/IB path.
	// DX11 additionally needs CreateVertexInputLayout (builds D3D11 input layout from VS bytecode).
	// OpenGL/Metal/WGSL use layout(location=N) / pipeline descriptors set up in CreateVertexBuffer.

	// Vertex format matching slug.hlsl VS_INPUT (80 bytes = 5 x float4)
	// Matches the reference implementation layout exactly.
	struct SlugVertex
	{
		float pos[4];  // xy = screen-space position (undilated), zw = outward vertex normal
		float tex[4];  // xy = em UV (undilated), zw = packed glyph data (bit-cast uint)
		float jac[4];  // inverse Jacobian: maps screen-space offset → em-space offset
		               //   = (1/sz, 0, 0, -1/sz) for axis-aligned glyph at pixel size sz
		float bnd[4];  // band transform: scaleX, scaleY, offsetX, offsetY
		float col[4];  // RGBA vertex color as floats
	};

	// Extended vertex for SLUG_GRADIENT permutation (112 bytes = 7 x float4)
	struct SlugGradientVertex
	{
		float pos[4];
		float tex[4];
		float jac[4];
		float bnd[4];
		float col[4];   // gradient color 0
		float grd[4];   // gradient params: dirX, dirY, scale, bias
		float col2[4];  // gradient color 1
	};

	// Data passed to the raw draw callback
	struct SlugDrawCBData
	{
		ImPlatform_ShaderProgram program;
		ImPlatform_VertexBuffer  vb;
		ImPlatform_IndexBuffer   ib;
		SlugFontCache*           atlas;   // read textures at draw time (latest after all uploads)
		unsigned int             indexCount;
	};

	static void SlugRawDraw(const ImDrawList*, const ImDrawCmd* cmd)
	{
		SlugDrawCBData* d = (SlugDrawCBData*)cmd->UserCallbackData;
		if (!d) return;
		ImTextureID curveTex = d->atlas ? d->atlas->curveTexture : NULL;
		ImTextureID bandTex  = d->atlas ? d->atlas->bandTexture  : NULL;
		if (!d->program || !d->vb || !d->ib || !curveTex || !bandTex) {
			if (d->vb) ImPlatform_DestroyVertexBuffer(d->vb);
			if (d->ib) ImPlatform_DestroyIndexBuffer(d->ib);
			IM_FREE(d);
			return;
		}

		ImPlatform_BindBuffers(d->vb, d->ib);
		ImPlatform_SetShaderTexture(d->program, "curveTexture", 0, curveTex);
		ImPlatform_SetShaderTexture(d->program, "bandTexture",  1, bandTex);
		ImPlatform_DrawIndexed(0, d->indexCount, 0);

		ImPlatform_DestroyVertexBuffer(d->vb);
		ImPlatform_DestroyIndexBuffer(d->ib);
		IM_FREE(d);
	}

	// ---- Public API implementation ------------------------------------------

	ImVec2 CalcTextSize(ImFont* font, float font_size,
	                    const char* text, const char* text_end, float* out_ascent)
	{
		if (!gs_pContext || !text) { if (out_ascent) *out_ascent = 0.0f; return ImVec2(0, 0); }
		if (!text_end) text_end = text + strlen(text);
		if (text >= text_end) { if (out_ascent) *out_ascent = 0.0f; return ImVec2(0, 0); }

		if (!font)           font      = ImGui::GetFont();
		if (font_size <= 0.0f) font_size = ImGui::GetFontSize();

		ImWidgetsSlugState* state = gs_pContext->slugState;
		if (!state) state = gs_pContext->slugState = IM_NEW(ImWidgetsSlugState);

		SlugFontCache* atlas = SlugGetOrCreateAtlas(state, font);
		if (!atlas) { if (out_ascent) *out_ascent = font_size; return ImVec2(0, font_size); }

		// Ensure all glyphs are built (no texture upload needed for measurement)
		const char* p = text;
		while (p < text_end)
		{
			unsigned int cp = 0;
			p += ImTextCharFromUtf8((unsigned int*)&cp, p, text_end);
			if (cp == 0) break;
			if (SlugFindGlyph(atlas, (ImWchar)cp) == NULL)
			{
				SlugGlyphEntry e;
				SlugBuildGlyph(atlas, (ImWchar)cp, &e);
			}
		}

		float width   = 0.0f;
		float maxY    =  0.0f;  // highest point above baseline (em units, positive)
		float minY    =  0.0f;  // lowest  point below baseline (em units, negative)

		p = text;
		while (p < text_end)
		{
			unsigned int cp = 0;
			p += ImTextCharFromUtf8((unsigned int*)&cp, p, text_end);
			if (cp == 0) break;
			SlugGlyphEntry* ge = SlugFindGlyph(atlas, (ImWchar)cp);
			if (!ge) continue;
			width += ge->advanceEm * font_size;
			maxY   = ImMax(maxY, ge->maxYEm);
			minY   = ImMin(minY, ge->minYEm);
		}

		if (out_ascent) *out_ascent = maxY * font_size;
		return ImVec2(width, (maxY - minY) * font_size);
	}

	void DrawText(ImDrawList* pDrawList, ImFont* font, float font_size,
	              ImVec2 pos, ImU32 col, const char* text, const char* text_end)
	{
		if (!pDrawList || !gs_pContext || !text || text == text_end) return;
		if (!text_end) text_end = text + strlen(text);
		if (text >= text_end) return;

		// Resolve font and size
		if (!font)      font      = ImGui::GetFont();
		if (font_size <= 0.0f) font_size = ImGui::GetFontSize();

#if IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER
		// Shaders must be compiled at startup via ImWidgetsFeatures_RichFont + CreateContext()
		if (!gs_pContext->slugShader.program) return;

		if (!gs_pContext->slugState)
			gs_pContext->slugState = IM_NEW(ImWidgetsSlugState);

		ImWidgetsSlugState* state = gs_pContext->slugState;

		// Get or create the font atlas for this font
		SlugFontCache* atlas = SlugGetOrCreateAtlas(state, font);
		if (!atlas) return;

		// Pre-build all glyphs and upload textures if anything is new
		bool anyNew = false;
		const char* p = text;
		while (p < text_end)
		{
			unsigned int cp = 0;
			p += ImTextCharFromUtf8((unsigned int*)&cp, p, text_end);
			if (cp == 0) break;
			if (SlugFindGlyph(atlas, (ImWchar)cp) == NULL)
			{
				SlugGlyphEntry e;
				if (cp >= 0x100000) {
					// Synthetic codepoint (MATH assembly part): build by glyph index
					int gi = (int)(cp - 0x100000);
					SlugBuildGlyphByIndex(atlas, gi, (ImWchar)cp, &e);
				} else {
					SlugBuildGlyph(atlas, (ImWchar)cp, &e);
				}
				anyNew = true;
			}
		}
		// NOTE: texture upload deferred until after shaping builds additional glyphs

		// ---- Text shaping (ligatures, contextual forms, RTL, OpenType features) ----
		struct ShapedGlyph { int glyphID; float advanceX; float offsetX; float offsetY; };
		ImVector<ShapedGlyph> shapedGlyphs;
#if IM_SUPPORT_LIGATURE
		// Skip shaping for synthetic codepoints (0x100000+ range, used by MATH table assembly)
		bool skipShaping = false;
		{
			unsigned int firstCp = 0;
			ImTextCharFromUtf8(&firstCp, text, text_end);
			if (firstCp >= 0x100000) skipShaping = true;
		}
		if (atlas->shapeCtx && atlas->shapeFont && !skipShaping)
		{
			int textLen = (int)(text_end - text);
			kbts_ShapeBegin(atlas->shapeCtx, KBTS_DIRECTION_DONT_KNOW, KBTS_LANGUAGE_DONT_KNOW);
			kbts_ShapeUtf8(atlas->shapeCtx, text, textLen, KBTS_USER_ID_GENERATION_MODE_CODEPOINT_INDEX);
			kbts_ShapeEnd(atlas->shapeCtx);

			kbts_run run;
			while (kbts_ShapeRun(atlas->shapeCtx, &run))
			{
				kbts_glyph* glyph;
				while (kbts_GlyphIteratorNext(&run.Glyphs, &glyph))
				{
					ShapedGlyph sg;
					sg.glyphID  = (int)glyph->Id;
					sg.advanceX = (float)glyph->AdvanceX * atlas->emScale;
					sg.offsetX  = (float)glyph->OffsetX  * atlas->emScale;
					sg.offsetY  = (float)glyph->OffsetY  * atlas->emScale;
					shapedGlyphs.push_back(sg);

					// Build glyph by ID if not already built (use high codepoint range to avoid collision)
					ImWchar glyphKey = (ImWchar)(0x100000 + sg.glyphID);
					if (!SlugFindGlyph(atlas, glyphKey))
					{
						SlugGlyphEntry e;
						if (SlugBuildGlyphByIndex(atlas, sg.glyphID, glyphKey, &e))
							anyNew = true;
					}
				}
			}
		}
#endif
		if (anyNew || atlas->dirty)
			SlugUploadTextures(atlas);
		if (!atlas->curveTexture || !atlas->bandTexture) return;

		// Count glyphs so we can allocate exactly
		int glyphCount = 0;
		if (shapedGlyphs.Size > 0)
			glyphCount = shapedGlyphs.Size;
		else
		{
			p = text;
			while (p < text_end)
			{
				unsigned int cp = 0;
				const char* next = p + ImTextCharFromUtf8((unsigned int*)&cp, p, text_end);
				if (cp == 0) break;
				SlugGlyphEntry* ge = SlugFindGlyph(atlas, (ImWchar)cp);
				if (ge && (ge->maxXEm - ge->minXEm) > 1e-5f && (ge->maxYEm - ge->minYEm) > 1e-5f)
					glyphCount++;
				p = next;
			}
		}
		if (glyphCount == 0) return;

		const float sz    = font_size;        // pixels per em
		const float invSz = 1.0f / sz;       // 1 screen pixel = invSz em units

		float penX = pos.x;

		// ---- Reference Slug implementation — single draw call ----
		//
		// All glyphs packed into one SlugVertex VB + uint16_t IB.
		// Per-glyph data (band loc, band transform, normal, Jacobian) lives in the vertex stream.
		// Dilation is performed in the vertex shader (SlugDilate) — no CPU pre-dilation.

		// Vertex attribute layout for slug VS_INPUT (5 attributes, 80 bytes).
		// Semantic names used by DX11; other backends use sequential location indices (0-4).
		static const ImPlatform_VertexAttribute kSlugAttribs[] = {
			{ ImPlatform_VertexFormat_Float4, offsetof(SlugVertex, pos), "POSITION" },
			{ ImPlatform_VertexFormat_Float4, offsetof(SlugVertex, tex), "TEXCOORD" },
			{ ImPlatform_VertexFormat_Float4, offsetof(SlugVertex, jac), "TEXCOORD" },
			{ ImPlatform_VertexFormat_Float4, offsetof(SlugVertex, bnd), "TEXCOORD" },
			{ ImPlatform_VertexFormat_Float4, offsetof(SlugVertex, col), "COLOR"    },
		};

		// Float color (ImU32 is 0xAABBGGRR)
		auto U32toF4 = [](ImU32 c, float* r, float* g, float* b, float* a) {
			*r = (float)((c >>  0) & 0xFF) / 255.0f;
			*g = (float)((c >>  8) & 0xFF) / 255.0f;
			*b = (float)((c >> 16) & 0xFF) / 255.0f;
			*a = (float)((c >> 24) & 0xFF) / 255.0f;
		};

		// Three vertex/index buffers: monochrome, solid color layers, gradient color layers
		ImVector<SlugVertex> vertsNorm,  vertsColor;
		ImVector<ImU16>      idxsNorm,   idxsColor;
		ImVector<SlugGradientVertex> vertsGrad;
		ImVector<ImU16>              idxsGrad;
		vertsNorm.reserve(glyphCount * 4);
		idxsNorm.reserve(glyphCount * 6);

		// Helper: emit one quad into a vertex/index buffer for a given glyph entry + color
		auto EmitQuad = [&](ImVector<SlugVertex>& vBuf, ImVector<ImU16>& iBuf,
		                    const SlugGlyphEntry* ge, float penX_, float posY_, ImU32 quadCol)
		{
			float sL = penX_ + ge->minXEm * sz;
			float sR = penX_ + ge->maxXEm * sz;
			float sT = posY_ - ge->maxYEm * sz;
			float sB = posY_ - ge->minYEm * sz;
			float uL = ge->minXEm, uR = ge->maxXEm;
			float uT = ge->maxYEm, uB = ge->minYEm;

			unsigned int gz = (unsigned int)(ImU16)ge->bandTexX
			                | ((unsigned int)(ImU16)ge->bandTexY << 16);
			unsigned int gw = (unsigned int)(ge->bandMaxX & 0xFF)
			                | ((unsigned int)(ge->bandMaxY & 0xFF) << 16);
			float fgz, fgw;
			memcpy(&fgz, &gz, 4);
			memcpy(&fgw, &gw, 4);

			float cR, cG, cB, cA;
			U32toF4(quadCol, &cR, &cG, &cB, &cA);

			ImU16 base = (ImU16)vBuf.Size;
			SlugVertex v;
			v.tex[2] = fgz; v.tex[3] = fgw;
			v.jac[0] = invSz; v.jac[1] = 0.0f;
			v.jac[2] = 0.0f;  v.jac[3] = -invSz;
			v.bnd[0] = ge->bandScaleX;  v.bnd[1] = ge->bandScaleY;
			v.bnd[2] = ge->bandOffsetX; v.bnd[3] = ge->bandOffsetY;
			v.col[0] = cR; v.col[1] = cG; v.col[2] = cB; v.col[3] = cA;

			v.pos[0] = sL; v.pos[1] = sT; v.pos[2] = -1.0f; v.pos[3] = -1.0f; v.tex[0] = uL; v.tex[1] = uT; vBuf.push_back(v);
			v.pos[0] = sR; v.pos[1] = sT; v.pos[2] = +1.0f; v.pos[3] = -1.0f; v.tex[0] = uR; v.tex[1] = uT; vBuf.push_back(v);
			v.pos[0] = sR; v.pos[1] = sB; v.pos[2] = +1.0f; v.pos[3] = +1.0f; v.tex[0] = uR; v.tex[1] = uB; vBuf.push_back(v);
			v.pos[0] = sL; v.pos[1] = sB; v.pos[2] = -1.0f; v.pos[3] = +1.0f; v.tex[0] = uL; v.tex[1] = uB; vBuf.push_back(v);

			iBuf.push_back(base + 0); iBuf.push_back(base + 1); iBuf.push_back(base + 2);
			iBuf.push_back(base + 0); iBuf.push_back(base + 2); iBuf.push_back(base + 3);
		};

		// Gradient quad emitter (SLUG_GRADIENT permutation — 7 float4 vertex)
		auto EmitGradQuad = [&](const SlugGlyphEntry* ge, float penX_, float posY_, const SlugColorLayer& cl)
		{
			float sL = penX_ + ge->minXEm * sz, sR = penX_ + ge->maxXEm * sz;
			float sT = posY_ - ge->maxYEm * sz, sB = posY_ - ge->minYEm * sz;
			float uL = ge->minXEm, uR = ge->maxXEm, uT = ge->maxYEm, uB = ge->minYEm;

			unsigned int gz = (unsigned int)(ImU16)ge->bandTexX | ((unsigned int)(ImU16)ge->bandTexY << 16);
			unsigned int gw = (unsigned int)(ge->bandMaxX & 0xFF) | ((unsigned int)(ge->bandMaxY & 0xFF) << 16);
			float fgz, fgw; memcpy(&fgz, &gz, 4); memcpy(&fgw, &gw, 4);

			float c0R, c0G, c0B, c0A, c1R, c1G, c1B, c1A;
			U32toF4(cl.gradColor0, &c0R, &c0G, &c0B, &c0A);
			U32toF4(cl.gradColor1, &c1R, &c1G, &c1B, &c1A);

			ImU16 base = (ImU16)vertsGrad.Size;
			SlugGradientVertex v = {};
			v.tex[2] = fgz; v.tex[3] = fgw;
			v.jac[0] = invSz; v.jac[1] = 0; v.jac[2] = 0; v.jac[3] = -invSz;
			v.bnd[0] = ge->bandScaleX; v.bnd[1] = ge->bandScaleY;
			v.bnd[2] = ge->bandOffsetX; v.bnd[3] = ge->bandOffsetY;
			v.col[0] = c0R; v.col[1] = c0G; v.col[2] = c0B; v.col[3] = c0A;
			v.grd[0] = cl.gradDirX; v.grd[1] = cl.gradDirY;
			v.grd[2] = cl.gradScale; v.grd[3] = cl.gradBias;
			v.col2[0] = c1R; v.col2[1] = c1G; v.col2[2] = c1B; v.col2[3] = c1A;

			v.pos[0]=sL; v.pos[1]=sT; v.pos[2]=-1; v.pos[3]=-1; v.tex[0]=uL; v.tex[1]=uT; vertsGrad.push_back(v);
			v.pos[0]=sR; v.pos[1]=sT; v.pos[2]=+1; v.pos[3]=-1; v.tex[0]=uR; v.tex[1]=uT; vertsGrad.push_back(v);
			v.pos[0]=sR; v.pos[1]=sB; v.pos[2]=+1; v.pos[3]=+1; v.tex[0]=uR; v.tex[1]=uB; vertsGrad.push_back(v);
			v.pos[0]=sL; v.pos[1]=sB; v.pos[2]=-1; v.pos[3]=+1; v.tex[0]=uL; v.tex[1]=uB; vertsGrad.push_back(v);

			idxsGrad.push_back(base+0); idxsGrad.push_back(base+1); idxsGrad.push_back(base+2);
			idxsGrad.push_back(base+0); idxsGrad.push_back(base+2); idxsGrad.push_back(base+3);
		};

		// Emit glyphs — either from shaped glyph list or codepoint iteration
		int shapedIdx = 0;
		p = text;
		while (shapedGlyphs.Size > 0 ? (shapedIdx < shapedGlyphs.Size) : (p < text_end))
		{
			SlugGlyphEntry* ge = NULL;
			float advance = 0;
			float glyphOffX = 0, glyphOffY = 0; // GPOS mark offsets (em-space)

			if (shapedGlyphs.Size > 0)
			{
				const ShapedGlyph& sg = shapedGlyphs[shapedIdx++];
				ImWchar glyphKey = (ImWchar)(0x100000 + sg.glyphID);
				ge = SlugFindGlyph(atlas, glyphKey);
				advance = sg.advanceX * sz;
				glyphOffX = sg.offsetX * sz;
				glyphOffY = sg.offsetY * sz;
			}
			else
			{
				unsigned int cp = 0;
				p += ImTextCharFromUtf8((unsigned int*)&cp, p, text_end);
				if (cp == 0) break;
				ge = SlugFindGlyph(atlas, (ImWchar)cp);
				if (ge) advance = ge->advanceEm * sz;
			}
			if (!ge) { penX += advance; continue; }

			// Apply GPOS offsets to pen position for this glyph
			float glyphPenX = penX + glyphOffX;
			float glyphPosY = pos.y - glyphOffY; // Y-up: positive offsetY moves glyph up

			if (ge->colorLayerCount > 0)
			{
				for (int li = 0; li < ge->colorLayerCount; li++)
				{
					const SlugColorLayer& cl = atlas->colorLayers[ge->colorLayerStart + li];
					const SlugGlyphEntry& le = atlas->glyphs[cl.glyphEntryIdx];
					if ((le.maxXEm - le.minXEm) < 1e-5f || (le.maxYEm - le.minYEm) < 1e-5f)
						continue;
					// Apply COLR v1 PaintTranslate offset
					float layerPenX = glyphPenX + cl.translateX * sz;
					float layerPosY = glyphPosY - cl.translateY * sz; // Y-up in font, Y-down in screen
					if (cl.hasGradient && gs_pContext->slugGradientShader.program)
					{
						EmitGradQuad(&le, layerPenX, layerPosY, cl);
					}
					else
					{
						ImU32 layerCol = (cl.color != 0) ? cl.color : col;
						EmitQuad(vertsColor, idxsColor, &le, layerPenX, layerPosY, layerCol);
					}
				}
			}
			else if ((ge->maxXEm - ge->minXEm) >= 1e-5f && (ge->maxYEm - ge->minYEm) >= 1e-5f)
			{
				EmitQuad(vertsNorm, idxsNorm, ge, glyphPenX, glyphPosY, col);
			}

			penX += advance;
		}

		if (vertsNorm.empty() && vertsColor.empty() && vertsGrad.empty()) return;

		// Helper: create GPU buffers, register draw callback, and destroy after draw
		auto IssueDrawCall = [&](ImVector<SlugVertex>& vBuf, ImVector<ImU16>& iBuf,
		                         ImPlatform_ShaderProgram prog)
		{
			if (vBuf.empty()) return;

			ImPlatform_VertexBufferDesc vbDesc = {};
			vbDesc.vertex_count    = (unsigned int)vBuf.Size;
			vbDesc.vertex_stride   = sizeof(SlugVertex);
			vbDesc.usage           = ImPlatform_BufferUsage_Stream;
			vbDesc.attributes      = kSlugAttribs;
			vbDesc.attribute_count = 5;

			ImPlatform_IndexBufferDesc ibDesc = {};
			ibDesc.index_count = (unsigned int)iBuf.Size;
			ibDesc.format      = ImPlatform_IndexFormat_UInt16;
			ibDesc.usage       = ImPlatform_BufferUsage_Stream;

			ImPlatform_VertexBuffer vb = ImPlatform_CreateVertexBuffer(vBuf.Data, &vbDesc);
			ImPlatform_IndexBuffer  ib = ImPlatform_CreateIndexBuffer(iBuf.Data, &ibDesc);
			if (!vb || !ib) { if (vb) ImPlatform_DestroyVertexBuffer(vb); if (ib) ImPlatform_DestroyIndexBuffer(ib); return; }

#if defined(IM_CURRENT_GFX) && (IM_CURRENT_GFX == IM_GFX_DIRECTX11)
			ImPlatform_CreateVertexInputLayout(vb, prog);
#endif
			SlugDrawCBData* cbd = (SlugDrawCBData*)IM_ALLOC(sizeof(SlugDrawCBData));
			cbd->program      = prog;
			cbd->vb           = vb;
			cbd->ib           = ib;
			cbd->atlas         = atlas;
			cbd->indexCount   = (unsigned int)iBuf.Size;

			ImPlatform_BeginCustomShader(pDrawList, prog);
			pDrawList->AddCallback(SlugRawDraw, cbd);
			pDrawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
			ImPlatform_EndCustomShader(pDrawList);
		};

		if (g_SlugDebugShader && gs_pContext->slugDebugShader.program)
		{
			IssueDrawCall(vertsNorm,  idxsNorm,  gs_pContext->slugDebugShader.program);
			IssueDrawCall(vertsColor, idxsColor, gs_pContext->slugDebugShader.program);
		}
		else
		{
			IssueDrawCall(vertsNorm,  idxsNorm,  gs_pContext->slugShader.program);
			IssueDrawCall(vertsColor, idxsColor, gs_pContext->slugColorShader.program);
		}

		// Gradient layers — separate draw call with extended vertex format (7 × float4)
		if (!vertsGrad.empty() && gs_pContext->slugGradientShader.program)
		{
			static const ImPlatform_VertexAttribute kGradAttribs[] = {
				{ ImPlatform_VertexFormat_Float4, offsetof(SlugGradientVertex, pos),  "POSITION" },
				{ ImPlatform_VertexFormat_Float4, offsetof(SlugGradientVertex, tex),  "TEXCOORD" },
				{ ImPlatform_VertexFormat_Float4, offsetof(SlugGradientVertex, jac),  "TEXCOORD" },
				{ ImPlatform_VertexFormat_Float4, offsetof(SlugGradientVertex, bnd),  "TEXCOORD" },
				{ ImPlatform_VertexFormat_Float4, offsetof(SlugGradientVertex, col),  "COLOR"    },
				{ ImPlatform_VertexFormat_Float4, offsetof(SlugGradientVertex, grd),  "TEXCOORD" },
				{ ImPlatform_VertexFormat_Float4, offsetof(SlugGradientVertex, col2), "COLOR"    },
			};
			ImPlatform_VertexBufferDesc vbDesc = {};
			vbDesc.vertex_count    = (unsigned int)vertsGrad.Size;
			vbDesc.vertex_stride   = sizeof(SlugGradientVertex);
			vbDesc.usage           = ImPlatform_BufferUsage_Stream;
			vbDesc.attributes      = kGradAttribs;
			vbDesc.attribute_count = 7;
			ImPlatform_IndexBufferDesc ibDesc = {};
			ibDesc.index_count = (unsigned int)idxsGrad.Size;
			ibDesc.format      = ImPlatform_IndexFormat_UInt16;
			ibDesc.usage       = ImPlatform_BufferUsage_Stream;
			ImPlatform_VertexBuffer vb = ImPlatform_CreateVertexBuffer(vertsGrad.Data, &vbDesc);
			ImPlatform_IndexBuffer  ib = ImPlatform_CreateIndexBuffer(idxsGrad.Data, &ibDesc);
			if (vb && ib)
			{
#if defined(IM_CURRENT_GFX) && (IM_CURRENT_GFX == IM_GFX_DIRECTX11)
				ImPlatform_CreateVertexInputLayout(vb, gs_pContext->slugGradientShader.program);
#endif
				SlugDrawCBData* cbd = (SlugDrawCBData*)IM_ALLOC(sizeof(SlugDrawCBData));
				cbd->program      = gs_pContext->slugGradientShader.program;
				cbd->vb           = vb;
				cbd->ib           = ib;
				cbd->atlas         = atlas;
				cbd->indexCount   = (unsigned int)idxsGrad.Size;
				ImPlatform_BeginCustomShader(pDrawList, gs_pContext->slugGradientShader.program);
				pDrawList->AddCallback(SlugRawDraw, cbd);
				pDrawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
				ImPlatform_EndCustomShader(pDrawList);
			}
			else { if (vb) ImPlatform_DestroyVertexBuffer(vb); if (ib) ImPlatform_DestroyIndexBuffer(ib); }
		}

#else
		// Fallback: use ImGui's built-in text rendering
		pDrawList->AddText(font, font_size, pos, col, text, text_end);
#endif
	}

	void DrawText(ImDrawList* pDrawList, ImVec2 pos, ImU32 col,
	              const char* text, const char* text_end)
	{
		DrawText(pDrawList, nullptr, 0.0f, pos, col, text, text_end);
	}

	// ---- Linear gradient text -----------------------------------------------

	static ImU32 LerpColor(ImU32 a, ImU32 b, float t)
	{
		float u = 1.0f - t;
		int r = (int)(((a >>  0) & 0xFF) * u + ((b >>  0) & 0xFF) * t + 0.5f);
		int g = (int)(((a >>  8) & 0xFF) * u + ((b >>  8) & 0xFF) * t + 0.5f);
		int bv= (int)(((a >> 16) & 0xFF) * u + ((b >> 16) & 0xFF) * t + 0.5f);
		int al= (int)(((a >> 24) & 0xFF) * u + ((b >> 24) & 0xFF) * t + 0.5f);
		return ((ImU32)al << 24) | ((ImU32)bv << 16) | ((ImU32)g << 8) | (ImU32)r;
	}

	void DrawTextGradient(ImDrawList* pDrawList, ImFont* font, float font_size,
	                       ImVec2 pos, ImU32 col_left, ImU32 col_right,
	                       const char* text, const char* text_end)
	{
#if IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER
		if (!gs_pContext || !text) return;
		if (!text_end) text_end = text + strlen(text);
		if (text >= text_end) return;
		if (!font)             font      = ImGui::GetFont();
		if (font_size <= 0.0f) font_size = ImGui::GetFontSize();

		ImWidgetsSlugState* state = gs_pContext->slugState;
		if (!state) state = gs_pContext->slugState = IM_NEW(ImWidgetsSlugState);

		if (!gs_pContext->slugShader.program) return;

		SlugFontCache* atlas = SlugGetOrCreateAtlas(state, font);
		if (!atlas) return;

		// Build glyphs
		bool anyNew = false;
		const char* p = text;
		while (p < text_end) {
			unsigned int cp = 0;
			p += ImTextCharFromUtf8((unsigned int*)&cp, p, text_end);
			if (cp == 0) break;
			if (!SlugFindGlyph(atlas, (ImWchar)cp)) {
				SlugGlyphEntry e; SlugBuildGlyph(atlas, (ImWchar)cp, &e); anyNew = true;
			}
		}
		if (anyNew || atlas->dirty) SlugUploadTextures(atlas);
		if (!atlas->curveTexture || !atlas->bandTexture) return;

		// Measure total width for gradient interpolation
		float totalWidth = CalcTextSize(font, font_size, text, text_end).x;
		if (totalWidth < 1e-5f) totalWidth = 1.0f;

		const float sz    = font_size;
		const float invSz = 1.0f / sz;
		float penX = pos.x;
		float startX = pos.x;

		static const ImPlatform_VertexAttribute kSlugAttribs[] = {
			{ ImPlatform_VertexFormat_Float4, offsetof(SlugVertex, pos), "POSITION" },
			{ ImPlatform_VertexFormat_Float4, offsetof(SlugVertex, tex), "TEXCOORD" },
			{ ImPlatform_VertexFormat_Float4, offsetof(SlugVertex, jac), "TEXCOORD" },
			{ ImPlatform_VertexFormat_Float4, offsetof(SlugVertex, bnd), "TEXCOORD" },
			{ ImPlatform_VertexFormat_Float4, offsetof(SlugVertex, col), "COLOR"    },
		};

		auto U32toF4 = [](ImU32 c, float* r, float* g, float* b, float* a) {
			*r = (float)((c >>  0) & 0xFF) / 255.0f;
			*g = (float)((c >>  8) & 0xFF) / 255.0f;
			*b = (float)((c >> 16) & 0xFF) / 255.0f;
			*a = (float)((c >> 24) & 0xFF) / 255.0f;
		};

		ImVector<SlugVertex> verts;
		ImVector<ImU16>      idxs;

		// Gradient EmitQuad: left vertices get colL, right vertices get colR
		auto EmitQuadGrad = [&](ImVector<SlugVertex>& vBuf, ImVector<ImU16>& iBuf,
		                        const SlugGlyphEntry* ge, float penX_, ImU32 colL, ImU32 colR)
		{
			float sL = penX_ + ge->minXEm * sz;
			float sR = penX_ + ge->maxXEm * sz;
			float sT = pos.y - ge->maxYEm * sz;
			float sB = pos.y - ge->minYEm * sz;
			float uL = ge->minXEm, uR = ge->maxXEm;
			float uT = ge->maxYEm, uB = ge->minYEm;

			unsigned int gz = (unsigned int)(ImU16)ge->bandTexX
			                | ((unsigned int)(ImU16)ge->bandTexY << 16);
			unsigned int gw = (unsigned int)(ge->bandMaxX & 0xFF)
			                | ((unsigned int)(ge->bandMaxY & 0xFF) << 16);
			float fgz, fgw;
			memcpy(&fgz, &gz, 4);
			memcpy(&fgw, &gw, 4);

			float cLR, cLG, cLB, cLA, cRR, cRG, cRB, cRA;
			U32toF4(colL, &cLR, &cLG, &cLB, &cLA);
			U32toF4(colR, &cRR, &cRG, &cRB, &cRA);

			ImU16 base = (ImU16)vBuf.Size;
			SlugVertex v;
			v.tex[2] = fgz; v.tex[3] = fgw;
			v.jac[0] = invSz; v.jac[1] = 0.0f;
			v.jac[2] = 0.0f;  v.jac[3] = -invSz;
			v.bnd[0] = ge->bandScaleX;  v.bnd[1] = ge->bandScaleY;
			v.bnd[2] = ge->bandOffsetX; v.bnd[3] = ge->bandOffsetY;

			// TL (left color)
			v.pos[0]=sL; v.pos[1]=sT; v.pos[2]=-1; v.pos[3]=-1; v.tex[0]=uL; v.tex[1]=uT;
			v.col[0]=cLR; v.col[1]=cLG; v.col[2]=cLB; v.col[3]=cLA; vBuf.push_back(v);
			// TR (right color)
			v.pos[0]=sR; v.pos[1]=sT; v.pos[2]=+1; v.pos[3]=-1; v.tex[0]=uR; v.tex[1]=uT;
			v.col[0]=cRR; v.col[1]=cRG; v.col[2]=cRB; v.col[3]=cRA; vBuf.push_back(v);
			// BR (right color)
			v.pos[0]=sR; v.pos[1]=sB; v.pos[2]=+1; v.pos[3]=+1; v.tex[0]=uR; v.tex[1]=uB;
			v.col[0]=cRR; v.col[1]=cRG; v.col[2]=cRB; v.col[3]=cRA; vBuf.push_back(v);
			// BL (left color)
			v.pos[0]=sL; v.pos[1]=sB; v.pos[2]=-1; v.pos[3]=+1; v.tex[0]=uL; v.tex[1]=uB;
			v.col[0]=cLR; v.col[1]=cLG; v.col[2]=cLB; v.col[3]=cLA; vBuf.push_back(v);

			iBuf.push_back(base+0); iBuf.push_back(base+1); iBuf.push_back(base+2);
			iBuf.push_back(base+0); iBuf.push_back(base+2); iBuf.push_back(base+3);
		};

		p = text;
		while (p < text_end) {
			unsigned int cp = 0;
			p += ImTextCharFromUtf8((unsigned int*)&cp, p, text_end);
			if (cp == 0) break;
			SlugGlyphEntry* ge = SlugFindGlyph(atlas, (ImWchar)cp);
			if (!ge) continue;
			float advance = ge->advanceEm * sz;

			// Gradient t at left and right edges of this glyph
			float tL = (penX + ge->minXEm * sz - startX) / totalWidth;
			float tR = (penX + ge->maxXEm * sz - startX) / totalWidth;
			tL = ImClamp(tL, 0.0f, 1.0f);
			tR = ImClamp(tR, 0.0f, 1.0f);
			ImU32 colL = LerpColor(col_left, col_right, tL);
			ImU32 colR = LerpColor(col_left, col_right, tR);

			if (ge->colorLayerCount > 0) {
				for (int li = 0; li < ge->colorLayerCount; li++) {
					const SlugColorLayer& cl = atlas->colorLayers[ge->colorLayerStart + li];
					const SlugGlyphEntry& le = atlas->glyphs[cl.glyphEntryIdx];
					if ((le.maxXEm - le.minXEm) < 1e-5f || (le.maxYEm - le.minYEm) < 1e-5f) continue;
					// For color layers, modulate the layer color with the gradient
					ImU32 layerCol = (cl.color != 0) ? cl.color : IM_COL32(255,255,255,255);
					// Blend gradient with layer color (multiply RGB, keep layer alpha)
					auto ModColor = [](ImU32 layer, ImU32 grad) -> ImU32 {
						int r = (((layer>> 0)&0xFF) * ((grad>> 0)&0xFF) + 127) / 255;
						int g = (((layer>> 8)&0xFF) * ((grad>> 8)&0xFF) + 127) / 255;
						int b = (((layer>>16)&0xFF) * ((grad>>16)&0xFF) + 127) / 255;
						int a = ((layer>>24)&0xFF);
						return ((ImU32)a<<24)|((ImU32)b<<16)|((ImU32)g<<8)|(ImU32)r;
					};
					EmitQuadGrad(verts, idxs, &le, penX, ModColor(layerCol, colL), ModColor(layerCol, colR));
				}
			} else if ((ge->maxXEm - ge->minXEm) >= 1e-5f && (ge->maxYEm - ge->minYEm) >= 1e-5f) {
				EmitQuadGrad(verts, idxs, ge, penX, colL, colR);
			}
			penX += advance;
		}

		if (verts.empty()) return;

		auto IssueDrawCall = [&](ImVector<SlugVertex>& vBuf, ImVector<ImU16>& iBuf,
		                         ImPlatform_ShaderProgram prog)
		{
			if (vBuf.empty()) return;
			ImPlatform_VertexBufferDesc vbDesc = {};
			vbDesc.vertex_count  = (unsigned int)vBuf.Size;
			vbDesc.vertex_stride = sizeof(SlugVertex);
			vbDesc.usage         = ImPlatform_BufferUsage_Stream;
			vbDesc.attributes    = kSlugAttribs;
			vbDesc.attribute_count = 5;
			ImPlatform_IndexBufferDesc ibDesc = {};
			ibDesc.index_count = (unsigned int)iBuf.Size;
			ibDesc.format      = ImPlatform_IndexFormat_UInt16;
			ibDesc.usage       = ImPlatform_BufferUsage_Stream;
			ImPlatform_VertexBuffer vb = ImPlatform_CreateVertexBuffer(vBuf.Data, &vbDesc);
			ImPlatform_IndexBuffer  ib = ImPlatform_CreateIndexBuffer(iBuf.Data, &ibDesc);
			SlugDrawCBData* cbd = (SlugDrawCBData*)IM_ALLOC(sizeof(SlugDrawCBData));
			cbd->program      = prog;
			cbd->vb           = vb;
			cbd->ib           = ib;
			cbd->atlas         = atlas;
			cbd->indexCount   = (unsigned int)iBuf.Size;
			ImPlatform_BeginCustomShader(pDrawList, prog);
			pDrawList->AddCallback(SlugRawDraw, cbd);
			pDrawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
			ImPlatform_EndCustomShader(pDrawList);
		};

		// All gradient glyphs use the color shader (preserves RGB, feathers alpha)
		IssueDrawCall(verts, idxs, gs_pContext->slugColorShader.program);
#else
		(void)pDrawList; (void)font; (void)font_size; (void)pos;
		(void)col_left; (void)col_right; (void)text; (void)text_end;
#endif
	}

	// ---- Debug visualization: draw Slug curve outlines ----------------------

	// Evaluate a quadratic Bezier at parameter t
	static ImVec2 EvalQuad(float p1x, float p1y, float p2x, float p2y, float p3x, float p3y, float t)
	{
		float u = 1.0f - t;
		return ImVec2(u*u*p1x + 2*u*t*p2x + t*t*p3x,
		              u*u*p1y + 2*u*t*p2y + t*t*p3y);
	}

	// Helper: draw debug curves/ctrl/bbox/bands for a single glyph's curve list
	static void SlugDebugDrawCurves(ImDrawList* dl, const ImVector<SlugCurve>& curves,
	    float penX, float posY, float sz, float ttx, float tty,
	    ImU32 curveCol, bool showCurves, bool showCtrl, const ImVec4& clip, int* budget)
	{
		const int SEGS = 16;
		for (int ci = 0; ci < curves.Size; ci++) {
			if (budget && *budget <= 0) return;
			const SlugCurve& cv = curves[ci];
			// Quick per-curve clip: compute screen-space bbox of control points
			float cxMin = cv.p1x, cxMax = cv.p1x, cyMin = cv.p1y, cyMax = cv.p1y;
			cxMin = ImMin(cxMin, cv.p2x); cxMax = ImMax(cxMax, cv.p2x);
			cyMin = ImMin(cyMin, cv.p2y); cyMax = ImMax(cyMax, cv.p2y);
			cxMin = ImMin(cxMin, cv.p3x); cxMax = ImMax(cxMax, cv.p3x);
			cyMin = ImMin(cyMin, cv.p3y); cyMax = ImMax(cyMax, cv.p3y);
			float sxMin = penX + (cxMin + ttx) * sz, sxMax = penX + (cxMax + ttx) * sz;
			float syMin = posY - (cyMax + tty) * sz, syMax = posY - (cyMin + tty) * sz;
			if (sxMax < clip.x || sxMin > clip.z || syMax < clip.y || syMin > clip.w) continue;

			if (showCurves) {
				ImVec2 prev(penX + (cv.p1x + ttx) * sz, posY - (cv.p1y + tty) * sz);
				for (int s = 1; s <= SEGS; s++) {
					float t = (float)s / SEGS;
					ImVec2 pt = EvalQuad(cv.p1x, cv.p1y, cv.p2x, cv.p2y, cv.p3x, cv.p3y, t);
					pt = ImVec2(penX + (pt.x + ttx) * sz, posY - (pt.y + tty) * sz);
					dl->AddLine(prev, pt, curveCol, 1.5f);
					prev = pt;
				}
			}
			if (showCtrl) {
				ImU32 ctrlCol = IM_COL32(255, 255, 255, 180);
				ImVec2 sp1(penX+(cv.p1x+ttx)*sz, posY-(cv.p1y+tty)*sz);
				ImVec2 sp2(penX+(cv.p2x+ttx)*sz, posY-(cv.p2y+tty)*sz);
				ImVec2 sp3(penX+(cv.p3x+ttx)*sz, posY-(cv.p3y+tty)*sz);
				dl->AddCircleFilled(sp1, 2.0f, curveCol);
				dl->AddCircleFilled(sp3, 2.0f, curveCol);
				dl->AddLine(sp1, sp2, ctrlCol, 1.0f);
				dl->AddCircleFilled(sp2, 1.5f, ctrlCol);
			}
			if (budget) (*budget)--;
		}
	}

	// Helper: extract curves for a glyph by glyph index from stbtt
	static void SlugExtractGlyphCurves(SlugFontCache* atlas, int gi, ImVector<SlugCurve>& curves)
	{
		stbtt_vertex* verts = NULL;
		int nVerts = stbtt_GetGlyphShape(&atlas->stbFont, gi, &verts);
		const float sc = atlas->emScale;
		float curX = 0, curY = 0;
		for (int i = 0; i < nVerts; i++) {
			const stbtt_vertex& v = verts[i];
			float vx = v.x*sc, vy = v.y*sc;
			if (v.type == STBTT_vmove) { curX=vx; curY=vy; continue; }
			SlugCurve c = {};
			c.p1x=curX; c.p1y=curY;
			if (v.type==STBTT_vline) { if(curX==vx&&curY==vy) continue; c.p2x=(curX+vx)*0.5f; c.p2y=(curY+vy)*0.5f; c.p3x=vx; c.p3y=vy; }
			else if (v.type==STBTT_vcurve) { c.p2x=v.cx*sc; c.p2y=v.cy*sc; c.p3x=vx; c.p3y=vy; }
			else if (v.type==STBTT_vcubic) { SlugCubicToQuads(curves, curX, curY, v.cx*sc, v.cy*sc, v.cx1*sc, v.cy1*sc, vx, vy); curX=vx; curY=vy; continue; }
			curves.push_back(c); curX=vx; curY=vy;
		}
		STBTT_free(verts, atlas->stbFont.userdata);
	}

	void DrawTextDebugCurves(ImDrawList* pDrawList, ImFont* font, float font_size,
	                          ImVec2 pos, const char* text, const char* text_end, int flags)
	{
		if (!gs_pContext || !text) return;
		if (!text_end) text_end = text + strlen(text);
		if (text >= text_end) return;
		if (!font)             font      = ImGui::GetFont();
		if (font_size <= 0.0f) font_size = ImGui::GetFontSize();

		ImWidgetsSlugState* state = gs_pContext->slugState;
		if (!state) return;
		SlugFontCache* atlas = SlugGetOrCreateAtlas(state, font);
		if (!atlas) return;

		const float sz = font_size;
		float penX = pos.x;

		ImVec4 clip;
		{
			ImVec2 wPos = ImGui::GetWindowPos();
			ImVec2 wSize = ImGui::GetWindowSize();
			clip = ImVec4(wPos.x, wPos.y, wPos.x + wSize.x, wPos.y + wSize.y);
		}

		int curveBudget = 2000;
		static const ImU32 kPalette[] = {
			IM_COL32(255, 80, 80, 255), IM_COL32(80, 200, 80, 255), IM_COL32(80, 120, 255, 255),
			IM_COL32(255, 200, 40, 255), IM_COL32(200, 80, 255, 255), IM_COL32(80, 220, 220, 255),
			IM_COL32(255, 140, 60, 255), IM_COL32(180, 255, 100, 255),
		};
		static const int kPaletteN = IM_ARRAYSIZE(kPalette);

		const bool showCurves = (flags & 1) != 0;
		const bool showCtrl   = (flags & 2) != 0;
		const bool showBBox   = (flags & 4) != 0;
		const bool showBands  = (flags & 8) != 0;

		// Use the same shaping path as DrawText
		struct DbgGlyph { int glyphID; float advanceX; float offsetX; float offsetY; };
		ImVector<DbgGlyph> shapedGlyphs;
#if IM_SUPPORT_LIGATURE
		if (atlas->shapeCtx && atlas->shapeFont)
		{
			int textLen = (int)(text_end - text);
			kbts_ShapeBegin(atlas->shapeCtx, KBTS_DIRECTION_DONT_KNOW, KBTS_LANGUAGE_DONT_KNOW);
			kbts_ShapeUtf8(atlas->shapeCtx, text, textLen, KBTS_USER_ID_GENERATION_MODE_CODEPOINT_INDEX);
			kbts_ShapeEnd(atlas->shapeCtx);
			kbts_run run;
			while (kbts_ShapeRun(atlas->shapeCtx, &run)) {
				kbts_glyph* glyph;
				while (kbts_GlyphIteratorNext(&run.Glyphs, &glyph)) {
					DbgGlyph dg;
					dg.glyphID  = (int)glyph->Id;
					dg.advanceX = (float)glyph->AdvanceX * atlas->emScale;
					dg.offsetX  = (float)glyph->OffsetX  * atlas->emScale;
					dg.offsetY  = (float)glyph->OffsetY  * atlas->emScale;
					shapedGlyphs.push_back(dg);
				}
			}
		}
#endif

		int shapedIdx = 0;
		const char* p = text;
		int glyphColorIdx = 0;
		while (shapedGlyphs.Size > 0 ? (shapedIdx < shapedGlyphs.Size) : (p < text_end))
		{
			int    gi = 0;
			float  advance = 0, offX = 0, offY = 0;

			if (shapedGlyphs.Size > 0) {
				const DbgGlyph& dg = shapedGlyphs[shapedIdx++];
				gi      = dg.glyphID;
				advance = dg.advanceX * sz;
				offX    = dg.offsetX * sz;
				offY    = dg.offsetY * sz;
			} else {
				unsigned int cp = 0;
				p += ImTextCharFromUtf8((unsigned int*)&cp, p, text_end);
				if (cp == 0) break;
				gi = stbtt_FindGlyphIndex(&atlas->stbFont, (int)cp);
				if (gi <= 0) continue; // match DrawText: missing glyphs skip with advance=0
				SlugGlyphEntry* dge = SlugFindGlyph(atlas, (ImWchar)cp);
				advance = dge ? dge->advanceEm * sz : 0;
			}

			float gPenX = penX + offX;
			float gPosY = pos.y - offY;

			auto Em2Scr = [&](float ex, float ey) -> ImVec2 {
				return ImVec2(gPenX + ex * sz, gPosY - ey * sz);
			};

			// Extract curves for this glyph
			if ((showCurves || showCtrl) && curveBudget > 0) {
				ImVector<SlugCurve> curves;
				SlugExtractGlyphCurves(atlas, gi, curves);
				ImU32 curveCol = kPalette[glyphColorIdx % kPaletteN];
				SlugDebugDrawCurves(pDrawList, curves, gPenX, gPosY, sz, 0, 0, curveCol, showCurves, showCtrl, clip, &curveBudget);
			}

			// BBox
			if (showBBox) {
				int bx0, by0, bx1, by1;
				if (stbtt_GetGlyphBox(&atlas->stbFont, gi, &bx0, &by0, &bx1, &by1)) {
					float sc = atlas->emScale;
					pDrawList->AddRect(Em2Scr(bx0*sc, by0*sc), Em2Scr(bx1*sc, by1*sc),
						kPalette[glyphColorIdx % kPaletteN], 0.0f, 0, 1.0f);
				}
			}

			glyphColorIdx++;
			penX += advance;
			if (penX > clip.z) break;
		}
	}

	void DrawTextDebugLayers(ImDrawList* pDrawList, ImFont* font, float font_size,
	                         ImVec2 pos, const char* text, const char* text_end)
	{
		if (!gs_pContext || !text) return;
		if (!text_end) text_end = text + strlen(text);
		if (text >= text_end) return;
		if (!font)             font      = ImGui::GetFont();
		if (font_size <= 0.0f) font_size = ImGui::GetFontSize();

		ImWidgetsSlugState* state = gs_pContext->slugState;
		if (!state) return;
		SlugFontCache* atlas = SlugGetOrCreateAtlas(state, font);
		if (!atlas) return;

		const float sz = font_size;
		float penX = pos.x;

		const char* p = text;
		while (p < text_end)
		{
			unsigned int cp = 0;
			p += ImTextCharFromUtf8((unsigned int*)&cp, p, text_end);
			if (cp == 0) break;

			SlugGlyphEntry* ge = SlugFindGlyph(atlas, (ImWchar)cp);
			if (!ge) continue;
			float advance = ge->advanceEm * sz;

			if (ge->colorLayerCount > 0)
			{
				for (int li = 0; li < ge->colorLayerCount; li++)
				{
					const SlugColorLayer& cl = atlas->colorLayers[ge->colorLayerStart + li];
					const SlugGlyphEntry& le = atlas->glyphs[cl.glyphEntryIdx];
					if ((le.maxXEm - le.minXEm) < 1e-5f || (le.maxYEm - le.minYEm) < 1e-5f)
						continue;
					// Layer quad in screen-space
					float sL = penX + le.minXEm * sz;
					float sR = penX + le.maxXEm * sz;
					float sT = pos.y - le.maxYEm * sz;
					float sB = pos.y - le.minYEm * sz;
					// Semi-transparent fill with the layer color
					ImU32 c = cl.color;
					ImU32 fillCol = (c & 0x00FFFFFF) | 0x60000000; // 37% alpha
					ImU32 lineCol = (c & 0x00FFFFFF) | 0xC0000000; // 75% alpha
					pDrawList->AddRectFilled(ImVec2(sL, sT), ImVec2(sR, sB), fillCol);
					pDrawList->AddRect(ImVec2(sL, sT), ImVec2(sR, sB), lineCol, 0.0f, 0, 1.0f);
					// Layer index label
					char lbl[8]; ImFormatString(lbl, 8, "%d", li);
					pDrawList->AddText(ImVec2(sL + 1, sT + 1), IM_COL32(255,255,255,200), lbl);
				}
			}
			else if ((ge->maxXEm - ge->minXEm) >= 1e-5f)
			{
				float sL = penX + ge->minXEm * sz;
				float sR = penX + ge->maxXEm * sz;
				float sT = pos.y - ge->maxYEm * sz;
				float sB = pos.y - ge->minYEm * sz;
				pDrawList->AddRect(ImVec2(sL, sT), ImVec2(sR, sB), IM_COL32(200,200,200,120), 0.0f, 0, 1.0f);
			}

			penX += advance;
		}
	}

#endif // IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER

	// ---- ImFontLoader: Slug atlas baking ------------------------------------
	// CPU-rasterizes font glyphs into ImGui's bitmap atlas.
	//
	// Color support:
	//   COLR v0    — full color (per-layer compositing into RGBA32 atlas)
	//   COLR v1    — flat color only (uses first gradient stop; GPU gradients not available in atlas)
	//   SVG        — monochrome fallback (SVG paths require a CPU vector rasterizer we don't have;
	//                would need NanoSVG, LunaSVG, or similar to render SVG path fills to bitmap)
	//   Monochrome — full (Alpha8 atlas, tintable via vertex color)
	//
	// Other limitations:
	//   - No ligature/contextual shaping (atlas is per-codepoint; use GPU Slug path for shaped text)
	//   - Fixed resolution (rasterized at baked size; GPU Slug is resolution-independent)
	//   - No kerning (ImGui's AddText doesn't apply kern pairs)

	struct SlugLoaderFontData
	{
		stbtt_fontinfo fontInfo;
		float          scaleFactor;  // 1.0 / unitsPerEm
		uint32_t       colrOff, cpalOff, svgOff;
	};

	static bool SlugLoader_FontSrcInit(ImFontAtlas* atlas, ImFontConfig* src)
	{
		(void)atlas;
		SlugLoaderFontData* fd = IM_NEW(SlugLoaderFontData);
		int offset = stbtt_GetFontOffsetForIndex((unsigned char*)src->FontData, 0);
		if (!stbtt_InitFont(&fd->fontInfo, (unsigned char*)src->FontData, offset))
		{ IM_DELETE(fd); return false; }
		fd->scaleFactor = stbtt_ScaleForMappingEmToPixels(&fd->fontInfo, 1.0f);
		const uint8_t* raw = (const uint8_t*)fd->fontInfo.data;
		uint32_t fs = (uint32_t)fd->fontInfo.fontstart;
		fd->colrOff = SlugFindTable(raw, fs, "COLR");
		fd->cpalOff = SlugFindTable(raw, fs, "CPAL");
		fd->svgOff  = SlugFindTable(raw, fs, "SVG ");
		src->FontLoaderData = fd;
		return true;
	}

	static void SlugLoader_FontSrcDestroy(ImFontAtlas* atlas, ImFontConfig* src)
	{
		(void)atlas;
		if (src->FontLoaderData) { IM_DELETE((SlugLoaderFontData*)src->FontLoaderData); src->FontLoaderData = NULL; }
	}

	static bool SlugLoader_FontSrcContainsGlyph(ImFontAtlas* atlas, ImFontConfig* src, ImWchar codepoint)
	{
		(void)atlas;
		SlugLoaderFontData* fd = (SlugLoaderFontData*)src->FontLoaderData;
		return stbtt_FindGlyphIndex(&fd->fontInfo, (int)codepoint) != 0;
	}

	static bool SlugLoader_FontBakedInit(ImFontAtlas* atlas, ImFontConfig* src, ImFontBaked* baked, void*)
	{
		(void)atlas;
		SlugLoaderFontData* fd = (SlugLoaderFontData*)src->FontLoaderData;
		if (src->MergeMode == false)
		{
			float scale = fd->scaleFactor * baked->Size;
			int ascent, descent, lineGap;
			stbtt_GetFontVMetrics(&fd->fontInfo, &ascent, &descent, &lineGap);
			baked->Ascent  = ImCeil(ascent * scale);
			baked->Descent = ImFloor(descent * scale);
		}
		return true;
	}

	static void SlugLoader_FontBakedDestroy(ImFontAtlas*, ImFontConfig*, ImFontBaked*, void*) {}

	// Helper: composite a layer's alpha mask with a color onto an RGBA32 buffer (SrcOver blending)
	static void SlugLoader_CompositeLayer(unsigned char* rgba, int w, int h,
	    const unsigned char* layerAlpha, ImU32 col)
	{
		float cR = (float)((col >>  0) & 0xFF) / 255.0f;
		float cG = (float)((col >>  8) & 0xFF) / 255.0f;
		float cB = (float)((col >> 16) & 0xFF) / 255.0f;
		float cA = (float)((col >> 24) & 0xFF) / 255.0f;
		for (int p = 0; p < w * h; p++) {
			float srcA = (layerAlpha[p] / 255.0f) * cA;
			if (srcA < 1.0f/255.0f) continue;
			float dstA = rgba[p*4+3] / 255.0f;
			float outA = srcA + dstA * (1.0f - srcA);
			float inv = outA > 0 ? 1.0f / outA : 1.0f;
			rgba[p*4+0] = (unsigned char)(ImClamp((cR * srcA + (rgba[p*4+0]/255.0f) * dstA * (1.0f - srcA)) * inv, 0.0f, 1.0f) * 255.0f + 0.5f);
			rgba[p*4+1] = (unsigned char)(ImClamp((cG * srcA + (rgba[p*4+1]/255.0f) * dstA * (1.0f - srcA)) * inv, 0.0f, 1.0f) * 255.0f + 0.5f);
			rgba[p*4+2] = (unsigned char)(ImClamp((cB * srcA + (rgba[p*4+2]/255.0f) * dstA * (1.0f - srcA)) * inv, 0.0f, 1.0f) * 255.0f + 0.5f);
			rgba[p*4+3] = (unsigned char)(ImClamp(outA, 0.0f, 1.0f) * 255.0f + 0.5f);
		}
	}

	// Helper: rasterize a layer glyph into a full-size alpha buffer at the correct offset
	static void SlugLoader_RasterizeLayer(stbtt_fontinfo* fi, int layerGI, float scale,
	    unsigned char* outAlpha, int w, int h, int bx0, int by0)
	{
		memset(outAlpha, 0, w * h);
		int lx0, ly0, lx1, ly1;
		stbtt_GetGlyphBitmapBox(fi, layerGI, scale, scale, &lx0, &ly0, &lx1, &ly1);
		int lw = lx1 - lx0, lh = ly1 - ly0;
		if (lw <= 0 || lh <= 0) return;
		ImVector<unsigned char> tmp; tmp.resize(lw * lh, 0);
		stbtt_MakeGlyphBitmap(fi, tmp.Data, lw, lh, lw, scale, scale, layerGI);
		int offX = lx0 - bx0, offY = ly0 - by0;
		for (int y = 0; y < lh; y++)
			for (int x = 0; x < lw; x++)
				if (offY + y >= 0 && offY + y < h && offX + x >= 0 && offX + x < w)
					outAlpha[(offY + y) * w + (offX + x)] = tmp[y * lw + x];
	}

	// Get color layers for a glyph from any color table (COLR v0, COLR v1, SVG).
	// Returns layer count. Fills outGlyphIDs (glyph indices) and outColors (RGBA).
	static int SlugLoader_GetColorLayers(SlugLoaderFontData* fd, int glyphID, ImWchar codepoint,
	    int* outGlyphIDs, ImU32* outColors, int maxLayers)
	{
		const uint8_t* data = (const uint8_t*)fd->fontInfo.data;

		// --- COLR v0 ---
		if (fd->colrOff && fd->cpalOff) {
			const uint8_t* colr = data + fd->colrOff;
			uint16_t numBase = SlugTTU16(colr + 2);
			if (numBase > 0) {
				uint32_t baseOff = SlugTTU32(colr + 4);
				uint32_t layerOff = SlugTTU32(colr + 8);
				const uint8_t* bases = colr + baseOff;
				const uint8_t* layers = colr + layerOff;
				int lo = 0, hi = (int)numBase - 1;
				while (lo <= hi) {
					int mid = (lo + hi) / 2;
					int gid = (int)SlugTTU16(bases + mid * 6);
					if (gid == glyphID) {
						int firstLayer = (int)SlugTTU16(bases + mid * 6 + 2);
						int numLayers  = (int)SlugTTU16(bases + mid * 6 + 4);
						if (numLayers > maxLayers) numLayers = maxLayers;
						const uint8_t* cpal = data + fd->cpalOff;
						uint32_t colorOff = SlugTTU32(cpal + 8);
						for (int i = 0; i < numLayers; i++) {
							outGlyphIDs[i] = (int)SlugTTU16(layers + (firstLayer + i) * 4);
							int palIdx = (int)SlugTTU16(layers + (firstLayer + i) * 4 + 2);
							if (palIdx == 0xFFFF) outColors[i] = IM_COL32(0,0,0,0);
							else { const uint8_t* cr = cpal + colorOff + palIdx * 4; outColors[i] = IM_COL32(cr[2], cr[1], cr[0], cr[3]); }
						}
						return numLayers;
					}
					else if (gid < glyphID) lo = mid + 1;
					else hi = mid - 1;
				}
			}
		}

		// --- COLR v1 (via the existing SlugGetColrV1Layers — requires a temporary SlugFontCache) ---
		if (fd->colrOff && fd->cpalOff) {
			const uint8_t* colr = data + fd->colrOff;
			if (SlugTTU16(colr) >= 1) {
				// Build a temporary SlugFontCache just for the v1 layer query
				SlugFontCache tmpCache = {};
				tmpCache.stbFont = fd->fontInfo;
				tmpCache.emScale = fd->scaleFactor;
				tmpCache.colrTableOffset = fd->colrOff;
				tmpCache.cpalTableOffset = fd->cpalOff;
				ImVector<int> v1GlyphIDs;
				ImVector<SlugColorLayer> v1Layers;
				int n = SlugGetColrV1Layers(&tmpCache, glyphID, v1GlyphIDs, v1Layers);
				if (n > 0) {
					if (n > maxLayers) n = maxLayers;
					for (int i = 0; i < n; i++) {
						outGlyphIDs[i] = v1GlyphIDs[i];
						outColors[i] = v1Layers[i].color; // solid fallback color (first gradient stop for gradients)
					}
					return n;
				}
			}
		}

		// --- SVG (parse paths, get colors — but we can only composite outlines, not SVG vector fills) ---
		// For SVG fonts, the atlas loader rasterizes the base glyph outline in monochrome for each
		// color path. This gives correct shapes but gradients/effects are lost.
		if (fd->svgOff) {
			const uint8_t* svg = data + fd->svgOff;
			uint32_t dlRel = SlugTTU32(svg + 2);
			const uint8_t* dl = svg + dlRel;
			uint16_t numEntries = SlugTTU16(dl);
			for (int i = 0; i < (int)numEntries; i++) {
				const uint8_t* rec = dl + 2 + i * 12;
				uint16_t sg = SlugTTU16(rec), eg = SlugTTU16(rec + 2);
				if (glyphID >= sg && glyphID <= eg) {
					// SVG data exists for this glyph — render as monochrome with the base glyph outline
					// (full SVG vector rasterization would require a CPU SVG renderer)
					// Use a single layer with white color so the glyph is tintable
					outGlyphIDs[0] = glyphID;
					outColors[0] = IM_COL32(255, 255, 255, 255);
					return 0; // return 0 to fall through to monochrome (SVG shapes need vector rasterization)
				}
			}
		}

		return 0;
	}

	static bool SlugLoader_FontBakedLoadGlyph(ImFontAtlas* atlas, ImFontConfig* src, ImFontBaked* baked, void*,
	                                            ImWchar codepoint, ImFontGlyph* out_glyph, float* out_advance_x)
	{
		SlugLoaderFontData* fd = (SlugLoaderFontData*)src->FontLoaderData;
		int gi = stbtt_FindGlyphIndex(&fd->fontInfo, (int)codepoint);
		if (gi == 0) return false;

		const float scale = fd->scaleFactor * baked->Size;
		int advance, lsb;
		stbtt_GetGlyphHMetrics(&fd->fontInfo, gi, &advance, &lsb);

		// Metrics-only mode
		if (out_advance_x != NULL) { *out_advance_x = advance * scale; return true; }

		out_glyph->Codepoint = codepoint;
		out_glyph->AdvanceX  = advance * scale;

		// Check for color layers (COLR v0, COLR v1, SVG)
		int layerGlyphIDs[256]; ImU32 layerColors[256];
		int numLayers = SlugLoader_GetColorLayers(fd, gi, codepoint, layerGlyphIDs, layerColors, 256);

		if (numLayers > 0)
		{
			// Color glyph: rasterize each layer, composite into RGBA32
			// Compute union bbox across all layers
			int bx0 = 0, by0 = 0, bx1 = 0, by1 = 0;
			bool first = true;
			for (int li = 0; li < numLayers; li++) {
				int lx0, ly0, lx1, ly1;
				stbtt_GetGlyphBitmapBox(&fd->fontInfo, layerGlyphIDs[li], scale, scale, &lx0, &ly0, &lx1, &ly1);
				if (first) { bx0=lx0; by0=ly0; bx1=lx1; by1=ly1; first=false; }
				else { bx0=ImMin(bx0,lx0); by0=ImMin(by0,ly0); bx1=ImMax(bx1,lx1); by1=ImMax(by1,ly1); }
			}
			int w = bx1 - bx0, h = by1 - by0;
			if (w <= 0 || h <= 0) return true;

			ImFontAtlasRectId packId = ImFontAtlasPackAddRect(atlas, w, h);
			if (packId == ImFontAtlasRectId_Invalid) return false;
			ImTextureRect* r = ImFontAtlasPackGetRect(atlas, packId);

			ImVector<unsigned char> rgba; rgba.resize(w * h * 4, 0);
			ImVector<unsigned char> layerAlpha; layerAlpha.resize(w * h, 0);

			for (int li = 0; li < numLayers; li++) {
				SlugLoader_RasterizeLayer(&fd->fontInfo, layerGlyphIDs[li], scale, layerAlpha.Data, w, h, bx0, by0);
				SlugLoader_CompositeLayer(rgba.Data, w, h, layerAlpha.Data, layerColors[li]);
			}

			out_glyph->X0 = (float)bx0;
			out_glyph->Y0 = (float)by0 + IM_ROUND(baked->Ascent);
			out_glyph->X1 = (float)bx1;
			out_glyph->Y1 = (float)by1 + IM_ROUND(baked->Ascent);
			out_glyph->Visible = true;
			out_glyph->Colored = true;
			out_glyph->PackId = packId;
			ImFontAtlasBakedSetFontGlyphBitmap(atlas, baked, src, out_glyph, r, rgba.Data, ImTextureFormat_RGBA32, w * 4);
		}
		else
		{
			// Monochrome glyph
			int bx0, by0, bx1, by1;
			stbtt_GetGlyphBitmapBox(&fd->fontInfo, gi, scale, scale, &bx0, &by0, &bx1, &by1);
			int w = bx1 - bx0, h = by1 - by0;
			if (w <= 0 || h <= 0) return true;

			ImFontAtlasRectId packId = ImFontAtlasPackAddRect(atlas, w, h);
			if (packId == ImFontAtlasRectId_Invalid) return false;
			ImTextureRect* r = ImFontAtlasPackGetRect(atlas, packId);

			ImVector<unsigned char> bitmap; bitmap.resize(w * h, 0);
			stbtt_MakeGlyphBitmap(&fd->fontInfo, bitmap.Data, w, h, w, scale, scale, gi);

			out_glyph->X0 = (float)bx0;
			out_glyph->Y0 = (float)by0 + IM_ROUND(baked->Ascent);
			out_glyph->X1 = (float)bx1;
			out_glyph->Y1 = (float)by1 + IM_ROUND(baked->Ascent);
			out_glyph->Visible = true;
			out_glyph->PackId = packId;
			ImFontAtlasBakedSetFontGlyphBitmap(atlas, baked, src, out_glyph, r, bitmap.Data, ImTextureFormat_Alpha8, w);
		}

		return true;
	}

	// Pre-build a glyph by glyph ID (not codepoint) into the Slug atlas.
	// Key = 0x100000 + glyphID, used for MATH table assembly parts.
	void SlugBuildGlyphByID(ImFont* font, int glyphID) {
		// No-op: assembly part glyphs are now built on-demand by DrawText
		// when it encounters 0x100000+ codepoints in the non-shaped path.
		// This avoids mid-frame texture uploads that could affect other fonts.
		(void)font; (void)glyphID;
	}

	// Get stb_truetype font info for MATH table parsing (used by LaTeX renderer)
	bool GetSlugFontInfo(ImFont* font, void* outStbttFontInfo, float* outEmScale) {
		stbtt_fontinfo* outInfo = (stbtt_fontinfo*)outStbttFontInfo;
		if (!gs_pContext || !gs_pContext->slugState) return false;
		SlugFontCache* atlas = SlugGetOrCreateAtlas(gs_pContext->slugState, font);
		if (!atlas) return false;
		if (outInfo) *outInfo = atlas->stbFont;
		if (outEmScale) *outEmScale = atlas->emScale;
		return true;
	}

	const ImFontLoader* GetSlugFontLoader()
	{
		static ImFontLoader loader;
		static bool inited = false;
		if (!inited) {
			loader.Name                      = "Slug";
			loader.FontSrcInit               = SlugLoader_FontSrcInit;
			loader.FontSrcDestroy            = SlugLoader_FontSrcDestroy;
			loader.FontSrcContainsGlyph      = SlugLoader_FontSrcContainsGlyph;
			loader.FontBakedInit             = SlugLoader_FontBakedInit;
			loader.FontBakedDestroy          = SlugLoader_FontBakedDestroy;
			loader.FontBakedLoadGlyph        = SlugLoader_FontBakedLoadGlyph;
			loader.FontBakedSrcLoaderDataSize = 0;
			inited = true;
		}
		return &loader;
	}

#ifndef IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER
// Stubs when custom shaders not available
#define IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER_WAS_UNDEF
#endif

#if defined(IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER_WAS_UNDEF)

	ImVec2 CalcTextSize(ImFont* font, float font_size,
	                    const char* text, const char* text_end, float* out_ascent)
	{
		if (!font)           font      = ImGui::GetFont();
		if (font_size <= 0.0f) font_size = ImGui::GetFontSize();
		ImFontBaked* baked = font->GetFontBaked(font_size);
		if (out_ascent) *out_ascent = baked ? baked->Ascent * (font_size / baked->Size) : font_size * 0.8f;
		// Delegate to ImGui for width; height from font metrics
		ImVec2 sz = font->GetFontBaked(font_size) ?
		    ImVec2(0, font_size) : ImVec2(0, font_size);
		(void)text; (void)text_end;
		return sz;
	}

	void DrawText(ImDrawList* pDrawList, ImFont* font, float font_size,
	              ImVec2 pos, ImU32 col, const char* text, const char* text_end)
	{
		if (!font)      font      = ImGui::GetFont();
		if (font_size <= 0.0f) font_size = ImGui::GetFontSize();
		if (pDrawList) pDrawList->AddText(font, font_size, pos, col, text, text_end);
	}

	void DrawText(ImDrawList* pDrawList, ImVec2 pos, ImU32 col,
	              const char* text, const char* text_end)
	{
		DrawText(pDrawList, nullptr, 0.0f, pos, col, text, text_end);
	}

	void DrawTextDebugCurves(ImDrawList*, ImFont*, float, ImVec2, const char*, const char*, int) {}
	void DrawTextDebugLayers(ImDrawList*, ImFont*, float, ImVec2, const char*, const char*) {}

#endif  // IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER


#endif // _DEAR_WIDGETS_SLUG_INCLUDED
