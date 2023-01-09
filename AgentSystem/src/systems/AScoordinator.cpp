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

#include "fileManager.hpp"

#include "systems/AScoordinator.hpp"

//TO DO:
// - Create control class with control structures as members

bool isInitialized = false;

namespace AS {
	networkParameters_t currentNetworkParams;
	dataControllerPointers_t agentDataControllers;

	const networkParameters_t* currentNetworkParams_cptr;
	const dataControllerPointers_t* agentDataControllers_cptr;
}

void AS::initializeASandCL() {

	if (isInitialized) {
		LOG_ERROR("AS already initialized. Aborting initialization.");
		return;
	}

	LOG_INFO("Loggers Initialized");

	currentNetworkParams.isNetworkInitialized = false;
	agentDataControllers.haveBeenCreated = false;
	agentDataControllers.haveData = false;

	createAgentDataControllers(&agentDataControllers);

	currentNetworkParams_cptr = (const networkParameters_t*)&currentNetworkParams;
	agentDataControllers_cptr = (const dataControllerPointers_t*)&agentDataControllers;

#ifdef DEBUG
	printf("\n\nData Controllers NON-CONST ptr: %p\n", &agentDataControllers);
	printf("Data Controllers CONST ptr:     %p\n\n", agentDataControllers_cptr);
	printf("\n\nLAcoldData_ptr             : %p\n", agentDataControllers_cptr->LAcoldData_ptr);
	printf("LAcoldData_ptr(from const) : % p\n\n", agentDataControllers.LAcoldData_ptr);
	
#endif // DEBUG

	if (agentDataControllers_cptr !=
		(const dataControllerPointers_t*)&agentDataControllers) {
		LOG_CRITICAL("Pointer and Const Pointer to Data Control are not matching!");
	}

	if (agentDataControllers.LAcoldData_ptr == NULL) {
		LOG_CRITICAL("DATA CONTROLLERS NOT CREATED: ptr to LAcold is NULL");
	}

	CL::init();

	isInitialized = true;

	LOG_INFO("Initialized");

	return;
}

void AS::clearNetwork() {
	agentDataControllers.GAcoldData_ptr->clearData();
	agentDataControllers.LAcoldData_ptr->clearData();
	agentDataControllers.GAstate_ptr->clearData();
	agentDataControllers.LAstate_ptr->clearData();
	agentDataControllers.GAdecision_ptr->clearData();
	agentDataControllers.LAdecision_ptr->clearData();
	agentDataControllers.haveData = false;

	LOG_TRACE("Network Cleared");
}

void AS::loadNetworkFromFile(std::string name) {
	LOG_TRACE("Trying to load network from a file");

	FILE* fp = acquireFilePointerToLoad(name);

	if (fp == NULL) {
		LOG_ERROR("Failed to open file, aborting load. Current network preserved.");
		return;
	}

	if (!fileIsCompatible(fp)) {
		LOG_ERROR("File format version incompatible. Aborting load. Current network preserved.");
		return;
	}

	LOG_TRACE("File Acquired and of compatiple version. Clearing current network");

	clearNetwork();
	bool result = loadNetworkFromFileToDataControllers(fp, agentDataControllers, 
		                                               currentNetworkParams);
}