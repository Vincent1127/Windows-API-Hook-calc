// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"
#include <stdio.h>

DWORD dwPfnOrig;          //������ַ
//DWORD dwPfnStrCmp;
DWORD dwJmp = NULL;
wchar_t *pszStr =L"hook��";

//wchar_t *pszStrAdmin = L"admin";
DWORD dwString;

// __declspec(naked) void WINAPI MySetWindowText()
// {
// 
//   _asm
//   {
//     mov     edi, edi
//     push    ebp
//     mov     ebp, esp
// 
//     mov eax, [ebp + 0Ch]
//     push eax
//     push pszStrAdmin
//     call dwPfnStrCmp
//     add esp, 8
//   }
// 
//   _asm
//   {
//     cmp eax, 0
//     jnz donothinglabel
//     mov eax, pszStr
//     mov[ebp + 0Ch], eax
// donothinglabel:
// 		mov ebx, dwPfnOrig
// 		add ebx, 5
// 		jmp		ebx
// 
// 	}
// }

__declspec(naked) void WINAPI MySetWindowText()
{

  _asm
  {
    mov  edi, edi
    push ebp
    mov  ebp, esp

    mov eax, [ebp + 0Ch]
    mov eax, [eax]
    //"Mi"��UNICODE��ʽ4D006900 (�����ж��Ƿ���"Microsoft Windows"�ַ���)
    //һ�ּ��׵����strcpy()�ķ���
    mov ebx, 69004dh  
    sub eax, ebx

    cmp eax, 0
    jnz DO_NOTHING_LABEL
    mov eax, pszStr
    mov[ebp + 0Ch], eax
DO_NOTHING_LABEL:
    mov ebx, dwPfnOrig
    add ebx, 5
    jmp ebx
  }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		dwPfnOrig = (DWORD)GetProcAddress(GetModuleHandle("user32.dll"), 
      "SetWindowTextW");
    
    char szDebug[256];
    sprintf(szDebug, "%p", dwPfnOrig);
    OutputDebugString(szDebug);

    //dwPfnStrCmp = (DWORD)GetProcAddress(GetModuleHandle("msvcrt.dll"), "strcmp");

		DWORD oldProtected;
		VirtualProtect((void*)dwPfnOrig, 0x1000, PAGE_EXECUTE_READWRITE, 
      &oldProtected);

		_asm
		{
			pushad 
			mov eax, dwPfnOrig
			mov ebx, offset MySetWindowText
			sub ebx, eax
			sub ebx, 5
			mov byte ptr[eax], 0e9h
			mov dword ptr[eax + 1], ebx
			popad
		}

	}

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

