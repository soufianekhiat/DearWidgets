@echo off
REM Generate the OpenGL GLSL pixel shaders from the HLSL sources.
REM
REM slangc targets Vulkan-GLSL, which declares a texture and its sampler as two
REM separate objects (`uniform texture2D` + `uniform sampler`). Desktop OpenGL
REM has no such types, so the post-process step below folds them back into
REM combined samplers -- without it every one of these shaders fails to compile
REM at runtime with "'xxx_0' : syntax error".
for %%G in (%cd%\shaders\hlsl_src\*) do .\bin\slang\bin\slangc.exe -lang hlsl -profile sm_5_1 %%G -stage pixel -entry main_ps -o shaders\glsl\%%~nG.glsl

python glsl_combine_samplers.py shaders\glsl
