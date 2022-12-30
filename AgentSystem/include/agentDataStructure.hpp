#pragma once

/*
* NOTE: we assume all LAs have maxNeighbours, which makes all agent's Data of a same,
* definite size after the network is initialized.
* 
* TO DO: Rework the structures : )
* TO DO: use fixed width types!
* 
1) Each LA's data will be comprised of:

TO DO: COLD:
-A name (of fixed maximum size); 

TO DO: STATE:
-On/off boolean, to signal whether it should be taken in account or ignored by the system (say, because the location has been destroyed);
-A position;
-A quantity of resources and its update rate;
-A strength;
-Strength threshold to start costing upkeep and current upkeep;
-The id of the GA it belongs to, if any;
-The information about which LAs it's connected to (as a bitfield);
-A list of the disposition towards each connected LA (can be positive or negative);
-The diplomatic stance towards each connected LA (eg: hostile, neutral, trade, ally);

TO DO: DECISION:
-A list of how much information it has on each connected LA;
-A list of LA decision offsets, which act as the "personality" of the agent;
-A list of active LA decision offsets given by the associated GA;
TO DO: -A list of the expected values, for each neighbor, of:
        resources, income, strenght, diplomacy and relations to this LA.
		TO DO: should evaluate personality and priorities as well? LEANING NO (NOT NOW)
TO DO: -Desires, Impediments and Potential Actions Data;

2) Each GA's data will be comprised of:

TO DO: COLD:
-A name (of fixed maximum size);

TO DO: STATE:
-On/off, as in the LAs;
-The information about which LAs belong to it (as a bitfield);
-The information about which other GAs it’s connected to (as a bitfield);
-GA resources (used to pay for Actions, acquired as % of LA incomes);
-A list of the disposition towards each connected GA (can be positive or negative);
-The diplomatic stance towards each connected GA (eg: hostile, neutral, trade, ally);
-Total resources and accumulation rates of the connected LAs;
-Total strength of the connected LAs.

TO DO: DECISION:
-A list of how much information it has on each connected GA;
-Four traits (4 ids), which will influence the GAs "personality";
TO DO: -A list of the expected values, for each neighbor, of:
       GA resources, totals, diplomacy and relations.
TO DO: -Desires, Impediments and Potential Actions Data;
*/

#include "fixedParameters.hpp"
#include "flagFields.hpp"

namespace AS {
	typedef struct {
		float x;
		float y;
	} pos_t;

	typedef struct {
		float current;
		float updateRate;
	} resources_t;

	typedef struct {
		float current;
		float thresholdToCostUpkeep;
		float currentUpkeep;
	} strenght_t;

	typedef struct {
		float thresholds[NUMBER_LA_OFFSETS];
	} LAdecisionOffsets_t;

	typedef struct {
		resources_t resources;
		strenght_t strenght;
	} parametersLA_t;

	typedef struct {
		pos_t position;
		int firstConnectedNeighborId;
		int numberConnectedNeighbors;
		LAflagField_t connectedNeighbors;
	} locationAndConnectionDataLA_t;

	typedef struct {
		int GAid;
		LAdecisionOffsets_t incentivesAndConstraintsFromGA;
		LAdecisionOffsets_t personality;
	} personalityAndGAinfluenceOnLA_t;

	typedef struct {
		int diplomaticStanceToNeighbors[MAX_LA_NEIGHBOURS];
		float dispositionToNeighbors_t[MAX_LA_NEIGHBOURS];
	} LAneighborRelations_t;

	typedef	float LAinfiltrationOnNeighbors_t[MAX_LA_NEIGHBOURS];

	typedef char agentName_t[NAME_LENGHT + 1];

	typedef int GApersonality[4];

	typedef struct {
		int diplomaticStanceToNeighbors[MAX_GA_QUANTITY];
		float dispositionToNeighbors_t[MAX_GA_QUANTITY];
	} GAneighborRelations_t;

	typedef	float GAinfiltrationOnNeighbors_t[MAX_GA_QUANTITY];

	typedef struct {
		parametersLA_t resourcesAndStrenghtLAs;
		LAflagField_t localAgentsBelongingToThis;
	} associatedLAparameters_t;
}

namespace LA {

	typedef struct {

	} coldData_t;

	typedef struct {

	} stateData_t;

	typedef struct {

	} decisionData_t;

	class ColdDataController {
	public:
		ColdDataController(uint32_t numberOfAgents) {
			data.reserve(numberOfAgents);
		}

		void addAgentData(coldData_t agentData) {
			data.push_back(agentData);
		}

		bool getAgentData(uint32_t agentID, coldData_t* recepient) {
			if (agentID > (data.size() - 1)) return false;

			*recepient = data[agentID];
			return true;
		}

	private:
		std::vector <coldData_t> data;
	};

	class StateController {
	public:
		StateController(uint32_t numberOfAgents) {
			data.reserve(numberOfAgents);
		}

		void addAgentData(stateData_t agentData) {
			data.push_back(agentData);
		}

		bool getAgentData(uint32_t agentID, stateData_t* recepient) {
			if (agentID > (data.size() - 1)) return false;

			*recepient = data[agentID];
			return true;
		}

	private:
		std::vector <stateData_t> data;
	};

	class DecisionSystem {
	public:
		DecisionSystem(uint32_t numberOfAgents) {
			data.reserve(numberOfAgents);
		}

		void addAgentData(decisionData_t agentData) {
			data.push_back(agentData);
		}

		bool getAgentData(uint32_t agentID, decisionData_t* recepient) {
			if (agentID > (data.size() - 1)) return false;

			*recepient = data[agentID];
			return true;
		}

	private:
		std::vector <decisionData_t> data;
	};
}

namespace GA {

	typedef struct {

	} coldData_t;

	typedef struct {

	} stateData_t;

	typedef struct {

	} decisionData_t;

	class ColdDataController {
	public:
		ColdDataController(uint32_t numberOfAgents) {
			data.reserve(numberOfAgents);
		}

		void addAgentData(coldData_t agentData) {
			data.push_back(agentData);
		}

		bool getAgentData(uint32_t agentID, coldData_t* recepient) {
			if (agentID > (data.size() - 1)) return false;

			*recepient = data[agentID];
			return true;
		}

	private:
		std::vector <coldData_t> data;
	};

	class StateController {
	public:
		StateController(uint32_t numberOfAgents) {
			data.reserve(numberOfAgents);
		}

		void addAgentData(stateData_t agentData) {
			data.push_back(agentData);
		}

		bool getAgentData(uint32_t agentID, stateData_t* recepient) {
			if (agentID > (data.size()-1)) return false;

			*recepient = data[agentID];
			return true;
		}

	private:
		std::vector <stateData_t> data;
	};

	class DecisionSystem {
	public:
		DecisionSystem(uint32_t numberOfAgents) {
			data.reserve(numberOfAgents);
		}

		void addAgentData(decisionData_t agentData) {
			data.push_back(agentData);
		}

		bool getAgentData(uint32_t agentID, decisionData_t* recepient) {
			if (agentID > (data.size() - 1)) return false;

			*recepient = data[agentID];
			return true;
		}

	private:
		std::vector <decisionData_t> data;
	};
}