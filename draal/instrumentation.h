#pragma once
#include <Windows.h>
#include "config.h"
#include "../libs/logger.h"

DWORD CreateInterruptedProcess(LPCSTR processNameIn, PROCESS_INFORMATION& processInfoOut, int argc, char* argv[], InstrumentationType& instrumentationType, Logger* logger);
DWORD GetBaseImageAddress(const PROCESS_INFORMATION& processInfoIn, void*& baseImageAddressOut, InstrumentationType& instrumentationType, Logger* logger);
DWORD ResumeProcess(const PROCESS_INFORMATION& processInfoIn, InstrumentationType& instrumentationType, Logger* logger);

