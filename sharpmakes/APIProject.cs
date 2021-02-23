using Sharpmake;

[module: Sharpmake.Include("common.cs")]

namespace DearWidgets
{
    [Sharpmake.Generate]
    public class APIProject : CommonProject
    {
        public APIProject()
        {
            Name = "DearWidgets";
            SourceRootPath = RootPath + @"\src\api";
        }

        [Configure()]
        public void ConfigureAPI(Configuration conf, DearTarget target)
        {
            conf.Output = Configuration.OutputType.Lib;
            //conf.AddPrivateDependency<KernelProject>(target);
            //conf.AddPrivateDependency<CoreProject>(target);

            //conf.Defines.Add("IMGUI_DEFINE_MATH_OPERATORS");
            //conf.IncludePaths.Add(@"[project.RootPath]/extern/Clipper/cpp/");
            //conf.IncludePaths.Add(@"[project.ExternPath]/glad/include");
        }
    }
}
