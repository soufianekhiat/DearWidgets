// font_downloader.h — demo-only font downloader for DearWidgets
//
// Pulls the ~60 preview fonts from Google Fonts / Fontshare / Velvetyne /
// Open Foundry on demand, so the repo doesn't ship any licensed demo fonts
// (only the LaTeX math font in workingdir/latex_fonts/).
//
// Usage (single-TU, stb-style):
//   #define FONT_DOWNLOADER_IMPLEMENTATION
//   #include "font_downloader.h"
//
// Then call DrawDownloadAllButton() / DrawCategoryDownloadButton("Serif")
// from the demo's Slug section, and AddFontIfExists() when loading.
//
// Platform HTTP: shells out to the `curl` CLI (ships with Windows 10+,
// macOS, and almost every Linux). One ifdef for _popen vs popen and mkdir.

#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include <string>
#include <vector>
#include <deque>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

#if defined(_WIN32)
    #include <direct.h>
    #define IMDW_POPEN  _popen
    #define IMDW_PCLOSE _pclose
    #define IMDW_MKDIR(p) _mkdir(p)
#else
    #include <unistd.h>
    #include <sys/types.h>
    #define IMDW_POPEN  popen
    #define IMDW_PCLOSE pclose
    #define IMDW_MKDIR(p) mkdir(p, 0755)
#endif

// ImWchar is a typedef (not a struct) in ImGui so no fwd-decl is possible;
// we don't need any ImGui types in this header's public API anyway.

namespace ImDwDownload
{
    enum Status : int { Missing = 0, Queued, Downloading, Done, Failed };

    struct Meta
    {
        const char* canonical;      // filename on disk, e.g. "Cinzel-Regular.ttf"
        const char* display;        // "Cinzel"
        const char* category;       // group key (matches kGrp* in demo.cpp)
        const char* source;         // "Google Fonts" / "Fontshare" / ... or nullptr
        const char* url;            // direct download URL, or nullptr if unmatched
        const char* archive_inner;  // path-inside-zip if url is a .zip archive; else nullptr
    };

    extern const Meta kFontMeta[];
    extern const int  kFontMetaCount;

    // --- Query ---
    bool FileExists( const char* path );
    Status GetStatus( int index );
    int GetDoneCount();
    int GetPendingCount();
    int FindByCanonical( const char* canonical );

    // --- Actions ---
    bool StartDownloadAll();
    bool StartDownloadCategory( const char* category );
    bool StartDownloadOne( int index );

    // --- UI helpers ---
    // Render "Download all fonts" button with progress label.
    void DrawDownloadAllButton();
    // Render a single "Download {category}" button. Returns true if clicked.
    bool DrawCategoryDownloadButton( const char* category );
    // Per-font inline button (used for missing-font placeholders).
    bool DrawSingleFontDownloadButton( int index );

    // --- Bookkeeping ---
    // Call once at startup (after all AddFontFromFileTTF) to refresh statuses.
    void RefreshStatuses();
    // Call each frame to drain finished-download results (updates atomics).
    void Tick();
    // Call on demo shutdown.
    void Shutdown();

    // --- Integration helpers ---
    // Convenience: returns "fonts/<canonical>" as std::string (demo is run from workingdir/)
    std::string PathFor( const Meta& m );
    std::string PathForCanonical( const char* canonical );
}

// ======================================================================
// IMPLEMENTATION
// ======================================================================
#ifdef FONT_DOWNLOADER_IMPLEMENTATION

#include <imgui.h>
#include <algorithm>
#include <cstdlib>
#include <cerrno>
#include <filesystem>

namespace ImDwDownload
{
    // --- Runtime state ---
    static std::vector<std::atomic<int>> s_status; // one per meta entry
    static std::mutex s_queue_mx;
    static std::deque<int> s_queue;
    static std::atomic<bool> s_worker_running{ false };
    static std::atomic<bool> s_worker_stop{ false };
    static std::thread s_worker;

    // --- Helpers ---
    bool FileExists( const char* path )
    {
        struct stat st;
        return stat( path, &st ) == 0 && ( st.st_mode & S_IFREG );
    }

    std::string PathFor( const Meta& m )
    {
        std::string p = "fonts/";
        p += m.canonical;
        return p;
    }

    std::string PathForCanonical( const char* canonical )
    {
        std::string p = "fonts/";
        p += canonical;
        return p;
    }

    static void EnsureFontsDir()
    {
        IMDW_MKDIR( "fonts" );
        (void)errno; // ignore "already exists"
    }

    static void InitStatusArray()
    {
        if ( (int)s_status.size() == kFontMetaCount ) return;
        s_status = std::vector<std::atomic<int>>( kFontMetaCount );
        RefreshStatuses();
    }

