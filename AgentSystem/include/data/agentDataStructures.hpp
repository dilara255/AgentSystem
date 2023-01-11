#pragma once

/*
* WARNING: THIS FILE MIGHT BE EXPOSED TO THE APPLICATION!
* 
* NOTE: we assume all LAs have maxNeighbours, which makes all agent's Data of a same,
* definite size after the network is initialized. We also assume maxLAs and maxGAs.
* See fixedParameters.hpp.
* 
* 
1) Each LA's data will be comprised of:

COLD:
-A name (of fixed maximum size); 
-An ID;

STATE:
-On/off boolean, to signal whether it should be taken in account or ignored by the system (say, because the location has been destroyed);
-A position;
-A quantity of resources and its update rate;
-A strength;
-Strength threshold to start costing upkeep and current upkeep;
-The id of the GA it belongs to, if any;
-The information about which LAs it's connected to (as a bitfield);
-A list of the disposition towards each connected LA (can be positive or negative);
-The diplomatic stance towards each connected LA (eg: hostile, neutral, trade, ally);

DECISION:
-A list of how much information it has on each connected LA;
-A list of LA decision offsets, which act as the "personality" of the agent;
-A list of active LA decision offsets given by the associated GA;
TO DO: -A list of the expected values, for each neighbor, of:
        resources, income, strenght, diplomacy and relations to this LA.
		TO DO: should evaluate personality and priorities as well? LEANING NO (NOT NOW)
TO DO: -Desires, Impediments and Potential Actions Data;

2) Each GA's data will be comprised of:

COLD:
-A name (of fixed maximum size);
-An ID;

STATE:
-On/off, as in the LAs;
-The information about which LAs belong to it (as a bitfield);
-The information about which other GAs it’s connected to (as a bitfield);
-GA resources (used to pay for Actions, acquired as % of LA incomes);
-A list of the disposition towards each connected GA (can be positive or negative);
-The diplomatic stance towards each connected GA (eg: hostile, neutral, trade, ally);
-Total resources and accumulation rates of the connected LAs;
-Total strength of the connected LAs.

DECISION:
-A list of how much information it has on each connected GA;
-Four traits (4 ids), which will influence the GAs "personality";
TO DO: -A list of the expected values, for each neighbor, of:
       GA resources, totals, diplomacy and relations.
TO DO: -Desires, Impediments and Potential Actions Data;
*/

#include "network/parameters.hpp"
#include "systems/actionSystem.hpp"

#include "flagFields.hpp"

namespace AS {
	//General:
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
		float externalGuard;
		float thresholdToCostUpkeep;
		float currentUpkeep;
	} strenght_t;

	typedef char agentName_t[NAME_LENGHT + 1];

	//LA related:
	typedef float LAdecisionOffsets_t[AS::TOTAL_CATEGORIES][AS::TOTAL_MODES];

	typedef struct {
		resources_t resources;
		strenght_t strenght;
	} LAparameters_t;

	typedef struct {
		pos_t position;
		int firstConnectedNeighborId;
		int numberConnectedNeighbors;
		LAflagField_t connectedNeighbors;
		int neighbourIDs[MAX_LA_NEIGHBOURS];
	} LAlocationAndConnectionData_t;

	typedef struct {
		LAdecisionOffsets_t incentivesAndConstraintsFromGA;
		LAdecisionOffsets_t personality;
	} LApersonalityAndGAinfluence_t;

	typedef struct {
		int diplomaticStanceToNeighbors[MAX_LA_NEIGHBOURS];
		float dispositionToNeighbors[MAX_LA_NEIGHBOURS];
		float dispositionToNeighborsLastStep[MAX_LA_NEIGHBOURS];
	} LAneighborRelations_t;

	typedef	float LAinfiltrationOnNeighbors_t[MAX_LA_NEIGHBOURS];

	//GA specific:
	typedef int GApersonality[GA_PERSONALITY_TRAITS];

	typedef struct {
		int diplomaticStanceToNeighbors[MAX_GA_QUANTITY];
		float dispositionToNeighbors[MAX_GA_QUANTITY];
		float dispositionToNeighborsLastStep[MAX_GA_QUANTITY];
	} GAneighborRelations_t;

	typedef	float GAinfiltrationOnNeighbors_t[MAX_GA_QUANTITY];

	typedef struct {
		resources_t LAesourceTotals;
		float LAstrenghtTotal;
		float GAresources;
	} GAparameterTotals_t;
}

namespace LA {
	
	typedef struct {
		AS::agentName_t name;
		unsigned id;
	} coldData_t;

	typedef struct {
		bool onOff;
		AS::LAneighborRelations_t relations;
		AS::LAlocationAndConnectionData_t locationAndConnections;
		AS::LAparameters_t parameters;
		unsigned GAid;
	} stateData_t;

	typedef struct {
		AS::LAinfiltrationOnNeighbors_t infiltration;
		AS::LApersonalityAndGAinfluence_t offsets;

		//TO DO: -A list of the expected values, for each neighbor, of:
		//resources, income, strenght, diplomacyand relations to this LA.
			
		//TO DO : -Desires, Impediments and Potential Actions Data;
	} decisionData_t;
}

//Data Controller Class declarations
namespace GA {

	typedef struct {
		AS::agentName_t name;
		unsigned id;
	} coldData_t;

	typedef struct {
		bool onOff;
		AS::GAneighborRelations_t relations;
		AS::LAflagField_t localAgentsBelongingToThis;
		AS::GAparameterTotals_t parameters;
		AS::GAflagField_t connectedGAs;
	} stateData_t;

	typedef struct {
		AS::GAinfiltrationOnNeighbors_t infiltration;
		AS::GApersonality personality;

		//TO DO: -A list of the expected values, for each neighbor, of:
		//GA resources, totals, diplomacy and relations.

		//TO DO : -Desires, Impediments and Potential Actions Data;
	} decisionData_t;
}