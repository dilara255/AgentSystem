/*
Classes and functions on this file are responsible for the coordination of the AS.
This includes:
- Coordinating initialization and termination;
- Main loop; and
- Access/references to the different systems.
*/

#include "AS_API.hpp"
#include "CL_internalAPI.hpp"

#include "network/parameters.hpp" //exposes "currentNetworkParams"
#include "data/agentDataControllers.hpp" //exposes "dataControllers"
#include "systems/actionSystem.hpp"

#include "fileManager.hpp"

#include "systems/AScoordinator.hpp"

//TO DO:
// - Create control class with control structures as members

bool isInitialized = false;

namespace AS {
	//these are the control structures/objects for the AS's main systems;
	//they're kinda in global scope for this file for now : )
	
	networkParameters_t currentNetworkParams;

	ActionSystem actionSystem; 
	dataControllerPointers_t agentDataControllerPtrs;
	networkParameters_t* currentNetworkParams_ptr;

	const ActionSystem* actionSystem_cptr;
	const ActionDataController* actionDataController_cptr;
	const networkParameters_t* currentNetworkParams_cptr;
	const dataControllerPointers_t* agentDataControllers_cptr;
}

bool AS::initializeASandCL() {

	if (isInitialized) {
		LOG_ERROR("AS already initialized. Aborting initialization.");
		return false;
	}

	LOG_INFO("Loggers Initialized");

	currentNetworkParams.isNetworkInitialized = false;
	agentDataControllerPtrs.haveBeenCreated = false;
	agentDataControllerPtrs.haveData = false;

	currentNetworkParams_cptr = (const networkParameters_t*)&currentNetworkParams;
	currentNetworkParams_ptr = &currentNetworkParams;
	agentDataControllers_cptr = (const dataControllerPointers_t*)&agentDataControllerPtrs;

	createAgentDataControllers(&agentDataControllerPtrs);

	actionSystem.initialize(&actionSystem_cptr);
	bool result = actionSystem_cptr->isInitialized();
	if (!result) {
		LOG_CRITICAL("Something went wrong initialing the Action System!");
		return false;
	}

	actionSystem.initializeDataController(currentNetworkParams_cptr, &actionDataController_cptr);

#ifdef DEBUG
	printf("\n\nData Controllers NON-CONST ptr: %p\n", &agentDataControllerPtrs);
	printf("Data Controllers CONST ptr:     %p\n\n", agentDataControllers_cptr);
	printf("\n\nLAcoldData_ptr             : %p\n", agentDataControllers_cptr->LAcoldData_ptr);
	printf("LAcoldData_ptr(from const) : % p\n\n", agentDataControllerPtrs.LAcoldData_ptr);
#endif // DEBUG

	if (agentDataControllers_cptr !=
		(const dataControllerPointers_t*)&agentDataControllerPtrs) {
		LOG_CRITICAL("Pointer and Const Pointer to Data Control are not matching!");
		return false;
	}

	if (agentDataControllerPtrs.LAcoldData_ptr == NULL) {
		LOG_CRITICAL("DATA CONTROLLERS NOT CREATED: ptr to LAcold is NULL");
		return false;
	}

	result = CL::init();
	if (!result) {
		LOG_CRITICAL("Something went wrong initialing the Communications Layer!");
		return false;
	}

	isInitialized = true;

	LOG_INFO("Initialized");

	return true;
}

void AS::clearNetwork() {
	agentDataControllerPtrs.GAcoldData_ptr->clearData();
	agentDataControllerPtrs.LAcoldData_ptr->clearData();
	agentDataControllerPtrs.GAstate_ptr->clearData();
	agentDataControllerPtrs.LAstate_ptr->clearData();
	agentDataControllerPtrs.GAdecision_ptr->clearData();
	agentDataControllerPtrs.LAdecision_ptr->clearData();
	agentDataControllerPtrs.haveData = false;
	currentNetworkParams.isNetworkInitialized = false;

	LOG_TRACE("Network Cleared");
}

bool AS::loadNetworkFromFile(std::string name) {
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
		                                              currentNetworkParams_ptr);
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

	return result; //not much information is given, but the app may decide what to do on failure
}

bool AS::saveNetworkToFile(std::string name) {
	LOG_INFO("Trying to save network to a file");

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
#ifdef DEBUG
		printf("Name: %s\n", name.c_str());
#endif // DEBUG

	}

	FILE* fp = acquireFilePointerToSave(name);
	
	if (fp == NULL) {
		LOG_ERROR("Failed to create new file, aborting save.");
		return false;
	}

	LOG_TRACE("File Acquired. Will save network");
	
	bool result = createNetworkFileFromData(fp, agentDataControllers_cptr, 
									        currentNetworkParams_cptr);
	if (!result) {
		LOG_ERROR("Saving failed!");
	}
	else {
		LOG_INFO("Network Saved to File!");
	}

	fclose(fp);
	LOG_TRACE("File closed");

	return result;
}