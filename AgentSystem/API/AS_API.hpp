//This is supposed to be thin, for direct control from the application only:
//- Initializing and quitting;
//- Running, stopping, pausing and unpausing;
//- Saving and loading. 
//All other communication (ie, reading and sending data) through the CL_externalAPI;

#pragma once

#include "core.hpp"

namespace AS {
	
	AS_API bool initializeASandCL();

	//Loads network and instantiates appropriate Client Data Handler.
	//If active, stops AS's main loop before loading.
	//Client is blocked from issuing data before new Client Data Handler is instantiated.
	//v
	//- WARNING: CLEARS active Network and Client Data Handler, no confirmation needed! 
	//Any logic to save current network first and etc should be handled by the CLIENT.
	AS_API bool loadNetworkFromFile(std::string name, bool runNetwork = false);

	//Creates thread to run AS's main loop, if it doesn't exist already. Stores the thread::id.
	//Checks some conditions before initializing, and returns false if any are not met.
	//AS has to be initialized and a network must have been loaded.
	AS_API bool run(bool fixedTimeStep = false);

	//Stops AS execution thread, marks it as stopped and clears the stored thread::id;
	//Returns false if this fails or if the Main Loop found errors while running.
	AS_API bool stop();

	AS_API bool isMainLoopRunning();
	AS_API bool chekIfMainLoopShouldBeRunning();

	//Pausing already paused loop has no effect. Effectively starts at the END of mainLoop.
	//Pause sleeps in cycles of targetStepTime until unpaused.
	AS_API void pauseMainLoop();

	//Unpausing already unpaused loop has no effect. Resumes after up to half target step time.
	AS_API void unpauseMainLoop();

	//Steps the mainLoop for "steps" steps and then pauses. If steps < 1, will treat as 1.
	//Steps are checked right before pausing: a single step is the same as unpause + pause.
	AS_API void stepMainLoopFor(int steps = 1);

	AS_API bool chekIfMainLoopShouldBePaused();
	AS_API bool checkIfMainLoopIsPaused();

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
		                             bool willResumeAfterSave = true, bool silent = false);

	//For now, this is mostly an alias to stop(), but cheks if running before calling it.
	//Future intended use is to also clean up mirror/client data and "unitialize" AS and CL.
	AS_API bool quit();
}
