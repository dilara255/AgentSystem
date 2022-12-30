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
-The id of the GA it belongs to, if any;
-The information about which LAs it's connected to (as a bitfield);
-A list of the disposition towards each connected LA (can be positive or negative);
-The diplomatic stance towards each connected LA (eg: hostile, neutral, trade, ally);

TO DO: DECISION:
-A list of how much information it has on each connected LA;
-A list of LA-decision-related thresholds, which act as the "personality" of the agent;
-A list of active constraints/incentives given by the associated GA;
TO DO: -Strength threshold to start costing upkeep and current upkeep;
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

#define NAME_LENGHT 30

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

	//TO DO: define MACRO an use an array
	typedef struct {
		float thr1;
		float thr2;
		float thr3;
		float thr4;
		float thr5;
		float thr6;
		/*...*/
	} localAgentThresholds_t;

	//TO DO: define MACRO an use an array
	typedef struct {
		float incentiveOrConstraint1;
		float incentiveOrConstraint2;
		float incentiveOrConstraint3;
		float incentiveOrConstraint4;
		float incentiveOrConstraint5;
		float incentiveOrConstraint6;
		/*...*/
	} incentivesAndConstraints_t;

	typedef int bitfield_t; //TO DO: change to Class with uint_64 field[N], N(MAX_AGENTS)
	//Also change name to void confusion with built-in

	typedef struct {
		resources_t resources;
		strenght_t strenght;
	} parametersLA_t;

	typedef struct {
		pos_t position;
		int firstConnectedNeighborId;
		int numberConnectedNeighbors;
		bitfield_t connectedNeighborsFirstElement; //TO DO: FIX: hacky, 
		//may need more than one. This has to remain the last item (WHY?). 
		//FIX BEFORE NEXT VERSION
	} locationAndConnectionDataLA_t;

	typedef struct {
		int GAid;
		incentivesAndConstraints_t incentivesAndConstraintsFromGA;
		localAgentThresholds_t thresholds;
	} thresholdsAndGAinfluenceOnLA_t;

	typedef struct {
		float* dispositionsToNeighbors_list;
		int* diplomaticStanceToNeighbors_list;
		float* informationAboutNeighbors_list;
	} neighborRelationPtrs_t;

	typedef char agentName_t[NAME_LENGHT + 1];

	//TO DO: Make into a Class, on separate file
	typedef struct {
		//for "exporting"
		int id; //defined on load, trying to minimize the id-distance between neighbors

		agentName_t name;

		bool on;

		parametersLA_t resourcesAndStrenght;

		locationAndConnectionDataLA_t locationAndConnections;

		//arrays created on load, 
		//fixed size == locationAndConnections.numberConnectedNeighbors
		neighborRelationPtrs_t neighborRelation_ptrs;

		thresholdsAndGAinfluenceOnLA_t thresholdsAndGAinfluence;
	} localAgent_t;

	typedef int GApersonality[4];

	//TO DO: Rellocate this?
	enum gaPersonalityTraits {/*add some*/ };

	typedef struct {
		parametersLA_t resourcesAndStrenghtLAs;
		bitfield_t localAgentsBelongingToThis; //Should be same size as LA onOff list
		//TO DO: FIX: HACKY: this has to remain the last item (WHY?). 
		//FIX BEFOR NEXT VERSION
	} associatedLAparameters_t;

	//TO DO: Make into a Class, on separate file
	typedef struct {
		//for "exporting"
		int id;

		agentName_t name;

		bool on;

		associatedLAparameters_t accumulatedLAparameters;

		bitfield_t connectedGAs;

		//arrays created on load, fixed size == numberConnectedGAs
		neighborRelationPtrs_t neighborRelation_ptrs;

		GApersonality traits;
	} globalAgent_t;
}

namespace LA {
	//TO DO: REWORK, based on LA: State, Decision, Cold Systems, as Classes
	//Old:
	//memory layout for AS: each field is contiguous between all agents, on its system.
	//6 "systems", indexed by id.
	//For AS "on" will actually be just a bitfield you check against the id. 
	//For the "export" structure it's a bool. 

	//calss LocalAgent with id as only member and constructors as only methods. 

	typedef struct {
		AS::agentName_t* nameList_ptr;
		AS::bitfield_t* onOffList_ptr; //I need ceil(NUMBER_OF_LAs/sizeof(int*8)) ints
		AS::parametersLA_t* parameterList_ptr;
		AS::locationAndConnectionDataLA_t* locationAndConnectionData_ptr;
		AS::neighborRelationPtrs_t* neighborRelationsList_ptr;
		AS::thresholdsAndGAinfluenceOnLA_t* tresholdsAndInfluenceData_ptr;
	} systemsPointers_t;
}

namespace GA {
	//TO DO: REWORK, based on GA: State, Decision, Cold Systems, as Classes
	//Old:
	//memory layout for AS: names / on / resources and strenghts / positions and connections  / 
	// neighbor stuff / personality
	//6 "systems", indexed by id, similar to LAs
	//For AS "on" will actually be just a bitfield you check against the id. For the "export" structure it's a bool.

	typedef struct {
		AS::agentName_t* nameList_ptr;
		AS::bitfield_t* onOffList_ptr; //I need ceil(NUMBER_OF_LAs/sizeof(int*8)) ints
		AS::associatedLAparameters_t* LAparameterList_ptr;
		AS::bitfield_t* GAconnectionsList_ptr;
		AS::neighborRelationPtrs_t* neighborRelationsList_ptr;
		AS::GApersonality* personalitiesList_ptr;
	} systemsPointers_t;
}

namespace AS { //TO DO: MAKE CLASS, change in AS_internal.hpp, rellocate
	typedef struct {
		int numberOfLAs;
		int numberOfGAs;

		int neighborMaxLAs;

		int lengthLAonOffList;
		int lengthGAonOffList;

		GA::systemsPointers_t GAsystemsPtrs;
		LA::systemsPointers_t LAsystemsPtrs;
	} agentsControl_t;
}