using Sharpmake;

[module: Sharpmake.Include("common.cs")]

[module: Sharpmake.Include("APIProject.cs")]
[module: Sharpmake.Include("DemoProject.cs")]

namespace DearWidgets
{
    [Sharpmake.Generate]
    public class DearWidgetsSolution : CommonSolution
    {
        public DearWidgetsSolution()
        {
            Name = "DearWidgets";
            AddTargets(new DearTarget(Platform.win64,
                                      TargetAPI.D3D12 | TargetAPI.D3D11 | TargetAPI.OpenGL3 | TargetAPI.Vulkan | TargetAPI.WGPU,
                                      Optimization.Debug | Optimization.Release,
                                      BuildType.APIOnly | BuildType.DemoOnly | BuildType.Full));
        }

        [Configure()]
        public void ConfigureAll(Configuration conf, DearTarget target)
        {
            conf.Name = "[target.Api]_[target.Optimization]";

            conf.SolutionFileName = "[target.Build]_[target.Api]_[target.Platform]_[solution.Name]";

            conf.SolutionPath = SolutionRootPath;

            if (target.Build == BuildType.APIOnly || target.Build == BuildType.Full)
            {
                conf.AddProject<APIProject>(target);
            }
            if (target.Build == BuildType.DemoOnly || target.Build == BuildType.Full)
            {
                conf.AddProject<DemoProject>(target);
            }
        }
    }

    public class Main
    {
        [Sharpmake.Main]
        public static void SharpmakeMain(Sharpmake.Arguments arguments)
        {
            Sharpmake.KitsRootPaths.SetUseKitsRootForDevEnv(DevEnv.vs2019, KitsRootEnum.KitsRoot10, Sharpmake.Options.Vc.General.WindowsTargetPlatformVersion.Latest);

            arguments.Generate<DearWidgetsSolution>();
        }
    }
}

