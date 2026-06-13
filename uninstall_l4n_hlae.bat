@echo off
setlocal

set "GAME_ROOT=%cd%"
if exist "%GAME_ROOT%\left4dead2.exe" goto root_found
cd ..
set "GAME_ROOT=%cd%"
if exist "%GAME_ROOT%\left4dead2.exe" goto root_found
echo left4dead2.exe not found. Run this from the game folder.
pause
exit /b 1

:root_found
cd /d "%GAME_ROOT%"
echo ============================================
echo   L4N + HLAE Bridge - Uninstaller
echo ============================================
echo.

:confirm
set "CONFIRM="
set /p "CONFIRM=Continue? (Y/N): "
if /i "%CONFIRM%"=="Y" goto do_uninstall
if /i "%CONFIRM%"=="N" exit /b 0
goto confirm

:do_uninstall
set "REMOVED=0"
set "WARNINGS=0"

echo.
echo [1/3] Restoring DXVK...
if exist "bin\dxvk_d3d9_real.dll" (
    echo   Found backup: bin\dxvk_d3d9_real.dll
    if exist "bin\dxvk_d3d9.dll" del /f "bin\dxvk_d3d9.dll" >nul 2>&1
    ren "bin\dxvk_d3d9_real.dll" "dxvk_d3d9.dll"
    if exist "bin\dxvk_d3d9.dll" (
        echo   Restored: bin\dxvk_d3d9.dll
        set /a REMOVED=REMOVED+1
    ) else (
        echo   [WARN] Rename failed
        set /a WARNINGS=WARNINGS+1
    )
) else (
    if exist "bin\dxvk_d3d9.dll" (
        echo   [WARN] No backup found - skipping DXVK restore
        set /a WARNINGS=WARNINGS+1
    ) else (
        echo   No DXVK changes to undo.
    )
)

echo.
echo [2/3] Removing bridge files...
for %%f in (l4n_hlae_launcher.exe eat_hook.dll launch_l4n_hlae.bat install_l4n_hlae.bat uninstall_l4n_hlae.bat) do (
    if exist "%%f" (
        del /f "%%f" >nul 2>&1
        if not exist "%%f" (
            echo   Removed: %%f
            set /a REMOVED=REMOVED+1
        )
    )
)

echo.
echo [3/3] Cleaning HLAE DLL copies...
set "CLEANED=0"
for %%f in (AfxHookSource.dll AfxCppCli.dll AfxHookGoldSrc.dll OpenEXR-3_3.dll OpenEXRCore-3_3.dll Iex-3_3.dll IlmThread-3_3.dll Imath-3_1.dll NGettext.dll msvcp140.dll msvcp140_1.dll msvcp140_2.dll msvcp140_atomic_wait.dll msvcp140_codecvt_ids.dll vcruntime140.dll vccorlib140.dll concrt140.dll ucrtbase.dll api-ms-win-*.dll API-MS-Win-*.dll AfxHook.dat) do (
    if exist "%%f" (
        del /f "%%f" >nul 2>&1
        if not exist "%%f" set /a CLEANED=CLEANED+1
    )
)
echo   Cleaned %CLEANED% DLL copies

echo.
echo ============================================
echo   Done. Removed: %REMOVED% items
echo ============================================
echo.
pause
