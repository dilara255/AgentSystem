/*
Classes and functions on this file are responsible for the coordination of the AS.
This includes:
- Coordinating initialization and termination;
- Main loop;
- Access/references to the different systems;
- Bulk data transfer to/from CL;
(may separete some if this file gets too large)
*/

#include "miscStdHeaders.h"

#include "AS_API.hpp"

#include "CL_internalAPI.hpp"

#include "network/parameters.hpp" //exposes "currentNetworkParams"
#include "data/agentDataControllers.hpp" //exposes "dataControllers"
#include "systems/actionSystem.hpp"

#include "network/fileManager.hpp"

#include "systems/AScoordinator.hpp"

//TO DO:
// - Create control class with control structures as members

bool isInitialized = false;

namespace AS {
	//these are the control structures/objects for the AS's main systems;
	//they're kinda in global scope for this file for now : )
	
	bool shouldMainLoopBeRunning;
	std::thread::id mainLoopId;
	std::thread mainLoopThread;

	networkParameters_t currentNetworkParams;

	ActionSystem actionSystem; 
	dataControllerPointers_t agentDataControllerPtrs;
	networkParameters_t* currentNetworkParams_ptr;

	const ActionSystem* actionSystem_cptr;
	const ActionDataController* actionDataController_cptr;
	const networkParameters_t* currentNetworkParams_cptr;
	const dataControllerPointers_t* agentDataControllers_cptr;
}

bool AS::quit() {
	
	return AS::stop();
}

bool AS::run() {
	LOG_TRACE("Starting Main Loop Thread...");
	
	if (shouldMainLoopBeRunning) {
		if (mainLoopThread.joinable()) {
			LOG_CRITICAL("Main Loop Thread already be running!");
			return false;
		}
		LOG_WARN("Main Loop Thread state control variable was set wrong. Will try to fix and start thread");
	}

	shouldMainLoopBeRunning = true;
	mainLoopThread = std::thread(mainLoop);
	mainLoopId = mainLoopThread.get_id();
	currentNetworkParams.lastMainLoopStartingTick = currentNetworkParams.mainLoopTicks;

	LOG_INFO("Started main loop on new thread");

	return true;
}

bool AS::stop() {
	LOG_TRACE("Stopping Main Loop Thread...");
		
	if (!shouldMainLoopBeRunning) {
		LOG_ERROR("Main Loop Thread already supposed to be stopped...");
		return false;
	}

	shouldMainLoopBeRunning = false;
		
	if (mainLoopThread.joinable()) {
		LOG_TRACE("Waiting for main loop to finish execution...");
		mainLoopThread.join();
		LOG_INFO("Done.");
		#ifdef AS_DEBUG
			printf("\nRan for %llu ticks\n", currentNetworkParams.mainLoopTicks - 
				                           currentNetworkParams.lastMainLoopStartingTick);
		#endif // AS_DEBUG		
		return true;
	}
	else {
		LOG_ERROR("Main Loop Thread was supposed to be active, but was not!");
		return false;
	}	
}

void AS::mainLoop() {
	uint64_t initialTick = currentNetworkParams.mainLoopTicks;
	
	do {
		sendReplacementDataToCL();
		currentNetworkParams.mainLoopTicks++;
		std::this_thread::sleep_for(std::chrono::milliseconds(TST_MAINLOOP_FREQUENCY_MS));
	} while (shouldMainLoopBeRunning);
}

bool AS::isMainLoopRunning() {
	return shouldMainLoopBeRunning;
}

bool AS::initializeASandCL() {
//TO DO: possibly extract functions

	if (isInitialized) {
		LOG_ERROR("AS already initialized. Aborting initialization.");
		return false;
	}

	LOG_INFO("Loggers Initialized");

	//Agent Data and Network Parameters:
	currentNetworkParams.isNetworkInitialized = false;
	agentDataControllerPtrs.haveBeenCreated = false;
	agentDataControllerPtrs.haveData = false;

	currentNetworkParams_cptr = (const networkParameters_t*)&currentNetworkParams;
	currentNetworkParams_ptr = &currentNetworkParams;
	agentDataControllers_cptr = (const dataControllerPointers_t*)&agentDataControllerPtrs;

	currentNetworkParams.lastMainLoopStartingTick = 0;
	currentNetworkParams.mainLoopTicks = 0;

	bool result = createAgentDataControllers(&agentDataControllerPtrs);
	if (!result) { return false; }

	#ifdef AS_DEBUG
		printf("\n\nData Controllers NON-CONST ptr: %p\n", &agentDataControllerPtrs);
		printf("Data Controllers CONST ptr:     %p\n\n", agentDataControllers_cptr);
		printf("\n\nLAcoldData_ptr             : %p\n", agentDataControllers_cptr->LAcoldData_ptr);
		printf("LAcoldData_ptr(from const) : % p\n\n", agentDataControllerPtrs.LAcoldData_ptr);
	#endif // AS_DEBUG 

	if (agentDataControllers_cptr !=
		(const dataControllerPointers_t*)&agentDataControllerPtrs) {
		LOG_CRITICAL("Pointer and Const Pointer to Data Control are not matching!");
		return false;
	}

	if (agentDataControllerPtrs.LAcoldData_ptr == NULL) {
		LOG_CRITICAL("DATA CONTROLLERS NOT CREATED: ptr to LAcold is NULL");
		return false;
	}

	//Action System:
	result = actionSystem.initialize(&actionSystem_cptr);
	result &= actionSystem_cptr->isInitialized();
	if (!result) {
		LOG_CRITICAL("Something went wrong initialing the Action System!");
		return false;
	}

	result = actionSystem.initializeDataController(currentNetworkParams_cptr,
		&actionDataController_cptr);
	if (!result) {
		LOG_CRITICAL("Something went wrong initialing the Action Data Controlers!");
		return false;
	}

	//Communications Layer:
	result = CL::init();
	if (!result) {
		LOG_CRITICAL("Something went wrong initialing the Communications Layer!");
		return false;
	}

	//Done!
	isInitialized = true;

	LOG_INFO("Initialized");

	return true;
}

