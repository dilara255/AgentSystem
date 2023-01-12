#pragma once

#include "agentDataStructures.hpp"

//TO DO: too much repetition
//TO DO: make these Classes singleton
namespace LA {
	class ColdDataController {
	public:
		ColdDataController(uint32_t numberOfAgents);

		const std::vector <coldData_t>* getDataCptr() const {
			return (const std::vector <coldData_t>*) & data; }

		bool addAgentData(coldData_t agentData);
		bool getAgentData(uint32_t agentID, coldData_t* recepient) const;
		void popLastAgent() { data.pop_back(); }

		size_t sizeOfDataInBytes() const;
		size_t capacityForDataInBytes() const;

		void clearData();
	private:
		std::vector <coldData_t> data;
	};

	class StateController {
	public:
		StateController(uint32_t numberOfAgents);

		const std::vector <stateData_t>* getDataCptr() const {
			return (const std::vector <stateData_t>*) & data; }

		bool addAgentData(stateData_t agentData);
		bool getAgentData(uint32_t agentID, stateData_t* recepient) const;
		void popLastAgent() { data.pop_back(); }

		size_t sizeOfDataInBytes() const;
		size_t capacityForDataInBytes() const;

		void clearData();
	private:
		std::vector <stateData_t> data;
	};

	class DecisionSystem {
	public:
		DecisionSystem(uint32_t numberOfAgents);

		const std::vector <decisionData_t>* getDataCptr() const {
			return (const std::vector <decisionData_t>*) & data; }

		bool addAgentData(decisionData_t agentData);
		bool getAgentData(uint32_t agentID, decisionData_t* recepient) const;
		void popLastAgent() { data.pop_back(); }

		size_t sizeOfDataInBytes() const;
		size_t capacityForDataInBytes() const;

		void clearData();
	private:
		std::vector <decisionData_t> data;
	};
}

namespace GA {
	class ColdDataController {
	public:
		ColdDataController(uint32_t numberOfAgents);

		const std::vector <coldData_t>* getDataCptr() const {
			return (const std::vector <coldData_t>*) & data; }

		bool addAgentData(coldData_t agentData);
		bool getAgentData(uint32_t agentID, coldData_t* recepient) const;
		void popLastAgent() { data.pop_back(); }

		size_t sizeOfDataInBytes() const;
		size_t capacityForDataInBytes() const;

		void clearData();
	private:
		std::vector <coldData_t> data;
	};

	class StateController {
	public:
		StateController(uint32_t numberOfAgents);

		const std::vector <stateData_t>* getDataCptr() const {
			return (const std::vector <stateData_t>*) & data; }

		bool addAgentData(stateData_t agentData);
		bool getAgentData(uint32_t agentID, stateData_t* recepient) const;
		void popLastAgent() { data.pop_back(); }

		size_t sizeOfDataInBytes() const;
		size_t capacityForDataInBytes() const;

		void clearData();
	private:
		std::vector <stateData_t> data;
	};

	class DecisionSystem {
	public:
		DecisionSystem(uint32_t numberOfAgents);

		const std::vector <decisionData_t>* getDataCptr() const {
			return (const std::vector <decisionData_t>*) & data; }

		bool addAgentData(decisionData_t agentData);
		bool getAgentData(uint32_t agentID, decisionData_t* recepient) const;
		void popLastAgent() { data.pop_back(); }

		size_t sizeOfDataInBytes() const;
		size_t capacityForDataInBytes() const;

		void clearData();
	private:
		std::vector <decisionData_t> data;
	};
}

namespace AS {
	typedef struct {
		bool haveBeenCreated;
		bool haveData;
		LA::ColdDataController* LAcoldData_ptr;
		LA::StateController* LAstate_ptr;
		LA::DecisionSystem* LAdecision_ptr;
		GA::ColdDataController* GAcoldData_ptr;
		GA::StateController* GAstate_ptr;
		GA::DecisionSystem* GAdecision_ptr;
	} dataControllerPointers_t;

	bool createAgentDataControllers(dataControllerPointers_t* agentDataControllers_ptr);
}