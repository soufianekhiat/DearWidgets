using Sharpmake;

[module: Sharpmake.Include("common.cs")]

[module: Sharpmake.Include("APIProject.cs")]

namespace DearWidgets
{
    [Sharpmake.Generate]
    public class DemoProject : CommonProject
    {
        public DemoProject()
        {
            Name = "DearWidgetsDemo";
            SourceRootPath = RootPath + @"\src\demo";

            // Use ImPlatform's bundled ImGui instead of the main extern/imgui
            // (checked out at the fork's LayoutDocking: docking + stack layout + draw-command hooks).
            SourceFiles.Add("[project.ExternPath]/ImPlatform/imgui/imgui.cpp");
            SourceFiles.Add("[project.ExternPath]/ImPlatform/imgui/imgui_stacklayout.cpp");
            SourceFiles.Add("[project.ExternPath]/ImPlatform/imgui/imgui_tables.cpp");
            SourceFiles.Add("[project.ExternPath]/ImPlatform/imgui/imgui_widgets.cpp");
            SourceFiles.Add("[project.ExternPath]/ImPlatform/imgui/misc/cpp/imgui_stdlib.cpp");
            SourceFiles.Add("[project.ExternPath]/ImPlatform/imgui/imgui_draw.cpp");
            SourceFiles.Add("[project.ExternPath]/ImPlatform/ImPlatform/ImPlatform.h");

            // Add ImGui backend implementations from ImPlatform
            // These are added unconditionally but only the relevant ones will be compiled based on the target API
            SourceFiles.Add("[project.ExternPath]/ImPlatform/imgui/backends/imgui_impl_dx9.cpp");
            SourceFiles.Add("[project.ExternPath]/ImPlatform/imgui/backends/imgui_impl_dx10.cpp");
            SourceFiles.Add("[project.ExternPath]/ImPlatform/imgui/backends/imgui_impl_dx11.cpp");
            SourceFiles.Add("[project.ExternPath]/ImPlatform/imgui/backends/imgui_impl_dx12.cpp");
            SourceFiles.Add("[project.ExternPath]/ImPlatform/imgui/backends/imgui_impl_opengl3.cpp");
            SourceFiles.Add("[project.ExternPath]/ImPlatform/imgui/backends/imgui_impl_vulkan.cpp");
            SourceFiles.Add("[project.ExternPath]/ImPlatform/imgui/backends/imgui_impl_win32.cpp");
        }

        [Configure()]
        public void ConfigureDemo(Configuration conf, DearTarget target)
        {
            conf.Output = Configuration.OutputType.Exe;
            conf.AddPrivateDependency<APIProject>(target);
			conf.TargetPath = RootPath + "/WorkingDir";

            conf.IncludePaths.Add(@"[project.RootPath]/src/demo/");
            // Use ImPlatform's bundled ImGui
            conf.IncludePaths.Add(@"[project.RootPath]/extern/ImPlatform/imgui/");
            conf.IncludePaths.Add(@"[project.RootPath]/extern/ImPlatform/imgui/backends/");
            conf.IncludePaths.Add(@"[project.RootPath]/extern/ImPlatform/ImPlatform/");
            // Add specific extern paths as needed
            conf.IncludePaths.Add(@"[project.RootPath]/extern/stb/");
            conf.IncludePaths.Add(@"[project.RootPath]/extern/");
            // Add graphics API libraries
            if (target.Api == TargetAPI.D3D9)
            {
                conf.LibraryFiles.Add("d3d9.lib");
            }
            if (target.Api == TargetAPI.D3D10)
            {
                conf.LibraryFiles.Add("d3d10.lib");
                conf.LibraryFiles.Add("d3dcompiler.lib");
                conf.LibraryFiles.Add("dxgi.lib");
            }
            if (target.Api == TargetAPI.D3D11)
            {
                conf.LibraryFiles.Add("d3d11.lib");
                conf.LibraryFiles.Add("d3dcompiler.lib");
                conf.LibraryFiles.Add("dxgi.lib");
            }
            if (target.Api == TargetAPI.D3D12)
            {
                conf.LibraryFiles.Add("d3d12.lib");
                conf.LibraryFiles.Add("d3dcompiler.lib");
                conf.LibraryFiles.Add("dxgi.lib");
            }
            if (target.Api == TargetAPI.OpenGL3)
            {
                conf.LibraryFiles.Add("opengl32.lib");
            }

			conf.VcxprojUserFile = new Configuration.VcxprojUserFileSettings();
			conf.VcxprojUserFile.LocalDebuggerWorkingDirectory = @"[project.RootPath]/WorkingDir/";
			//@"$(SolutionDir)WorkingDir";
        }
    }
}
