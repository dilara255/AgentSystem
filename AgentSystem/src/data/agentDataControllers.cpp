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
#include "AS_internal.hpp"

void AS::createAgentDataControllers() {
	LOG_TRACE("Trying to create Agent Data Controllers\n");

	if (agentDataControllers.haveBeenCreated) {
		LOG_WARN("Data Controllers already exist: aborting re-creation\n");
		return;
	}

	agentDataControllers.LAcoldData_ptr = new LA::ColdDataController(MAX_LA_QUANTITY);
	agentDataControllers.LAstate_ptr = new LA::StateController(MAX_LA_QUANTITY);
	agentDataControllers.LAdecision_ptr = new LA::DecisionSystem(MAX_LA_QUANTITY);
	agentDataControllers.GAcoldData_ptr = new GA::ColdDataController(MAX_GA_QUANTITY);
	agentDataControllers.GAstate_ptr = new GA::StateController(MAX_GA_QUANTITY);
	agentDataControllers.GAdecision_ptr = new GA::DecisionSystem(MAX_GA_QUANTITY);

	agentDataControllers.haveBeenCreated = true;

	LOG_INFO("Data Controllers created\n");
}

namespace LA {
	//Cold:
	ColdDataController::ColdDataController(uint32_t numberOfAgents) {
		if (numberOfAgents > MAX_LA_QUANTITY) numberOfAgents = MAX_LA_QUANTITY;
		data.reserve(numberOfAgents);
	}

	void ColdDataController::addAgentData(coldData_t agentData) {
		data.push_back(agentData);
	}

	bool ColdDataController::getAgentData(uint32_t agentID, coldData_t* recepient) {
		if (agentID > (data.size() - 1)) return false;

		*recepient = data[agentID];
		return true;
	}

	size_t ColdDataController::sizeOfDataInBytes() {
		return data.size() * sizeof(data[0]);
	}

	size_t ColdDataController::capacityForDataInBytes() {
		return data.capacity() * sizeof(data[0]);
	}

	//State:
	StateController::StateController(uint32_t numberOfAgents) {
		if (numberOfAgents > MAX_LA_QUANTITY) numberOfAgents = MAX_LA_QUANTITY;
		data.reserve(numberOfAgents);
	}

	void StateController::addAgentData(stateData_t agentData) {
		data.push_back(agentData);
	}

	bool StateController::getAgentData(uint32_t agentID, stateData_t* recepient) {
		if (agentID > (data.size() - 1)) return false;

		*recepient = data[agentID];
		return true;
	}

	size_t StateController::sizeOfDataInBytes() {
		return data.size() * sizeof(data[0]);
	}

	size_t StateController::capacityForDataInBytes() {
		return data.capacity() * sizeof(data[0]);
	}

	//Decision:
	DecisionSystem::DecisionSystem(uint32_t numberOfAgents) {
		if (numberOfAgents > MAX_LA_QUANTITY) numberOfAgents = MAX_LA_QUANTITY;
		data.reserve(numberOfAgents);
	}

	void DecisionSystem::addAgentData(decisionData_t agentData) {
		data.push_back(agentData);
	}

	bool DecisionSystem::getAgentData(uint32_t agentID, decisionData_t* recepient) {
		if (agentID > (data.size() - 1)) return false;

		*recepient = data[agentID];
		return true;
	}

	size_t DecisionSystem::sizeOfDataInBytes() {
		return data.size() * sizeof(data[0]);
	}

	size_t DecisionSystem::capacityForDataInBytes() {
		return data.capacity() * sizeof(data[0]);
	}
}

namespace GA {
	//Cold:
	ColdDataController::ColdDataController(uint32_t numberOfAgents) {
		if (numberOfAgents > MAX_GA_QUANTITY) numberOfAgents = MAX_GA_QUANTITY;
		data.reserve(numberOfAgents);
	}

	void ColdDataController::addAgentData(coldData_t agentData) {
		data.push_back(agentData);
	}

	bool ColdDataController::getAgentData(uint32_t agentID, coldData_t* recepient) {
		if (agentID > (data.size() - 1)) return false;

		*recepient = data[agentID];
		return true;
	}