    void RefreshStatuses()
    {
        InitStatusArray(); // safe re-entry
        for ( int i = 0; i < kFontMetaCount; ++i )
        {
            int cur = s_status[i].load();
            if ( cur == Downloading || cur == Queued ) continue;
            bool ok = FileExists( PathFor( kFontMeta[i] ).c_str() );
            s_status[i].store( ok ? Done : Missing );
        }
    }

    Status GetStatus( int index )
    {
        if ( index < 0 || index >= kFontMetaCount ) return Missing;
        if ( (int)s_status.size() != kFontMetaCount ) InitStatusArray();
        return (Status)s_status[index].load();
    }

    int GetDoneCount()
    {
        if ( (int)s_status.size() != kFontMetaCount ) InitStatusArray();
        int n = 0;
        for ( int i = 0; i < kFontMetaCount; ++i )
            if ( s_status[i].load() == Done ) ++n;
        return n;
    }

    int GetPendingCount()
    {
        if ( (int)s_status.size() != kFontMetaCount ) InitStatusArray();
        int n = 0;
        for ( int i = 0; i < kFontMetaCount; ++i )
        {
            int s = s_status[i].load();
            if ( s == Queued || s == Downloading ) ++n;
        }
        return n;
    }

    int FindByCanonical( const char* canonical )
    {
        for ( int i = 0; i < kFontMetaCount; ++i )
            if ( std::strcmp( kFontMeta[i].canonical, canonical ) == 0 )
                return i;
        return -1;
    }

    // --- Download core ---
    // Shells out to `curl` — on Windows this resolves via PATH (curl.exe is in
    // system32 since Win10 1803). On Unix `popen` finds it in /usr/bin.
    // -L follows redirects (Google/Fontshare both redirect).
    // -f exits nonzero on 4xx/5xx.
    // -sS: silent, but show errors on stderr.
    // Quoting: URL is surrounded by double quotes; filenames in our manifest
    // use ASCII-only characters, so shell injection is not a concern.
    static bool CurlDownload( const char* url, const char* dest_path )
    {
        char cmd[4096];
#if defined(_WIN32)
        // Use the system-bundled curl (Win10 1803+) instead of whatever is
        // first in PATH — avoids MSYS/Git-Bash-curl picking up a different
        // cert bundle in mixed environments.
        //
        // NB: do NOT wrap the exe path in extra quotes. _popen invokes
        // `cmd.exe /c <cmdline>`; if <cmdline> starts and ends with `"` and
        // has multiple quote pairs, cmd /c strips the outer pair and leaves
        // the command mangled (see "cmd /c quoting rules"). %SystemRoot%
        // contains no spaces, so quoting is unnecessary.
        std::snprintf( cmd, sizeof( cmd ),
            "%%SystemRoot%%\\System32\\curl.exe -L -fsS --retry 2 --max-time 60 -o \"%s\" \"%s\" 2>&1",
            dest_path, url );
#else
        std::snprintf( cmd, sizeof( cmd ),
            "curl -L -fsS --retry 2 --max-time 60 -o \"%s\" \"%s\" 2>&1",
            dest_path, url );
#endif
        FILE* fp = IMDW_POPEN( cmd, "r" );
        if ( !fp ) return false;
        // Drain output (prevents pipe stalls).
        char buf[512];
        while ( std::fgets( buf, sizeof( buf ), fp ) ) { /* discard */ }
        int rc = IMDW_PCLOSE( fp );
        return rc == 0 && FileExists( dest_path );
    }

    // Extract a single file from a ZIP archive.
    //   - Windows: hard-codes the system bsdtar at %SystemRoot%\System32\tar.exe
    //     (Win10 1803+ ships it). Avoids picking up MSYS/Git-Bash's GNU tar
    //     which cannot read zip files.
    //   - Unix/macOS: uses `unzip` (present by default on macOS, almost always
    //     installed on Linux).
    // Extracts `inner_path` from `zip_path` into `out_dir`, then returns the
    // full resulting path in `out_extracted_path`.
    static bool ExtractSingleFromZip(
        const char* zip_path, const char* inner_path,
        const char* out_dir, std::string& out_extracted_path )
    {
        char cmd[4096];
#if defined(_WIN32)
        // Same cmd /c quoting trap as CurlDownload — don't quote the exe path.
        std::snprintf( cmd, sizeof( cmd ),
            "%%SystemRoot%%\\System32\\tar.exe -xf \"%s\" -C \"%s\" \"%s\" 2>&1",
            zip_path, out_dir, inner_path );
#else
        // -o: overwrite without prompting. -d: output dir.
        std::snprintf( cmd, sizeof( cmd ),
            "unzip -o -q \"%s\" \"%s\" -d \"%s\" 2>&1",
            zip_path, inner_path, out_dir );
#endif
        FILE* fp = IMDW_POPEN( cmd, "r" );
        if ( !fp ) return false;
        char buf[512];
        while ( std::fgets( buf, sizeof( buf ), fp ) ) { /* discard */ }
        int rc = IMDW_PCLOSE( fp );
        if ( rc != 0 ) return false;

        out_extracted_path = std::string( out_dir ) + "/" + inner_path;
        return FileExists( out_extracted_path.c_str() );
    }

