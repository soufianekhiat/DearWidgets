using System;
using Sharpmake;
using Microsoft.Win32;
using System.IO;
using System.Collections.Generic;

namespace DearWidgets
{
    static class Extern
    {
        static public string RootPath = @"[project.SharpmakeCsPath]";
        static public string ExternPath = RootPath + @"\extern";
    }

    [Fragment, Flags]
    public enum TargetAPI
    {
        D3D9 = (1 << 0),
        D3D10 = (1 << 1),
        D3D11 = (1 << 2),
        D3D12 = (1 << 3),
        Vulkan = (1 << 4),
        OpenGL2 = (1 << 5),
        OpenGL3 = (1 << 6),
        Metal = (1 << 7),
        WGPU = (1 << 8),
    }

    [Fragment, Flags]
    public enum BuildType
    {
        APIOnly = (1 << 0),
        DemoOnly = (1 << 1),
        Full = (1 << 2)
    }

    public class DearTarget : Target
    {
        public DearTarget()
            : base()
        { }

        public DearTarget(Platform platform, TargetAPI api, Optimization optimization, BuildType buildType)
            : base(platform, DevEnv.vs2022, optimization)
        {
            Api = api;
            Build = buildType;
        }

        public TargetAPI Api;
        public BuildType Build;
    }

    public class CommonSolution : Sharpmake.Solution
    {
        public string SolutionRootPath = @"[solution.SharpmakeCsPath]\..";

        public CommonSolution()
            : base( typeof( DearTarget ) )
        { }
    }

    public class CommonProject : Sharpmake.Project
	{
		public string ProjectRootPath	= @"[project.SharpmakeCsPath]\..";
		public string TmpPath			= @"[project.RootPath]\tmp";
		public string ExternPath		= @"[project.RootPath]\extern";
		public string ProjectsPath		= @"[project.RootPath]\projects";
		public string OutputPath		= @"[project.RootPath]\WorkingDir";

		public CommonProject()
			: base( typeof( DearTarget ) )
		{	
			RootPath = ProjectRootPath;
			CustomProperties.Add( "VcpkgEnabled", "false" );

			AddTargets(new DearTarget(	Platform.win64 | Platform.linux,// | Platform.mac,
										TargetAPI.D3D9 | TargetAPI.D3D10 | TargetAPI.OpenGL3,
										Optimization.Debug | Optimization.Release,
										BuildType.APIOnly | BuildType.DemoOnly | BuildType.Full ) );
		}