	size_t ColdDataController::sizeOfDataInBytes() {
		return data.size() * sizeof(data[0]);
	}

	size_t ColdDataController::capacityForDataInBytes() {
		return data.capacity() * sizeof(data[0]);
	}

	//State:
	StateController::StateController(uint32_t numberOfAgents) {
		if (numberOfAgents > MAX_GA_QUANTITY) numberOfAgents = MAX_GA_QUANTITY;
		data.reserve(numberOfAgents);
	}

	void StateController::addAgentData(stateData_t agentData) {
		data.push_back(agentData);
	}

	bool StateController::getAgentData(uint32_t agentID, stateData_t* recepient) {
		if (agentID > (data.size() - 1)) return false;

		*recepient = data[agentID];
		return true;
	}

	size_t StateController::sizeOfDataInBytes() {
		return data.size() * sizeof(data[0]);
	}

	size_t StateController::capacityForDataInBytes() {
		return data.capacity() * sizeof(data[0]);
	}

	//Decision:
	DecisionSystem::DecisionSystem(uint32_t numberOfAgents) {
		if (numberOfAgents > MAX_GA_QUANTITY) numberOfAgents = MAX_GA_QUANTITY;
		data.reserve(numberOfAgents);
	}

	void DecisionSystem::addAgentData(decisionData_t agentData) {
		data.push_back(agentData);
	}

	bool DecisionSystem::getAgentData(uint32_t agentID, decisionData_t* recepient) {
		if (agentID > (data.size() - 1)) return false;

		*recepient = data[agentID];
		return true;
	}

	size_t DecisionSystem::sizeOfDataInBytes() {
		return data.size() * sizeof(data[0]);
	}

	size_t DecisionSystem::capacityForDataInBytes() {
		return data.capacity() * sizeof(data[0]);
	}
}

namespace AS {
	void testDataContainerCapacity() {
#ifdef DEBUG
		printf("\nData structure sizes (bytes):\n");
		printf("LA: Cold: %zi, State : %zi Decision : %zi\n",
			sizeof(LA::coldData_t), sizeof(LA::stateData_t), sizeof(LA::decisionData_t));
		printf("GA: Cold: %zi, State : %zi Decision : %zi\n",
			sizeof(GA::coldData_t), sizeof(GA::stateData_t), sizeof(GA::decisionData_t));
#endif // DEBUG

		size_t LAagentSize = sizeof(LA::coldData_t) + sizeof(LA::stateData_t) + sizeof(LA::decisionData_t);
		size_t GAagentSize = sizeof(GA::coldData_t) + sizeof(GA::stateData_t) + sizeof(GA::decisionData_t);

		//capacities are set for the maximums, not for specific network
		size_t LAtotalSize = LAagentSize * MAX_LA_QUANTITY;
		size_t GAtotalSize = GAagentSize * MAX_GA_QUANTITY;

#ifdef DEBUG
		printf("Bytes per LA: %zi, per GA: %zi\n", LAagentSize, GAagentSize);
		printf("LA total bytes: %zi, GA total: %zi\n", LAtotalSize, GAtotalSize);
#endif // DEBUG

		size_t actualLAsize = agentDataControllers.LAcoldData_ptr->capacityForDataInBytes() +
			agentDataControllers.LAstate_ptr->capacityForDataInBytes() +
			agentDataControllers.LAdecision_ptr->capacityForDataInBytes();

		size_t actualGAsize = agentDataControllers.GAcoldData_ptr->capacityForDataInBytes() +
			agentDataControllers.GAstate_ptr->capacityForDataInBytes() +
			agentDataControllers.GAdecision_ptr->capacityForDataInBytes();

		if (actualLAsize != LAtotalSize) {
			LOG_CRITICAL("LA data capacity at controller doesn't match expected");
			printf("--> is %zi instead\n", actualLAsize);
		}
		else {
			LOG_TRACE("LA data capacity at controller is as expected");
		}
		if (actualGAsize != GAtotalSize) {
			LOG_CRITICAL("GA data capacity at controller doesn't match expected");
			printf("--> is %zi instead\n", actualGAsize);
		}
		else {
			LOG_TRACE("GA data capacity at controller is as expected");
		}
	}
}