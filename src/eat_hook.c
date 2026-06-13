/*
 * eat_hook.c — KERNEL32 EAT Hook for L4N+HLAE Bridge
 * Developer: sparkling  Version: 1.3  2026-06-13
 */
#include <windows.h>

static HMODULE (WINAPI *True_LoadLibraryExA)(LPCSTR, HANDLE, DWORD);
static BOOL     g_bInIatReplay = FALSE;
static DWORD   *g_pEatFunctions = NULL;
static DWORD    g_OriginalRva = 0;
static WORD     g_TargetOrdinal = 0;

static void Log(const char *msg)
{
	HANDLE h = CreateFileA("eat_hook.log", FILE_APPEND_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
	if (h != INVALID_HANDLE_VALUE) { DWORD w; WriteFile(h,msg,lstrlenA(msg),&w,NULL); WriteFile(h,"\r\n",2,&w,NULL); CloseHandle(h); }
}

typedef void (WINAPI *PFN_ExitProcess)(UINT);
static PFN_ExitProcess g_RealExitProcess = NULL;
static HMODULE g_hAfxHook = NULL;
static void WINAPI Hook_ExitProcess(UINT u) { if(g_hAfxHook)FreeLibrary(g_hAfxHook); if(g_RealExitProcess)g_RealExitProcess(u); TerminateProcess(GetCurrentProcess(),u); }

HMODULE WINAPI Hook_LoadLibraryExA(LPCSTR lp, HANDLE hf, DWORD fl)
{
	HMODULE r = True_LoadLibraryExA(lp,hf,fl);
	if(!lp||!r) return r;
	if(!g_bInIatReplay && !lstrcmpiA(lp+lstrlenA(lp)-10,"engine.dll")){
		g_bInIatReplay=TRUE; g_hAfxHook=GetModuleHandleA("AfxHookSource.dll");
		Log("[eat_hook] engine.dll - IAT replay");
		typedef HMODULE(WINAPI*P)(LPCSTR,HANDLE,DWORD);
		HMODULE e=GetModuleHandleA(NULL); P*i=(P*)((BYTE*)e+0x0009600c); (*i)("engine.dll",NULL,0);
	}
	if(!lstrcmpiA(lp+lstrlenA(lp)-10,"client.dll")){
		HMODULE e=GetModuleHandleA("engine.dll");
		if(e){ FARPROC*i=(FARPROC*)((BYTE*)e+0x0034e184); ((HMODULE(WINAPI*)(LPCSTR))*i)("client.dll"); }
	}
	return r;
}

static void PatchExport(HMODULE m, const char *fn, void *hk, void **real)
{
	PIMAGE_DOS_HEADER d=(PIMAGE_DOS_HEADER)m; PIMAGE_NT_HEADERS n=(PIMAGE_NT_HEADERS)((BYTE*)m+d->e_lfanew);
	DWORD r=n->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress; if(!r)return;
	PIMAGE_EXPORT_DIRECTORY e=(PIMAGE_EXPORT_DIRECTORY)((BYTE*)m+r);
	DWORD *ns=(DWORD*)((BYTE*)m+e->AddressOfNames); WORD *os=(WORD*)((BYTE*)m+e->AddressOfNameOrdinals);
	DWORD *fs=(DWORD*)((BYTE*)m+e->AddressOfFunctions);
	for(DWORD i=0;i<e->NumberOfNames;i++) if(!lstrcmpA((char*)((BYTE*)m+ns[i]),fn)){
		WORD o=os[i]; *real=(void*)((BYTE*)m+fs[o]);
		DWORD old; VirtualProtect(&fs[o],4,PAGE_READWRITE,&old); fs[o]=(DWORD)hk-(DWORD)m; VirtualProtect(&fs[o],4,old,&old); break; }
}

static void InstallHooks(void)
{
	HMODULE k=GetModuleHandleA("kernel32.dll"); if(!k)return;
	True_LoadLibraryExA=(HMODULE(WINAPI*)(LPCSTR,HANDLE,DWORD))GetProcAddress(k,"LoadLibraryExA");
	if(True_LoadLibraryExA){
		PIMAGE_DOS_HEADER d=(PIMAGE_DOS_HEADER)k; PIMAGE_NT_HEADERS n=(PIMAGE_NT_HEADERS)((BYTE*)k+d->e_lfanew);
		DWORD r=n->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		PIMAGE_EXPORT_DIRECTORY e=(PIMAGE_EXPORT_DIRECTORY)((BYTE*)k+r);
		DWORD*ns=(DWORD*)((BYTE*)k+e->AddressOfNames);WORD*os=(WORD*)((BYTE*)k+e->AddressOfNameOrdinals);DWORD*fs=(DWORD*)((BYTE*)k+e->AddressOfFunctions);
		for(DWORD i=0;i<e->NumberOfNames;i++) if(!lstrcmpA((char*)((BYTE*)k+ns[i]),"LoadLibraryExA"))
			{g_pEatFunctions=fs;g_OriginalRva=fs[os[i]];g_TargetOrdinal=os[i];break;}
		PatchExport(k,"LoadLibraryExA",Hook_LoadLibraryExA,(void**)&True_LoadLibraryExA);
		True_LoadLibraryExA=(HMODULE(WINAPI*)(LPCSTR,HANDLE,DWORD))((BYTE*)k+g_OriginalRva);
	}
	PatchExport(k,"ExitProcess",Hook_ExitProcess,(void**)&g_RealExitProcess);
	Log("[eat_hook] hooks installed");
}

BOOL WINAPI DllMain(HINSTANCE h, DWORD r, LPVOID v)
{
	if(r==DLL_PROCESS_ATTACH){DisableThreadLibraryCalls(h);InstallHooks();}
	else if(r==DLL_PROCESS_DETACH && !v){
		if(g_pEatFunctions&&g_OriginalRva){DWORD old;VirtualProtect(&g_pEatFunctions[g_TargetOrdinal],4,PAGE_READWRITE,&old);g_pEatFunctions[g_TargetOrdinal]=g_OriginalRva;VirtualProtect(&g_pEatFunctions[g_TargetOrdinal],4,old,&old);}
	}
	return TRUE;
}