bool AS::sendReplacementDataToCL() {

//agentDataControllers_cptr.
	return  CL::acceptReplacementData(currentNetworkParams_cptr,
				actionDataController_cptr->getActionsLAsCptr(),
				actionDataController_cptr->getActionsGAsCptr(),
				agentDataControllers_cptr->LAcoldData_ptr->getDataCptr(),
				agentDataControllers_cptr->LAstate_ptr->getDataCptr(),
				agentDataControllers_cptr->LAdecision_ptr->getDataCptr(),
				agentDataControllers_cptr->GAcoldData_ptr->getDataCptr(),
				agentDataControllers_cptr->GAstate_ptr->getDataCptr(),
				agentDataControllers_cptr->GAdecision_ptr->getDataCptr());	
}

namespace AS {
	void clearNetwork() {
		agentDataControllerPtrs.GAcoldData_ptr->clearData();
		agentDataControllerPtrs.LAcoldData_ptr->clearData();
		agentDataControllerPtrs.GAstate_ptr->clearData();
		agentDataControllerPtrs.LAstate_ptr->clearData();
		agentDataControllerPtrs.GAdecision_ptr->clearData();
		agentDataControllerPtrs.LAdecision_ptr->clearData();
		agentDataControllerPtrs.haveData = false;
		currentNetworkParams.isNetworkInitialized = false;

		actionSystem.data.clearData();

		LOG_TRACE("Network Cleared");
	}
}

bool AS::loadNetworkFromFile(std::string name, bool runNetwork) {
	LOG_INFO("Trying to load network from a file");

	if (!isInitialized) {
		LOG_ERROR("Agent System and Communications Layer must be initialized before loading. Aborting.");
		return false;
	}

	FILE* fp = acquireFilePointerToLoad(name);

	if (fp == NULL) {
		LOG_ERROR("Failed to open file, aborting load. Current network preserved.");
		return false;
	}

	if (!fileIsCompatible(fp)) {
		LOG_ERROR("File format version incompatible. Aborting load. Current network preserved.");
		fclose(fp);
		LOG_TRACE("File closed");
		return false;
	}

	clearNetwork(); //in order to load the new one
	strcpy(currentNetworkParams.name, name.c_str());
	LOG_TRACE("File Acquired and of compatiple version. Network cleared and new name set");

	bool result;
	result = loadNetworkFromFileToDataControllers(fp, agentDataControllerPtrs, 
		                                              currentNetworkParams_ptr,
		                                                     &actionSystem.data);
	if (!result) {
		LOG_ERROR("Load failed. Will clear the network.");
		clearNetwork(); //we don't leave an incomplete state behind. Marks data as not initialized.
	}
	
	//TO DO: do whatever else, clear on error

	if(result){
		agentDataControllerPtrs.haveData = true;
		currentNetworkParams.isNetworkInitialized = true;
	}

	fclose(fp);
	LOG_TRACE("File closed");
	
	//TO DO: check capacities and sizes to make sure things are in order

	LOG_INFO("File loaded...");

	if(result && runNetwork) { result = AS::run(); }

	return result; //not much information is given, but the app may decide what to do on failure
}

bool AS::saveNetworkToFile(std::string name, bool shouldOverwrite) {
	LOG_INFO("Trying to save network to a file...");

	bool result;
	bool shouldResumeThread = false;

	if (shouldMainLoopBeRunning) {
		LOG_TRACE("Will stop main loop (if saving fails, mainLoop is not resumed either)");
		result = stop();
		if (result) {
			shouldResumeThread = true;
			LOG_TRACE("Will resume main loop after saving");
		}
		else {
			LOG_WARN("Something weird is going on with the Main Loop Thread. Saving will proceed but thread won't be resumed...");
		}
	}

	if (!isInitialized) {
		LOG_ERROR("Agent System and Communications Layer must be initialized before saving. Aborting.");
		return false;
	}

	if (!(agentDataControllerPtrs.haveData && currentNetworkParams.isNetworkInitialized)) {
		LOG_ERROR("Network not properly initialized. Can't save. Aborting.");
		return false;
	}

	if (name == "") {
		LOG_TRACE("Using name stored on network params");
		name = currentNetworkParams.name;
		#ifdef AS_DEBUG
			printf("Name: %s\n", name.c_str());
		#endif // AS_DEBUG

	}

	FILE* fp = acquireFilePointerToSave(name, shouldOverwrite);
	
	if (fp == NULL) {
		LOG_ERROR("Failed to create new file, aborting save.");
		return false;
	}

	LOG_TRACE("File Acquired. Will save network");
	
	result = createNetworkFileFromData(fp, agentDataControllers_cptr, 
									        currentNetworkParams_cptr,
		                                    actionDataController_cptr);
	if (!result) {
		LOG_ERROR("Saving failed!");
		return false;
	}
	else {
		LOG_INFO("Network Saved to File!");
	}

	fclose(fp);
	LOG_TRACE("File closed.");
	
	if (shouldResumeThread) { 
		LOG_TRACE("File closed. Will resume mainLoop"); 
		result &= run(); 
	}

	return result;
}