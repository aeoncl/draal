#include "instrumentation.h"
#include "debuggerInstrumentation.h"

DWORD CreateInterruptedProcess(LPCSTR processNameIn, PROCESS_INFORMATION& processInfoOut, int argc, char* argv[], InstrumentationType& instrumentationType, Logger* logger)
{
	switch (instrumentationType) {
		case InstrumentationType::Suspend: return -1;
		case InstrumentationType::Debugger: return CreateDebuggedProcess(processNameIn, processInfoOut, argc, argv, logger);
	}
}

DWORD GetBaseImageAddress(const PROCESS_INFORMATION& processInfoIn, void*& baseImageAddressOut, InstrumentationType& instrumentationType, Logger* logger)
{
	switch (instrumentationType) {
		case InstrumentationType::Suspend: return -1;
		case InstrumentationType::Debugger: return GetBaseImageAddressViaDebugEvent(processInfoIn, baseImageAddressOut, logger);
	}
}

DWORD ResumeProcess(const PROCESS_INFORMATION& processInfoIn, InstrumentationType& instrumentationType, Logger* logger)
{
	switch (instrumentationType) {
	case InstrumentationType::Suspend: return -1;
	case InstrumentationType::Debugger: return ResumeDebuggedProcessAndDetach(processInfoIn, logger);
	}
}
