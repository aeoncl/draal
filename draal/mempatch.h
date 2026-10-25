#pragma once
#include <vector>
#include <Windows.h>
#include "../libs/logger.h"


DWORD SanitizeImportAddressTable(HANDLE processIn, void* baseImageAddressIn, const std::vector<std::string>& dllsToRemove, Logger* logger);
DWORD InjectLibrairies(HANDLE processIn, const std::vector<std::string>& dllNames, Logger* logger);
bool IsDllBlacklisted(const char* dllName, const std::vector<std::string>& dllsToRemove);