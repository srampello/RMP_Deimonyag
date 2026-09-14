@echo off
setlocal
cd /d "%~dp0"

if exist "dist\Deimonyag_Control_Telemetry.exe" (
    start "" "dist\Deimonyag_Control_Telemetry.exe"
    exit /b 0
)

if exist ".venv\Scripts\pythonw.exe" (
    start "" ".venv\Scripts\pythonw.exe" "app.py"
    exit /b 0
)

echo No se encontro el ejecutable ni el entorno virtual.
echo.
echo Ejecuta primero:
echo   build_windows.bat

echo O instala las dependencias y ejecuta app.py.
pause
exit /b 1
