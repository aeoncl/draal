#pragma once
#include "instrumentation.h"
#include "debuggerInstrumentation.h"
#include "suspendInstrumentation.h"

DWORD CreateInterruptedProcess(LPCSTR processNameIn, const std::string& appModelIdOverrideIn, PROCESS_INFORMATION& processInfoOut, int argc, char* argv[], InstrumentationType& instrumentationType, Logger* logger)
{
	switch (instrumentationType) {
		case InstrumentationType::Suspend: return CreateSuspendedProcess(processNameIn, appModelIdOverrideIn, processInfoOut, argc, argv, logger);
		case InstrumentationType::Debugger: return CreateDebuggedProcess(processNameIn, appModelIdOverrideIn, processInfoOut, argc, argv, logger);
	}
}

DWORD GetBaseImageAddress(const PROCESS_INFORMATION& processInfoIn, void*& baseImageAddressOut, InstrumentationType& instrumentationType, Logger* logger)
{
	switch (instrumentationType) {
		case InstrumentationType::Suspend: return GetRemoteBaseImageAddressFromPEB(processInfoIn, baseImageAddressOut, logger);
		case InstrumentationType::Debugger: return GetBaseImageAddressViaDebugEvent(processInfoIn, baseImageAddressOut, logger);
	}
}

DWORD ResumeProcess(const PROCESS_INFORMATION& processInfoIn, InstrumentationType& instrumentationType, Logger* logger)
{
	switch (instrumentationType) {
	case InstrumentationType::Suspend: return ResumeProcessThread(processInfoIn, logger);
	case InstrumentationType::Debugger: return ResumeDebuggedProcessAndDetach(processInfoIn, logger);
	}
}
