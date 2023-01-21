/*
Classes and functions on this file are responsible for the coordination of the AS.
This includes:
- Coordinating initialization and termination;
- Main loop;
- Access/references to the different systems;
- Bulk data transfer to/from CL;
(may separete some if this file gets too large)

TODO-CRITICAL: Break up this file. Main loop should probably stay, move most of the rest
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
	
	bool shouldMainLoopBeRunning = false;
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
	
	bool result = AS::stop();
	
	//TO DO: Kill all the stuff in the pointers here
	//TO DO: implement and call CL::quit, doing the same for the CL;
	//isInitialized = false;
	
	return result;
}

//Creates thread to run AS's main loop, if it doesn't exist already. Stores the thread::id;
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
	currentNetworkParams.lastMainLoopStartingTick = currentNetworkParams.mainLoopTicks;
	mainLoopThread = std::thread(mainLoop);
	mainLoopId = mainLoopThread.get_id();

	LOG_INFO("Started main loop on new thread");

	return true;
}

//Main loop STARTS sleeping, does its thing*, consumes Client Changes and sends data to the CL.
//After this it just updates timmings. Ticks are updated right before AS sends its data.
//*includes progressing all actions and updating the states and the decisions of each agent;
//TO DO: implement what is described : )
void AS::mainLoop() {
	uint64_t initialTick = currentNetworkParams.mainLoopTicks;

	//std::chrono::microseconds stepStartTime;
	std::chrono::microseconds stepTimeMicro = std::chrono::microseconds(0);
	std::chrono::microseconds timeToSleepMicro = std::chrono::microseconds(TST_MAINLOOP_FREQUENCY_MS*1000);
	std::chrono::microseconds timeToSleepMicroThisStepMicro;
	bool result = false;

	do {

		#if (defined AS_DEBUG) || VERBOSE_RELEASE
			printf("Should be running? %d,\n",(int)shouldMainLoopBeRunning);
		#endif

		if (!shouldMainLoopBeRunning) { LOG_TRACE("Main loop will sleep first"); }
		std::this_thread::sleep_for(timeToSleepMicroThisStepMicro);
		//stepStartTime = blah (get the current time in micro)
		
		//do stuff

		if (!shouldMainLoopBeRunning) { LOG_TRACE("Main loop will get Client Data"); }
		result = CL::getNewClientData(currentNetworkParams_ptr, &agentDataControllerPtrs,
			                          &(actionSystem.data), shouldMainLoopBeRunning);
		if (!result) { LOG_ERROR("Failed to get Client Data!"); }
		
		if (!shouldMainLoopBeRunning) { LOG_TRACE("Main loop will update ticks"); }
		currentNetworkParams.mainLoopTicks++;

		if (!shouldMainLoopBeRunning) { LOG_TRACE("Main loop will send AS Data to CL"); }
		result = sendReplacementDataToCL();
		if (!result) { LOG_ERROR("Failed to send AS Data to the CL!"); }

		if (!shouldMainLoopBeRunning) { LOG_TRACE("Main loop will calculate time to sleep"); }
		//stepTimeMicro = "current time in micro" - stepStartTime
		timeToSleepMicroThisStepMicro = timeToSleepMicro - stepTimeMicro;

		if (!shouldMainLoopBeRunning) { LOG_TRACE("Main loop should stop now!"); }
	} while (shouldMainLoopBeRunning);
}

//Stops AS execution thread, marks it as stopped and clears the stored thread::id;
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
		mainLoopId = std::thread::id(); //default-constructs to "no thread" value
		LOG_INFO("Done.");

		#if (defined AS_DEBUG) || VERBOSE_RELEASE
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

	#if (defined AS_DEBUG) || VERBOSE_RELEASE
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
	result = CL::init(currentNetworkParams_cptr);
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
	if(runNetwork) { LOG_INFO("Will start Main Loop if loading succeeds"); }

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

	//TO DO: BLOCK CLIENT CHANGES (add some "isLoading" flag on the CL?)

	if (shouldMainLoopBeRunning) {
		LOG_TRACE("Will stop Main Loop Thread");
		if(stop()){
			LOG_TRACE("Thread stopped");
		}
		else {
			LOG_WARN("Something weird is going on with the Main Loop Thread. Saving will proceed but thread won't be resumed...");
		}
	}

	clearNetwork(); //in order to load the new one
	strcpy(currentNetworkParams.name, name.c_str());
	LOG_TRACE("File Acquired and of compatiple version. Network cleared and new name set");

	bool result;
	result = loadNetworkFromFileToDataControllers(fp, agentDataControllerPtrs, 
		                                              currentNetworkParams_ptr,
		                                                     &actionSystem.data);
	
	fclose(fp);
	LOG_TRACE("File closed");

	if (!result) {
		LOG_ERROR("Load failed. Will clear the network.");
		clearNetwork(); //we don't leave an incomplete state behind. Marks data as not initialized.
	}
	
	//TO DO: check capacities and sizes to make sure things are in order?

	if(result){
		agentDataControllerPtrs.haveData = true;
		currentNetworkParams.isNetworkInitialized = true;
		LOG_INFO("File loaded...");

		//TO DO: This could be just an update(), which clears + shrinks + reserves stuff.
		result = CL::createClientDataHandler(*currentNetworkParams_ptr);
	}
	
	//TO DO: do whatever else should be done after load, clear on error
	
	if(result && runNetwork) { result = AS::run(); }

	return result; //not much information is given, but the app may decide what to do on failure
}

bool AS::saveNetworkToFile(std::string name, bool shouldOverwrite) {
	LOG_INFO("Trying to save network to a file...");

	if (!CL::blockClientDataForAmoment()) {
		LOG_ERROR("Couldn't acquire mutex to synchronize with Client Changes. Will abort saving.");
		return false;
	}

	if (!isInitialized) {
		LOG_ERROR("Agent System and Communications Layer must be initialized before saving. Aborting.");
		return false;
	}

	if (!(agentDataControllerPtrs.haveData && currentNetworkParams.isNetworkInitialized)) {
		LOG_ERROR("Network not properly initialized. Can't save. Aborting.");
		return false;
	}

	bool shouldResumeThread = false;

	if (shouldMainLoopBeRunning) {
		LOG_TRACE("Will stop main loop (if saving fails, mainLoop is not resumed either)");
		if(stop()){
			shouldResumeThread = true;
			LOG_TRACE("Will resume main loop after saving");
		}
		else {
			LOG_WARN("Something weird is going on with the Main Loop Thread. Saving will proceed but thread won't be resumed...");
		}
	}

	if (name == "") {
		LOG_TRACE("Using name stored on network params");
		name = currentNetworkParams.name;
		#if (defined AS_DEBUG) || VERBOSE_RELEASE
			printf("Name: %s\n", name.c_str());
		#endif // AS_DEBUG

	}

	FILE* fp = acquireFilePointerToSave(name, shouldOverwrite);
	
	if (fp == NULL) {
		LOG_ERROR("Failed to create new file, aborting save.");
		return false;
	}

	LOG_TRACE("File Acquired. Will save network");
	
	bool result = createNetworkFileFromData(fp, agentDataControllers_cptr, 
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