for %%S in (glsl msl wgsl wgpu) do (
	echo Generate %%S
	for %%G in (%cd%\shaders\hlsl_src\*) do (
		echo Vextex

		if %%S == msl .\bin\slang\bin\slangc.exe -lang hlsl -profile sm_5_1 %%G -stage vertex -entry main_vs -target metal -o shaders\%%S\%%~nG_vs.%%S
		if not %%S == msl .\bin\slang\bin\slangc.exe -lang hlsl -profile sm_5_1 %%G -stage vertex -entry main_vs -o shaders\%%S\%%~nG_vs.%%S

		echo Pixel

		if %%S == msl .\bin\slang\bin\slangc.exe -lang hlsl -profile sm_5_1 %%G -stage pixel -entry main_ps -target metal -o shaders\%%S\%%~nG_ps.%%S
		if not %%S == msl .\bin\slang\bin\slangc.exe -lang hlsl -profile sm_5_1 %%G -stage pixel -entry main_ps -o shaders\%%S\%%~nG_ps.%%S
	)
)
