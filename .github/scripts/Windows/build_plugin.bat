for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do call "%%i\Common7\Tools\VsDevCmd.bat" -arch=%1
qmake
nmake
nmake install
