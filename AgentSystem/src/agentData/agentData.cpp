#include "miscStdHeaders.h"
#include "core.hpp"

#include "logAPI.hpp"

#include "agentData/controllers.hpp"
#include "AS_internal.hpp"


/*
* * Note: all structures have a fixed size once maxNeighbours is defined;
* 
* TO DO : Complete Rework.
* Use vectors to gold the Data, even though their size won't change after load.
* (so adding elements on load and clearing is very simple, and no(direct) malloc is used)
* (performance impact should be negligible with optimizations.TO DO : quick test of this)
*/

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

void testDataContainerCapacity(uint32_t numberOfLAs, uint32_t numberOfGAs){
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

/*DEPRECATED, didn't used set maximuns and assumptions around them.
* Keeping here for a couple versions just to be sure I don't need it.
* TO DO: DELETE
* 
void allocateMemoryForAgentSystems(int numberLAs, int numberGAs, int neighborMaxLAs) {
	//Allocates the entire thing and sets pointers.
	//See agentDataStructure.hpp for the layout.
	
	AS::agentControlSystem_ptr.numberOfLAs = numberLAs;
	AS::agentControlSystem_ptr.numberOfGAs = numberGAs;
	AS::agentControlSystem_ptr.neighborMaxLAs = neighborMaxLAs;

	AS::agentControlSystem_ptr.lengthLAonOffList = 
		(int)ceil((float)numberLAs/(sizeof(AS::bitfield_t)*8));
	AS::agentControlSystem_ptr.lengthGAonOffList = 
		(int)ceil((float)numberGAs/(sizeof(AS::bitfield_t)*8));

	int lengthLAonOffList = AS::agentControlSystem_ptr.lengthLAonOffList;
	int lengthGAonOffList = AS::agentControlSystem_ptr.lengthGAonOffList;
	int neighborRelationBytes = 2*sizeof(float) + sizeof(int);
	int LAneighborRelationDataSize = neighborRelationBytes * neighborMaxLAs * numberLAs;
	int GAconnectionsLenght = lengthGAonOffList;

	AS::agentControlSystem_ptr.LAsystemsPtrs.nameList_ptr =
		(AS::agentName_t*)malloc(sizeof(AS::agentName_t) * numberLAs);
	AS::agentControlSystem_ptr.LAsystemsPtrs.onOffList_ptr =
		(AS::bitfield_t*)malloc(sizeof(AS::bitfield_t) * lengthLAonOffList);
	AS::agentControlSystem_ptr.LAsystemsPtrs.parameterList_ptr =
		(AS::parametersLA_t*)malloc(sizeof(AS::parametersLA_t) * numberLAs);
	//FIX: this is kinda hacky:
	AS::agentControlSystem_ptr.LAsystemsPtrs.locationAndConnectionData_ptr =
		(AS::locationAndConnectionDataLA_t*)malloc(
			(sizeof(AS::locationAndConnectionDataLA_t)+(numberLAs -1)*sizeof(AS::bitfield_t))
				* numberLAs);
	AS::agentControlSystem_ptr.LAsystemsPtrs.neighborRelationsList_ptr =
		(AS::neighborRelationPtrs_t*)malloc(LAneighborRelationDataSize);
	AS::agentControlSystem_ptr.LAsystemsPtrs.tresholdsAndInfluenceData_ptr =
		(AS::thresholdsAndGAinfluenceOnLA_t*)malloc(
			sizeof(AS::thresholdsAndGAinfluenceOnLA_t) * numberLAs);

	AS::agentControlSystem_ptr.GAsystemsPtrs.nameList_ptr =
		(AS::agentName_t*)malloc(sizeof(AS::agentName_t) * numberGAs);
	AS::agentControlSystem_ptr.GAsystemsPtrs.onOffList_ptr =
		(AS::bitfield_t*)malloc(sizeof(AS::bitfield_t) * lengthGAonOffList);
	//FIX: this is kinda hacky:
	AS::agentControlSystem_ptr.GAsystemsPtrs.LAparameterList_ptr =
		(AS::associatedLAparameters_t*)malloc(
			(sizeof(AS::associatedLAparameters_t)+(numberLAs-1)*sizeof(AS::bitfield_t))
				*numberGAs);
	//FIX: this is kinda hacky:
	AS::agentControlSystem_ptr.GAsystemsPtrs.GAconnectionsList_ptr =
		(AS::bitfield_t*)malloc(sizeof(AS::bitfield_t)*GAconnectionsLenght*numberGAs);
	AS::agentControlSystem_ptr.GAsystemsPtrs.neighborRelationsList_ptr =
		(AS::neighborRelationPtrs_t*)malloc(neighborRelationBytes*numberGAs);
	AS::agentControlSystem_ptr.GAsystemsPtrs.personalitiesList_ptr =
		(AS::GApersonality*)malloc(
			sizeof(AS::GApersonality) * numberGAs);

	//TO DO: set internal pointers 
	//actually, re-think pointers inside "internal" structs
	//TO DO: tie this to some initialization function and also set and verify some values
}
*/


