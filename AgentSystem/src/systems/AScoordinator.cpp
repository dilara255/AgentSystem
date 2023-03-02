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

#include "data/dataMisc.hpp"

//TODO:
// - Create control class with control structures as members

bool isInitialized = false;

namespace AS {
	//these are the control structures/objects for the AS's main systems;
	//they're kinda in global scope for this file for now : )
	
	bool shouldMainLoopBeRunning = false;
	std::thread::id mainLoopId;
	std::thread mainLoopThread;

	networkParameters_t currentNetworkParams;
	AS::PRNserver prnServer;

	ActionSystem actionSystem; 
	dataControllerPointers_t agentDataControllerPtrs;
	networkParameters_t* currentNetworkParams_ptr;

	const ActionSystem* actionSystem_cptr;
	const ActionDataController* actionDataController_cptr;
	const networkParameters_t* currentNetworkParams_cptr;
	const dataControllerPointers_t* agentDataControllers_cptr;
}

bool AS::quit() {
	
	bool result = true;
	if (chekIfMainLoopShouldBeRunning()) {
		result = AS::stop();
	}	
	
	//TODO: Kill all the stuff in the pointers here
	//TODO: implement and call CL::quit, doing the same for the CL;
	//isInitialized = false;
	
	return result;
}

bool AS::initializeASandCL() {
//TODO: possibly extract functions

	if (isInitialized) {
		LOG_ERROR("AS already initialized. Aborting initialization.");
		return false;
	}

	LOG_INFO("Loggers Initialized");

	//Agent Data and Network Parameters:
	bool result = defaultNetworkParameters(&currentNetworkParams);
	if (!result) { return false; }

	currentNetworkParams.isNetworkInitialized = false;
	agentDataControllerPtrs.haveBeenCreated = false;
	agentDataControllerPtrs.haveData = false;

	currentNetworkParams_cptr = (const networkParameters_t*)&currentNetworkParams;
	currentNetworkParams_ptr = &currentNetworkParams;
	agentDataControllers_cptr = (const dataControllerPointers_t*)&agentDataControllerPtrs;

	result = createAgentDataControllers(&agentDataControllerPtrs);
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

	//TODO: test the others as well (extract)
	if (agentDataControllerPtrs.LAcoldData_ptr == NULL) {
		LOG_CRITICAL("DATA CONTROLLERS NOT CREATED: ptr to LAcold is NULL");
		return false;
	}

	//Action System:
	result = actionSystem.initialize(&actionSystem_cptr, &actionDataController_cptr,
		                                                  currentNetworkParams_cptr);
	result &= actionSystem_cptr->isInitialized();
	if (!result) {
		LOG_CRITICAL("Something went wrong initialing the Action System!");
		return false;
	}
	result &= actionDataController_cptr->isInitialized();
	if (!result) {
		LOG_CRITICAL("Something went wrong initialing the Action Data Controlers!");
		return false;
	}

	result = CL::init(currentNetworkParams_cptr);
	if (!result) {
		LOG_CRITICAL("Something went wrong initialing the Communications Layer!");
		return false;
	}

	result = initMainLoopControl(&shouldMainLoopBeRunning, &mainLoopId, &mainLoopThread,
		               &actionSystem, &agentDataControllerPtrs, currentNetworkParams_ptr,
		               &prnServer);
	if(!result) {
		LOG_CRITICAL("Something went wrong initialing Main Loop Controller!");
		return false;
	}

	//Done!
	isInitialized = true;

	LOG_INFO("Initialized");

	return true;
}

bool AS::sendReplacementDataToCL(bool silent) {

	return  CL::replaceMirrorData(currentNetworkParams_cptr,
				actionDataController_cptr->getActionsLAsCptr(),
				actionDataController_cptr->getActionsGAsCptr(),
				agentDataControllers_cptr->LAcoldData_ptr->getDataCptr(),
				agentDataControllers_cptr->LAstate_ptr->getDataCptr(),
				agentDataControllers_cptr->LAdecision_ptr->getDataCptr(),
				agentDataControllers_cptr->GAcoldData_ptr->getDataCptr(),
				agentDataControllers_cptr->GAstate_ptr->getDataCptr(),
				agentDataControllers_cptr->GAdecision_ptr->getDataCptr(),
				silent);	
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

		actionSystem.getDataDirectPointer()->clearData();

		LOG_TRACE("Network Cleared");
	}
}

