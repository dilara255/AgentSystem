/*
Assumes defined maximum local and global agents, as well as maximum actions
per agent, so the size of all data structures is fixed for a given network.
*/

#include "miscStdHeaders.h"
#include "core.hpp"

#include "logAPI.hpp"

#include "data/agentDataControllers.hpp"
#include "systems/agentSystems.hpp"

namespace AS {
	bool ActionSystem::initializeDataController(const networkParameters_t* pp,
								const ActionDataController** actionDataController_cptr_ptr) {

	LOG_TRACE("Will create (empty) Action Data controllers for LAs and GAs");

	*actionDataController_cptr_ptr = (const ActionDataController*)&(this->data);
	this->data.initialize(pp->maxActions, pp->numberLAs, pp->numberGAs);

	if (!(*actionDataController_cptr_ptr)->isInitialized()) {
		LOG_CRITICAL("Couldn't initialize Action Data Controllers!");
		return false;
	}		

	return true;
}

	bool ActionDataController::initialize(int maxActionsPerAgent, int numberLas, int numberGAs) {

		LOG_TRACE("Initializing LA and GA Action Data Controllers and setting capacity");

		m_isInitialized = true;
		return true;
	}

	bool ActionDataController::addActionData(actionData_t actionData) {

		return true;
	}

	bool ActionDataController::getAgentData(int localOrGlobal, uint32_t agentID, actionData_t* recepient) const {

		return true;
	}

	size_t ActionDataController::sizeOfDataInBytesLAs() const {

		return 1;
	}

	size_t ActionDataController::sizeOfDataInBytesGAs() const {

		return 1;
	}

	size_t ActionDataController::capacityForDataInBytesLAs() const {

		return 1;
	}

	size_t ActionDataController::capacityForDataInBytesGAs() const {

		return 1;
	}

	void ActionDataController::clearData() {

	}


}
