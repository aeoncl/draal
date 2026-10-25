#include "../libs/logger.h"
#include <Windows.h>
#include <string>
#include <vector>

enum class InstrumentationType {
	Suspend,
	Debugger
};

class DraalConfig {

public: 
	std::string targetExecutable;
	std::vector<std::string> dllsToRemove;
	std::vector<std::string> dllsToInject;
	bool loggingEnabled;
	InstrumentationType instrumentationType;
	static DraalConfig LoadConfig(Logger* LOGGER, DWORD& errorCode);
	static DraalConfig Empty();

};

std::string ResolveDefaultLogDirectory(DWORD& errorCode);
InstrumentationType deserializeInstrumentationTypeOrDefault(const char* instrumentationType);
Logger* CreateLogger(bool enabled, const char* name);

