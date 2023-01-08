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

LA::ColdDataController* LAcoldDataController_ptr;
LA::StateController* LAstateController_ptr;
LA::DecisionSystem* LAdecisionDataController_ptr;
GA::ColdDataController* GAcoldDataController_ptr;
GA::StateController* GAstateController_ptr;
GA::DecisionSystem* GAdecisionDataController_ptr;
bool dataControllersCreated = false;

void testDataContainerCapacity(uint32_t numberOfLAs, uint32_t numberOfGAs);

void createAgentDataControllers(uint32_t numberOfLAs, uint32_t numberOfGAs) {
	LOG_TRACE("Trying to create Agent Data Controllers\n");

	if (dataControllersCreated) {
		LOG_WARN("Data Controllers already exist: aborting re-creation\n");
		return;
	}

	LAcoldDataController_ptr = new LA::ColdDataController(numberOfLAs);
	LAstateController_ptr = new LA::StateController(numberOfLAs);
	LAdecisionDataController_ptr = new LA::DecisionSystem(numberOfLAs);
	GAcoldDataController_ptr = new GA::ColdDataController(numberOfGAs);
	GAstateController_ptr = new GA::StateController(numberOfGAs);
	GAdecisionDataController_ptr = new GA::DecisionSystem(numberOfGAs);

	dataControllersCreated = true;

	testDataContainerCapacity(numberOfLAs, numberOfGAs);

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

void testDataContainerCapacity(uint32_t numberOfLAs, uint32_t numberOfGAs) {
#ifdef DEBUG
	printf("\nData structure sizes (bytes):\n");
	printf("LA: Cold: %zi, State : %zi Decision : %zi\n",
		sizeof(LA::coldData_t), sizeof(LA::stateData_t), sizeof(LA::decisionData_t));
	printf("GA: Cold: %zi, State : %zi Decision : %zi\n",
		sizeof(GA::coldData_t), sizeof(GA::stateData_t), sizeof(GA::decisionData_t));
#endif // DEBUG

	size_t LAagentSize = sizeof(LA::coldData_t) + sizeof(LA::stateData_t) + sizeof(LA::decisionData_t);
	size_t GAagentSize = sizeof(GA::coldData_t) + sizeof(GA::stateData_t) + sizeof(GA::decisionData_t);

	size_t LAtotalSize = LAagentSize * numberOfLAs;
	size_t GAtotalSize = GAagentSize * numberOfGAs;

#ifdef DEBUG
	printf("Bytes per LA: %zi, per GA: %zi\n", LAagentSize, GAagentSize);
	printf("LA total bytes: %zi, GA total: %zi\n", LAtotalSize, GAtotalSize);
#endif // DEBUG

	size_t actualLAsize = LAcoldDataController_ptr->capacityForDataInBytes() +
		LAstateController_ptr->capacityForDataInBytes() +
		LAdecisionDataController_ptr->capacityForDataInBytes();

	size_t actualGAsize = GAcoldDataController_ptr->capacityForDataInBytes() +
		GAstateController_ptr->capacityForDataInBytes() +
		GAdecisionDataController_ptr->capacityForDataInBytes();

	if (actualLAsize != LAtotalSize) {
		LOG_ERROR("LA data capacity at controller doesn't match expected");
		printf("--> is %zi instead\n", actualLAsize);
	}
	else {
		LOG_TRACE("LA data capacity at controller is as expected");
	}
	if (actualGAsize != GAtotalSize) {
		LOG_ERROR("GA data capacity at controller doesn't match expected");
		printf("--> is %zi instead\n", actualGAsize);
	}
	else {
		LOG_TRACE("GA data capacity at controller is as expected");
	}
}