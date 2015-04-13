cd %~DP0
for /r %%i in (assets\models\*.fbx) do vs2013\release\ModelConvert.exe "%%i" "%%i.json"
for /r %%i in (assets\models\*.dae) do vs2013\release\ModelConvert.exe "%%i" "%%i.json"
for /r %%i in (assets\models\*.obj) do vs2013\release\ModelConvert.exe "%%i" "%%i.json"
pause