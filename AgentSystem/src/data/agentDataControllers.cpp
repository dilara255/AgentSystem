/*
* This file:
* - Defines the data controller classes;
* - Holds their only instances;
* - Defines methods to access their data.* 
* 
* Note: all structures have a fixed size once the maximum local agents,
* global agents and neighbours are defined. The use of vectors simply helps
* initialization and loading of different networks, avoiding more manual
* memory management.
*
* (outside of loading, performance impact should be minimal. TO DO: test this)
*/

#include "miscStdHeaders.h"
#include "core.hpp"

#include "logAPI.hpp"

#include "data/agentDataControllers.hpp"

bool AS::createAgentDataControllers(AS::dataControllerPointers_t* agentDataControllers_ptr) {
	LOG_TRACE("Trying to create Agent Data Controllers\n");

	if (agentDataControllers_ptr->haveBeenCreated) {
		LOG_WARN("Data Controllers already exist: aborting re-creation\n");
		return false;
	}

	agentDataControllers_ptr->LAcoldData_ptr = new LA::ColdDataController(MAX_LA_QUANTITY);
	agentDataControllers_ptr->LAstate_ptr = new LA::StateController(MAX_LA_QUANTITY);
	agentDataControllers_ptr->LAdecision_ptr = new LA::DecisionSystem(MAX_LA_QUANTITY);
	agentDataControllers_ptr->GAcoldData_ptr = new GA::ColdDataController(MAX_GA_QUANTITY);
	agentDataControllers_ptr->GAstate_ptr = new GA::StateController(MAX_GA_QUANTITY);
	agentDataControllers_ptr->GAdecision_ptr = new GA::DecisionSystem(MAX_GA_QUANTITY);

	agentDataControllers_ptr->haveBeenCreated = true;

	LOG_INFO("Data Controllers created\n");
	return true;
}

namespace LA {
	//Cold:
	ColdDataController::ColdDataController(uint32_t numberOfAgents) {
		if (numberOfAgents > MAX_LA_QUANTITY) numberOfAgents = MAX_LA_QUANTITY;
		data.reserve(numberOfAgents);
	}

	bool ColdDataController::addAgentData(coldData_t agentData) {
		if (data.size() >= MAX_LA_QUANTITY) {
			LOG_ERROR("Couldn't add LA's cold data: MAX_LA_QUANTITY reached");
			return false;
		}
		else{
			data.push_back(agentData);
			return true;
		}
	}

	bool ColdDataController::getAgentData(uint32_t agentID, coldData_t* recepient) const {
		if (agentID >= data.size()) return false;

		*recepient = data[agentID];
		return true;
	}

	size_t ColdDataController::sizeOfDataInBytes() const {
		return data.size() * sizeof(data[0]);
	}

	size_t ColdDataController::capacityForDataInBytes() const {
		return data.capacity() * sizeof(data[0]);
	}

	void ColdDataController::clearData() {
		data.clear();
	}

	//State:
	StateController::StateController(uint32_t numberOfAgents) {
		if (numberOfAgents > MAX_LA_QUANTITY) numberOfAgents = MAX_LA_QUANTITY;
		data.reserve(numberOfAgents);
	}

	bool StateController::addAgentData(stateData_t agentData) {
		if (data.size() >= MAX_LA_QUANTITY) {
			LOG_ERROR("Couldn't add LA's state data: MAX_LA_QUANTITY reached");
			return false;
		}
		else {
			data.push_back(agentData);
			return true;
		}
	}

	bool StateController::getAgentData(uint32_t agentID, stateData_t* recepient) const {
		if (agentID >= data.size()) return false;

		*recepient = data[agentID];
		return true;
	}

	size_t StateController::sizeOfDataInBytes() const {
		return data.size() * sizeof(data[0]);
	}

	size_t  StateController::capacityForDataInBytes() const {
		return data.capacity() * sizeof(data[0]);
	}

	void StateController::clearData() {
		data.clear();
	}

	//Decision:
	DecisionSystem::DecisionSystem(uint32_t numberOfAgents) {
		if (numberOfAgents > MAX_LA_QUANTITY) numberOfAgents = MAX_LA_QUANTITY;
		data.reserve(numberOfAgents);
	}

	bool DecisionSystem::addAgentData(decisionData_t agentData) {
		if (data.size() >= MAX_LA_QUANTITY) {
			LOG_ERROR("Couldn't add LA's decision data: MAX_LA_QUANTITY reached");
			return false;
		}
		else {
			data.push_back(agentData);
			return true;
		}
	}

	bool DecisionSystem::getAgentData(uint32_t agentID, decisionData_t* recepient) const {
		if (agentID >= data.size()) return false;

		*recepient = data[agentID];
		return true;
	}

	size_t DecisionSystem::sizeOfDataInBytes() const {
		return data.size() * sizeof(data[0]);
	}

	size_t DecisionSystem::capacityForDataInBytes() const {
		return data.capacity() * sizeof(data[0]);
	}

	void DecisionSystem::clearData() {
		data.clear();
	}
}

namespace GA {
	//Cold:
	ColdDataController::ColdDataController(uint32_t numberOfAgents) {
		if (numberOfAgents > MAX_GA_QUANTITY) numberOfAgents = MAX_GA_QUANTITY;
		data.reserve(numberOfAgents);
	}

	bool ColdDataController::addAgentData(coldData_t agentData) {
		if (data.size() >= MAX_GA_QUANTITY) {
			LOG_ERROR("Couldn't add GA's cold data: MAX_LA_QUANTITY reached");
			return false;
		}
		else {
			data.push_back(agentData);
			return true;
		}
	}

