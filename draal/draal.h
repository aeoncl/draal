#pragma once
#include <vector>
#include "config.h"


int main(int argc, char* argv[]);
void SetupLogger(bool enabled, const char* name);
void LogInstrumentationType(InstrumentationType& instrumentationType);
void Cleanup();