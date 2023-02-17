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
	AS_API bool quit();	
	AS_API bool isMainLoopRunning();
	AS_API bool chekIfMainLoopShouldBeRunning();
	AS_API bool chekIfMainLoopShouldBePaused();
	//pausing already paused loop has no effect. Pause effective starts after mainLoop step
	//pause sleeps in cycles of targetStepTime until unpaused
	AS_API void pauseMainLoop();
	//unpausing already unpaused loop has no effect. Resumes after up to target step time
	AS_API void unpauseMainLoop();

	//Loads network and instantiates appropriate Client Data Handler.
	//If active, stops AS's main loop before loading.
	//Client is blocked from issuing data before new Client Data Handler is instantiated.
	//v
	//- WARNING: CLEARS active Network and Client Data Handler, no confirmation needed! 
	//Any logic to save current network first and etc should be handled by the CLIENT.
	AS_API bool loadNetworkFromFile(std::string name, bool runNetwork = false);

	//Saves active network. If AS's main loop is running, saves AFTER step is done.
	//Client changes issued BEFORE call will be consumed before saving.
	//If network is running, other client changes issued after call to this but before
	//the network is stopped will also be saved.
	//If we're not resuming after save, then issuing of further changes will be blocked.
	//AS's main loop will only actually resume if previously active.
	//Default fileName uses network name (as stored by AS).
	//NOTE: if willResumeAfterSave == false, issuing changes while AS sleeps before saving
	//makes these changes be absorbed one step before they would otherwise, in order to
	//preserve issued changes.
	AS_API bool saveNetworkToFile(std::string fileName = "", bool shouldOverwrite = false,
		                                                  bool willResumeAfterSave = true);

	//****For Testing****
	AS_API void CLsanityTest();
	AS_API bool testContainersAndAgentObjectCreation();
	AS_API void sayHello();
	AS_API bool testFileCreation(std::string nameNoDefaults, std::string nameWithDefaults);
	AS_API bool testDataTransferFromAStoCL(void);
	AS_API bool testGotNewValuesFromASthroughCL();
	AS_API bool testMainLoopStopAndRestart();
	//*******************
}
