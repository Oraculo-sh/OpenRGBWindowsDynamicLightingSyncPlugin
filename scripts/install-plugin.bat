@echo off

:: Encontra o diretùrio de plugins do OpenRGB
set "OPENRGB_PLUGIN_DIR=%APPDATA%\OpenRGB\plugins"

:: Verifica se o diretùrio de plugins existe
if not exist "%OPENRGB_PLUGIN_DIR%" (
    echo Diretorio de plugins do OpenRGB nao encontrado em %OPENRGB_PLUGIN_DIR%
    exit /b 1
)

:: Copia a DLL do plugin para o diretùrio de plugins do OpenRGB
set "DLL_TO_COPY="
for %%f in ("C:\Github\WindowsDynamicLightingSync\release\OpenRGBWindowsDynamicLightingSyncPlugin_*.dll") do (
    set "DLL_TO_COPY=%%f"
)

if not defined DLL_TO_COPY (
    echo ERRO: Nao foi possivel encontrar o arquivo DLL do plugin no diretorio de release.
    exit /b 1
)

copy "%DLL_TO_COPY%" "%OPENRGB_PLUGIN_DIR%"

echo Plugin instalado com sucesso!