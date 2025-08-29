@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Windows Dynamic Lighting Sync Plugin
echo Build Script (Windows 64-bit)
echo ========================================
echo.

REM Set up paths
set PROJECT_DIR=%~dp0..
set SRC_DIR=%PROJECT_DIR%\src
set SCRIPTS_DIR=%PROJECT_DIR%\scripts
set RELEASE_DIR=%PROJECT_DIR%\release

echo Project Directory: %PROJECT_DIR%
echo Source Directory: %SRC_DIR%
echo Scripts Directory: %SCRIPTS_DIR%
echo Release Directory: %RELEASE_DIR%
echo.

REM Execute clean script first
echo ========================================
echo STEP 1: CLEANING PREVIOUS BUILD
echo ========================================
call "%SCRIPTS_DIR%\clean.bat"
if %ERRORLEVEL% neq 0 (
    echo ERROR: Clean script failed!
    exit /b 1
)
echo Clean completed successfully.
echo.

REM Configure Visual Studio Build Tools Environment (64-bit)
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

REM Verify MSBuild is available
where msbuild >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: MSBuild not found after setup.
    exit /b 1
)
echo MSBuild verified.

REM Configure Qt 5.15.0 (using PATH)
where qmake >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: qmake not found in PATH.
    echo Please ensure Qt 5.15.0 is properly installed and in PATH.
    exit /b 1
)

REM Verify Qt version
echo Checking Qt version...
qmake -version
echo Qt 5.15.0 configured from PATH.
echo.

REM Navigate to source directory
echo ========================================
echo STEP 3: CONFIGURING PROJECT
echo ========================================
cd /d "%SRC_DIR%"
if %ERRORLEVEL% neq 0 (
    echo ERROR: Could not navigate to %SRC_DIR%
    exit /b 1
)

echo Current directory: %CD%
echo.

REM Configure with qmake for Windows 64-bit
echo Configuring project with qmake (Windows 64-bit)...
qmake WindowsDynamicLightingSync.pro -spec win32-msvc "CONFIG+=release"
if %ERRORLEVEL% neq 0 (
    echo ERROR: qmake configuration failed!
    exit /b 1
)
echo Project configured successfully.
echo.

REM Build with nmake
echo ========================================
echo STEP 4: COMPILING PLUGIN
echo ========================================
echo Building with nmake...
nmake
if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed!
    exit /b 1
)
echo Build completed successfully!
echo.

REM Verify DLL was created
echo ========================================
echo STEP 5: VERIFYING BUILD OUTPUT
echo ========================================
if exist "%RELEASE_DIR%\WindowsDynamicLightingSync.dll" (
    echo Plugin DLL found: %RELEASE_DIR%\WindowsDynamicLightingSync.dll
    
    REM Get file size
    for %%A in ("%RELEASE_DIR%\WindowsDynamicLightingSync.dll") do (
        echo File size: %%~zA bytes
    )
    
    REM Get file timestamp
    for %%A in ("%RELEASE_DIR%\WindowsDynamicLightingSync.dll") do (
        echo Last modified: %%~tA
    )
    
    echo.
    echo ========================================
    echo BUILD SUCCESSFUL!
    echo ========================================
    echo Plugin compiled successfully for Windows 64-bit
    echo Location: %RELEASE_DIR%\WindowsDynamicLightingSync.dll
    echo.
    echo To install the plugin:
    echo 1. Copy the DLL to your OpenRGB plugins directory
    echo 2. Restart OpenRGB
    echo 3. Check Settings ^> Plugins to verify it loaded
    echo ========================================
    
) else (
    echo ERROR: Plugin DLL was not created!
    echo Expected location: %RELEASE_DIR%\WindowsDynamicLightingSync.dll
    echo.
    echo Checking for DLL in other locations...
    if exist "release\WindowsDynamicLightingSync.dll" (
        echo Found DLL in local release directory: release\WindowsDynamicLightingSync.dll
        echo Moving to project release directory...
        if not exist "%RELEASE_DIR%" mkdir "%RELEASE_DIR%"
        copy "release\WindowsDynamicLightingSync.dll" "%RELEASE_DIR%\" >nul
        if exist "%RELEASE_DIR%\WindowsDynamicLightingSync.dll" (
            echo Plugin successfully moved to: %RELEASE_DIR%\WindowsDynamicLightingSync.dll
        ) else (
            echo ERROR: Failed to move plugin to release directory
            exit /b 1
        )
    ) else (
        echo No DLL found in any expected location
        exit /b 1
    )
)

echo.
echo Build process completed successfully!
echo ========================================
pause