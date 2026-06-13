@echo off
setlocal

rem install_l4n_hlae.bat - L4N + HLAE Bridge Installer
rem Developer: sparkling  Version: 1.3

set "GAME_ROOT=%cd%"
if exist "%GAME_ROOT%\left4dead2.exe" goto root_found
cd ..
set "GAME_ROOT=%cd%"
if exist "%GAME_ROOT%\left4dead2.exe" goto root_found

echo [ERROR] left4dead2.exe not found!
echo Put this script inside the Left 4 Dead 2 game folder.
echo Steam -^> Library -^> right-click L4D2 -^> Browse local files
pauseexit /b 1

:root_found
cd /d "%GAME_ROOT%"
echo Game root: %cd%

set "HLAE_DIR="
for /d %%d in (*hlae* *HLAE* *Hlae* *HL*) do (
    if exist "%%d\AfxHookSource.dll" if exist "%%d\AfxCppCli.dll" set "HLAE_DIR=%%d"
)
if "%HLAE_DIR%"==" " set "HLAE_DIR="
if defined HLAE_DIR goto hlae_found

echo [ERROR] HLAE folder not found!
echo Download from https://github.com/advancedfx/advancedfx/releases
echo Extract to a folder with "hlae" in its name, e.g. hlae_2_189_7\
echo Make sure AfxHookSource.dll exists inside.
pauseexit /b 1

:hlae_found
echo HLAE folder: %HLAE_DIR%
echo ============================================
echo   L4N + HLAE Bridge - Installer
echo ============================================

if not exist "bin\left4neko.dll" echo [WARNING] left4neko.dll not found - L4N may not be installed.

echo.
echo [1/3] Copying HLAE DLLs...
set "COPIED=0"
for %%f in ("%HLAE_DIR%\*.dll") do (copy /y "%%f" ".\" >nul 2>&1 & set /a COPIED=COPIED+1)
echo   Copied %COPIED% DLLs from %HLAE_DIR%.

echo [2/3] Setting up DXVK proxy...
set "DXVK_OK=0"
if exist "bin\dxvk_d3d9_real.dll" set "DXVK_OK=1"
if not exist "bin\dxvk_d3d9_real.dll" if exist "bin\dxvk_d3d9.dll" (
    echo   Backing up: bin\dxvk_d3d9.dll -^> bin\dxvk_d3d9_real.dll
    move /y "bin\dxvk_d3d9.dll" "bin\dxvk_d3d9_real.dll" >nul
    set "DXVK_OK=1"
)
if %DXVK_OK%==0 echo   [WARNING] bin\dxvk_d3d9.dll not found. Is DXVK installed?

if exist "bin\dxvk_d3d9_real.dll" (
    if exist "bin\dxvk_d3d9.dll" copy /y "bin\dxvk_d3d9.dll" "bin\dxvk_d3d9.dll.bak" >nul 2>&1
    copy /y "%~dp0bin\dxvk_d3d9.dll" "bin\dxvk_d3d9.dll" >nul
    echo   DXVK proxy deployed. Original: bin\dxvk_d3d9_real.dll
)

echo [3/3] Copying launcher files...
copy /y "%~dp0l4n_hlae_launcher.exe" ".\" >nul
copy /y "%~dp0eat_hook.dll" ".\" >nul
copy /y "%~dp0launch_l4n_hlae.bat" ".\" >nul
copy /y "%~dp0uninstall_l4n_hlae.bat" ".\" >nul
echo   Done.

echo.
echo ============================================
echo   Installation complete!
echo   Launch: double-click launch_l4n_hlae.bat
echo   Verify: in game console type "mirv_input"
echo   VAC WARNING: Use only for demos / single player!
echo ============================================
pause

