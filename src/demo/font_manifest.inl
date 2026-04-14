// font_manifest.inl — metadata for on-demand demo-font downloads.
//
// Generated from a research pass against Google Fonts / Fontshare /
// Velvetyne / Open Foundry (per user requirement: only these 4 sources).
// Entries with url==nullptr had no stylistic equivalent on those sources
// — the UI shows "no replacement available" for those slots.
//
// The `category` strings MUST match the kGrp* labels used in demo.cpp's
// Slug section (see ShowDrawTextDemo) so per-category download buttons
// render in the right place.
//
// `archive_inner`: when url points to a ZIP archive (Fontshare, Velvetyne,
// codeberg), this is the path-within-archive of the desired font file.
// The downloader extracts that one file and renames it to `canonical`.
// For direct .ttf/.otf URLs, archive_inner is nullptr.
//
// Canonical filenames mirror the download artifact so the file on disk
// matches what you'd get by fetching the URL yourself.

#pragma once
#include "font_downloader.h"

namespace ImDwDownload
{
    // ---- group labels must match kGrp* in demo.cpp ----
    static const char* kCCode   = "Programming / Code";
    static const char* kCSerif  = "Serif";
    static const char* kCScript = "Script / Handwriting";
    static const char* kCDisp   = "Display / Decorative";
    static const char* kCCFF    = "CFF Monochrome";
    static const char* kCColr0  = "Color: COLR v0";
    static const char* kCSVG    = "Color: SVG";
    static const char* kCColr1  = "Color: COLR v1 / Gradient";
    static const char* kCArabic = "Arabic";

