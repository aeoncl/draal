#pragma once
#include "../libs/logger.h"
#include <Windows.h>
#include <string>
#include <vector>

enum class InstrumentationType {
	//Uses CREATE_SUSPENDED flag + NtQueryInformationProcess
	Suspend,
	//Uses a debugger, works with wine but less elegant
	Debugger
};

class DraalConfig {

public: 
	std::string targetExecutable;
	std::vector<std::string> dllsToRemove;
	std::vector<std::string> dllsToInject;
	std::string appModelIdOverride;
	bool loggingEnabled;
	InstrumentationType instrumentationType;
	static DraalConfig LoadConfig(Logger* LOGGER, DWORD& errorCode);
	static DraalConfig Empty();

};

std::string ResolveDefaultLogDirectory(DWORD& errorCode);
InstrumentationType deserializeInstrumentationTypeOrDefault(const char* instrumentationType);
Logger* CreateLogger(bool enabled, const char* name);

