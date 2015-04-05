@echo off

set START=%time%
echo %time%
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\vcvars32.bat"
msbuild /t:rebuild /maxcpucount:8 /nologo /m vs2013\Sieged.sln /p:Configuration=Release
taskkill /f /im msbuild.exe
echo %START% - %time%

:START



rmdir /q /s bin
mkdir bin
xcopy /s /y vs2013\release\Sieged.exe bin\

xcopy /s /y blib\assets\*.* bin\assets\
xcopy /s /y assets\*.* bin\assets\


del Sieged.rar
cd bin
"c:\Program Files\WinRAR\Rar.exe" -r a ..\Sieged.rar *
cd ..
rmdir /q /s bin
xcopy /y Sieged.rar w:\dump\
pause