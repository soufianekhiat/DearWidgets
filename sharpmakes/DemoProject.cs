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

            SourceFiles.Add("[project.ExternPath]/glad/src/glad.c");
            SourceFiles.Add("[project.ExternPath]/imgui/imgui.cpp");
            SourceFiles.Add("[project.ExternPath]/imgui/imgui_tables.cpp");
            SourceFiles.Add("[project.ExternPath]/imgui/imgui_widgets.cpp");
            SourceFiles.Add("[project.ExternPath]/imgui/misc/cpp/imgui_stdlib.cpp");
            SourceFiles.Add("[project.ExternPath]/imgui/imgui_draw.cpp");

            // Mains
            SourceFiles.Add(@"[project.RootPath]/src/demo/main_dx11.cpp");
            SourceFiles.Add(@"[project.RootPath]/src/demo/main_dx12.cpp");

            // Backend
            SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_win32.h");
            SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_win32.cpp");

            SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_dx11.h");
            SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_dx11.cpp");

            SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_dx12.h");
            SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_dx12.cpp");

            SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_opengl3.h");
            SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_opengl3.cpp");

            SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_vulkan.h");
            SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_vulkan.cpp");

            SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_wgpu.h");
            SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_wgpu.cpp");
        }

        [Configure()]
        public void ConfigureDemo(Configuration conf, DearTarget target)
        {
            conf.Output = Configuration.OutputType.Exe;
            conf.AddPrivateDependency<APIProject>(target);
			conf.TargetPath = RootPath + "/WorkingDir";

            conf.IncludePaths.Add(@"[project.RootPath]/src/demo/");
            conf.IncludePaths.Add(@"[project.RootPath]/extern/imgui/backends/");
            //conf.IncludePrivatePaths.Add(@"[project.RootPath]\codebase\SpanMultipleSrcDirs\dir_individual_files");
            //conf.AddPrivateDependency<CoreProject>(target);

            //SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_opengl3.h");
            //SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_opengl3.cpp");
            //SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_glfw.h");
            //SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_glfw.cpp");

            if (target.Api == TargetAPI.D3D11)
            {
                conf.LibraryFiles.Add("d3d11.lib");
                conf.LibraryFiles.Add("d3dcompiler.lib");
                conf.LibraryFiles.Add("dxgi.lib");
            }
            //if (target.Build == BuildType.DemoOnly)
            //{
            //    if (target.Api == TargetAPI.D3D11)
            //    {
            //        SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_dx11.h");
            //        SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_dx11.cpp");
            //    }

            //    if (target.Api == TargetAPI.D3D12)
            //    {
            //        SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_dx12.h");
            //        SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_dx12.cpp");
            //    }

            //    if (target.Api == TargetAPI.OpenGL3)
            //    {
            //        SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_opengl3.h");
            //        SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_opengl3.cpp");
            //    }

            //    if (target.Api == TargetAPI.Vulkan)
            //    {
            //        SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_vulkan.h");
            //        SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_vulkan.cpp");
            //    }

            //    if (target.Api == TargetAPI.WGPU)
            //    {
            //        SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_wgpu.h");
            //        SourceFiles.Add("[project.ExternPath]/imgui/backends/imgui_impl_wgpu.cpp");
            //    }
            //}

			conf.VcxprojUserFile = new Configuration.VcxprojUserFileSettings();
			conf.VcxprojUserFile.LocalDebuggerWorkingDirectory = @"[project.RootPath]/WorkingDir/";
			//@"$(SolutionDir)WorkingDir";
        }
    }
}
