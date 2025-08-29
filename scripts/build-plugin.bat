@echo off
setlocal enabledelayedexpansion

echo Building Windows Dynamic Lighting Sync Plugin...
echo.

REM Set up paths
set PROJECT_DIR=%~dp0..
set SRC_DIR=%PROJECT_DIR%\src
set RELEASE_DIR=%PROJECT_DIR%\release

REM Configure Visual Studio Build Tools Environment
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
    echo Visual Studio 2019 BuildTools environment configured.
) else (
    echo ERROR: Visual Studio 2019 BuildTools vcvars64.bat not found.
    pause
    exit /b 1
)

REM Verify MSBuild is available
where msbuild >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: MSBuild not found after environment setup.
    pause
    exit /b 1
)
echo MSBuild verified in environment.

REM Configure Qt 5.15.0 (using PATH)
where qmake >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: qmake not found in PATH. Please ensure Qt 5.15.0 is in PATH.
    pause
    exit /b 1
)

REM Verify Qt version
echo Checking Qt version...
qmake -version
echo Qt 5.15.0 configured from PATH.

REM Navigate to source directory
cd /d "%SRC_DIR%"
if %ERRORLEVEL% neq 0 (
    echo ERROR: Could not navigate to %SRC_DIR%
    pause
    exit /b 1
)

echo Current directory: %CD%
echo.

REM Clean previous build
echo Cleaning previous build...
if exist Makefile nmake clean >nul 2>&1
if exist Makefile.Release nmake -f Makefile.Release clean >nul 2>&1
if exist Makefile.Debug nmake -f Makefile.Debug clean >nul 2>&1

REM Configure with qmake
echo Configuring with qmake...
qmake WindowsDynamicLightingSync.pro -spec win32-msvc
if %ERRORLEVEL% neq 0 (
    echo ERROR: qmake failed!
    pause
    exit /b 1
)

REM Build with nmake (from Visual Studio environment)
echo Building with nmake...
nmake
if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo Build completed successfully!

REM Create release directory
if not exist "%RELEASE_DIR%" mkdir "%RELEASE_DIR%"

REM Copy DLL
if exist "release\WindowsDynamicLightingSync.dll" (
    copy "release\WindowsDynamicLightingSync.dll" "%RELEASE_DIR%\" >nul
    echo Plugin copied to %RELEASE_DIR%
) else if exist "debug\WindowsDynamicLightingSync.dll" (
    copy "debug\WindowsDynamicLightingSync.dll" "%RELEASE_DIR%\" >nul
    echo Plugin (debug) copied to %RELEASE_DIR%
) else (
    echo WARNING: Plugin DLL not found
)

echo.
echo Build process completed!
pause