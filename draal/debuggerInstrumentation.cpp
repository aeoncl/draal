#include "debuggerInstrumentation.h"
#include "../libs/directoryUtils.h"
#include <vector>

DWORD CreateDebuggedProcess(LPCSTR processNameIn, PROCESS_INFORMATION& processInfoOut, int argc, char* argv[], Logger* logger)
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
	startupInfo.cb = sizeof(STARTUPINFOA);

	BOOL result = CreateProcessA(
		targetPath.c_str(),
		commandLine.empty() ? NULL : commandLineBuffer.data(),
		NULL,
		NULL,
		FALSE,
		DEBUG_ONLY_THIS_PROCESS,
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

	logger->LogLine("%s started under debugger (pid %lu)", processNameIn, processInfoOut.dwProcessId);
	return ERROR_SUCCESS;
}

DWORD GetBaseImageAddressViaDebugEvent(const PROCESS_INFORMATION& processInfoIn, void*& baseImageAddressOut, Logger* logger)
{
	for (;;) {
		DEBUG_EVENT debugEvent = {};
		if (!WaitForDebugEvent(&debugEvent, 30000)) {
			DWORD error = GetLastError();
			logger->LogLine("Fatal: WaitForDebugEvent failed/timed out with error: 0x%x", error);
			return error != ERROR_SUCCESS ? error : WAIT_TIMEOUT;
		}

		if (debugEvent.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
			baseImageAddressOut = debugEvent.u.CreateProcessInfo.lpBaseOfImage;

			// The event hands us an open handle to the image file; we don't need it.
			if (debugEvent.u.CreateProcessInfo.hFile != NULL) {
				CloseHandle(debugEvent.u.CreateProcessInfo.hFile);
			}

			logger->LogLine("CREATE_PROCESS_DEBUG_EVENT received for pid %lu. Image base: 0x%p",
				debugEvent.dwProcessId, baseImageAddressOut);

			// Leave this event un-continued on purpose: the debuggee stays stopped
			// so the caller can patch the IAT before any of its code runs.
			return ERROR_SUCCESS;
		}

		if (debugEvent.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) {
			logger->LogLine("Fatal: target exited before CREATE_PROCESS_DEBUG_EVENT was seen");
			return ERROR_APP_INIT_FAILURE;
		}

		// Some other event arrived first; acknowledge it and keep waiting.
		ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE);
	}
}

DWORD ResumeDebuggedProcessAndDetach(const PROCESS_INFORMATION& processInfoIn, Logger* logger)
{
	// Keep the child alive once draal detaches and exits.
	DebugSetProcessKillOnExit(FALSE);

	// Acknowledge the pending CREATE_PROCESS_DEBUG_EVENT so the loader proceeds.
	if (!ContinueDebugEvent(processInfoIn.dwProcessId, processInfoIn.dwThreadId, DBG_CONTINUE)) {
		DWORD error = GetLastError();
		logger->LogLine("Fatal: ContinueDebugEvent failed with error: 0x%x", error);
		return error;
	}

	// Detach; the debuggee continues to run on its own.
	if (!DebugActiveProcessStop(processInfoIn.dwProcessId)) {
		DWORD error = GetLastError();
		logger->LogLine("Fatal: DebugActiveProcessStop failed with error: 0x%x", error);
		return error;
	}

	logger->LogLine("Patched and detached; target now running (pid %lu).", processInfoIn.dwProcessId);
	return ERROR_SUCCESS;
}
