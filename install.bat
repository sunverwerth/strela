@echo off
set STRELAPATH=%HOMEDRIVE%%HOMEPATH%\.strela\

if not exist "Release\strela.exe" (
	echo !!! strela.exe does not exist. Please build the Visual Studio project in release mode and rerun this script
	exit /B 1
)

echo ### Creating strela folder in %STRELAPATH%
if not exist "%STRELAPATH%" (
	mkdir %STRELAPATH%
	echo ### Done
) else (
	echo ### Already exists
)
 
echo ### Creating lib folder in %STRELAPATH%
if not exist "%STRELAPATH%\lib" (
	mkdir %STRELAPATH%\lib
	echo ### Done
) else (
	echo ### Already exists
)
 

echo ### Copying strela.exe 
copy Release\strela.exe %STRELAPATH%
echo ### Done

echo ### Copying standard library
xcopy /E /I /Y /F Std %STRELAPATH%\lib\Std
echo ### Done