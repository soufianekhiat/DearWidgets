@ECHO OFF
.\extern\Sharpmake\bootstrap.bat
.\extern\Sharpmake\CompileSharpmake.bat "Sharpmake.sln" "Release" "Any CPU"
xcopy extern\Sharpmake\tmp\bin\release\*.* sharpmakes\bin\*.*



echo GLFW

curl -L -o tmp\glfw.zip https://github.com/glfw/glfw/releases/download/3.3.4/glfw-3.3.4.bin.WIN64.zip
tar -xf tmp\glfw.zip -C ./extern/
cd extern
REM rename glfw-3.3.2.bin.WIN64 glfw
rename glfw-3.3.4.bin.WIN64 glfw
cd ..
xcopy extern\glfw\lib-vc2019\glfw3.dll WorkingDir\glfw3.dll /Y

