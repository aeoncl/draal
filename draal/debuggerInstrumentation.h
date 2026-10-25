#pragma once
#include <Windows.h>
#include "../libs/logger.h"

DWORD CreateDebuggedProcess(LPCSTR processNameIn, const std::string& appModelIdOverride, PROCESS_INFORMATION& processInfoOut, int argc, char* argv[], Logger* logger);
DWORD GetBaseImageAddressViaDebugEvent(const PROCESS_INFORMATION& processInfoIn, void*& baseImageAddressOut, Logger* logger);
DWORD ResumeDebuggedProcessAndDetach(const PROCESS_INFORMATION& processInfoIn, Logger* logger);