		[Configure()]
		public virtual void ConfigureAll(Configuration conf, DearTarget target)
		{
			AddPlatformSourceExclusion(conf, target);

			conf.Name				= "[target.Api]_[target.Optimization]";
			conf.ProjectFileName	= "[project.Name]_[target.DevEnv]_[target.Platform]";
			conf.ProjectPath		= ProjectsPath + @"\[target.Platform]\[target.Build]\[project.Name]";
			conf.IntermediatePath	= @"[project.TmpPath]\[target.Api]\[target.Platform]\[project.Name]\[target.Optimization]";
			conf.TargetPath			= @"[project.OutputPath]";
			conf.TargetLibraryPath	= @"[project.RootPath]\tmp\lib\[target.Platform]_[target.Api]_[target.Optimization]";

			conf.IncludePaths.Add(@"[project.RootPath]\src\api");
			conf.IncludePaths.Add(@"[project.RootPath]\");
			conf.IncludePaths.Add(@"[project.ExternPath]");

			conf.Defines.Add("ImDrawIdx=ImU32");
			conf.Defines.Add("IMGUI_DEFINE_MATH_OPERATORS");
			conf.Defines.Add("IMGUI_DISABLE_OBSOLETE_FUNCTIONS");
            //conf.IncludePaths.Add(@"[project.ExternPath]/imgui/");
            conf.IncludePaths.Add(@"[project.RootPath]/extern/imgui/");
			conf.IncludePaths.Add(@"[project.ExternPath]/glad/include");

			//conf.Defines.Add("_CRT_SECURE_NO_WARNINGS");
			conf.Options.Add(Sharpmake.Options.Vc.Compiler.Inline.Disable);
			conf.Options.Add(Sharpmake.Options.Vc.Compiler.CppLanguageStandard.Latest);
			conf.Options.Add(Sharpmake.Options.Vc.Compiler.CompileAsWinRT.Disable);
			conf.Options.Add(Sharpmake.Options.Vc.Compiler.RTTI.Disable);
			conf.Options.Add(Sharpmake.Options.Vc.Compiler.FloatingPointExceptions.Disable);
			conf.Options.Add(Sharpmake.Options.Vc.Compiler.FloatingPointModel.Fast);
			conf.Options.Add(Sharpmake.Options.Vc.Compiler.EnhancedInstructionSet.AdvancedVectorExtensions);
			conf.Options.Add(Sharpmake.Options.Vc.Compiler.Exceptions.Enable);
			conf.Options.Add(Sharpmake.Options.Vc.Compiler.Intrinsic.Enable);
            conf.Options.Add(Sharpmake.Options.Vc.Compiler.ConformanceMode.Enable);

			conf.AdditionalCompilerOptions.Add("/Zo");
			conf.Options.Add(new Sharpmake.Options.Vc.Librarian.DisableSpecificWarnings(
				"4221" // This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library
				));

            //conf.Options.Add(new Sharpmake.Options.Vc.Compiler.DisableSpecificWarnings(
            //    //"4100", // unreferenced formal parameter
            //    "4324"  // structure was padded due to alignment specifier
            //    ));

			conf.Defines.Add( "NOMINMAX" );

			if (target.Optimization == Optimization.Debug)
				conf.Defines.Add("__DEAR_DEBUG__");
			else
				conf.Defines.Add("__DEAR_RELEASE__");
			//conf.Defines.Add("ImDrawIdx=unsigned int");
		}

		[Configure(Platform.mac)]
		public virtual void ConfigureMax(Configuration conf, DearTarget target)
		{
			conf.Defines.Add("__DEAR_MAC__");
		}

		[Configure(Platform.linux)]
		public virtual void ConfigureLinux(Configuration conf, DearTarget target)
		{
			conf.Defines.Add("__DEAR_LINUX__");
		}

		[Configure(Platform.win64)]
		public virtual void ConfigureWindows(Configuration conf, DearTarget target)
		{
			conf.Defines.Add("__DEAR_WIN__");
			conf.LibraryFiles.Add("Wtsapi32"); // Enables applications to receive WTS messages from windows
			conf.AdditionalLinkerOptions.Add("/ignore:4098,4099,4217,4221");
			conf.Defines.Add("WINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP");
			conf.Defines.Add("_WIN32_WINNT=0x0600");
			conf.Options.Add(Sharpmake.Options.Vc.Linker.RandomizedBaseAddress.Disable);
			//// GLFW
            //conf.IncludePaths.Add(@"[project.RootPath]/extern/GLFW/include");
			//conf.LibraryPaths.Add(@"[project.ExternPath]/glfw/lib-vc2019/");
			//conf.LibraryFiles.Add("glfw3.lib");
			conf.LibraryFiles.Add("winmm.lib");
			conf.LibraryFiles.Add("comctl32.lib");
			conf.LibraryFiles.Add("msvcrt.lib");
			conf.LibraryFiles.Add("msvcmrt.lib");
		}

        [Configure(Optimization.Debug)]//, ConfigurePriority(2)]
		public virtual void ConfigureDebug(Configuration conf, DearTarget target)
		{
			conf.Options.Add(Sharpmake.Options.Vc.Compiler.Inline.Disable);
		}

        [Configure(Optimization.Release)]//, ConfigurePriority(3)]
		public virtual void ConfigureRelease(Configuration conf, DearTarget target)
		{
			conf.Options.Add(Sharpmake.Options.Vc.Compiler.Optimization.MaximizeSpeed);
			conf.Options.Add(Sharpmake.Options.Vc.General.WholeProgramOptimization.LinkTime);
			conf.Options.Add(Sharpmake.Options.Vc.Linker.LinkTimeCodeGeneration.UseLinkTimeCodeGeneration);
			conf.Options.Add(Sharpmake.Options.Vc.Linker.EnableCOMDATFolding.RemoveRedundantCOMDATs);
			conf.Options.Add(Sharpmake.Options.Vc.Linker.Reference.EliminateUnreferencedData);
		}

		[Configure(TargetAPI.D3D9)]
		public virtual void ConfigureD3D9(Configuration conf, DearTarget target)
		{
			conf.Defines.Add("__DEAR_GFX_DX9__");
        }

		[Configure(TargetAPI.D3D10)]
		public virtual void ConfigureD3D10(Configuration conf, DearTarget target)
		{
			conf.Defines.Add("__DEAR_GFX_DX10__");
        }

		[Configure(TargetAPI.D3D11)]
		public virtual void ConfigureD3D11(Configuration conf, DearTarget target)
		{
			conf.Defines.Add("__DEAR_GFX_DX11__");
        }

		[Configure(TargetAPI.D3D12)]
		public virtual void ConfigureD3D12(Configuration conf, DearTarget target)
		{
			conf.Defines.Add("__DEAR_GFX_DX12__");
        }

		[Configure(TargetAPI.OpenGL2)]
		public virtual void ConfigureOpenGL2(Configuration conf, DearTarget target)
		{
			conf.Defines.Add("__DEAR_GFX_OGL2__");
        }

		[Configure(TargetAPI.OpenGL3)]
		public virtual void ConfigureOpenGL3(Configuration conf, DearTarget target)
		{
			conf.Defines.Add("__DEAR_GFX_OGL3__");
        }

		[Configure(TargetAPI.Vulkan)]
		public virtual void ConfigureVulkan(Configuration conf, DearTarget target)
		{
			conf.Defines.Add("__DEAR_GFX_VULKAN__");
        }

		[Configure(TargetAPI.Metal)]
		public virtual void ConfigureMetal(Configuration conf, DearTarget target)
		{
			conf.Defines.Add("__DEAR_METAL__");
        }

		[Configure(TargetAPI.WGPU)]
		public virtual void ConfigureWGPU(Configuration conf, DearTarget target)
		{
			conf.Defines.Add("__DEAR_WGPU__");
        }

		void AddPlatformSourceExclusion(Configuration conf, DearTarget target)
		{
			var excludedFileSuffixes	= new List< string >();
			var excludedFolders			= new List< string >();
            
            if (target.Api != TargetAPI.D3D12)
            {
                excludedFileSuffixes.Add("dx12");
                //excludedFolders.Add("dx12");
            }

            if (target.Api != TargetAPI.Vulkan)
            {
                excludedFileSuffixes.Add("vulkan");
                //excludedFolders.Add("vulkan");
            }

            if (target.Api != TargetAPI.D3D11)
            {
                excludedFileSuffixes.Add("dx11");
                //excludedFolders.Add("dx11");
            }

            if (target.Api != TargetAPI.OpenGL3)
            {
                excludedFileSuffixes.Add("opengl3");
                //excludedFolders.Add("opengl3");
            }

            if (target.Api != TargetAPI.WGPU)
            {
                excludedFileSuffixes.Add("wgpu");
                //excludedFolders.Add("wgpu");
            }

            conf.SourceFilesBuildExcludeRegex.Add(@"\.*_(" + string.Join("|", excludedFileSuffixes.ToArray()) + @")\.cpp$");
            //conf.SourceFilesBuildExcludeRegex.Add(@"\.*_(" + string.Join("|", excludedFileSuffixes.ToArray()) + @")\.h$");
            //conf.SourceFilesBuildExcludeRegex.Add(@"\.*\\(" + string.Join("|", excludedFolders.ToArray()) + @")\\");
        }

		[Configure(OutputType.Lib)]
		public virtual void OutputTypeLib(Configuration conf, DearTarget target)
		{
			if (target.Platform == Platform.win64)
			{
				if (target.Optimization == Optimization.Debug)
					conf.Options.Add(Sharpmake.Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDebug);
				else
					conf.Options.Add(Sharpmake.Options.Vc.Compiler.RuntimeLibrary.MultiThreaded);
			}

			if (target.Platform == Platform.win64)
				//conf.Options.Add(Sharpmake.Options.Vc.General.NativeEnvironment.Enable);
				conf.Options.Add(Options.Vc.General.PreferredToolArchitecture.x64);
		}
	}
}