    // Download-and-unpack for archive entries. Downloads to fonts/.tmp_<idx>.zip,
    // extracts `archive_inner`, copies to fonts/<canonical>, cleans up.
    static bool DownloadAndUnpack( int index )
    {
        const Meta& m = kFontMeta[index];
        namespace fs = std::filesystem;

        char tmp_zip[128], extract_dir[128];
        std::snprintf( tmp_zip, sizeof( tmp_zip ), "fonts/.tmp_%d.zip", index );
        std::snprintf( extract_dir, sizeof( extract_dir ), "fonts/.tmp_%d_d", index );

        // Clean slate
        std::error_code ec;
        fs::remove_all( extract_dir, ec );
        fs::remove( tmp_zip, ec );
        fs::create_directory( extract_dir, ec );

        if ( !CurlDownload( m.url, tmp_zip ) ) return false;

        std::string extracted;
        if ( !ExtractSingleFromZip( tmp_zip, m.archive_inner, extract_dir, extracted ) )
        {
            fs::remove_all( extract_dir, ec );
            fs::remove( tmp_zip, ec );
            return false;
        }

        std::string final_path = PathFor( m );
        fs::remove( final_path, ec );
        fs::rename( extracted, final_path, ec );
        if ( ec )
        {
            // Cross-device? fall back to copy+remove.
            ec.clear();
            fs::copy_file( extracted, final_path, fs::copy_options::overwrite_existing, ec );
            fs::remove( extracted, ec );
        }

        fs::remove_all( extract_dir, ec );
        fs::remove( tmp_zip, ec );

        return FileExists( final_path.c_str() );
    }

    static void WorkerLoop()
    {
        while ( !s_worker_stop.load() )
        {
            int index = -1;
            {
                std::lock_guard<std::mutex> g( s_queue_mx );
                if ( !s_queue.empty() )
                {
                    index = s_queue.front();
                    s_queue.pop_front();
                }
            }
            if ( index < 0 )
            {
                // nothing to do — worker exits, will be re-spawned on next job
                break;
            }

            const Meta& m = kFontMeta[index];
            s_status[index].store( Downloading );

            if ( !m.url )
            {
                s_status[index].store( Failed );
                continue;
            }

            EnsureFontsDir();
            bool ok;
            if ( m.archive_inner )
            {
                ok = DownloadAndUnpack( index );
            }
            else
            {
                // Download to .downloading then atomic-rename, so the main
                // thread (which hot-loads any file that appears in fonts/)
                // never sees a partial TTF.
                std::string dest = PathFor( m );
                std::string tmp  = dest + ".downloading";
                std::error_code ec;
                std::filesystem::remove( tmp, ec );
                ok = CurlDownload( m.url, tmp.c_str() );
                if ( ok )
                {
                    std::filesystem::remove( dest, ec );
                    std::filesystem::rename( tmp, dest, ec );
                    if ( ec )
                    {
                        ec.clear();
                        std::filesystem::copy_file( tmp, dest,
                            std::filesystem::copy_options::overwrite_existing, ec );
                        std::filesystem::remove( tmp, ec );
                    }
                    ok = FileExists( dest.c_str() );
                }
                else
                {
                    std::filesystem::remove( tmp, ec );
                }
            }
            s_status[index].store( ok ? Done : Failed );
        }
        s_worker_running.store( false );
    }

    static void KickWorker()
    {
        if ( s_worker_running.exchange( true ) ) return; // already running
        if ( s_worker.joinable() ) s_worker.join();
        s_worker_stop.store( false );
        s_worker = std::thread( WorkerLoop );
    }

    static void EnqueueOne( int index )
    {
        if ( index < 0 || index >= kFontMetaCount ) return;
        int expected = Missing;
        // Only re-queue items that are Missing or Failed.
        int cur = s_status[index].load();
        if ( cur == Done || cur == Queued || cur == Downloading ) return;
        if ( !kFontMeta[index].url ) { s_status[index].store( Failed ); return; }
        s_status[index].store( Queued );
        {
            std::lock_guard<std::mutex> g( s_queue_mx );
            s_queue.push_back( index );
        }
        (void)expected;
    }

