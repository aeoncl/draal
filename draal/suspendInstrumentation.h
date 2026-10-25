#pragma once
#include <Windows.h>
#include "../libs/logger.h"

typedef LONG(WINAPI* NtQueryInformationProcess_t)(
    HANDLE, ULONG, PVOID, ULONG, PULONG
    );

typedef struct _PEB
{
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
    BYTE BitField;
    HANDLE Mutant;
    PVOID ImageBaseAddress;
} PEB, * PPEB;


typedef struct _PROCESS_BASIC_INFORMATION {
    ULONG ExitStatus;
    PPEB PebBaseAddress;
    ULONG AffinityMask;
    ULONG BasePriority;
    ULONG UniqueProcessId;
    ULONG InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION, * PPROCESS_BASIC_INFORMATION;


DWORD CreateSuspendedProcess(LPCSTR processNameIn, const std::string& appModelIdOverride, PROCESS_INFORMATION& processInfoOut, int argc, char* argv[], Logger* logger);
DWORD GetRemoteBaseImageAddressFromPEB(const PROCESS_INFORMATION& processInfoIn, void*& addressOut, Logger* logger);
DWORD ResumeProcessThread(const PROCESS_INFORMATION& processInfoIn, Logger* logger);