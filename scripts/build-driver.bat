@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Windows Dynamic Lighting Sync - Driver
echo Build Script (Windows 64-bit)
echo ========================================
echo.

REM ------------------------------
REM Paths
REM ------------------------------
set "PROJECT_DIR=%~dp0.."
set "DRIVER_DIR=%PROJECT_DIR%\driver"
set "SCRIPTS_DIR=%PROJECT_DIR%\scripts"
set "BUILD_DIR=%PROJECT_DIR%\build"
set "RELEASE_DIR=%PROJECT_DIR%\release"

echo Project Directory: %PROJECT_DIR%
echo Driver Directory: %DRIVER_DIR%
echo Scripts Directory: %SCRIPTS_DIR%
echo Build Directory: %BUILD_DIR%
echo Release Directory: %RELEASE_DIR%
echo.

REM ------------------------------
REM STEP 1: CLEAN PREVIOUS BUILD
REM ------------------------------
echo ========================================
echo STEP 1: CLEANING PREVIOUS BUILD
echo ========================================
call "%SCRIPTS_DIR%\clean.bat"
if %ERRORLEVEL% neq 0 (
    echo ERROR: Clean script failed!
    exit /b 1
)

echo Performing driver-specific cleanup...
cd /d "%DRIVER_DIR%"
if %ERRORLEVEL% neq 0 (
    echo ERROR: Could not navigate to %DRIVER_DIR%
    exit /b 1
)
if exist Makefile del /q Makefile 2>nul
if exist Makefile.Debug del /q Makefile.Debug 2>nul
if exist Makefile.Release del /q Makefile.Release 2>nul
if exist .qmake.stash del /q .qmake.stash 2>nul
echo Clean completed successfully.
echo.

REM --------------------------------------------------
REM STEP 2: CONFIGURE BUILD ENVIRONMENT (MSVC + Qt)
REM --------------------------------------------------
echo ========================================
echo STEP 2: CONFIGURING BUILD ENVIRONMENT
echo ========================================
set VS_FOUND=0
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
    echo Visual Studio 2019 BuildTools 64-bit configured.
    set VS_FOUND=1
)
if !VS_FOUND!==0 if exist "C:\Program Files\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
    echo Visual Studio 2019 Professional 64-bit configured.
    set VS_FOUND=1
)
if !VS_FOUND!==0 if exist "C:\Program Files\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
    echo Visual Studio 2019 Community 64-bit configured.
    set VS_FOUND=1
)
if !VS_FOUND!==0 (
    echo ERROR: Visual Studio 2019 vcvars64.bat not found.
    echo Please install Visual Studio 2019 with C++ build tools.
    exit /b 1
)

REM Ensure we are in a plain cmd context for the rest of the script
set "CMDCMDLINE=%CMDCMDLINE%"

REM Verify nmake is available
where nmake >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: nmake not found after MSVC setup.
    exit /b 1
)
echo nmake verified.

REM Verify qmake (Qt 5.15.x recommended) is available in PATH
where qmake >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: qmake not found in PATH.
    echo Please ensure Qt 5.15.x is properly installed and in PATH.
    exit /b 1
)
echo Checking Qt version...
qmake -version
echo.

REM --------------------------------------
REM STEP 3: CONFIGURE PROJECT WITH QMAKE
REM --------------------------------------
echo ========================================
echo STEP 3: CONFIGURING PROJECT
echo ========================================
cd /d "%DRIVER_DIR%"
if %ERRORLEVEL% neq 0 (
    echo ERROR: Could not navigate to %DRIVER_DIR%
    exit /b 1
)

echo Current directory: %CD%
echo Configuring project with qmake (Windows 64-bit, release)...
qmake WindowsDynamicLightingDriver.pro -spec win32-msvc "CONFIG+=release"
if %ERRORLEVEL% neq 0 (
    echo ERROR: qmake configuration failed!
    exit /b 1
)
echo Project configured successfully.
echo.

REM ------------------------------
REM STEP 4: BUILD WITH NMAKE
REM ------------------------------
echo ========================================
echo STEP 4: COMPILING DRIVER
echo ========================================
echo Building with nmake...
nmake
if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed!
    exit /b 1
)
echo Build completed successfully!
echo.

REM -----------------------------------------
REM STEP 5: VERIFY BUILD OUTPUT (EXE exists)
REM -----------------------------------------
echo ========================================
echo STEP 5: VERIFYING BUILD OUTPUT
echo ========================================
set "EXE_PATH=%RELEASE_DIR%\WindowsDynamicLightingDriver.exe"

if not exist "%EXE_PATH%" (
    echo Driver EXE not found in release directory, checking fallback locations...
    if exist "%DRIVER_DIR%\release\WindowsDynamicLightingDriver.exe" (
        echo Found in driver\\release, moving to project release directory...
        if not exist "%RELEASE_DIR%" mkdir "%RELEASE_DIR%"
        copy /y "%DRIVER_DIR%\release\WindowsDynamicLightingDriver.exe" "%RELEASE_DIR%\" >nul
        set "EXE_PATH=%RELEASE_DIR%\WindowsDynamicLightingDriver.exe"
    ) else (
        echo ERROR: Driver executable not created or not found!
        echo Expected: %EXE_PATH%
        exit /b 1
    )
)

echo Driver executable found: %EXE_PATH%
for %%A in ("%EXE_PATH%") do (
    echo File size: %%~zA bytes
    echo Last modified: %%~tA
)

echo.
echo ========================================
echo BUILD SUCCESSFUL!
echo ========================================
echo Driver compiled successfully for Windows 64-bit
echo Location: %EXE_PATH%
echo.
echo To run the driver locally:
echo 1. Open a terminal and run:
echo    "%EXE_PATH%"
echo 2. Leave it running to accept protocol connections.
echo 3. Use the plugin/client to connect to the local server.
echo ========================================

echo.
echo Build process completed successfully!
echo ========================================
pause
