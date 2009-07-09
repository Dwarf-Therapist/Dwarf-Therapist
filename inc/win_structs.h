#ifndef WIN_STRUCTS_H
#define WIN_STRUCTS_H
#include <windows.h>
#include <ntsecapi.h>
#include <stdio.h>


typedef struct _PEB {
    bool InheritedAddressSpace;
    bool ReadImageFileExecOptions;
    bool BeingDebugged;
    bool Spare;
    HANDLE Mutant;
    PVOID ImageBaseAddress;
} PEB, *PPEB;

typedef NTSTATUS (NTAPI *_NtQueryInformationProcess)(
    HANDLE ProcessHandle,
    DWORD ProcessInformationClass,
    PVOID ProcessInformation,
    DWORD ProcessInformationLength,
    PDWORD ReturnLength
    );

typedef struct _PROCESS_BASIC_INFORMATION
{
    DWORD ExitStatus;
    PVOID PebBaseAddress;
    DWORD AffinityMask;
    DWORD BasePriority;
    DWORD UniqueProcessId;
    DWORD ParentProcessId;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;

PVOID GetPebAddress(HANDLE ProcessHandle)
{
    _NtQueryInformationProcess NtQueryInformationProcess =
        (_NtQueryInformationProcess)GetProcAddress(
        GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
    PROCESS_BASIC_INFORMATION pbi;

    NtQueryInformationProcess(ProcessHandle, 0, &pbi, sizeof(pbi), NULL);
    return pbi.PebBaseAddress;
}

#endif // WIN_STRUCTS_H
