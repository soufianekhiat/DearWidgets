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
            //AddTargets(new DearTarget(Platform.linux,// | Platform.mac,
            //                          TargetAPI.OpenGL3,
            //                          Optimization.Debug | Optimization.Release,
            //                          BuildType.APIOnly | BuildType.DemoOnly | BuildType.Full));

            AddTargets(new DearTarget(Platform.win64,// | Platform.mac,
                                      TargetAPI.OpenGL3 | TargetAPI.D3D9 | TargetAPI.D3D10 | TargetAPI.D3D12,
                                      Optimization.Debug | Optimization.Release,
                                      BuildType.APIOnly | BuildType.DemoOnly | BuildType.Full));
        }

        [Configure()]
        public void ConfigureAll(Configuration conf, DearTarget target)
        {
            conf.Name = "[target.Api]_[target.Optimization]";

            //conf.SolutionFileName = "[solution.Name]_[target.Build]_[target.Platform]_[target.Api]";
            conf.SolutionFileName = "[solution.Name]_[target.Build]";

            conf.SolutionPath = SolutionRootPath;

			if (target.Build == BuildType.APIOnly || target.Build == BuildType.Full)
			{
				conf.AddProject< APIProject >(target);
			}
			if (target.Build == BuildType.DemoOnly || target.Build == BuildType.Full)
			{
				conf.AddProject< DemoProject >(target);
			}
		}
    }

    public class Main
    {
        [Sharpmake.Main]
        public static void SharpmakeMain(Sharpmake.Arguments arguments)
        {
            Sharpmake.KitsRootPaths.SetUseKitsRootForDevEnv(DevEnv.vs2022, KitsRootEnum.KitsRoot10, Sharpmake.Options.Vc.General.WindowsTargetPlatformVersion.Latest);

            arguments.Generate<DearWidgetsSolution>();
        }
    }
}

