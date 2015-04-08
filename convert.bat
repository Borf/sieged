cd %~DP0
for /r %%i in (assets\models\*.fbx) do modelconvert\bin\ModelConvert.exe "%%i" "%%i.json"
for /r %%i in (assets\models\*.dae) do modelconvert\bin\ModelConvert.exe "%%i" "%%i.json"
for /r %%i in (assets\models\*.obj) do modelconvert\bin\ModelConvert.exe "%%i" "%%i.json"
pause