    bool StartDownloadAll()
    {
        InitStatusArray();
        for ( int i = 0; i < kFontMetaCount; ++i ) EnqueueOne( i );
        KickWorker();
        return true;
    }

    bool StartDownloadCategory( const char* category )
    {
        if ( !category ) return false;
        InitStatusArray();
        int n = 0;
        for ( int i = 0; i < kFontMetaCount; ++i )
        {
            if ( std::strcmp( kFontMeta[i].category, category ) == 0 )
            {
                EnqueueOne( i );
                ++n;
            }
        }
        if ( n > 0 ) KickWorker();
        return n > 0;
    }

    bool StartDownloadOne( int index )
    {
        InitStatusArray();
        EnqueueOne( index );
        KickWorker();
        return true;
    }

    void Tick()
    {
        // If worker exited but queue still has items (rare race), restart it.
        bool running = s_worker_running.load();
        bool have_work = false;
        {
            std::lock_guard<std::mutex> g( s_queue_mx );
            have_work = !s_queue.empty();
        }
        if ( have_work && !running ) KickWorker();
    }

    void Shutdown()
    {
        s_worker_stop.store( true );
        if ( s_worker.joinable() ) s_worker.join();
    }

    // --- UI ---
    void DrawDownloadAllButton()
    {
        InitStatusArray();
        int done = GetDoneCount();
        int pend = GetPendingCount();
        int total = kFontMetaCount;

        ImGui::PushID( "##DwDlAll" );
        bool any_pending = pend > 0;
        if ( any_pending )
        {
            ImGui::BeginDisabled();
            ImGui::Button( "Download all fonts" );
            ImGui::EndDisabled();
        }
        else
        {
            if ( ImGui::Button( "Download all fonts" ) ) StartDownloadAll();
        }
        ImGui::SameLine();
        ImGui::Text( "(%d / %d ready%s)",
            done, total,
            pend > 0 ? ", downloading..." : "" );
        ImGui::PopID();
    }

    bool DrawCategoryDownloadButton( const char* category )
    {
        InitStatusArray();
        int done = 0, total = 0, pend = 0, unmatched = 0;
        for ( int i = 0; i < kFontMetaCount; ++i )
        {
            if ( std::strcmp( kFontMeta[i].category, category ) != 0 ) continue;
            ++total;
            int s = s_status[i].load();
            if ( s == Done ) ++done;
            else if ( s == Queued || s == Downloading ) ++pend;
            if ( !kFontMeta[i].url ) ++unmatched;
        }
        if ( total == 0 ) return false;

        char label[96];
        std::snprintf( label, sizeof( label ), "Download %s (%d/%d)###dl_%s",
            category, done, total, category );

        ImGui::PushID( category );
        bool clicked = false;
        bool disabled = ( done + unmatched >= total ) || ( pend > 0 );
        if ( disabled )
        {
            ImGui::BeginDisabled();
            ImGui::SmallButton( label );
            ImGui::EndDisabled();
        }
        else
        {
            if ( ImGui::SmallButton( label ) )
            {
                clicked = true;
                StartDownloadCategory( category );
            }
        }
        if ( unmatched > 0 )
        {
            ImGui::SameLine();
            ImGui::TextDisabled( "(%d unmatched)", unmatched );
        }
        ImGui::PopID();
        return clicked;
    }

    bool DrawSingleFontDownloadButton( int index )
    {
        if ( index < 0 || index >= kFontMetaCount ) return false;
        const Meta& m = kFontMeta[index];
        int s = s_status[index].load();

        ImGui::PushID( index + 10000 );
        bool clicked = false;
        if ( !m.url )
        {
            ImGui::TextDisabled( "no replacement on Google/Fontshare/Velvetyne/Open Foundry" );
        }
        else if ( s == Done )
        {
            ImGui::TextDisabled( "downloaded" );
        }
        else if ( s == Queued || s == Downloading )
        {
            ImGui::TextDisabled( "downloading..." );
        }
        else if ( s == Failed )
        {
            if ( ImGui::SmallButton( "retry" ) ) { clicked = true; StartDownloadOne( index ); }
            ImGui::SameLine();
            ImGui::TextDisabled( "failed" );
        }
        else
        {
            if ( ImGui::SmallButton( "download" ) ) { clicked = true; StartDownloadOne( index ); }
        }
        ImGui::PopID();
        return clicked;
    }

} // namespace ImDwDownload

#endif // FONT_DOWNLOADER_IMPLEMENTATION
