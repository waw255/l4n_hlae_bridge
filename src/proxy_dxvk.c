/*
 * proxy_dxvk.c — DXVK d3d9 Proxy DLL Stub
 * proxy_dxvk.c — DXVK d3d9 代理 DLL 桩
 *
 * Developer / 开发者: sparkling
 * Version / 版本: 1.3
 *
 * What this does / 功能说明:
 *   This is a minimal DLL that provides a DllMain stub. All actual
 *   Direct3D9 exports are forwarded to the real DXVK d3d9 via the
 *   proxy_dxvk.def file's export forwarding directives.
 *
 *   这是一个提供 DllMain 桩的最小 DLL。所有实际的 Direct3D9 导出
 *   通过 proxy_dxvk.def 文件的导出转发指令转发到真正的 DXVK d3d9。
 *
 *   The .def file contains directives like:
 *     Direct3DCreate9 = dxvk_d3d9_real.Direct3DCreate9
 *   This tells the Windows PE loader: when someone calls
 *   Direct3DCreate9 from OUR DLL, load dxvk_d3d9_real.dll and
 *   return its Direct3DCreate9 function instead.
 *
 *   .def 文件包含如下指令：
 *     Direct3DCreate9 = dxvk_d3d9_real.Direct3DCreate9
 *   这告诉 Windows PE 加载器：当有人从我们的 DLL 调用
 *   Direct3DCreate9 时，加载 dxvk_d3d9_real.dll 并
 *   返回其 Direct3DCreate9 函数。
 *
 * Why this proxy exists / 为什么需要这个代理:
 *   It ensures that our bridge components (eat_hook.dll) load BEFORE
 *   DXVK initializes the Vulkan rendering pipeline. Without this proxy,
 *   the game directly loads DXVK, and the loading order can be unpredictable.
 *
 *   它确保我们的桥接组件（eat_hook.dll）在 DXVK 初始化 Vulkan 渲染管线
 *   之前加载。没有这个代理，游戏直接加载 DXVK，加载顺序可能不可预测。
 *
 * DXVK d3d9 has 20 exports (from d3d9.def):
 * DXVK d3d9 有 20 个导出（来自 d3d9.def）:
 *   Direct3DShaderValidatorCreate9, PSGPError, PSGPSampleTexture,
 *   D3DPERF_BeginEvent, D3DPERF_EndEvent, D3DPERF_GetStatus,
 *   D3DPERF_QueryRepeatFrame, D3DPERF_SetMarker, D3DPERF_SetOptions,
 *   D3DPERF_SetRegion, DebugSetLevel, DebugSetMute,
 *   Direct3D9EnableMaximizedWindowedModeShim, Direct3DCreate9,
 *   Direct3DCreate9Ex, DXVK_RegisterAnnotation,
 *   DXVK_UnRegisterAnnotation, Direct3D9ForceHybridEnumeration,
 *   Direct3DCreate9On12, Direct3DCreate9On12Ex
 *
 * All 20 are forwarded in proxy_dxvk.def to dxvk_d3d9_real.dll
 *
 * 全部 20 个在 proxy_dxvk.def 中转发到 dxvk_d3d9_real.dll
 */

#include <windows.h>

/* DllMain — DLL Entry Point, minimal stub
 * DllMain — DLL 入口点，最小桩
 *
 * We don't need to do anything special here. All the export forwarding
 * is handled by the Windows PE loader via the .def file. We just return
 * TRUE to signal successful loading.
 *
 * 我们不需要在这里做任何特殊操作。所有导出转发由 Windows PE 加载器
 * 通过 .def 文件处理。我们只需返回 TRUE 表示加载成功。 */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	(void)hinstDLL;
	(void)lpvReserved;

	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hinstDLL);
	}
	return TRUE;
}
