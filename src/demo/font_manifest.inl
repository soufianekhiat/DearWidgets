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
    static const char* kCLig    = "Ligature Showcase";

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
        { "Rye-Regular.ttf", "Steelworks Vintage -> Rye", kCSerif, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/rye/Rye-Regular.ttf",
          nullptr },
        // Additional Serif -- Google Fonts
        { "Alegreya[wght].ttf", "Alegreya", kCSerif, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/alegreya/Alegreya%5Bwght%5D.ttf",
          nullptr },
        { "Fraunces[SOFT,WONK,opsz,wght].ttf", "Fraunces", kCSerif, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/fraunces/Fraunces%5BSOFT,WONK,opsz,wght%5D.ttf",
          nullptr },
        { "Italiana-Regular.ttf", "Italiana", kCSerif, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/italiana/Italiana-Regular.ttf",
          nullptr },
        { "YesevaOne-Regular.ttf", "Yeseva One", kCSerif, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/yesevaone/YesevaOne-Regular.ttf",
          nullptr },

        // ---- Script / Handwriting ----
        // Script / Handwriting — chosen to span distinct traditions rather
        // than all being English-roundhand variants. Styles: pencil, sharpie,
        // felt-tip, loose marker, personal cursive, retro brush, monoline,
        // bouncy casual, dry brush, bold brush, copperplate, Italian script,
        // retro connected, retro italic brush, formal engraved, bold display
        // script, expressive display script.
        { "Sacramento-Regular.ttf", "Bright Marching -> Sacramento", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/sacramento/Sacramento-Regular.ttf",
          nullptr },  // monoline upright
        { "KaushanScript-Regular.ttf", "Camood -> Kaushan Script", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/kaushanscript/KaushanScript-Regular.ttf",
          nullptr },  // bold brush
        { "GreatVibes-Regular.ttf", "Cherona -> Great Vibes", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/greatvibes/GreatVibes-Regular.ttf",
          nullptr },  // classic English copperplate
        { "LoveLight-Regular.ttf", "Love Light", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/lovelight/LoveLight-Regular.ttf",
          nullptr },  // decorative / glitter script (keeper)
        { "Sail-Regular.ttf", "Metafora Stylistic -> Sail", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/sail/Sail-Regular.ttf",
          nullptr },  // bold monoline display script
        { "HomemadeApple-Regular.ttf", "Regina -> Homemade Apple", kCScript, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/apache/homemadeapple/HomemadeApple-Regular.ttf",
          nullptr },  // personal cursive (photographed real hand)

        // ---- Display / Decorative — curated Velvetyne picks ----
        // Bollgo -> Flor de Ruina (baroque organic).
        { "FlorDeRuina-Flor.otf", "Bollgo -> Flor de Ruina", kCDisp, "Velvetyne",
          "https://gitlab.com/velvetyne/flor-des-ruina/-/archive/main/flor-des-ruina-main.zip",
          "flor-des-ruina-main/fonts/otf/FlorDeRuina-Flor.otf" },
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
        { "Aktura-Regular.otf", "Aktura", kCSerif, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/aktura",
          "Aktura_Complete/Fonts/OTF/Aktura-Regular.otf" },
        { "Britney-Regular.otf", "Britney", kCSerif, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/britney",
          "Britney_Complete/Fonts/OTF/Britney-Regular.otf" },
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
        // Ships two display styles (Zero / One), no "Regular" -- use Zero as the base.
        { "KohinoorZerone-Regular.otf", "Kohinoor Zerone", kCDisp, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/kohinoor-zerone",
          "KohinoorZerone_Complete/Fonts/OTF/KohinoorZerone-Zero.otf" },
        { "Monoton-Regular.ttf", "Monoton", kCDisp, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/monoton/Monoton-Regular.ttf",
          nullptr },

        // ---- CFF Monochrome ----
        // Manbow Clear -> Array (Fontshare zip — OTF/CFF for the cubic-Bezier demo).
        { "Array-Regular.otf", "Manbow Clear -> Array", kCCFF, "Fontshare",
          "https://api.fontshare.com/v2/fonts/download/array",
          "Array_Complete/Fonts/OTF/Array-Regular.otf" },
        // Manbow Spots — CC0, Typodermic Fonts (Ray Larabie), DaFont.
        { "ManBow-Spots.otf", "Manbow Spots", kCCFF, "Typodermic (DaFont, CC0)",
          "https://dl.dafont.com/dl/?f=manbow",
          "Manbow Spots.otf" },
        // Manbow Lines — CC0, Typodermic Fonts (Ray Larabie), DaFont.
        { "ManBow-Lines.otf", "Manbow Lines", kCCFF, "Typodermic (DaFont, CC0)",
          "https://dl.dafont.com/dl/?f=manbow",
          "Manbow Lines.otf" },

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
        // Aquaphonic (OT-SVG) — free for personal & commercial use, 1001fonts.
        { "Aquaphonic-Downpour.otf", "Aquaphonic Downpour", kCSVG, "1001fonts",
          "https://www.1001fonts.com/download/aquaphonic.zip",
          "Aquaphonic-Downpour.otf" },
        // Cimero Pro (OT-SVG) — freeware, 1001fonts. Use the www.1001fonts.com
        // host: the st.1001fonts.net CDN blocks bots and returns a JPEG.
        { "CimeroPro.otf", "Cimero Pro", kCSVG, "1001fonts",
          "https://www.1001fonts.com/download/cimero-pro.zip",
          "CimeroPro.otf" },
        // Color Tube (OT-SVG) — Fontfabric / Ivan Filipov, free for commercial
        // use, 1001fonts.
        { "ColorTube.otf", "Color Tube", kCSVG, "1001fonts",
          "https://www.1001fonts.com/download/colortube.zip",
          "ColorTube.otf" },
        // Gilbert Color Bold (OT-SVG) — CC-BY-SA 4.0, direct OTF from the
        // TypeWithPride repo (file was renamed; the old preview5 path 404s).
        { "GilbertColorBold.otf", "Gilbert Color Bold", kCSVG, "CC-BY-SA 4.0",
          "https://raw.githubusercontent.com/Fontself/TypeWithPride/master/fonts/Gilbert-Color%20Bold%20Preview_1005.otf",
          nullptr },
        // Multicolore Pro (OT-SVG) — free for personal & commercial use, 1001fonts.
        // The OTF is nested one folder deep inside the archive.
        { "Multicolore-Pro.otf", "Multicolore Pro", kCSVG, "1001fonts",
          "https://www.1001fonts.com/download/multicolore-pro.zip",
          "Multicolore Pro by neogrey creative/Multicolore Pro.otf" },
        // Primecolor (OT-SVG) — SIL OFL, 1001fonts.
        { "Primecolor-G.ttf", "Primecolor G", kCSVG, "SIL OFL (1001fonts)",
          "https://www.1001fonts.com/download/primecolor.zip",
          "Primecolor-G.ttf" },
        { "Primecolor-M.ttf", "Primecolor M", kCSVG, "SIL OFL (1001fonts)",
          "https://www.1001fonts.com/download/primecolor.zip",
          "Primecolor-M.ttf" },
        // Fattern (OT-SVG) — free for commercial use, 1001fonts.
        // st.1001fonts.net blocks bots (returns JPEG); use www.1001fonts.com instead.
        { "Fattern.otf", "Fattern", kCSVG, "FCC (1001fonts)",
          "https://www.1001fonts.com/download/fattern.zip",
          "Fattern-GO6zm.otf" },
        // Noto Color Emoji SVG (Adobe, SIL OFL) — pure OT-SVG build of Noto Color Emoji.
        { "NotoColorEmoji-SVG.otf", "Noto Color Emoji (OT-SVG)", kCSVG, "Adobe (GitHub)",
          "https://github.com/adobe-fonts/noto-emoji-svg/releases/download/2.100/NotoColorEmoji-SVG.otf",
          nullptr },

        // ---- Color: COLR v1 / Gradient ----
        { "Nabla[EDPT,EHLT].ttf", "Nabla", kCColr1, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/nabla/Nabla%5BEDPT,EHLT%5D.ttf",
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
        // Vazirmatn (rastikerdar, OFL) — modern Persian/Arabic sans. Non-Google upstream.
        { "Vazirmatn-Regular.ttf", "Vazirmatn", kCArabic, "rastikerdar (GitHub)",
          "https://github.com/rastikerdar/vazirmatn/releases/download/v33.003/vazirmatn-v33.003.zip",
          "fonts/ttf/Vazirmatn-Regular.ttf" },
        // AmiriQuran upstream (aliftype, OFL) — black-only Quran variant from canonical source.
        { "AmiriQuran.ttf", "Amiri Quran (upstream)", kCArabic, "aliftype (GitHub)",
          "https://github.com/aliftype/amiri/releases/download/1.003/Amiri-1.003.zip",
          "Amiri-1.003/AmiriQuran.ttf" },

        // ---- Ligature Showcase ----
        // Fonts chosen for rich ligature sets: programming (=>, ->, ::),
        // classical text (fi, fl, ffi, ct, st, Th, sp), historical
        // (medieval æ, œ), and decorative script (letter-pair flourishes).
        // All open-source licenses (OFL / Apache / Fontshare).
        { "JetBrainsMono[wght].ttf", "JetBrains Mono (programming)", kCLig, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/jetbrainsmono/JetBrainsMono%5Bwght%5D.ttf",
          nullptr },
        { "EBGaramond[wght].ttf", "EB Garamond (classical oldstyle)", kCLig, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/ebgaramond/EBGaramond%5Bwght%5D.ttf",
          nullptr },
        { "MeaCulpa-Regular.ttf", "Mea Culpa (extreme calligraphy)", kCLig, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/meaculpa/MeaCulpa-Regular.ttf",
          nullptr },
        // Junicode (Peter S. Baker, OFL) — medievalist typography: insular
        // characters, Old English / runic / medieval Latin ligatures.
        { "Junicode-Regular.ttf", "Junicode (medieval historical)", kCLig, "psb1558 (GitHub)",
          "https://github.com/psb1558/Junicode-font/releases/download/v2.222/Junicode_2.222.zip",
          "Junicode/TTF/Junicode-Regular.ttf" },

        { "MonteCarlo-Regular.ttf", "MonteCarlo (Spencerian with flourishes)", kCLig, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/montecarlo/MonteCarlo-Regular.ttf",
          nullptr },
        // Unusual 3+ char ligatures: Unifraktur Maguntia (OFL) — historical
        // German blackletter with ſch / ſſi / ch / ck / ll / tz discretionary
        // ligs absent from any Roman-serif font.
        { "UnifrakturMaguntia-Book.ttf", "Unifraktur Maguntia (ſch/ſſi blackletter)", kCLig, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/unifrakturmaguntia/UnifrakturMaguntia-Book.ttf",
          nullptr },
        // Allura (OFL) — Sudtipos-style formal script with whole-syllable
        // word ligatures (The / tion / ion / ing / are) rather than
        // letter-pair flourishes.
        { "Allura-Regular.ttf", "Allura (script word ligs: The/tion/ing)", kCLig, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/allura/Allura-Regular.ttf",
          nullptr },
        { "Caudex-Regular.ttf", "Caudex (medievalist scholarly)", kCLig, "Google Fonts",
          "https://raw.githubusercontent.com/google/fonts/main/ofl/caudex/Caudex-Regular.ttf",
          nullptr },
        // Chomsky (Fredrick R. Brennan, OFL) — NYT-masthead gothic-blackletter
        // hybrid; ships OTF only from GitHub releases.
    };

    const int kFontMetaCount = (int)( sizeof( kFontMeta ) / sizeof( kFontMeta[0] ) );

} // namespace ImDwDownload
