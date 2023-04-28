/*
Assumes defined maximum local and global agents, as well as maximum actions
per agent, so the size of all data structures is fixed for a given network.
*/

//TODO: TESTING
//TODO: methods could check wheter hasData

#include "miscStdHeaders.h"
#include "core.hpp"

#include "logAPI.hpp"

#include "data/agentDataControllers.hpp"
#include "systems/actionSystem.hpp"

#include "data/dataMisc.hpp"

namespace AS {
	bool ActionSystem::initializeDataController(const networkParameters_t* pp,
								const ActionDataController** actionDataController_cptr_ptr) {

		LOG_TRACE("Will initialize Action Data controllers for LAs and GAs");
		
		*actionDataController_cptr_ptr = (const ActionDataController*)&(this->data);

		int effectiveGAs = pp->numberGAs - 1;

		this->data.initialize(pp->maxActions, pp->numberLAs, effectiveGAs);

		if (!(*actionDataController_cptr_ptr)->isInitialized()) {
			LOG_CRITICAL("Couldn't initialize Action Data Controllers!");
			return false;
		}		

		return true;
	}

	//Populates the action data vectors with actions with slotIsUsed set to 0 
	//Expects the number of EFFECTIVE GAs
	bool ActionDataController::initialize(int maxActionsPerAgent, int numberLAs, int numberGAs) {

		LOG_TRACE("Initializing LA and GA Action Data Controllers and setting capacity");

		if (numberLAs > MAX_LA_QUANTITY) numberLAs = MAX_LA_QUANTITY;
		if (numberGAs > MAX_GA_QUANTITY) numberGAs = MAX_GA_QUANTITY;
		if (maxActionsPerAgent > MAX_ACTIONS_PER_AGENT) maxActionsPerAgent = MAX_ACTIONS_PER_AGENT;
		
		m_maxActionsPerAgent = maxActionsPerAgent;
		m_numberLAs = numberLAs;
		m_numberGAs = numberGAs; //we expect to receive the number of effective GAs

		//In case we had already initialized before, the old data is cleared:
		if (m_hasData) {
			dataLAs.clear();
			dataGAs.clear();

			m_hasData = false;
		}

		dataLAs.reserve(m_numberLAs * m_maxActionsPerAgent);
		dataGAs.reserve(m_numberGAs * m_maxActionsPerAgent);

		actionData_t emptyLocalAction = AS::getDefaultAction(AS::scope::LOCAL);
		actionData_t emptyGlobalAction = AS::getDefaultAction(AS::scope::GLOBAL);

		for (int i = 0; i < m_maxActionsPerAgent; i++) {
			for (int j = 0; j < m_numberLAs; j++) {
				dataLAs.push_back(emptyLocalAction);
			}
			for (int j = 0; j < m_numberGAs; j++) {
				dataGAs.push_back(emptyGlobalAction);
			}
		}

		m_isInitialized = true;
		m_hasData = true;

		LOG_INFO("Action system initialized!");

		return true;
	}

	//Sets the agent's first unnocupied action slot to actionData
	//returns false if no slots are available
	bool ActionDataController::addActionData(actionData_t actionData) {

		if (!m_isInitialized) {
			LOG_ERROR("Action system not initialized: can't add action data");
			return false;
		}

		std::vector<AS::actionData_t>* data_ptr = NULL;

		if (actionData.ids.scope == (uint32_t)scope::LOCAL) {
			data_ptr = getDirectLAdataPtr();
		}
		else if (actionData.ids.scope == (uint32_t)scope::GLOBAL) {
			data_ptr = getDirectGAdataPtr();
		}
		else {
			LOG_ERROR("Tried to add malformed action, aborted...");
			return false;
		}
		if (data_ptr == NULL) {
			LOG_ERROR("Failed to acquire action data pointer");
			return false;
		}

		int startingIndex = actionData.ids.origin * m_maxActionsPerAgent;
		int nextAgentsStartingIndex = (actionData.ids.origin + 1) * m_maxActionsPerAgent;

		bool emptySlotFound = false;
		int possibleEmptySlot = startingIndex;

		while ((possibleEmptySlot < nextAgentsStartingIndex) && !emptySlotFound) {
			
			emptySlotFound = (data_ptr->at(possibleEmptySlot).ids.slotIsUsed == 0);
			possibleEmptySlot++;
		}

		if (emptySlotFound) {

			int slot = possibleEmptySlot - 1;

			data_ptr->at(slot) = actionData;

			return true;
		}
		else {

			return false;
		}
	}

	bool ActionDataController::getAction(AS::scope localOrGlobal, uint32_t actionID,
												       actionData_t* recepient) const {
		
		if (!m_hasData) {
			LOG_ERROR("Tried to get action, but action system has no data");
			return false;
		}

		if (localOrGlobal == scope::LOCAL) {
			if (actionID < dataLAs.size()) {
				*recepient = dataLAs[actionID];
				return true;
			}

			else{
				LOG_ERROR("Tried to get out of range Local Action. Aborting");
				#if (defined AS_DEBUG) || VERBOSE_RELEASE
					printf("\nLocal/Global: %d , ID: %d , size: %d ", localOrGlobal, actionID,
																		(int)dataLAs.size());
				#endif // AS_DEBUG 
				return false;
			}
		}

		else if (localOrGlobal == scope::GLOBAL) {
			if (actionID < dataGAs.size()) {
				*recepient = dataGAs[actionID];
				return true;
			}

			else{
				LOG_ERROR("Tried to get out of range Global Action. Aborting");
				#if (defined AS_DEBUG) || VERBOSE_RELEASE
					printf("\nLocal/Global: %d , ID: %d , size: %d ", localOrGlobal, actionID,
																		(int)dataGAs.size());
				#endif // AS_DEBUG 
				return false;
			}
		}

		else {
			LOG_ERROR("Tried to add malformed action, aborted...");
			return false;
		}
	}

	bool ActionDataController::getAgentsAction(AS::scope localOrGlobal, uint32_t agentID, 
		                                       int actionNumber, actionData_t* recepient) const {
		
		if (!m_hasData) {
			LOG_ERROR("Tried to get agent's action, but action system has no data");
			return false;
		}

		if (actionNumber > (m_maxActionsPerAgent - 1)) {
			LOG_ERROR("Tried to get agent Action Data which is out of range. Aborting.");
			return false;
		}

		if (localOrGlobal == scope::LOCAL) {
			if (agentID >= (dataLAs.size()/m_maxActionsPerAgent) ) {
				LOG_ERROR("Couldn't get LA action: id too large");
				return false;
			}

			*recepient = dataLAs[(agentID * m_maxActionsPerAgent) + actionNumber];
			return true;
		}
		else if (localOrGlobal == scope::GLOBAL) {
			if (agentID >= (dataGAs.size() / m_maxActionsPerAgent) ) {
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

	//clears all data but keeps the system initialized
	void ActionDataController::clearData() {
		dataLAs.clear();
		dataGAs.clear();

		m_hasData = false;

		LOG_TRACE("All Action Data cleared!");

		initialize(m_maxActionsPerAgent, m_numberLAs, m_numberGAs);
	}
}
