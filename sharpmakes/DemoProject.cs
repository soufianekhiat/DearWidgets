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

            SourceFiles.Add("[project.ExternPath]/imgui/imgui.cpp");
            SourceFiles.Add("[project.ExternPath]/imgui/imgui_tables.cpp");
            SourceFiles.Add("[project.ExternPath]/imgui/imgui_widgets.cpp");
            SourceFiles.Add("[project.ExternPath]/imgui/misc/cpp/imgui_stdlib.cpp");
            SourceFiles.Add("[project.ExternPath]/imgui/imgui_draw.cpp");
            SourceFiles.Add("[project.ExternPath]/ImPlatform/ImPlatform/ImPlatform.h");
        }

        [Configure()]
        public void ConfigureDemo(Configuration conf, DearTarget target)
        {
            conf.Output = Configuration.OutputType.Exe;
            conf.AddPrivateDependency<APIProject>(target);
			conf.TargetPath = RootPath + "/WorkingDir";

            conf.IncludePaths.Add(@"[project.RootPath]/src/demo/");
            conf.IncludePaths.Add(@"[project.RootPath]/extern/stb/");
            conf.IncludePaths.Add(@"[project.RootPath]/extern/imgui/");
            conf.IncludePaths.Add(@"[project.RootPath]/extern/imgui/examples/libs/glfw/include/");
            conf.LibraryPaths.Add(@"[project.RootPath]/extern/imgui/examples/libs/glfw/");
            conf.IncludePaths.Add(@"[project.RootPath]/extern/imgui/backends/");
            conf.IncludePaths.Add(@"[project.RootPath]/extern/ImPlatform/ImPlatform/");
            //conf.IncludePrivatePaths.Add(@"[project.RootPath]\codebase\SpanMultipleSrcDirs\dir_individual_files");
            //conf.AddPrivateDependency<CoreProject>(target);

            if (target.Api == TargetAPI.D3D9)
            {
                conf.LibraryFiles.Add("opengl32.lib");
            }
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
            if (target.Api == TargetAPI.D3D11)
            {
                conf.LibraryFiles.Add("d3d12.lib");
                conf.LibraryFiles.Add("d3dcompiler.lib");
                conf.LibraryFiles.Add("dxgi.lib");
            }

			conf.VcxprojUserFile = new Configuration.VcxprojUserFileSettings();
			conf.VcxprojUserFile.LocalDebuggerWorkingDirectory = @"[project.RootPath]/WorkingDir/";
			//@"$(SolutionDir)WorkingDir";
        }
    }
}
