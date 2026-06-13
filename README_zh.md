# L4N + HLAE 兼容桥接

> **开发者:** sparkling  
> **版本:** 1.3  
> **日期:** 2026-06-10
>
> English: [README.md](README.md)

> ⚠️ **AI 生成声明**
>
> 本项目**全部代码、测试、开发均由 AI 完成**，无任何人工参与编写代码。
>
> - 代码中的 bug **无法由人工修复**——此项目的维护者不直接编写或 debug 源代码
> - 提交到 [Issues](https://github.com/sparkling/l4n-hlae-bridge/issues) 的 bug 报告**可能无法得到有效响应**
> - 请仅将此项目作为参考实现，自行承担使用风险

---

让 **L4N 客户端补丁** 与 **HLAE (Half-Life Advanced Effects) AfxHookSource.dll** 在 Left 4 Dead 2 中共存。

---

## 目录

1. [这个工具是什么](#这个工具是什么)
2. [系统要求](#系统要求)
3. [快速开始（熟手向）](#快速开始熟手向)
4. [超详细安装教程（新手友好）](#超详细安装教程新手友好)
5. [如何启动](#如何启动)
6. [如何验证是否生效](#如何验证是否生效)
7. [技术原理](#技术原理)
8. [卸载](#卸载)
9. [文件说明](#文件说明)
10. [从源码构建（开发者向）](#从源码构建开发者向)
11. [已知限制](#已知限制)
12. [安全警告](#安全警告)

---

## 这个工具是什么

- **L4N** 是一个给 L4D2 带来现代渲染（PBR、NekoToon）、模型功能、bug 修复等的客户端补丁。它会替换 `left4dead2.exe`。
- **HLAE** 是一个用于录制电影/自由视角回放 demo 的工具。核心是 `AfxHookSource.dll`。
- **问题**：L4N 修改过的 exe 阻止 HLAE 注入 `AfxHookSource.dll`。L4N 的 readme 也明确写了"目前不兼容 HLAE"。
- **这个桥接工具** 通过自定义启动器 + 内核级 DLL hook 让两者同时工作。

---

## 系统要求

| 要求 | 详情 |
|------|------|
| 操作系统 | Windows 10 或更新（64位） |
| 游戏 | Left 4 Dead 2（Steam 正版） |
| L4N | 已安装，`left4dead2.exe` 是 L4N 版本（Windows 中约 607 KB） |
| HLAE | 已安装到游戏目录中任何名称包含 "hlae" 的文件夹内（如 `hlae_2_189_7\`） |
| DXVK | `bin\dxvk_d3d9.dll` 已存在（~4.3 MB），由启动器自动通过 `-vulkan` 加载 |
| 磁盘空间 | 约 20MB（用于在游戏根目录复制 HLAE DLL 依赖） |

---

## 快速开始（熟手向）

如果你知道自己在做什么：

```
1. 把 hlae_2_189_7\ 里的所有 .dll 文件复制到游戏根目录
2. 把 bin\dxvk_d3d9.dll 改名为 bin\dxvk_d3d9_real.dll
3. 把 l4n_hlae_bridge\bin\dxvk_d3d9.dll 复制到 bin\dxvk_d3d9.dll
4. 把 l4n_hlae_launcher.exe 和 eat_hook.dll 复制到游戏根目录
5. 运行: l4n_hlae_launcher.exe left4dead2.exe eat_hook.dll AfxHookSource.dll -steam -insecure -vulkan
```

---

## 超详细安装教程（新手友好）

### 第零步：找到游戏文件夹

Left 4 Dead 2 的安装位置通常在这里：

```
E:\SteamLibrary\steamapps\common\Left 4 Dead 2\
```

我们称之为**游戏根目录**。下面的所有文件路径都是相对于这个文件夹的。

**如何找到它：**
1. 打开 Steam → 游戏库
2. 右键点击 "Left 4 Dead 2" → 管理 → 浏览本地文件
3. 此时弹出的文件夹就是游戏根目录

### 第一步：确认前置条件

在安装桥接工具之前，请确认以下项目已经设置好：

**1a. L4N 已安装**

在游戏根目录中检查这些文件是否存在：
```
left4dead2.exe          （大小约 607 KB —— 这是 L4N 的 exe）
bin\left4neko.dll       （应该存在，约 4.5 MB）
```

**如何判断当前是 L4N 版本还是原版？**
- 原版 exe：Windows 中显示约 357 KB
- L4N 版本：Windows 中显示约 607 KB

查看方法：在游戏根目录中右键 `left4dead2.exe` → 属性 → 查看文件大小。

如果 exe 是原版（约 357 KB），请重新运行 L4N 安装程序（将 L4N 压缩包中所有文件复制覆盖到游戏根目录）。

**1b. HLAE 已安装**

检查游戏根目录中是否有名称包含 "hlae" 的文件夹（如 `hlae_2_189_7\`）。其内必须有以下文件：
```
hlae_2_189_7\            （或任何以 "hlae" 开头的文件夹名）
├── AfxHookSource.dll    ← HLAE 核心
├── AfxCppCli.dll        ← HLAE 依赖
├── HLAE.exe             ← HLAE 主程序
├── injector.exe         ← HLAE 注入器
└── ... （很多 DLL 文件）
```

如果这个文件夹不存在：
1. 去 https://github.com/advancedfx/advancedfx/releases 下载最新版
2. 解压便携版（zip 文件）
3. 把解压出来的所有文件放到 `hlae_2_189_7\` 文件夹中
4. 确保 `AfxHookSource.dll` 在 `hlae_2_189_7\` 里面

**1c. DXVK 已设置**

检查 `bin\dxvk_d3d9.dll` 是否存在（约 4.3 MB）。如果不存在，说明 DXVK 还没安装。桥接启动器通过 `-vulkan` 参数自动加载 DXVK，无需在 Steam 中设置启动项。

**1d. 启动脚本已处理所有参数**

启动脚本（`launch_l4n_hlae.bat`）会自动传递 `-steam -insecure -vulkan`。你无需在 Steam 中配置任何启动项——直接双击启动脚本即可。

`-insecure` 参数防止连接到 VAC 安全服务器（HLAE 安全要求）。

### 第二步：复制桥接文件

**2a.** 把 `l4n_hlae_bridge` 文件夹整个复制到游戏根目录。

复制后应该能看到：
```
Left 4 Dead 2\
├── l4n_hlae_bridge\        ← 桥接工具文件夹
│   ├── README.md
│   ├── README_zh.md
│   ├── AGENTS.md
│   ├── LICENSE
│   ├── install_l4n_hlae.bat
│   ├── launch_l4n_hlae.bat
│   ├── uninstall_l4n_hlae.bat
│   ├── l4n_hlae_launcher.exe
│   ├── eat_hook.dll
│   ├── bin\dxvk_d3d9.dll
│   └── src\ ...
├── hlae_2_189_7\           ← 你的 HLAE 安装
├── left4dead2.exe          ← L4N 的 exe
└── ... （其他游戏文件）
```

### 第三步：运行安装脚本

**3a.** 从 `l4n_hlae_bridge\` 文件夹中双击 `install_l4n_hlae.bat`。

**脚本自动检测逻辑：**
- **游戏根目录**：先在当前目录找 `left4dead2.exe`，找不到则到上级目录找
- **HLAE 文件夹**：扫描所有名称匹配 `*hlae*`、`*HLAE*`、`*Hlae*` 的文件夹，验证其中是否同时存在 `AfxHookSource.dll` 和 `AfxCppCli.dll`

**脚本执行的 3 个步骤：**

| 步骤 | 操作 |
|------|------|
| 1/3 复制 HLAE DLL | 将检测到的 HLAE 文件夹中**全部** `.dll` 文件复制到游戏根目录 |
| 2/3 部署 DXVK 代理 | 将 `bin\dxvk_d3d9.dll` 备份为 `bin\dxvk_d3d9_real.dll`（仅首次），然后用 14 KB 的桥接代理覆盖。如果已有一个代理，先另存为 `.dll.bak`。 |
| 3/3 复制启动器文件 | 将 `l4n_hlae_launcher.exe`、`eat_hook.dll`、`launch_l4n_hlae.bat`、`uninstall_l4n_hlae.bat` 复制到游戏根目录 |

**可能出现的警告：**
- `left4neko.dll not found` — L4N 可能未安装
- `bin\dxvk_d3d9.dll not found` — DXVK 可能未安装

**可能出现的错误：**
- `left4dead2.exe not found!` — 脚本不在游戏目录结构中。会提示 Steam 浏览本地文件的路径。
- `HLAE folder not found!` — 找不到包含 `AfxHookSource.dll` 的文件夹。会打印 HLAE 下载链接。

### 第四步：验证安装

运行 `install_l4n_hlae.bat` 之后，检查游戏根目录中是否存在以下文件：

| 文件 | 大小 | 来源 |
|------|------|------|
| `l4n_hlae_launcher.exe` | ~48 KB | 桥接包 |
| `eat_hook.dll` | ~16 KB | 桥接包 |
| `AfxHookSource.dll` | ~2.3 MB | 从 hlae_2_189_7 复制 |
| `AfxCppCli.dll` | ~162 KB | 从 hlae_2_189_7 复制 |
| `OpenEXR-3_3.dll` | ~772 KB | 从 hlae_2_189_7 复制 |
| `bin\dxvk_d3d9.dll` | ~14 KB | 桥接代理（已替换） |
| `bin\dxvk_d3d9_real.dll` | ~4.5 MB | 原版 DXVK（已备份） |

确认 `bin\dxvk_d3d9_real.dll` 存在——这是你原始 DXVK d3d9 的备份。

---

## 如何启动

### 方法一：使用启动脚本（推荐）

在游戏根目录双击 `launch_l4n_hlae.bat`（也可从 `l4n_hlae_bridge\` 文件夹内双击）。

**脚本内部行为：**
- 自动检测游戏根目录：先在当前目录找 `l4n_hlae_launcher.exe`、`eat_hook.dll`、`AfxHookSource.dll`，找不到则到上级目录
- exe 大小检查：如果 `left4dead2.exe` < 500 KB，警告"可能是原版 Steam 版本——L4N 可能未激活"
- 未找到必要文件时：报错并退出，提示先运行 `install_l4n_hlae.bat`
- 启动命令：`l4n_hlae_launcher.exe left4dead2.exe eat_hook.dll AfxHookSource.dll -steam -insecure -vulkan`

### 方法二：命令行

在游戏根目录打开命令提示符，运行：
```
l4n_hlae_launcher.exe left4dead2.exe eat_hook.dll AfxHookSource.dll -steam -insecure -vulkan
```

### 方法三：创建快捷方式

1. 右键桌面 → 新建 → 快捷方式
2. 位置填入：`E:\SteamLibrary\steamapps\common\Left 4 Dead 2\launch_l4n_hlae.bat`
3. 名称：`L4D2 L4N+HLAE`
4. （可选）右键快捷方式 → 属性 → 更改图标 → 浏览到 `left4dead2.ico`

### 重要提示

- **不要** 通过 Steam 直接启动 —— HLAE 不会加载
- **不要** 联机进入服务器 —— 仅用于 demo 回放/单人模式
- 启动后会有一个控制台窗口保持打开 —— **关闭此窗口** 即可安全退出游戏（使用 `TerminateProcess` 绕过已知的退出崩溃）
- 如果通过游戏内菜单或 Alt+F4 正常退出，L4N 可能在关闭时弹出断言窗口——虽无害但关闭启动器窗口可完全避免

---

## 如何验证是否生效

游戏进入主菜单后，按 `~` 键打开控制台，输入：

```
mirv_input
```

**预期结果：**
- 命令应该能够执行，不会显示 "Unknown command"
- 观看 demo 时能在暂停状态下自由移动视角

**如果显示 "Unknown command"：**
- 再等几秒钟重试（初始化需要一些时间）
- 尝试先加载任意地图，再在控制台测试
- 查看[常见问题](#常见问题)

其他可测试的 HLAE 命令：
```
mirv_campath        — 镜头路径系统
mirv_deathmsg       — 击杀信息控制
mirv_fov            — 视角控制
mirv_streams        — 录制流
```

---

## 技术原理

### 核心问题

L4N 替换了 `left4dead2.exe`，这个修改过的 exe 在启动早期就加载了 `left4neko.dll`。AfxHookSource.dll 通常通过 IAT（导入地址表）补丁来拦截游戏的 DLL 加载机制，但 L4N 的 exe 导致钩子链断开：

1. 引擎 DLL（`engine.dll`、`client.dll`）在 AfxHookSource 的钩子生效之前就加载了
2. 即使钩子安装成功，调用也可能绕过钩子走不同的导入路径

### 解决方案：三组件桥接

```
┌─────────────────────────────────┐
│  l4n_hlae_launcher.exe          │  组件1: 注入器
│  ① CreateProcess(SUSPENDED)     │  以冻结状态启动L4N的exe
│  ② CreateRemoteThread          ——→ 加载 eat_hook.dll
│  ③ CreateRemoteThread          ——→ 加载 AfxHookSource.dll
│  ④ ResumeThread                 │  解冻 - 游戏开始运行
└─────────────────────────────────┘
          ↓
┌─────────────────────────────────┐
│  eat_hook.dll                   │  组件2: EAT钩子（导出表钩子）
│  ┌──────────────────────────┐   │
│  │ 挂钩 KERNEL32 导出表(EAT):│   │  在系统最底层拦截
│  │   LoadLibraryExA          │   │  所有模块的LoadLibrary调用
│  ├──────────────────────────┤   │
│  │ 监控 DLL 加载链:          │   │  看到完整顺序:
│  │   left4neko.dll →         │   │  left4neko → filesystem_stdio
│  │   filesystem_stdio.dll →  │   │  → engine → client
│  │   engine.dll →            │   │
│  │   client.dll              │   │
│  ├──────────────────────────┤   │
│  │ IAT 回放机制:              │   │  engine.dll加载时:
│  │   读取 left4dead2.exe     │   │  通过左4死2exe的已挂钩IAT
│  │   的已挂钩IAT指针          │   │  调用 LoadLibrary("engine.dll")
│  │   触发 AfxHookSource      │   │  → 触发 LibraryHooksA
│  │   的初始化流程             │   │  → 补丁 engine.dll 的IAT
│  │                           │   │
│  │   client.dll加载时:        │   │  client.dll加载时:
│  │   通过 engine.dll 的      │   │  通过引擎DLL的已挂钩IAT
│  │   已挂钩IAT回放            │   │  回放 "client.dll"
│  │                           │   │  → 设置 g_H_ClientDll
│  └──────────────────────────┘   │  → 启用 CreateInterface 拦截
└─────────────────────────────────┘
          ↓
┌─────────────────────────────────┐
│  bin\dxvk_d3d9.dll (14KB)       │  组件3: DXVK代理
│  全部20个D3D9/DXVK导出          │  透明转发 →
│  转发到: dxvk_d3d9_real.dll     │  (原始DXVK, 4.5MB)
└─────────────────────────────────┘
```

### DLL 加载顺序

```
1. left4dead2.exe (L4N) ← SUSPENDED冻结状态启动
2. eat_hook.dll 注入      → 补丁 KERNEL32 导出表(EAT)
3. AfxHookSource.dll 注入 → 补丁 left4dead2.exe 导入表(IAT)
4. 进程解冻(RESUMED) → 主线程开始执行:

   left4dead2.exe
     ├─ LoadLibrary("left4neko.dll")      ← EAT钩子拦截
     ├─ LoadLibrary("filesystem_stdio.dll") ← EAT钩子拦截
     ├─ LoadLibrary("engine.dll")         ← EAT钩子拦截
     │                                      → IAT回放触发
     │                                      → engine IAT被补丁
     │    └─ LoadLibrary("client.dll")    ← EAT钩子拦截
     │                                      → IAT回放触发
     │                                      → g_H_ClientDll 被设置
     │    └─ GetProcAddress(clientDll,     ← engine IAT钩子拦截
     │                      "CreateInterface")
     │       → new_Client_CreateInterface
     │       → VClient016 vtable 被补丁
     │       → new_CVClient_Init_Swarm
     │       → MySetup() → mirv_* 命令已注册
     └─ Direct3DCreate9()
          → bin\dxvk_d3d9.dll (代理)
          → bin\dxvk_d3d9_real.dll (DXVK)
```

---

## 卸载

### 推荐方式：使用卸载脚本

从游戏根目录（或 `l4n_hlae_bridge\` 文件夹内）双击 `uninstall_l4n_hlae.bat`。

脚本自动检测游戏根目录（与安装器相同的两级查找逻辑），然后显示 Y/N 确认提示。输入 `Y` 后执行三步：

| 步骤 | 操作 |
|------|------|
| 1/3 还原 DXVK | 删除代理 `dxvk_d3d9.dll`，将备份 `dxvk_d3d9_real.dll` 重命名回 `dxvk_d3d9.dll`。无备份则跳过。 |
| 2/3 删除桥接文件 | 移除游戏根目录中的：`l4n_hlae_launcher.exe`、`eat_hook.dll`、`launch_l4n_hlae.bat`、`install_l4n_hlae.bat`、`uninstall_l4n_hlae.bat` |
| 3/3 清理 HLAE DLL | 删除从 HLAE 复制过来的所有 DLL（16+ 个文件：`AfxHookSource.dll`、`AfxCppCli.dll`、OpenEXR 系列、MSVC 运行时等） |

最后显示移除数量和警告数。你的原始 HLAE 文件夹和 L4N 安装不受影响。

### 备选方式：手动卸载

```batch
REM 恢复原版 DXVK d3d9
del bin\dxvk_d3d9.dll
ren bin\dxvk_d3d9_real.dll dxvk_d3d9.dll

REM 删除桥接文件
del l4n_hlae_launcher.exe
del eat_hook.dll
del launch_l4n_hlae.bat
del install_l4n_hlae.bat

REM 删除复制的 HLAE DLL（可选 - 不影响游戏正常运行）
del AfxHookSource.dll AfxCppCli.dll OpenEXR-3_3.dll
del msvcp140*.dll vcruntime140.dll ucrtbase.dll
del api-ms-win-*.dll concrt140.dll vccorlib140.dll
del Iex-3_3.dll IlmThread-3_3.dll Imath-3_1.dll
del NGettext.dll OpenEXRCore-3_3.dll

REM 删除桥接文件夹
rmdir /s /q l4n_hlae_bridge
```

如果你验证游戏文件完整性，Steam 会自动重新下载 `bin\dxvk_d3d9.dll`。

---

## 文件说明

| 文件 | 大小 | 描述 |
|------|------|------|
| `l4n_hlae_launcher.exe` | 48 KB | SUSPENDED 进程注入器 |
| `eat_hook.dll` | 16 KB | KERNEL32 导出表(EAT)钩子，拦截 LoadLibrary |
| `bin\dxvk_d3d9.dll` | 14 KB | DXVK 代理 —— 转发 20 个 D3D9 导出到真正 DXVK |
| `bin\dxvk_d3d9_real.dll` | ~4.3 MB | 原始 DXVK d3d9（备份，未被修改） |
| `install_l4n_hlae.bat` | ~3 KB | 一键安装脚本 |
| `launch_l4n_hlae.bat` | ~0.1 KB | 快速启动（在终端中直接运行注入器） |
| `uninstall_l4n_hlae.bat` | ~2 KB | 卸载脚本 |
| `README.md` | | 英文文档 |
| `README_zh.md` | 本文件 | 中文文档 |
| `AGENTS.md` | | 开发者参考文档 |
| `LICENSE` | | MIT 许可证 |
| `src\eat_hook.c` | ~2 KB | EAT hook 源码 |
| `src\inject2.c` | ~5 KB | 注入器源码 |
| `src\proxy_dxvk.c` | ~2 KB | DXVK 代理源码 (DllMain 桩) |
| `src\proxy_dxvk.def` | ~1 KB | DXVK 代理导出转发定义 |

---

## 从源码构建（开发者向）

### 要求

- MinGW-w64 i686 (32位) — 测试过 GCC 16.1.0
- Windows 10+ SDK

### 编译 eat_hook.dll

```bash
gcc -shared -m32 -o eat_hook.dll src/eat_hook.c -s -static-libgcc -Wl,--subsystem,windows
```

### 编译 l4n_hlae_launcher.exe

```bash
gcc -m32 -mconsole -o l4n_hlae_launcher.exe src/inject2.c -s -static-libgcc
```

### 编译 DXVK 代理

```bash
echo int __stdcall DllMain(void* h,unsigned long r,void* v){return 1;} > stub.c
gcc -shared -m32 -o bin/dxvk_d3d9.dll stub.c src/proxy_dxvk.def -s -static-libgcc -Wl,--subsystem,windows,--kill-at
```

### 测试

```bash
l4n_hlae_launcher.exe left4dead2.exe eat_hook.dll AfxHookSource.dll -steam -insecure -vulkan
```

---

## 已知限制

- **依赖特定 IAT 布局：** IAT 回放机制依赖 L4N 的 `left4dead2.exe` 和 `engine.dll` 中特定的 RVA 值。如果 L4N 的 exe 结构在未来版本中有重大变化，桥接可能需要更新。
  - `left4dead2.exe` LoadLibraryExA IAT @ RVA 0x9600c
  - `engine.dll` GetProcAddress IAT @ RVA 0x0034e180
  - `engine.dll` LoadLibraryA IAT @ RVA 0x0034e184
- **仅 32 位：** Left 4 Dead 2 是 32 位程序。此桥接仅支持 32 位。
- **DXVK -vulkan 模式：** 目前仅测试了通过 `-vulkan` 启动项加载 DXVK 的方式。
- **HLAE 版本：** 使用 HLAE 2.189.7 构建和测试。其他版本可能可用，但不保证。
- **退出崩溃：** 通过游戏内菜单或 Alt+F4 退出时可能触发 L4N 断言弹窗。**解决方案：** 关闭启动器控制台窗口——它使用 `TerminateProcess` 干净结束游戏，完全绕过 shutdown 流程。

---

## 安全警告

> ⚠️ **VAC 警告**
>
> HLAE 通过钩子注入游戏进程运行。此桥接使其能随 L4N 一起运行。
>
> - **绝对不要** 在使用此桥接时进入 VAC 安全服务器
> - 启动脚本已包含 `-insecure`。如果手动启动，也要确保命令行中包含 `-insecure`
> - **仅** 用于以下场景：
>   - Demo 回放和录制
>   - 单人模式/离线模式
>   - 与朋友的本地监听服务器（确保设置 sv_lan）
> - L4N 开发者的免责声明同样适用：使用时风险自负

---

## 常见问题

### 游戏窗口弹出后立即崩溃

可能是杀毒软件误报。尝试：
1. 暂时关闭 Windows Defender 实时保护
2. 把游戏文件夹添加到杀毒软件排除列表
3. 很多杀毒软件会标记 DLL 注入器为可疑程序

### 启动器显示 "LoadLibrary returned: FAILED"

注入器无法加载某个 DLL。检查：
1. `eat_hook.dll` 是否在游戏根目录
2. `AfxHookSource.dll` 是否在游戏根目录
3. 所有 HLAE DLL 依赖是否都在游戏根目录
4. 重新运行 `install_l4n_hlae.bat` 重新复制所有文件

### 游戏正常运行但等很久 HLAE 命令也不生效

可能 L4N exe 的 IAT 布局已经改变。检查：
1. 是否使用 L4N v2.33.2（其他版本可能有不同的 IAT 布局）
2. 在控制台运行 `l4n_env_report` 查看 L4N 版本
3. IAT 回放机制针对特定的 RVA 值——如果 L4N 更新了 exe，这些值可能变了
4. 尝试加载一个地图后再测试 `mirv_input`

### 想同时用 HLAE 的 GUI 界面

HLAE.exe 的 GUI 和此启动器互斥。如果你需要 HLAE 的图形界面：
1. 先用此启动器启动游戏
2. 然后单独运行 `hlae_2_189_7\HLAE.exe`
3. HLAE GUI 可以通过 IPC 与已加载的 AfxHookSource.dll 通信
4. 注意：部分 GUI 功能可能有限，因为不是通过 HLAE 内置的 Custom Loader 启动

---

## 许可证与致谢

- **桥接代码**（注入器、EAT hook、DXVK 代理）：MIT 许可证，作者 sparkling
- **L4N**：版权归 L4N 团队所有
- **HLAE / AfxHookSource**：版权归 advancedfx 团队所有（MIT 许可证）
- **DXVK**：版权归 DXVK 团队所有（zlib/libpng 许可证）

本项目是第三方兼容层，与 L4N、HLAE 或 DXVK 项目无关，也不受其认可。
