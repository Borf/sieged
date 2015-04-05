cd %~DP0
for /r %%i in (assets\models\*.fbx) do modelconvert\bin\ModelConvert.exe "%%i" "%%i.json"
cd vs2013
Release\Sieged.exe