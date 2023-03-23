#pragma once

#include "core.hpp"

#define AS_TST_INIT_EXPECTED_NUMBER 5619419
#define AS_TST_EXPECTED_ARR0 231879
#define AS_TST_EXPECTED_ARR1 954263
static const char* fileNameNoDefaults = "testNetworkNoDefaults.txt";
static const char* fileNameWithDefaults = "testNetworkWithDefaults.txt";
static const char* customFilename = "TestFileCustomName.txt";
static const char* updateTestFilename = fileNameWithDefaults;
static const char* updateTestOutputFilename = "updateTestOut.txt";

namespace AS {
	AS_API void CLsanityTest();
	AS_API bool testContainersAndAgentObjectCreation();
	AS_API void sayHello();
	AS_API bool testFileCreation(std::string nameNoDefaults, std::string nameWithDefaults);
	AS_API bool testDataTransferFromAStoCL(void);
	AS_API bool testGotNewValuesFromASthroughCL();
	AS_API bool testMainLoopStopAndRestart();
	AS_API bool testNeighbourIDsetting();
	AS_API bool testChoppedPRNdrawing(bool printResults, bool dump);
	AS_API bool testActionVariationsInfo(bool printResults = false);
	AS_API bool testMultipleAgentChopCalculations(bool log = false);
	AS_API bool testWarningAndErrorCountingAndDisplaying(bool printResults = false);
	AS_API bool testDecisionStepTiming(bool printResults = false);
	AS_API bool testUpdateRead(bool printResults = false);
}