@echo off

set START=%time%
echo %time%
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\vcvars32.bat"
msbuild /t:rebuild /maxcpucount:8 /nologo /m vs2013\Sieged.sln /p:Configuration=Release
taskkill /f /im msbuild.exe
echo %START% - %time%

:START


REM echo set DIR=%%CD%% > copygame.bat
REM echo cd "games\%%1\%%1\bin\x86\Release\" >> copygame.bat
REM echo ..\..\..\..\..\..\GamePlayable\bin\Release\GamePlayable.exe "%%1" >> copygame.bat
REM echo if %%ERRORLEVEL%%==0 GOTO YAY >> copygame.bat
REM echo cd %%DIR%% >> copygame.bat
REM echo GOTO END >> copygame.bat
REM echo :YAY >> copygame.bat
REM echo cd %%DIR%% >> copygame.bat
REM echo xcopy /y "games\%%1\%%1\bin\x86\Release\%%1.dll" patg\games >> copygame.bat
REM echo xcopy /s /y "games\%%1\%%1\bin\x86\Release\Content\*" patg\Content >> copygame.bat
REM echo :END >> copygame.bat

rmdir /q /s bin
mkdir bin
xcopy /s /y vs2013\release\Sieged.exe bin\

xcopy /s /y blib\assets\*.* bin\assets\
xcopy /s /y assets\*.* bin\assets\
mkdir patg\games



del Sieged.rar
cd bin
"c:\Program Files\WinRAR\Rar.exe" -r a ..\Sieged.rar *
cd ..
rmdir /q /s bin
xcopy /y Sieged.rar w:\dump\
pause