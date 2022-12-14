#pragma once

#include "core.hpp"

//This is supposed to be thin, for direct control from the application only:
//initializing, saving, loading, quitting, pretty much. Other communication through CL


//****For Testing****
#define AS_TST_INIT_EXPECTED_NUMBER 5619419
#define AS_TST_EXPECTED_ARR0 231879
#define AS_TST_EXPECTED_ARR1 954263
static const char* fileNameNoDefaults = "testNetworkNoDefaults.txt";
static const char* fileNameWithDefaults = "testNetworkWithDefaults.txt";
static const char* customFilename = "TestFileCustomName.txt";
//*******************

namespace AS {
	
	AS_API bool initializeASandCL();
	
	//WARNING: loading clears current network, no confirmation needed!
	//Logic to save current network first and etc should be handled by the CLIENT
	//Save Always FAILS if a file of the same name already exists
	AS_API bool loadNetworkFromFile(std::string name);
	AS_API bool saveNetworkToFile(std::string name = "");

	//****For Testing****
	AS_API void CLsanityTest();
	AS_API bool testContainersAndAgentObjectCreation();
	AS_API void sayHello();
	AS_API bool testFileCreation(std::string nameNoDefaults, std::string nameWithDefaults);
	AS_API bool testDataTransferFromAStoCL(void);
	AS_API bool testGotNewValuesFromASthroughCL();
	//*******************
}
