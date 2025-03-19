for %%G in (%cd%\shaders\hlsl_src\*) do .\bin\slang\bin\slangc.exe -lang hlsl -profile sm_5_1 %%G -stage pixel -entry main_ps -o shaders\glsl\%%~nG.glsl
