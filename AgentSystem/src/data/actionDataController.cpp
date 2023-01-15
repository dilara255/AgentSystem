/*
Assumes defined maximum local and global agents, as well as maximum actions
per agent, so the size of all data structures is fixed for a given network.
*/

//TO DO: TESTING
//TO DO: methods could check wheter hasData

#include "miscStdHeaders.h"
#include "core.hpp"

#include "logAPI.hpp"

#include "data/agentDataControllers.hpp"
#include "systems/actionSystem.hpp"

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

		if (numberLas > MAX_LA_QUANTITY) numberLas = MAX_LA_QUANTITY;
		if (numberGAs > MAX_GA_QUANTITY) numberGAs = MAX_GA_QUANTITY;
		if (maxActionsPerAgent > MAX_ACTIONS_PER_AGENT) maxActionsPerAgent = MAX_ACTIONS_PER_AGENT;
		
		m_maxActionsPerAgent = maxActionsPerAgent;

		dataLAs.reserve(numberLas * maxActionsPerAgent);
		dataGAs.reserve(numberGAs * maxActionsPerAgent);

		m_isInitialized = true;
		return true;
	}

	bool ActionDataController::addActionData(actionData_t actionData) {

		if (actionData.ids.scope == LOCAL) {
			if (dataLAs.size() > ( (MAX_LA_QUANTITY*MAX_ACTIONS_PER_AGENT) - 1)) {
				LOG_ERROR("Couldn't add LA action: maximum reached");
				return false;
			}

			dataLAs.push_back(actionData);
			return true;
		}
		else if (actionData.ids.scope == GLOBAL) {
			if (dataGAs.size() > ((MAX_GA_QUANTITY * MAX_ACTIONS_PER_AGENT) - 1)) {
				LOG_ERROR("Couldn't add GA action: maximum reached");
				return false;
			}

			dataGAs.push_back(actionData);
			return true;
		}
		else {
			LOG_ERROR("Tried to add malformed action, aborted...");
			return false;
		}
	}

	bool ActionDataController::getAction(int localOrGlobal, uint32_t actionID,
												       actionData_t* recepient) const {
		if (localOrGlobal == AS::LOCAL) {
			if (actionID < dataLAs.size()) {
				*recepient = dataLAs[actionID];
				return true;
			}

			else{
				LOG_ERROR("Tried to get out of range Local Action. Aborting");
				#ifdef AS_DEBUG
					printf("\nLocal/Global: %i, ID: %i, size: %i", localOrGlobal, actionID,
																		(int)dataLAs.size());
				getchar();
				#endif // AS_DEBUG 
				return false;
			}
		}

		else if (localOrGlobal == AS::GLOBAL) {
			if (actionID < dataGAs.size()) {
				*recepient = dataGAs[actionID];
				return true;
			}

			else{
				LOG_ERROR("Tried to get out of range Global Action. Aborting");
				#ifdef AS_DEBUG
					printf("\nLocal/Global: %i, ID: %i, size: %i", localOrGlobal, actionID,
																		(int)dataGAs.size());
					getchar();
				#endif // AS_DEBUG 
				return false;
			}
		}

		else {
			LOG_ERROR("Tried to add malformed action, aborted...");
			return false;
		}
	}

	bool ActionDataController::getAgentData(int localOrGlobal, uint32_t agentID, 
		                                    int actionNumber, actionData_t* recepient) const {
		if (actionNumber > (m_maxActionsPerAgent - 1)) {
			LOG_ERROR("Tried to get agent Action Data which is out of range. Aborting.");
			return false;
		}

		if (localOrGlobal == LOCAL) {
			if (agentID > ( (dataLAs.size()/m_maxActionsPerAgent) - 1) ) {
				LOG_ERROR("Couldn't get LA action: id too large");
				return false;
			}

			*recepient = dataLAs[ (agentID*m_maxActionsPerAgent) + actionNumber];
			return true;
		}
		else if (localOrGlobal == GLOBAL) {
			if (agentID > ((dataGAs.size() / m_maxActionsPerAgent) - 1) ) {
			LOG_ERROR("Couldn't get GA action: id too large");
			return false;
			}

			*recepient = dataGAs[(agentID * m_maxActionsPerAgent) + actionNumber];
			return true;
		}
		else {
			LOG_ERROR("Tried to get malformed action, aborted...");
			return false;
		}
		
	}

	size_t ActionDataController::sizeOfDataInBytesLAs() const {

		return dataLAs.size() * sizeof(dataLAs[0]);
	}

	size_t ActionDataController::sizeOfDataInBytesGAs() const {

		return dataGAs.size() * sizeof(dataGAs[0]);
	}

	size_t ActionDataController::capacityForDataInBytesLAs() const {

		return dataLAs.capacity() * sizeof(dataLAs[0]);
	}

	size_t ActionDataController::capacityForDataInBytesGAs() const {

		return dataGAs.capacity() * sizeof(dataGAs[0]);
	}

	void ActionDataController::clearData() {
		dataLAs.clear();
		dataGAs.clear();

		m_hasData = false;
		LOG_TRACE("All Action Data cleared!");
	}
}
