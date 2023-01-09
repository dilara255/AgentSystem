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

//TO DO:
// - "clearNetwork()";
// - For saving, pass const dataControllerPointers_t *
// - Pass dataControllerPointers_t * for loading
// - Create control class with control structures as members

bool isInitialized = false;

namespace AS {
	networkParameters_t currentNetworkParams;
	dataControllerPointers_t agentDataControllers;
}

void AS::initializeASandCL() {
	
	if (isInitialized) {
		LOG_ERROR("AS already initialized. Aborting initialization.");
		return;
	}

	LOG_INFO("Loggers Initialized");
	
	currentNetworkParams.isNetworkInitialized = false;
	agentDataControllers.haveBeenCreated = false;

	createAgentDataControllers();

	CL::init();

	isInitialized = true;

	LOG_INFO("Initialized");

	return;
}