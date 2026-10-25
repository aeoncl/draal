#pragma once
#include "suspendInstrumentation.h"
#include "../libs/directoryUtils.h"
#include <vector>

DWORD CreateSuspendedProcess(LPCSTR processNameIn, const std::string& appModelIdOverride, PROCESS_INFORMATION& processInfoOut, int argc, char* argv[], Logger* logger)
{
	std::string exeDir = GetProcessDirectory();
	std::string targetPath = exeDir + processNameIn;

	std::string commandLine = "\"" + targetPath + "\"";
	for (int i = 1; i < argc; i++) {
		commandLine += " ";
		commandLine += "\"";
		commandLine += argv[i];
		commandLine += "\"";
	}

	logger->LogLine("Command line args to passthrough: %s", commandLine.c_str());

	std::vector<char> commandLineBuffer(commandLine.begin(), commandLine.end());
	commandLineBuffer.push_back('\0');

	STARTUPINFOA startupInfo = {};
	if (!appModelIdOverride.empty()) {
		startupInfo.dwFlags |= STARTF_TITLEISAPPID;
		startupInfo.lpTitle = (LPSTR)appModelIdOverride.c_str();
	}
	startupInfo.cb = sizeof(STARTUPINFOA);

	BOOL result = CreateProcessA(
		targetPath.c_str(),
		commandLine.empty() ? NULL : commandLineBuffer.data(),
		NULL,
		NULL,
		FALSE,
		CREATE_SUSPENDED,
		NULL,
		exeDir.c_str(),
		&startupInfo,
		&processInfoOut
	);

	if (!result) {
		DWORD error = GetLastError();
		logger->LogLine("Could not create %s process. Error Code: 0x%x", processNameIn, error);
		return error;
	}

	logger->LogLine("%s started suspended (pid %lu)", processNameIn, processInfoOut.dwProcessId);
	return ERROR_SUCCESS;
}

DWORD GetRemoteBaseImageAddressFromPEB(const PROCESS_INFORMATION& processInfoIn, void*& addressOut, Logger* logger)
{
	NtQueryInformationProcess_t NtQueryInformationProcess = (NtQueryInformationProcess_t)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
	if (NtQueryInformationProcess == nullptr) {
		logger->LogLine("Fatal: Could not find NtQueryInformationProcess in ntdll.dll");
		return ERROR_APP_INIT_FAILURE;
	}

	PROCESS_BASIC_INFORMATION pbi = {};
	LONG ntQueryResult = NtQueryInformationProcess(processInfoIn.hProcess, 0, &pbi, sizeof(pbi), NULL);
	if (ntQueryResult != 0) {
		logger->LogLine("Fatal: NtQueryInformationProcess failed with NT_ERROR: 0x%x", ntQueryResult);
		return ntQueryResult;
	}

	PEB peb = {};
	BOOL readProcessMemoryResult = ReadProcessMemory(processInfoIn.hProcess, pbi.PebBaseAddress, &peb, sizeof(peb), NULL);
	if (!readProcessMemoryResult) {
		DWORD error = GetLastError();
		logger->LogLine("Fatal: Could not extract ImageBaseAddress from PEB. ReadProcessMemory failed with error: 0x%x", error);
		return error;
	}

	addressOut = peb.ImageBaseAddress;
	return ERROR_SUCCESS;
}

DWORD ResumeProcessThread(const PROCESS_INFORMATION& processInfoIn, Logger* logger)
{
	DWORD resumeResult = ResumeThread(processInfoIn.hThread);
	if (resumeResult == -1) {
		DWORD error = GetLastError();
		logger->LogLine("Fatal: Failed to ResumeThread: 0x%x", error);
		return error;
	}
	return ERROR_SUCCESS;
}
