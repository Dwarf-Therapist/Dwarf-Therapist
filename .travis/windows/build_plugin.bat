call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=%1
"%2\bin\qmake.exe"
nmake
nmake install
