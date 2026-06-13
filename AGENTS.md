# AGENTS.md — L4N+HLAE Bridge

## Build

Compiler: MinGW-w64 i686 (32-bit), tested GCC 16.1.0.

```bash
# eat_hook.dll (core KERNEL32 EAT hook)
gcc -shared -m32 -o eat_hook.dll src/eat_hook.c -s -static-libgcc -Wl,--subsystem,windows

# l4n_hlae_launcher.exe (SUSPENDED injector)
gcc -m32 -mconsole -o l4n_hlae_launcher.exe src/inject2.c -s -static-libgcc

# DXVK proxy stub (generate stub.c, then build)
echo int __stdcall DllMain(void* h,unsigned long r,void* v){return 1;} > stub.c
gcc -shared -m32 -o bin/dxvk_d3d9.dll stub.c src/proxy_dxvk.def -s -static-libgcc -Wl,--subsystem,windows,--kill-at
```

## Architecture

Three-component injection chain:
1. **`l4n_hlae_launcher.exe`** — `CreateProcess(SUSPENDED)` + `CreateRemoteThread(LoadLibrary)` for two DLLs, then `ResumeThread`. Monitors process until exit.
2. **`eat_hook.dll`** — Patches `KERNEL32!LoadLibraryExA` EAT export. When `engine.dll`/`client.dll` load, triggers "IAT replay" through already-hooked `left4dead2.exe` import table to bootstrap AfxHookSource's initialization chain.
3. **`bin/dxvk_d3d9.dll`** — Pure export-forwarding proxy (14 KB). All 20 DXVK d3d9 exports forwarded to `dxvk_d3d9_real.dll` with exact DXVK ordinal numbers preserved.

**Critical RVA values** (L4N v2.33.2 exe):
- `left4dead2.exe` LoadLibraryExA IAT @ RVA `0x9600c`
- `engine.dll` GetProcAddress IAT @ RVA `0x0034e180`
- `engine.dll` LoadLibraryA IAT @ RVA `0x0034e184`

If L4N updates the exe, these may change and the bridge will need re-verification.

## Encoding rules

Batch files (.bat) must be pure ASCII — no UTF-8, no Chinese in `rem` comments. Chinese text only in `echo` statements after `chcp 65001`. PowerShell `Set-Content -Encoding ASCII` is safe.

## Install/uninstall testing

Install script runs from `l4n_hlae_bridge\` folder (uses `%~dp0` for paths). Auto-detects game root by looking for `left4dead2.exe` in current dir then parent. Auto-detects HLAE folder by scanning for directories containing "hlae" with `AfxHookSource.dll` inside.

To test uninstall: temporarily replace `pause` with `rem pause` and `set /p CONFIRM=` with `set CONFIRM=Y` for automated testing.

## Known gotchas

- DXVK proxy must preserve ordinals (`Direct3DCreate9 @37`, etc.) — the game and mods use `GetProcAddress(MAKEINTRESOURCE)`. The `.def` ordinal syntax is `= target.func @N`.
- `mirv_input camera` mouse is locked by L4N VGUI — documented as known limitation with `-hide_neko` workaround.
- In-game exit triggers L4N assertion popup — workaround: close launcher console window (uses `TerminateProcess`).
