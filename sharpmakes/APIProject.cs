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
            conf.IncludePaths.Add(@"[project.RootPath]/extern/ImPlatform/ImPlatform/");

            // HarfBuzz text shaping backend — internal validation only; remove before shipping.
            // External users who want HarfBuzz must set it up themselves (see dear_widgets_text_shape.h).
            conf.Defines.Add("DW_SHAPER_BACKEND_HARFBUZZ");
            conf.IncludePaths.Add(@"[project.RootPath]/extern/harfbuzz/src");
        }

        [Configure(BuildType.Full | BuildType.DemoOnly)]
        public void ConfigureFullBuild(Configuration conf, DearTarget target)
        {
            // For Full and DemoOnly builds, exclude implatform_impl.cpp since the demo provides IMPLATFORM_IMPLEMENTATION
            conf.SourceFilesBuildExclude.Add(@"[project.SourceRootPath]\implatform_impl.cpp");
        }
    }
}
