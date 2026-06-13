#include <windows.h>
#include <stdio.h>

static HANDLE g_hGameProcess = NULL;

static BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_CLOSE_EVENT || dwCtrlType == CTRL_C_EVENT)
	{
		if (g_hGameProcess)
		{
			printf("\nTerminating game process (safe exit)...\n");
			TerminateProcess(g_hGameProcess, 0);
			WaitForSingleObject(g_hGameProcess, 3000);
		}
	}
	return TRUE;
}

int main(int argc, char *argv[])
{
	PROCESS_INFORMATION pi = {0};
	STARTUPINFOA si = {0};
	char cmdline[4096] = {0};
	char targetPath[MAX_PATH];
	char dll1[MAX_PATH], dll2[MAX_PATH];
	LPVOID p1, p2;
	HANDLE h1, h2;
	HANDLE hLog;

	if (argc < 3)
	{
		printf("Usage: l4n_hlae_launcher.exe <game.exe> <dll1.dll> [dll2.dll] [game args...]\n");
		return 1;
	}

	GetFullPathNameA(argv[1], MAX_PATH, targetPath, NULL);
	GetFullPathNameA(argv[2], MAX_PATH, dll1, NULL);
	if (argc > 3 && argv[3][0] != '-') {
		GetFullPathNameA(argv[3], MAX_PATH, dll2, NULL);
	} else {
		dll2[0] = '\0';
	}

	lstrcpyA(cmdline, "\"");
	lstrcatA(cmdline, targetPath);
	lstrcatA(cmdline, "\"");
	for (int i = (dll2[0] ? 4 : 3); i < argc; i++) {
		lstrcatA(cmdline, " ");
		lstrcatA(cmdline, argv[i]);
	}

	/* Open log file */
	hLog = CreateFileA("inject.log",
		GENERIC_WRITE, FILE_SHARE_READ, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

#define LOG(msg) do { \
	DWORD _t = GetTickCount(); \
	printf("[T+%5lu ms] %s\n", _t, msg); \
	if (hLog != INVALID_HANDLE_VALUE) { \
		char _b[512]; int _l = wsprintfA(_b, "[T+%5lu ms] %s\r\n", _t, msg); \
		DWORD _w; WriteFile(hLog, _b, _l, &_w, NULL); FlushFileBuffers(hLog); \
	} \
} while(0)

	LOG("INJECT_START");
	printf("Game : %s\nDLL1 : %s\nDLL2 : %s\nArgs : %s\n\n",
		targetPath, dll1, dll2[0]?dll2:"(none)", cmdline + lstrlenA(targetPath) + 3);

	si.cb = sizeof(si);
	LOG("CreateProcess(SUSPENDED)");
	if (!CreateProcessA(targetPath, cmdline, NULL, NULL, FALSE,
		CREATE_SUSPENDED, NULL, NULL, &si, &pi))
	{
		LOG("ERROR: CreateProcess failed");
		if (hLog != INVALID_HANDLE_VALUE) CloseHandle(hLog);
		return 1;
	}
	LOG("Process created OK");

	/* Store handle for Ctrl+C / window close handler */
	g_hGameProcess = pi.hProcess;
	SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

	/* Inject DLL1 */
	{
		char buf[256];
		wsprintfA(buf, "Injecting DLL1: %s", dll1);
		LOG(buf);
	}
	p1 = VirtualAllocEx(pi.hProcess, NULL, lstrlenA(dll1)+1, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(pi.hProcess, p1, dll1, lstrlenA(dll1)+1, NULL);
	h1 = CreateRemoteThread(pi.hProcess, NULL, 0,
		(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"),
		p1, 0, NULL);
	WaitForSingleObject(h1, 15000);
	{
		DWORD ec;
		GetExitCodeThread(h1, &ec);
		char buf[256];
		wsprintfA(buf, "DLL1 LoadLibrary ret=0x%08lX (%s)", ec, ec?"OK":"FAILED");
		LOG(buf);
	}
	CloseHandle(h1);
	VirtualFreeEx(pi.hProcess, p1, 0, MEM_RELEASE);

	/* Inject DLL2 */
	if (dll2[0]) {
		char buf[256];
		wsprintfA(buf, "Injecting DLL2: %s", dll2);
		LOG(buf);
		p2 = VirtualAllocEx(pi.hProcess, NULL, lstrlenA(dll2)+1, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
		WriteProcessMemory(pi.hProcess, p2, dll2, lstrlenA(dll2)+1, NULL);
		h2 = CreateRemoteThread(pi.hProcess, NULL, 0,
			(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"),
			p2, 0, NULL);
		WaitForSingleObject(h2, 15000);
		{
			DWORD ec;
			GetExitCodeThread(h2, &ec);
			char buf[256];
			wsprintfA(buf, "DLL2 LoadLibrary ret=0x%08lX (%s)", ec, ec?"OK":"FAILED");
			LOG(buf);
		}
		CloseHandle(h2);
		VirtualFreeEx(pi.hProcess, p2, 0, MEM_RELEASE);
	}

	LOG("ResumeThread");
	ResumeThread(pi.hThread);
	LOG("Game running - monitoring...");

	/* Monitor game process until exit */
	printf("\n==============================================\n");
	printf("  L4N + HLAE Bridge - Game Running\n");
	printf("  PID: %lu\n", pi.dwProcessId);
	printf("  Close this window to force-terminate the game\n");
	printf("  (avoids exit crash)\n");
	printf("==============================================\n");

	while (1)
	{
		DWORD exitCode;
		if (GetExitCodeProcess(pi.hProcess, &exitCode))
		{
			if (exitCode != STILL_ACTIVE)
			{
				char buf[128];
				wsprintfA(buf, "Process exited naturally, code=0x%08lX (%lu)", exitCode, exitCode);
				LOG(buf);
				break;
			}
		}
		else
		{
			LOG("GetExitCodeProcess failed - process may have terminated");
			break;
		}
		Sleep(500);
	}

	/* Cleanup */
	LOG("INJECT_END");
	if (hLog != INVALID_HANDLE_VALUE) CloseHandle(hLog);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return 0;
}
