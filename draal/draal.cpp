#define WIN32_LEAN_AND_MEAN 

#include <iostream>
#include <windows.h>
#include "../libs/logger.h"
#include "../libs/directoryUtils.h"
#include "draal.h"
#include <detours.h>
#include "config.h"
#include "mempatch.h"
#include "instrumentation.h"
#pragma comment(lib, "detours")

Logger* LOGGER = nullptr;
PROCESS_INFORMATION PROCESS_INFO = {};

int main(int argc, char* argv[])
{
	Logger* bootstrapLogger = CreateLogger(true, "init");
	DWORD configError = ERROR_SUCCESS;
	DraalConfig config = DraalConfig::LoadConfig(bootstrapLogger, configError);
	if (configError != ERROR_SUCCESS) {
		bootstrapLogger->LogLine("Failed to load config: %d", configError);
		return configError;
	}
	delete bootstrapLogger;

	SetupLogger(config.loggingEnabled, config.targetExecutable.c_str());

	DWORD createMainProcessResult = CreateInterruptedProcess(config.targetExecutable.c_str(), PROCESS_INFO, argc, argv, config.instrumentationType, LOGGER);

	if (createMainProcessResult != ERROR_SUCCESS) {
		Cleanup();
		return createMainProcessResult;
	}

	void* baseImageAddress = nullptr;
	DWORD baseImageAddressResult = GetBaseImageAddress(PROCESS_INFO, baseImageAddress, config.instrumentationType, LOGGER);
	if (baseImageAddressResult != ERROR_SUCCESS) {
		Cleanup();
		return baseImageAddressResult;
	}

	LOGGER->LogLine("Got base Image Addr: 0x%p", baseImageAddress);

	DWORD sanitizeIATResult = SanitizeImportAddressTable(PROCESS_INFO.hProcess, baseImageAddress, config.dllsToRemove, LOGGER);
	if (sanitizeIATResult != ERROR_SUCCESS) {
		Cleanup();
		return sanitizeIATResult;
	}

	DWORD injectResult = InjectLibrairies(PROCESS_INFO.hProcess, config.dllsToInject, LOGGER);
	if (injectResult != ERROR_SUCCESS) {
		Cleanup();
		return injectResult;
	}

	DWORD resumeResult = ResumeProcess(PROCESS_INFO, config.instrumentationType, LOGGER);
	if (resumeResult != ERROR_SUCCESS) {
		Cleanup();
		return resumeResult;
	}

	Cleanup();
	return EXIT_SUCCESS;
}


void SetupLogger(bool enabled, const char* name) {
	LOGGER = CreateLogger(enabled, name);

	LOGGER->LogLine("          A   A   A");
	LOGGER->LogLine("         | | | | | |");
	LOGGER->LogLine("       __| |_| |_| |___nnnnnn_____----____-===-----");
	LOGGER->LogLine("  _    __/--| |-| |-|_|---~~~-------~~~---\\==/~~\\.");
	LOGGER->LogLine("O=|-|OOOOO--<=X===X===X=>-| | |-----| | |>  HHK   |");
	LOGGER->LogLine("  ~    ~~\\--| |-| |-|~|---___-------___---/==\\__/'");
	LOGGER->LogLine("         | | | | | |");
	LOGGER->LogLine("         | | | | | |     DRAAL");
	LOGGER->LogLine("          V   V   V");
}

void Cleanup() {
	CloseHandle(PROCESS_INFO.hProcess);
	CloseHandle(PROCESS_INFO.hThread);

	if (LOGGER != nullptr) {
		delete LOGGER;
	}
}