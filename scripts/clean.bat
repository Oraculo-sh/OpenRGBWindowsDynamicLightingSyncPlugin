@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Windows Dynamic Lighting Sync Plugin
echo Clean Script (Automated)
echo ========================================
echo.

REM Paths
set PROJECT_ROOT=%~dp0..
set SRC_DIR=%PROJECT_ROOT%\src
set BUILD_DIR=%PROJECT_ROOT%\build
set RELEASE_DIR=%PROJECT_ROOT%\release

echo Project Root: %PROJECT_ROOT%
echo Source Directory: %SRC_DIR%
echo Build Directory: %BUILD_DIR%
echo Release Directory: %RELEASE_DIR%
echo.

echo Cleaning build artifacts...

REM Clean build directory (entire folder)
if exist "%BUILD_DIR%" (
    echo Removing build directory...
    rmdir /s /q "%BUILD_DIR%" 2>nul
    if exist "%BUILD_DIR%" (
        echo WARNING: Could not completely remove build directory
    ) else (
        echo - Removed build directory
    )
)

REM Clean release directory (entire folder)
if exist "%RELEASE_DIR%" (
    echo Removing release directory...
    rmdir /s /q "%RELEASE_DIR%" 2>nul
    if exist "%RELEASE_DIR%" (
        echo WARNING: Could not completely remove release directory
    ) else (
        echo - Removed release directory
    )
)

REM Change to source directory for cleaning compilation files
cd /d "%SRC_DIR%"
if errorlevel 1 (
    echo ERROR: Failed to change to source directory
    exit /b 1
)

echo Cleaning compilation files in src directory...

REM Clean Qt build files
if exist Makefile (
    del /q Makefile 2>nul
    echo - Removed Makefile
)
if exist Makefile.Debug (
    del /q Makefile.Debug 2>nul
    echo - Removed Makefile.Debug
)
if exist Makefile.Release (
    del /q Makefile.Release 2>nul
    echo - Removed Makefile.Release
)
if exist *.pro.user (
    del /q *.pro.user 2>nul
    echo - Removed .pro.user files
)

REM Clean local build directories in src
if exist release (
    rmdir /s /q release 2>nul
    echo - Removed local release directory
)
if exist debug (
    rmdir /s /q debug 2>nul
    echo - Removed local debug directory
)
if exist _intermediate_* (
    for /d %%i in (_intermediate_*) do (
        rmdir /s /q "%%i" 2>nul
        echo - Removed %%i directory
    )
)

REM Clean object files
if exist *.obj (
    del /q *.obj 2>nul
    echo - Removed .obj files
)
if exist *.o (
    del /q *.o 2>nul
    echo - Removed .o files
)

REM Clean temporary files
if exist *.tmp (
    del /q *.tmp 2>nul
    echo - Removed .tmp files
)
if exist *.temp (
    del /q *.temp 2>nul
    echo - Removed .temp files
)
if exist *~ (
    del /q *~ 2>nul
    echo - Removed backup files
)

REM Clean Visual Studio files
if exist *.pdb (
    del /q *.pdb 2>nul
    echo - Removed .pdb files
)
if exist *.ilk (
    del /q *.ilk 2>nul
    echo - Removed .ilk files
)
if exist *.exp (
    del /q *.exp 2>nul
    echo - Removed .exp files
)
if exist *.lib (
    del /q *.lib 2>nul
    echo - Removed .lib files
)

REM Clean Qt generated files
if exist moc_*.cpp (
    del /q moc_*.cpp 2>nul
    echo - Removed moc_*.cpp files
)
if exist moc_*.h (
    del /q moc_*.h 2>nul
    echo - Removed moc_*.h files
)
if exist ui_*.h (
    del /q ui_*.h 2>nul
    echo - Removed ui_*.h files
)
if exist qrc_*.cpp (
    del /q qrc_*.cpp 2>nul
    echo - Removed qrc_*.cpp files
)

REM Clean qmake cache
if exist .qmake.stash (
    del /q .qmake.stash 2>nul
    echo - Removed .qmake.stash
)

echo.
echo ========================================
echo CLEAN COMPLETED!
echo ========================================
echo All build artifacts have been removed.
echo Ready for fresh compilation.
echo ========================================