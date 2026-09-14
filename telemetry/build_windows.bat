@echo off
setlocal
cd /d "%~dp0"

echo ==============================================
echo  Deimonyag - Compilar ejecutable para Windows
echo ==============================================
echo.

where python >nul 2>nul
if errorlevel 1 (
    echo ERROR: Python no esta disponible en PATH.
    echo Instala Python 3.11 o 3.12 y vuelve a intentar.
    pause
    exit /b 1
)

if not exist ".venv\Scripts\python.exe" (
    echo Creando entorno virtual...
    python -m venv .venv
    if errorlevel 1 goto :error
)

call ".venv\Scripts\activate.bat"

python -m pip install --upgrade pip
if errorlevel 1 goto :error

python -m pip install -r requirements.txt pyinstaller
if errorlevel 1 goto :error

echo.
echo Compilando Deimonyag_Control_Telemetry.exe...
python -m PyInstaller --noconfirm --clean --onefile --windowed --name Deimonyag_Control_Telemetry --collect-all pyqtgraph app.py
if errorlevel 1 goto :error

echo.
echo ==============================================
echo  COMPILACION FINALIZADA

echo  Archivo:
echo  %CD%\dist\Deimonyag_Control_Telemetry.exe
echo.
echo  Los CSV se guardaran en una carpeta logs junto al ejecutable.
echo ==============================================
pause
exit /b 0

:error
echo.
echo ERROR: no se pudo completar la compilacion.
pause
exit /b 1
