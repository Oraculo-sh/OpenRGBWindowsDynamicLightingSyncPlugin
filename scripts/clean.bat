@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Windows Dynamic Lighting Sync Plugin
echo Clean Script
echo ========================================
echo.

REM Paths
set PROJECT_ROOT=%~dp0..
set SRC_DIR=%PROJECT_ROOT%\src
set RELEASE_DIR=%PROJECT_ROOT%\release

echo Project Root: %PROJECT_ROOT%
echo Source Directory: %SRC_DIR%
echo Release Directory: %RELEASE_DIR%
echo.

echo Cleaning build artifacts...

REM Change to source directory
cd /d "%SRC_DIR%"
if errorlevel 1 (
    echo ERROR: Failed to change to source directory
    pause
    exit /b 1
)

REM Clean Qt build files
echo Removing Qt build files...
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

REM Clean build directories
echo Removing build directories...
if exist release (
    rmdir /s /q release 2>nul
    echo - Removed release directory
)
if exist debug (
    rmdir /s /q debug 2>nul
    echo - Removed debug directory
)
if exist _intermediate_release (
    rmdir /s /q _intermediate_release 2>nul
    echo - Removed _intermediate_release directory
)
if exist _intermediate_debug (
    rmdir /s /q _intermediate_debug 2>nul
    echo - Removed _intermediate_debug directory
)
if exist _intermediate_* (
    rmdir /s /q _intermediate_* 2>nul
    echo - Removed other intermediate directories
)

REM Clean object files
echo Removing object files...
if exist *.obj (
    del /q *.obj 2>nul
    echo - Removed .obj files
)
if exist *.o (
    del /q *.o 2>nul
    echo - Removed .o files
)

REM Clean temporary files
echo Removing temporary files...
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
echo Removing Visual Studio files...
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

REM Clean moc files
echo Removing Qt moc files...
if exist moc_*.cpp (
    del /q moc_*.cpp 2>nul
    echo - Removed moc_*.cpp files
)
if exist moc_*.h (
    del /q moc_*.h 2>nul
    echo - Removed moc_*.h files
)

REM Clean ui files
echo Removing Qt ui files...
if exist ui_*.h (
    del /q ui_*.h 2>nul
    echo - Removed ui_*.h files
)

REM Clean qrc files
echo Removing Qt resource files...
if exist qrc_*.cpp (
    del /q qrc_*.cpp 2>nul
    echo - Removed qrc_*.cpp files
)

REM Clean qmake cache
echo Removing qmake cache files...
if exist .qmake.stash (
    del /q .qmake.stash 2>nul
    echo - Removed .qmake.stash
)

REM Optional: Clean release directory
echo.
set /p CLEAN_RELEASE="Do you want to clean the release directory? (y/N): "
if /i "%CLEAN_RELEASE%"=="y" (
    if exist "%RELEASE_DIR%\*.dll" (
        del /q "%RELEASE_DIR%\*.dll" 2>nul
        echo - Removed DLL files from release directory
    )
    if exist "%RELEASE_DIR%\*.exe" (
        del /q "%RELEASE_DIR%\*.exe" 2>nul
        echo - Removed EXE files from release directory
    )
)

echo.
echo ========================================
echo CLEAN COMPLETED!
echo ========================================
echo All build artifacts have been removed.
echo You can now run build.bat for a fresh build.
echo.
pause