bool AS::loadNetworkFromFile(std::string name, bool runNetwork) {
	
	bool shouldBlockClientFromIssuingData = CL::isClintDataInitialized();
	std::mutex* clientDataMutex_ptr = NULL;

	if (shouldBlockClientFromIssuingData) {
		clientDataMutex_ptr = CL::blockClientData();
		if(clientDataMutex_ptr == NULL){
			LOG_ERROR("Couldn't acquire mutex to block further Client Changes. Will abort loading.");
			return false;
		}
		else {
			LOG_TRACE("Mutex Acquired");
		}
	}

	LOG_INFO("Trying to load network from a file");
	if(runNetwork) { LOG_INFO("Will start Main Loop if loading succeeds"); }

	bool result = true;
	if (!isInitialized) {
		LOG_ERROR("Agent System and Communications Layer must be initialized before loading. Aborting.");
		result = false;
		goto cleanUpAndReturn;
	}

	FILE* fp = acquireFilePointerToLoad(name);

	if (fp == NULL) {
		LOG_ERROR("Failed to open file, aborting load. Current network preserved.");
		result = false;
		goto cleanUpAndReturn;
	}

	if (!fileIsCompatible(fp)) {
		LOG_ERROR("File format version incompatible. Aborting load. Current network preserved.");
		result = false;
		goto cleanUpAndReturn;
	}

	if (shouldMainLoopBeRunning) {
		LOG_TRACE("Will stop Main Loop Thread");
		if(stop()){
			LOG_TRACE("Thread stopped");
		}
		else {
			LOG_WARN("Something weird is going on with the Main Loop Thread. Loading will proceed but may fail...");
		}
	}

	clearNetwork(); //in order to load the new one
	strcpy(currentNetworkParams.name, name.c_str());
	LOG_TRACE("File Acquired and of compatiple version. Network cleared and new name set");

	result = loadNetworkFromFileToDataControllers(fp, agentDataControllerPtrs, 
		                                          currentNetworkParams_ptr,
		                                          actionSystem.getDataDirectPointer());

	fclose(fp);
	LOG_TRACE("File closed");

	if (!result) {
		LOG_ERROR("Load failed mid-way through. Will clear the network.");
		clearNetwork(); //we don't leave an incomplete state behind. Marks data as not initialized.
		goto cleanUpAndReturn;
	}

	//TODO: check capacities and sizes to make sure things are in order?

	agentDataControllerPtrs.haveData = true;
	currentNetworkParams.isNetworkInitialized = true;
	LOG_INFO("File loaded...");

	//if acquired, release mutex so we can delete the old Handler to create a new one
	if (clientDataMutex_ptr != NULL) {
		clientDataMutex_ptr->unlock();
		clientDataMutex_ptr = NULL; //so we don't try to release again later
		LOG_TRACE("Mutex Released closed");
	}

	//TODO: This could be just an update(), which clears + shrinks + reserves stuff.
	result = CL::createClientDataHandler(*currentNetworkParams_ptr);
	if(!result){
		LOG_ERROR("Failed to create new Client Data Handler!");
		goto cleanUpAndReturn;
	}

	result = sendReplacementDataToCL(true);
	if(!result){
		LOG_ERROR("Failed to send new Network Data to the mirror!");
		goto cleanUpAndReturn;
	}
	
	if(runNetwork) { 
		result = AS::run(); 
		if(!result){
			LOG_ERROR("Couldn't restart AS's main loop after load!");
			goto cleanUpAndReturn;
		}
	}

cleanUpAndReturn:
	if (fp != NULL) {
		fclose(fp);
		LOG_TRACE("File closed");
	}
	if (clientDataMutex_ptr != NULL) {
		clientDataMutex_ptr->unlock();
		LOG_TRACE("Mutex Released");
	}

	return result; //not much info is given, but the app may decide what to do on failure
}

bool AS::saveNetworkToFile(std::string name, bool shouldOverwrite, bool willResumeAfterSave, bool silent) {
	LOG_INFO("Trying to save network to a file...");

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
		if(!silent) { LOG_TRACE("Will stop main loop (if saving fails, mainLoop will not resume either)"); }
		if(stop()){
			shouldResumeThread = willResumeAfterSave;
			if(!silent) { LOG_TRACE("Will resume main loop after saving"); }
		}
		else {
			LOG_WARN("Something weird is going on with the Main Loop Thread. Saving will proceed but thread won't be resumed...");
		}
	}

	//In case we're quitting, let's not loose any issued data changes:
	FILE* fp = NULL; //in case of failure, this has to be checked
	std::mutex* clientDataMutex_ptr = NULL;
	if(!willResumeAfterSave){
		if (CL::isClintDataInitialized()) {
			//Make sure any dangling data issued by the Client is captured:
			bool result = CL::getNewClientData(currentNetworkParams_ptr, 
											   &agentDataControllerPtrs,
						   					   actionSystem.getDataDirectPointer(), 
											   silent);
			if (!result) {
				LOG_ERROR("Couldn't receive Client Changes. Will abort saving");
				goto returnWithError;
			}
		}
	}

	//Proceed with save:

	if (name == "") {
		if(!silent) { LOG_TRACE("Using name stored on network params"); }
		name = currentNetworkParams.name;
		#if (defined AS_DEBUG) || VERBOSE_RELEASE
			printf("Name: %s\n", name.c_str());
		#endif // AS_DEBUG

	}

	fp = acquireFilePointerToSave(name, shouldOverwrite);
	
	if (fp == NULL) {
		LOG_ERROR("Failed to create new file, aborting save.");
		goto returnWithError;
	}

	if(!silent) { LOG_TRACE("File Acquired. Will save network"); }
	
	bool result = createNetworkFileFromData(fp, agentDataControllers_cptr, 
		 	   				                currentNetworkParams_cptr,
		                                    actionDataController_cptr);
	if (!result) {
		LOG_ERROR("Saving failed!");
		goto returnWithError;
	}
	else {
		LOG_INFO("Network Saved to File!");
	}

	fclose(fp);
	LOG_TRACE("File closed.");
	
	if (shouldResumeThread) { 
		if(!silent) { LOG_TRACE("File closed. Will resume mainLoop..."); }
		result = run(); 
		if (!result) {
			LOG_ERROR("Failed to resume AS's main loop!"); 
			goto returnWithError;
		}
	}

	return true;

returnWithError:

	if (fp != NULL) {
		fclose(fp);
		if(!silent) { LOG_TRACE("File closed"); }
	}
	if (clientDataMutex_ptr != NULL) {
		clientDataMutex_ptr->unlock();
		if(!silent) { LOG_TRACE("Mutex Released"); }
	}

	return false;
}