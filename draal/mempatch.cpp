#pragma once
#include "mempatch.h"
#include <detours.h>
#pragma comment(lib, "detours")

DWORD SanitizeImportAddressTable(HANDLE processIn, void* baseImageAddressIn, const std::vector<std::string>& dllsToRemove, Logger* logger) {
	IMAGE_DOS_HEADER dos_header;
	BOOL readDosResult = ReadProcessMemory(processIn, baseImageAddressIn, &dos_header, sizeof(dos_header), NULL);
	if (!readDosResult) {
		DWORD error = GetLastError();
		logger->LogLine("Fatal: Could not read DOS Header. ReadProcessMemory failed with error: 0x%x", error);
		return error;
	}

	IMAGE_NT_HEADERS nt_headers;
	void* nt_headers_addr = (char*)baseImageAddressIn + dos_header.e_lfanew;
	BOOL readNtResult = ReadProcessMemory(processIn, nt_headers_addr, &nt_headers, sizeof(nt_headers), NULL);
	if (!readNtResult) {
		DWORD error = GetLastError();
		logger->LogLine("Fatal: Could not read NT Header. ReadProcessMemory failed with error: 0x%x", error);
		return error;
	}

	IMAGE_DATA_DIRECTORY imageDataDirectory = nt_headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (imageDataDirectory.Size == 0) {
		//We know that our target binary has an import table so, fail here.
		logger->LogLine("Fatal: Could not find Import Directory in NT Header");
		return ERROR_APP_INIT_FAILURE;
	}

	void* currentImportDescriptorAddress = (char*)baseImageAddressIn + imageDataDirectory.VirtualAddress;
	IMAGE_IMPORT_DESCRIPTOR currentImportDescriptor;

	DWORD passedCount = 0;
	for (;;) {
		BOOL readImportDescriptorResult = ReadProcessMemory(processIn, currentImportDescriptorAddress, &currentImportDescriptor, sizeof(currentImportDescriptor), NULL);
		if (!readImportDescriptorResult) {
			DWORD error = GetLastError();
			logger->LogLine("Fatal: Could not read Import Descriptor. ReadProcessMemory failed with error: 0x%x", error);
			return error;
		}

		if (currentImportDescriptor.Name == 0 && currentImportDescriptor.FirstThunk == 0)
			break;

		char dllName[256] = {};
		void* dllNameAddress = (char*)baseImageAddressIn + currentImportDescriptor.Name;
		BOOL readDllNameResult = ReadProcessMemory(processIn, dllNameAddress, dllName, sizeof(dllName) - 1, NULL);
		if (!readDllNameResult) {
			DWORD error = GetLastError();
			logger->LogLine("Fatal: Could not read DllName in Import Descriptor. ReadProcessMemory failed with error: 0x%x", error);
			return error;
		}

		if (IsDllBlacklisted(dllName, dllsToRemove)) {
			logger->LogLine("Stripping DLL: %s imageDataDirectorySize: %d passedCount: %d", dllName, imageDataDirectory.Size, passedCount);

			DWORD imageDataDirectoryCount = imageDataDirectory.Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);
			SIZE_T remainingBytes = (imageDataDirectoryCount - passedCount) * sizeof(IMAGE_IMPORT_DESCRIPTOR);

			IMAGE_IMPORT_DESCRIPTOR zeroed = { 0 };
			BOOL removeDllResult = WriteProcessMemory(processIn, currentImportDescriptorAddress, &zeroed, sizeof(zeroed), NULL);
			if (!removeDllResult) {
				DWORD error = GetLastError();
				logger->LogLine("Fatal: Could not remove Import Descriptor for DLL %s. WriteProcessMemory failed with error: 0x%x", dllName, error);
				return error;
			}

			if (remainingBytes != 0) {
				logger->LogLine("Shifting remaining entries... remainingBytes: %d", remainingBytes);
				//Shifting remaining entries

				void* nextDescriptorAddress = (char*)currentImportDescriptorAddress + sizeof(IMAGE_IMPORT_DESCRIPTOR);
				std::vector<BYTE> buffer(remainingBytes);
				BOOL readRemainingImageDataDirectoryResult = ReadProcessMemory(processIn, nextDescriptorAddress, buffer.data(), remainingBytes, NULL);
				if (!readRemainingImageDataDirectoryResult) {
					DWORD error = GetLastError();
					logger->LogLine("Fatal: Could not read remaining Image Data Directory to shift. ReadProcessMemory failed with error: 0x%x", error);
					return error;
				}

				logger->LogLine("Buffer size %d...", buffer.size());

				BOOL writeRemainingImageDataDirectoryResult = WriteProcessMemory(processIn, currentImportDescriptorAddress, buffer.data(), remainingBytes, NULL);
				if (!writeRemainingImageDataDirectoryResult) {
					DWORD error = GetLastError();
					logger->LogLine("Fatal: Could not write remaining Image Data Directory to shift. WriteProcessMemory failed with error: 0x%x", error);
					return error;
				}

				//Replay current offset because we shifted
				continue;
			}
		}
		else {
			logger->LogLine("Keeping DLL: %s", dllName);
		}

		passedCount++;
		currentImportDescriptorAddress = (char*)currentImportDescriptorAddress + sizeof(IMAGE_IMPORT_DESCRIPTOR);

	}

	return ERROR_SUCCESS;
}

DWORD InjectLibrairies(HANDLE processIn, const std::vector<std::string>& dllNames, Logger* logger) {

	std::vector<LPCSTR> ptrs;
	for (const auto& dll : dllNames) {
		logger->LogLine("Injecting DLL: %s...", dll.c_str());
		ptrs.push_back(dll.c_str());
	}

	BOOL result = DetourUpdateProcessWithDll(processIn, ptrs.data(), static_cast<DWORD>(ptrs.size()));
	if (!result) {
		DWORD error = GetLastError();
		logger->LogLine("Fatal: Could not inject DLLS in process. DetourUpdateProcessWithDll failed with error: 0x%x", error);
		return error;
	}

	return ERROR_SUCCESS;
}


bool IsDllBlacklisted(const char* dllName, const std::vector<std::string>& dllsToRemove) {
	for (const auto& dllToRemove : dllsToRemove) {
		if (_stricmp(dllName, dllToRemove.c_str()) == 0)
			return true;
	}
	return false;
}