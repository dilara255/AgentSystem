#pragma once

#include "data/agentDataStructures.hpp"

namespace AS {
	void createAgentDataControllers();
}

//TO DO: too much repetition
//TO DO: make these Classes singleton
namespace LA {
	class ColdDataController {
	public:
		ColdDataController(uint32_t numberOfAgents);
		void addAgentData(coldData_t agentData);
		bool getAgentData(uint32_t agentID, coldData_t* recepient);
		size_t sizeOfDataInBytes();
		size_t capacityForDataInBytes();
	private:
		std::vector <coldData_t> data;
	};

	class StateController {
	public:
		StateController(uint32_t numberOfAgents);
		void addAgentData(stateData_t agentData);
		bool getAgentData(uint32_t agentID, stateData_t* recepient);
		size_t sizeOfDataInBytes();
		size_t capacityForDataInBytes();
	private:
		std::vector <stateData_t> data;
	};

	class DecisionSystem {
	public:
		DecisionSystem(uint32_t numberOfAgents);
		void addAgentData(decisionData_t agentData);
		bool getAgentData(uint32_t agentID, decisionData_t* recepient);
		size_t sizeOfDataInBytes();
		size_t capacityForDataInBytes();
	private:
		std::vector <decisionData_t> data;
	};
}

namespace GA {
	class ColdDataController {
	public:
		ColdDataController(uint32_t numberOfAgents);
		void addAgentData(coldData_t agentData);
		bool getAgentData(uint32_t agentID, coldData_t* recepient);
		size_t sizeOfDataInBytes();
		size_t capacityForDataInBytes();
	private:
		std::vector <coldData_t> data;
	};

	class StateController {
	public:
		StateController(uint32_t numberOfAgents);
		void addAgentData(stateData_t agentData);
		bool getAgentData(uint32_t agentID, stateData_t* recepient);
		size_t sizeOfDataInBytes();
		size_t capacityForDataInBytes();
	private:
		std::vector <stateData_t> data;
	};

	class DecisionSystem {
	public:
		DecisionSystem(uint32_t numberOfAgents);
		void addAgentData(decisionData_t agentData);
		bool getAgentData(uint32_t agentID, decisionData_t* recepient);
		size_t sizeOfDataInBytes();
		size_t capacityForDataInBytes();
	private:
		std::vector <decisionData_t> data;
	};
}

namespace AS {
	typedef struct {
		bool haveBeenCreated;
		LA::ColdDataController* LAcoldData_ptr;
		LA::StateController* LAstate_ptr;
		LA::DecisionSystem* LAdecision_ptr;
		GA::ColdDataController* GAcoldData_ptr;
		GA::StateController* GAstate_ptr;
		GA::DecisionSystem* GAdecision_ptr;
	} dataControllerPointers_t;

	extern dataControllerPointers_t agentDataControllers;
}
