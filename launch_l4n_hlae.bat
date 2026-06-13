@echo off
setlocal

cd /d "%~dp0"

rem Auto-detect game root (works from game root OR l4n_hlae_bridge\)
if exist "l4n_hlae_launcher.exe" if exist "eat_hook.dll" if exist "AfxHookSource.dll" goto ready
cd ..
if exist "l4n_hlae_launcher.exe" if exist "eat_hook.dll" if exist "AfxHookSource.dll" goto ready

echo [ERROR] Required files not found. Run install_l4n_hlae.bat first.
pause
exit /b 1

:ready
set "EXE_SIZE=0"
for %%A in (left4dead2.exe) do set "EXE_SIZE=%%~zA"
if %EXE_SIZE% lss 500000 echo [WARNING] left4dead2.exe may be original Steam version - L4N may not be active.

echo ============================================
echo   L4N + HLAE Bridge - Launcher
echo   Game root: %cd%
echo ============================================
echo.
echo Launching Left 4 Dead 2...
echo Close this window to safely exit the game.
echo.

l4n_hlae_launcher.exe left4dead2.exe eat_hook.dll AfxHookSource.dll -steam -insecure -vulkan
exit
