// modifyCalc.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<Windows.h>
 

DWORD lpdwProcessId = NULL;

__declspec(naked) HANDLE WINAPI MyGetProcess(char *pszWindowName)
{
	__asm
	{
		push ebp
		mov ebp, esp
		push pszWindowName
		push 0
		call FindWindowA
		cmp eax, 0
		jz labret
		lea esi, lpdwProcessId
		push esi
		push eax
		call GetWindowThreadProcessId
		cmp eax, 0
		jz labret

		push lpdwProcessId
		push FALSE
		push PROCESS_ALL_ACCESS
		call OpenProcess

		labret :
		leave
		retn 4
	}
}

bool WINAPI HookCreateRemoteThread(HANDLE ulPrcess, char* ulModuleName)
{
	int		R_Size = strlen(ulModuleName) + 1;
	LPVOID	R_Memery = VirtualAllocEx(ulPrcess, NULL, R_Size, MEM_COMMIT,
    PAGE_EXECUTE_READWRITE);

  if (!R_Memery)
  {
    goto LAB_EXIT;
  }

	WriteProcessMemory(ulPrcess, R_Memery, ulModuleName, R_Size, NULL);

	DWORD	R_ThreadId = 0;
	HANDLE	R_ThreadHandle = CreateRemoteThread(ulPrcess, NULL, NULL, 
    (LPTHREAD_START_ROUTINE)LoadLibraryA, R_Memery, NULL, &R_ThreadId);
	
  if (WaitForSingleObject(R_ThreadHandle,/*INFINITE*/10 * 1000) == WAIT_TIMEOUT)
	{
		return false;
	}

	VirtualFreeEx(ulPrcess, R_Memery, R_Size, MEM_DECOMMIT);
	CloseHandle(R_ThreadHandle);
	
  return true;
LAB_EXIT:
  if (R_Memery)
  {
    VirtualFreeEx(ulPrcess, R_Memery, R_Size, MEM_DECOMMIT);
  }

	return false;
}

char*	WINAPI AntiHookGetCurrentDirectoryA(char* pszReturn, char* pszFileName)
{
	GetModuleFileNameA(NULL, pszReturn, MAX_PATH);
	lstrcpyA(strrchr(pszReturn, '\\') + 1, pszFileName);

	return	pszReturn;
}

int main()
{
	HANDLE hProcess = NULL;
	char *pszWindowName = _TEXT("计算器");

	_asm
	{
		pushad
		push pszWindowName
		call MyGetProcess
		mov hProcess, eax
		popad
	}

	char szPtch[MAXBYTE];
	char szBuff[MAXBYTE];
	
  ZeroMemory(szBuff, MAXBYTE);
	GetCurrentDirectoryA(MAXBYTE, szPtch);
	wsprintf(szBuff, "%s%s", szPtch, "\\CalcDll.dll");
	
  HookCreateRemoteThread(hProcess, szBuff);
	
  return 0;
}