	bool ColdDataController::getAgentData(uint32_t agentID, coldData_t* recepient) const {
		if (agentID >= data.size()) return false;

		*recepient = data[agentID];
		return true;
	}

	size_t ColdDataController::sizeOfDataInBytes() const {
		return data.size() * sizeof(data[0]);
	}

	size_t ColdDataController::capacityForDataInBytes() const {
		return data.capacity() * sizeof(data[0]);
	}

	void ColdDataController::clearData() {
		data.clear();
	}

	//State:
	StateController::StateController(uint32_t numberOfAgents) {
		if (numberOfAgents > MAX_GA_QUANTITY) numberOfAgents = MAX_GA_QUANTITY;
		data.reserve(numberOfAgents);
	}

	bool StateController::addAgentData(stateData_t agentData) {
		if (data.size() >= MAX_GA_QUANTITY) {
			LOG_ERROR("Couldn't add GA's state data: MAX_LA_QUANTITY reached");
			return false;
		}
		else {
			data.push_back(agentData);
			return true;
		}
	}

	bool StateController::getAgentData(uint32_t agentID, stateData_t* recepient) const {
		if (agentID >= data.size()) return false;

		*recepient = data[agentID];
		return true;
	}

	size_t StateController::sizeOfDataInBytes() const {
		return data.size() * sizeof(data[0]);
	}

	size_t StateController::capacityForDataInBytes() const {
		return data.capacity() * sizeof(data[0]);
	}

	void StateController::clearData() {
		data.clear();
	}

	//Decision:
	DecisionSystem::DecisionSystem(uint32_t numberOfAgents) {
		if (numberOfAgents > MAX_GA_QUANTITY) numberOfAgents = MAX_GA_QUANTITY;
		data.reserve(numberOfAgents);
	}

	bool DecisionSystem::addAgentData(decisionData_t agentData) {
		if (data.size() >= MAX_GA_QUANTITY) {
			LOG_ERROR("Couldn't add GA's decision data: MAX_LA_QUANTITY reached");
			return false;
		}
		else {
			data.push_back(agentData);
			return true;
		}
	}

	//this is crashing
	bool DecisionSystem::getAgentData(uint32_t agentID, decisionData_t* recepient) const {
		if (agentID >= data.size()) return false;

		*recepient = data[agentID];
		return true;
	}

	size_t DecisionSystem::sizeOfDataInBytes() const {
		return data.size() * sizeof(data[0]);
	}

	size_t DecisionSystem::capacityForDataInBytes() const {
		return data.capacity() * sizeof(data[0]);
	}

	void DecisionSystem::clearData() {
		data.clear();
	}
}

namespace AS {
	bool testDataContainerCapacity(const dataControllerPointers_t* agentDataControllers_cptr) {
		#ifdef AS_DEBUG
			printf("\nData structure sizes (bytes):\n");
			printf("LA: Cold: %zi, State : %zi Decision : %zi\n",
				sizeof(LA::coldData_t), sizeof(LA::stateData_t), sizeof(LA::decisionData_t));
			printf("GA: Cold: %zi, State : %zi Decision : %zi\n",
				sizeof(GA::coldData_t), sizeof(GA::stateData_t), sizeof(GA::decisionData_t));
		#endif // AS_DEBUG 

		size_t LAagentSize = sizeof(LA::coldData_t) + sizeof(LA::stateData_t) + sizeof(LA::decisionData_t);
		size_t GAagentSize = sizeof(GA::coldData_t) + sizeof(GA::stateData_t) + sizeof(GA::decisionData_t);

		//capacities are set for the maximums, not for specific network
		size_t LAtotalSize = LAagentSize * MAX_LA_QUANTITY;
		size_t GAtotalSize = GAagentSize * MAX_GA_QUANTITY;

		#ifdef AS_DEBUG
			printf("Bytes per LA: %zi, per GA: %zi\n", LAagentSize, GAagentSize);
			printf("LA total bytes: %zi, GA total: %zi\n", LAtotalSize, GAtotalSize);
			printf("\n\nData Controllers NON-CONST ptr: %p\n", agentDataControllers_cptr);
			printf("LAcoldData_ptr: %p\n\n", agentDataControllers_cptr->LAcoldData_ptr);

		#endif // AS_DEBUG 
		
		size_t actualLAsize = 
			agentDataControllers_cptr->LAcoldData_ptr->capacityForDataInBytes() +
			agentDataControllers_cptr->LAstate_ptr->capacityForDataInBytes() +
			agentDataControllers_cptr->LAdecision_ptr->capacityForDataInBytes();

		size_t actualGAsize = 
			agentDataControllers_cptr->GAcoldData_ptr->capacityForDataInBytes() +
			agentDataControllers_cptr->GAstate_ptr->capacityForDataInBytes() +
			agentDataControllers_cptr->GAdecision_ptr->capacityForDataInBytes();

		bool result = actualLAsize == LAtotalSize;
		result &= actualGAsize == GAtotalSize;

		if (!result) {
			LOG_CRITICAL("LA and OR data capacity at controllers doesn't match expected");
			printf("--> LA is %zi instead of %zi\n", actualLAsize, LAtotalSize);
			printf("--> GA is %zi instead of %zi\n", actualGAsize, GAtotalSize);
			return false;
		}
		else {
			LOG_TRACE("LA and GA data capacity at controllers is as expected");
			return true;
		}
	}
}