    const Meta kFontMeta[] = {
        // ---- Programming / Code ----
        { "FiraCode[wght].ttf", "Fira Code", kCCode, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/firacode/FiraCode%5Bwght%5D.ttf",
          nullptr },
        // Monblock -> Sligoil (Velvetyne zip; default branch is `main`, not master).
        { "Sligoil-Micro.otf", "Monblock -> Sligoil", kCCode, "Velvetyne",
          "https://gitlab.com/velvetyne/sligoil/-/archive/main/sligoil-main.zip",
          "sligoil-main/fonts/otf/Sligoil-Micro.otf" },

        // ---- Serif ----
        { "Cinzel[wght].ttf", "Cinzel", kCSerif, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/cinzel/Cinzel%5Bwght%5D.ttf",
          nullptr },
        { "AlfaSlabOne-Regular.ttf", "Alfa Slab One", kCSerif, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/alfaslabone/AlfaSlabOne-Regular.ttf",
          nullptr },
        { "CinzelDecorative-Regular.ttf", "Classical Aesthetics -> Cinzel Decorative", kCSerif, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/cinzeldecorative/CinzelDecorative-Regular.ttf",
          nullptr },
        { "UnifrakturCook-Bold.ttf", "Foglihten No07 -> UnifrakturCook", kCSerif, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/unifrakturcook/UnifrakturCook-Bold.ttf",
          nullptr },
        { "PlayfairDisplaySC-Regular.ttf", "Prida 61 -> Playfair Display SC", kCSerif, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/playfairdisplaysc/PlayfairDisplaySC-Regular.ttf",
          nullptr },
        { "Rye-Regular.ttf", "Steelworks Vintage -> Rye", kCSerif, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/rye/Rye-Regular.ttf",
          nullptr },
        // Trench Slab (Fontshare zip — slab serif).
        { "TrenchSlab-Regular.otf", "Trench Slab", kCSerif, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/trench-slab",
          "TrenchSlab_Complete/Fonts/OTF/TrenchSlab-Regular.otf" },

        // ---- Script / Handwriting ----
        // Script / Handwriting — chosen to span distinct traditions rather
        // than all being English-roundhand variants. Styles: pencil, sharpie,
        // felt-tip, loose marker, personal cursive, retro brush, monoline,
        // bouncy casual, dry brush, bold brush, copperplate, Italian script,
        // retro connected, retro italic brush, formal engraved, bold display
        // script, expressive display script.
        { "Caveat[wght].ttf", "Allessa -> Caveat", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/caveat/Caveat%5Bwght%5D.ttf",
          nullptr },  // pencil handwriting
        { "Sacramento-Regular.ttf", "Bright Marching -> Sacramento", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/sacramento/Sacramento-Regular.ttf",
          nullptr },  // monoline upright
        { "KaushanScript-Regular.ttf", "Camood -> Kaushan Script", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/kaushanscript/KaushanScript-Regular.ttf",
          nullptr },  // bold brush
        { "GreatVibes-Regular.ttf", "Cherona -> Great Vibes", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/greatvibes/GreatVibes-Regular.ttf",
          nullptr },  // classic English copperplate
        { "Pacifico-Regular.ttf", "Daeling -> Pacifico", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/pacifico/Pacifico-Regular.ttf",
          nullptr },  // retro 50s brush sign-painting
        { "PermanentMarker-Regular.ttf", "Flowmery -> Permanent Marker", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/apache/permanentmarker/PermanentMarker-Regular.ttf",
          nullptr },  // bold sharpie
        { "AlexBrush-Regular.ttf", "Galins -> Alex Brush", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/alexbrush/AlexBrush-Regular.ttf",
          nullptr },  // dry brush calligraphy
        { "ShadowsIntoLight.ttf", "Gallante -> Shadows Into Light", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/shadowsintolight/ShadowsIntoLight.ttf",
          nullptr },  // loose marker handwriting
        { "Yellowtail-Regular.ttf", "Kleymissky -> Yellowtail", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/apache/yellowtail/Yellowtail-Regular.ttf",
          nullptr },  // retro connected brush
        { "LoveLight-Regular.ttf", "Love Light", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/lovelight/LoveLight-Regular.ttf",
          nullptr },  // decorative / glitter script (keeper)
        { "LeckerliOne-Regular.ttf", "Metafora Alternate -> Leckerli One", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/leckerlione/LeckerliOne-Regular.ttf",
          nullptr },  // retro bold italic brush
        { "Sail-Regular.ttf", "Metafora Stylistic -> Sail", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/sail/Sail-Regular.ttf",
          nullptr },  // bold monoline display script
        { "Satisfy-Regular.ttf", "Migullon -> Satisfy", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/apache/satisfy/Satisfy-Regular.ttf",
          nullptr },  // casual friendly brush
        { "DancingScript[wght].ttf", "Milssky -> Dancing Script", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/dancingscript/DancingScript%5Bwght%5D.ttf",
          nullptr },  // bouncy casual cursive
        { "HomemadeApple-Regular.ttf", "Regina -> Homemade Apple", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/apache/homemadeapple/HomemadeApple-Regular.ttf",
          nullptr },  // personal cursive (photographed real hand)
        { "PetitFormalScript-Regular.ttf", "Retro Heart You -> Petit Formal Script", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/petitformalscript/PetitFormalScript-Regular.ttf",
          nullptr },  // retro engraved formal
        // Rosehot -> Melodrama (Fontshare zip) — expressive high-contrast script-display.
        { "Melodrama-Regular.otf", "Rosehot -> Melodrama", kCScript, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/melodrama",
          "Melodrama_Complete/Fonts/OTF/Melodrama-Regular.otf" },
        { "GochiHand-Regular.ttf", "Sophiemelanie -> Gochi Hand", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/gochihand/GochiHand-Regular.ttf",
          nullptr },  // felt-tip pen casual
        // Sharpie (Fontshare zip — marker handwriting).
        { "Sharpie-Regular.otf", "Sharpie", kCScript, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/sharpie",
          "Sharpie_Complete/Fonts/OTF/Sharpie-Regular.otf" },

        // ---- Display / Decorative — curated Velvetyne picks ----
        // Bollgo -> Flor de Ruina (baroque organic).
        { "FlorDeRuina-Flor.otf", "Bollgo -> Flor de Ruina", kCDisp, "Velvetyne",
          "https://gitlab.com/velvetyne/flor-des-ruina/-/archive/main/flor-des-ruina-main.zip",
          "flor-des-ruina-main/fonts/otf/FlorDeRuina-Flor.otf" },
        // Boucher -> Amdal (bold Tifinagh display).
        { "AMDAL-Regular.otf", "Boucher -> Amdal", kCDisp, "Velvetyne",
          "https://gitlab.com/velvetyne/amdal/-/archive/master/amdal-master.zip",
          "amdal-master/fonts/AMDAL-Regular.otf" },
        // Dotted -> Bianzhidai (plastic-weave pixel art — concept match for Dotted).
        { "Bianzhidai-NoBG-Base.otf", "Dotted -> Bianzhidai", kCDisp, "Velvetyne",
          "https://github.com/sdfggvfvj/bianzhidai-2.0/archive/main.zip",
          "bianzhidai-2.0-main/fonts/bianzhidai_noBG/OTF/bianzhidai_noBG-Base.otf" },
        // Frantically -> Mess (chaotic — concept match for Frantically).
        { "Mess.otf", "Frantically -> Mess", kCDisp, "Velvetyne",
          "https://gitlab.com/velvetyne/Mess/-/archive/master/Mess-master.zip",
          "Mess-master/fonts/Mess.otf" },
        // Gimbo -> Pilowlava (bulbous molten display — Velvetyne / StudioTriple).
        { "Pilowlava-Regular.otf", "Gimbo -> Pilowlava", kCDisp, "Velvetyne",
          "https://gitlab.com/StudioTriple/pilowlava/-/archive/master/pilowlava-master.zip",
          "pilowlava-master/Fonts/Pilowlava-Regular.otf" },
        // Ginga -> Interlope (interlocking geometric).
        { "Interlope-Regular.otf", "Ginga -> Interlope", kCDisp, "Velvetyne",
          "https://gitlab.com/velvetyne/interlope/-/archive/main/interlope-main.zip",
          "interlope-main/font/otf/Interlope-Regular.otf" },
        // Magnolia -> Letters (abstract letterforms — concept fit for line monogram).
        { "Letters-Torn.otf", "Magnolia -> Letters", kCDisp, "Velvetyne",
          "https://gitlab.com/velvetyne/letters/-/archive/main/letters-main.zip",
          "letters-main/fonts/Letters-Torn.otf" },
        // Molgeth -> Fungal (growing organic display, Grow=400 Thickness=500 static instance).
        { "Fungal-Grow400Thickness500.ttf", "Molgeth -> Fungal (Grow400/Thk500)", kCDisp, "Velvetyne",
          "https://gitlab.com/velvetyne/fungal/-/archive/main/fungal-main.zip",
          "fungal-main/fonts/ttf/Fungal-Grow400Thickness500.ttf" },
        // Square Lily -> Lithops (organic rock-like succulent display).
        { "Lithops-Regular.otf", "Square Lily -> Lithops", kCDisp, "Velvetyne",
          "https://gitlab.com/daytonamess/lithops/-/archive/main/lithops-main.zip",
          "lithops-main/fonts/Lithops-Regular.otf" },
        // --- 3 extra Velvetyne slots (overflow from user's curated list) ---
        { "Ouvrieres-Affamees.otf", "Ouvrieres (Affamees)", kCDisp, "Velvetyne",
          "https://github.com/laureazz/Ouvrieres/archive/main.zip",
          "Ouvrieres-main/otf/Ouvrieres-affamees.otf" },
        { "PicNic-Regular.otf", "Picnic", kCDisp, "Velvetyne",
          "https://gitlab.com/mariellenils/PicNic/-/archive/main/PicNic-main.zip",
          "PicNic-main/fonts/otf/PicNic-Regular.otf" },

        // ---- Display / Decorative — Fontshare batch ----
        { "Comico-Regular.otf", "Comico", kCDisp, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/comico",
          "Comico_Complete/Fonts/OTF/Comico-Regular.otf" },
        { "Supreme-Regular.otf", "Supreme", kCDisp, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/supreme",
          "Supreme_Complete/Fonts/OTF/Supreme-Regular.otf" },
        { "BespokeStencil-Regular.otf", "Bespoke Stencil", kCDisp, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/bespoke-stencil",
          "BespokeStencil_Complete/Fonts/OTF/BespokeStencil-Regular.otf" },
        { "Aktura-Regular.otf", "Aktura", kCSerif, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/aktura",
          "Aktura_Complete/Fonts/OTF/Aktura-Regular.otf" },
        { "Britney-Regular.otf", "Britney", kCSerif, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/britney",
          "Britney_Complete/Fonts/OTF/Britney-Regular.otf" },
        { "Styro-Regular.otf", "Styro", kCDisp, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/styro",
          "Styro_Complete/Fonts/OTF/Styro-Regular.otf" },
        { "Boxing-Regular.otf", "Boxing", kCDisp, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/boxing",
          "Boxing_Complete/Fonts/OTF/Boxing-Regular.otf" },
        { "Kola-Regular.otf", "Kola", kCDisp, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/kola",
          "Kola_Complete/Fonts/OTF/Kola-Regular.otf" },
        { "Zina-Regular.otf", "Zina", kCSerif, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/zina",
          "Zina_Complete/Fonts/OTF/Zina-Regular.otf" },
        { "Kihim-Regular.otf", "Kihim", kCSerif, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/kihim",
          "Kihim_Complete/Fonts/OTF/Kihim-Regular.otf" },
        { "Striper-Regular.otf", "Striper", kCDisp, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/striper",
          "Striper_Complete/Fonts/OTF/Striper-Regular.otf" },
        { "KohinoorZerone-Regular.otf", "Kohinoor Zerone", kCDisp, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/kohinoor-zerone",
          "KohinoorZerone_Complete/Fonts/OTF/KohinoorZerone-Regular.otf" },

        // ---- CFF Monochrome ----
        // Manbow Clear -> Array (Fontshare zip — OTF/CFF for the cubic-Bezier demo).
        { "Array-Regular.otf", "Manbow Clear -> Array", kCCFF, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/array",
          "Array_Complete/Fonts/OTF/Array-Regular.otf" },
        // Manbow Lines -> Tanker (Fontshare zip — OTF/CFF).
        { "Tanker-Regular.otf", "Manbow Lines -> Tanker", kCCFF, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/tanker",
          "Tanker_Complete/Fonts/OTF/Tanker-Regular.otf" },
        // Manbow Spots — unmatched (no halftone/dot CFF on the 4 sources).
        { "ManbowSpots_placeholder.otf", "Manbow Spots (unmatched)", kCCFF, nullptr,
          nullptr, nullptr },
        // Manbow Tone — unmatched (no screentone CFF on the 4 sources).
        { "ManbowTone_placeholder.otf", "Manbow Tone (unmatched)", kCCFF, nullptr,
          nullptr, nullptr },

        // ---- Color: COLR v0 ----
        { "Noto-COLRv1.ttf", "Twemoji -> Noto Color Emoji (COLRv1)", kCColr0, "Google Fonts",
          "https://raw.githubusercontent.com/googlefonts/noto-emoji/main/fonts/Noto-COLRv1.ttf",
          nullptr },
        { "CoralPixels-Regular.ttf", "Coral Pixels", kCColr0, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/coralpixels/CoralPixels-Regular.ttf",
          nullptr },
        // OpenMoji Color COLRv0 (HfG Gmuend, CC-BY-SA 4.0).
        { "OpenMoji-color-glyf_colr_0.ttf", "OpenMoji Color (COLRv0)", kCColr0, "OpenMoji",
          "https://raw.githubusercontent.com/hfg-gmuend/openmoji/master/font/OpenMoji-color-glyf_colr_0/OpenMoji-color-glyf_colr_0.ttf",
          nullptr },

        // ---- Color: SVG ----
        // All unmatched: none of the 4 sources ship OT-SVG color fonts.
        { "AquaphonicDownpour_placeholder.otf", "Aquaphonic Downpour (unmatched)", kCSVG, nullptr,
          nullptr, nullptr },
        { "AquaphonicDrizzle_placeholder.otf", "Aquaphonic Drizzle (unmatched)", kCSVG, nullptr,
          nullptr, nullptr },
        { "CimeroPro_placeholder.otf", "Cimero Pro (unmatched)", kCSVG, nullptr,
          nullptr, nullptr },
        { "ColorTube_placeholder.otf", "Color Tube (unmatched)", kCSVG, nullptr,
          nullptr, nullptr },
        { "GilbertColor_placeholder.otf", "Gilbert Color Bold (unmatched)", kCSVG, nullptr,
          nullptr, nullptr },
        { "Multicolore_placeholder.otf", "Multicolore Pro (unmatched)", kCSVG, nullptr,
          nullptr, nullptr },
        { "PrimecolorG_placeholder.ttf", "Primecolor G (unmatched)", kCSVG, nullptr,
          nullptr, nullptr },
        { "PrimecolorM_placeholder.ttf", "Primecolor M (unmatched)", kCSVG, nullptr,
          nullptr, nullptr },
        { "Fattern_placeholder.otf", "Fattern (unmatched)", kCSVG, nullptr,
          nullptr, nullptr },
        // Noto Color Emoji SVG (Adobe, SIL OFL) — pure OT-SVG build of Noto Color Emoji.
        { "NotoColorEmoji-SVG.otf", "Noto Color Emoji (OT-SVG)", kCSVG, "Adobe (GitHub)",
          "https://github.com/adobe-fonts/noto-emoji-svg/releases/download/2.100/NotoColorEmoji-SVG.otf",
          nullptr },
        // Twitter Color Emoji SVGinOT (13rac1, CC-BY 4.0 art / MIT code) — Twemoji
        // packaged as OT-SVG. Downloaded from the Win release zip.
        { "TwitterColorEmoji-SVGinOT.ttf", "Twitter Color Emoji (SVGinOT)", kCSVG, "13rac1 (GitHub)",
          "https://github.com/13rac1/twemoji-color-font/releases/download/v15.1.0/TwitterColorEmoji-SVGinOT-Win-15.1.0.zip",
          "TwitterColorEmoji-SVGinOT-Win-15.1.0/TwitterColorEmoji-SVGinOT.ttf" },

        // ---- Color: COLR v1 / Gradient ----
        { "Nabla[EDPT,EHLT].ttf", "Nabla", kCColr1, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/nabla/Nabla%5BEDPT,EHLT%5D.ttf",
          nullptr },
        // Primecolor CV1 -> Bungee Spice. Same file as the standard Bungee Spice entry;
        // keeping separate slot so the demo shows two COLRv1 renders side-by-side.
        { "BungeeSpice-Regular.ttf", "Primecolor CV1 -> Bungee Spice", kCColr1, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/bungeespice/BungeeSpice-Regular.ttf",
          nullptr },
        { "BungeeSpice-Regular.ttf", "Bungee Spice", kCColr1, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/bungeespice/BungeeSpice-Regular.ttf",
          nullptr },
        { "Honk[MORF,SHLN].ttf", "Honk", kCColr1, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/honk/Honk%5BMORF,SHLN%5D.ttf",
          nullptr },
        // OpenMoji Color COLRv1 (HfG Gmuend, CC-BY-SA 4.0) — vector gradients.
        { "OpenMoji-color-glyf_colr_1.ttf", "OpenMoji Color (COLRv1)", kCColr1, "OpenMoji",
          "https://raw.githubusercontent.com/hfg-gmuend/openmoji/master/font/OpenMoji-color-glyf_colr_1/OpenMoji-color-glyf_colr_1.ttf",
          nullptr },
        // Microsoft Fluent Emoji webfont (tetunori build, MIT) — COLRv1 via nanoemoji.
        // Note: ~87 MB — larger than typical demo fonts.
        { "FluentEmojiColor.ttf", "Microsoft Fluent Emoji (COLRv1)", kCColr1, "tetunori (GitHub)",
          "https://raw.githubusercontent.com/tetunori/fluent-emoji-webfont/main/dist/FluentEmojiColor.ttf",
          nullptr },
        // Amiri Quran Colored (aliftype, OFL) — Arabic Quran with COLRv1 markup colors.
        { "AmiriQuranColored.ttf", "Amiri Quran Colored (COLR)", kCColr1, "aliftype (GitHub)",
          "https://github.com/aliftype/amiri/releases/download/1.003/Amiri-1.003.zip",
          "Amiri-1.003/AmiriQuranColored.ttf" },

        // ---- Arabic ----
        { "Amiri-Regular.ttf", "Amiri", kCArabic, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/amiri/Amiri-Regular.ttf",
          nullptr },
        { "ArefRuqaaInk-Bold.ttf", "Aref Ruqaa Ink Bold", kCArabic, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/arefruqaaink/ArefRuqaaInk-Bold.ttf",
          nullptr },
        { "ArefRuqaaInk-Regular.ttf", "Aref Ruqaa Ink Regular", kCArabic, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/arefruqaaink/ArefRuqaaInk-Regular.ttf",
          nullptr },
        { "BlakaInk-Regular.ttf", "Blaka Ink", kCArabic, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/blakaink/BlakaInk-Regular.ttf",
          nullptr },
        { "ReemKufiInk-Regular.ttf", "Reem Kufi Ink", kCArabic, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/reemkufiink/ReemKufiInk-Regular.ttf",
          nullptr },
        { "ReemKufiFun[wght].ttf", "Reem Kufi Fun", kCArabic, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/reemkufifun/ReemKufiFun%5Bwght%5D.ttf",
          nullptr },
        { "CairoPlay[slnt,wght].ttf", "Cairo Play Bold", kCArabic, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/cairoplay/CairoPlay%5Bslnt,wght%5D.ttf",
          nullptr },
        { "CairoPlay[slnt,wght].ttf", "Cairo Play ExtraLight", kCArabic, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/cairoplay/CairoPlay%5Bslnt,wght%5D.ttf",
          nullptr },
        // Vazirmatn (rastikerdar, OFL) — modern Persian/Arabic sans. Non-Google upstream.
        { "Vazirmatn-Regular.ttf", "Vazirmatn", kCArabic, "rastikerdar (GitHub)",
          "https://github.com/rastikerdar/vazirmatn/releases/download/v33.003/vazirmatn-v33.003.zip",
          "fonts/ttf/Vazirmatn-Regular.ttf" },
        // AmiriQuran upstream (aliftype, OFL) — black-only Quran variant from canonical source.
        { "AmiriQuran.ttf", "Amiri Quran (upstream)", kCArabic, "aliftype (GitHub)",
          "https://github.com/aliftype/amiri/releases/download/1.003/Amiri-1.003.zip",
          "Amiri-1.003/AmiriQuran.ttf" },
    };

    const int kFontMetaCount = (int)( sizeof( kFontMeta ) / sizeof( kFontMeta[0] ) );

} // namespace ImDwDownload
