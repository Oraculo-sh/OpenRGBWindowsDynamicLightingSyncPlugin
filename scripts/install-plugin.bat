@echo off

:: Encontra o diretório de plugins do OpenRGB
set "OPENRGB_PLUGIN_DIR=%APPDATA%\OpenRGB\plugins"

:: Verifica se o diretório de plugins existe
if not exist "%OPENRGB_PLUGIN_DIR%" (
    echo Diretorio de plugins do OpenRGB nao encontrado em %OPENRGB_PLUGIN_DIR%
    exit /b 1
)

:: Copia a DLL do plugin para o diretório de plugins do OpenRGB
copy "C:\Github\WindowsDynamicLightingSync\release\WindowsDynamicLightingSync.dll" "%OPENRGB_PLUGIN_DIR%"

echo Plugin instalado com sucesso!