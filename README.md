# L4N + HLAE Compatibility Bridge

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-1.3-blue)](https://github.com/sparkling/l4n-hlae-bridge/releases)

> 中文文档：[README_zh.md](README_zh.md)

A compatibility layer that allows [L4N](https://afdian.com/a/neko_left4dead2) (L4D2 client rendering patch) and [HLAE](https://github.com/advancedfx/advancedfx) (Half-Life Advanced Effects) to coexist in Left 4 Dead 2.

## Problem

L4N replaces `left4dead2.exe` with a modified version that loads `left4neko.dll`. This prevents HLAE's `AfxHookSource.dll` from injecting properly — a known incompatibility acknowledged by the L4N documentation.

## Solution

A three-component bridge that injects before L4N's initialization sequence, using:

1. **SUSPENDED process injection** (`l4n_hlae_launcher.exe`) — creates the game frozen, injects `eat_hook.dll` + `AfxHookSource.dll`, then resumes
2. **KERNEL32 EAT hook** (`eat_hook.dll`) — monitors `LoadLibraryExA` at the OS export-table level, triggers IAT replay when `engine.dll`/`client.dll` load to bootstrap AfxHookSource's initialization chain
3. **DXVK export proxy** (`bin/dxvk_d3d9.dll`) — forwards all 20 DXVK d3d9 exports to `dxvk_d3d9_real.dll` with exact DXVK ordinal numbers preserved

## Quick Start

1. Copy the `l4n_hlae_bridge` folder into your L4D2 game root
2. Double-click `install_l4n_hlae.bat` from inside the folder
3. Double-click `launch_l4n_hlae.bat` to start the game

## Usage

```
# Launch (from game root or l4n_hlae_bridge/ folder)
launch_l4n_hlae.bat

# In game console, verify HLAE is working
mirv_input
```

**Safe exit:** Close the launcher console window — it uses `TerminateProcess` to kill the game cleanly, avoiding L4N assertion popups on shutdown.

## Commands (in-game console)

| Command | Effect |
|---------|--------|
| `mirv_input` | Toggle HLAE free camera |
| `mirv_campath` | Camera path system |
| `mirv_streams` | Stream recording |
| `mirv_deathmsg` | Death notice control |
| `mirv_fov` | Field of view control |

See [HLAE Wiki](https://github.com/advancedfx/advancedfx/wiki) for full command reference.

## Installation Details

The installer automatically:
- Detects your game root (works from any subfolder)
- Finds your HLAE installation (any folder containing "hlae" in its name)
- Backs up your original DXVK d3d9 and deploys the proxy
- Copies all required files into place

**Requirements:**
- Windows 10+ (64-bit)
- Left 4 Dead 2 (Steam version)
- L4N client patch installed
- HLAE installed in the game root
- DXVK loaded via `-vulkan` launch option

## Uninstall

Double-click `uninstall_l4n_hlae.bat` from the `l4n_hlae_bridge` folder. It restores the original DXVK, removes all bridge files, and cleans up HLAE DLL copies. Your L4N and HLAE installations are untouched.

## Known Limitations

- **Exit crash:** Using in-game Exit or Alt+F4 may show an L4N assertion popup. Close the launcher console window instead.
- **Mouse lock in `mirv_input camera`:** L4N's VGUI intercepts mouse input at the DirectInput level, causing cursor lock. Workaround: add `-hide_neko` to the launch command (disables L4N rendering but restores mouse control).
- **IAT layout dependent:** The IAT replay mechanism relies on specific RVA values in L4N's `left4dead2.exe` (v2.33.2 tested). Future L4N versions may need re-verification.
- **32-bit only:** L4D2 is a 32-bit application.

## Build from Source

Requires MinGW-w64 i686 (32-bit), tested with GCC 16.1.0.

```bash
# eat_hook.dll (core KERNEL32 EAT hook)
gcc -shared -m32 -o eat_hook.dll src/eat_hook.c -s -static-libgcc -Wl,--subsystem,windows

# l4n_hlae_launcher.exe (SUSPENDED injector)
gcc -m32 -mconsole -o l4n_hlae_launcher.exe src/inject2.c -s -static-libgcc

# DXVK proxy stub (generate stub.c, then build)
echo int __stdcall DllMain(void* h,unsigned long r,void* v){return 1;} > stub.c
gcc -shared -m32 -o bin/dxvk_d3d9.dll stub.c src/proxy_dxvk.def -s -static-libgcc -Wl,--subsystem,windows,--kill-at
```

## Files

| File | Size | Description |
|------|------|-------------|
| `l4n_hlae_launcher.exe` | 48 KB | SUSPENDED process injector |
| `eat_hook.dll` | 16 KB | KERNEL32 EAT hook, intercepts LoadLibrary |
| `bin\dxvk_d3d9.dll` | 14 KB | DXVK proxy — forwards 20 D3D9 exports to real DXVK |
| `bin\dxvk_d3d9_real.dll` | ~4.3 MB | Original DXVK d3d9 (backup, untouched) |
| `install_l4n_hlae.bat` | ~3 KB | One-click install script |
| `launch_l4n_hlae.bat` | <1 KB | Quick launch |
| `uninstall_l4n_hlae.bat` | ~2 KB | Cleanup script |
| `README.md` | | English documentation |
| `README_zh.md` | | Chinese documentation |
| `src\eat_hook.c` | ~2 KB | EAT hook source |
| `src\inject2.c` | ~5 KB | Injector source |
| `src\proxy_dxvk.c` | ~2 KB | DXVK proxy DllMain stub |
| `src\proxy_dxvk.def` | ~1 KB | DXVK proxy export forwarding definitions |

## License

MIT License. See [LICENSE](LICENSE).

The bridge code is original work. L4N, HLAE, and DXVK are projects of their respective authors and are distributed under their own licenses.

## Security

⚠️ HLAE hooks into the game process. The launcher passes `-insecure` to prevent joining VAC-secured servers. Use only for demo playback and single-player. Using this bridge on VAC servers may result in a ban.
