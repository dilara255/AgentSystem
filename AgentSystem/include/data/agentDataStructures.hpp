#pragma once

/*
* WARNING: THIS FILE WILL BE EXPOSED TO THE APPLICATION!
* - TODO-CRITICAL: MOVE TO API/AS_dataTypes
* 
* This file declares the basic data structres of agents, as well as enum classes for each field.
* These structures build up from primitives to "cold", "state" and "decision" data types,
* each with a variation for LAs and another for GAs.
* 
* NOTE: In order for the structures to have definite size in compilation, we assume:
* MAX_LA_NEIGHBOURS, MAX_LA_QUANTITY and MAX_GA_QUANTITY.
*/

#include "../include/network/parameters.hpp"
#include "../include/systems/actionSystem.hpp"

#include "core.hpp"

#include "flagFields.hpp"

//Starting from the top,
//how many identifiers are needed to get to the deepest fields in these structures?
#define AS_MAX_SUB_FIELD_DEPTH 4

namespace AS {

	enum class DataCategory { NETWORK_PARAMS, 
		                      LA_COLD, LA_STATE, LA_DECISION,
		                      GA_COLD, GA_STATE, GA_DECISION,
		                      LA_ACTION, GA_ACTION, 
		      	              TOTAL_DATA_CATEGORIES };
	
	//Here, we will build up to the Cold, State and Decision data structures
	 
	//First, the basic type definitions (made out of primitives or AZ::Flagfields):

	typedef char agentName_t[NAME_LENGHT + 1];
	

	typedef struct {
		float current;
		float updateRate;

		enum class fields { CURRENT, INCOME,
							TOTAL_RESOURCE_FIELDS };
	} AS_API resources_t;

	typedef struct {
		float current;
		float externalGuard;
		float thresholdToCostUpkeep;
		float currentUpkeep;

		enum class fields { CURRENT, GUARDING, THRESHOLD, UPKEEP,
			                TOTAL_STRENGHT_FIELDS };
	} AS_API strenght_t;

	typedef struct {
		float x;
		float y;

		enum class fields { X, Y,
			                TOTAL_POSITION_FIELDS };
	} AS_API pos_t;

	//TODO: = operator PLEASE (and hunt down explicit loops)
	typedef float AS_API 
		LAdecisionOffsets_t[(int)AS::actCategories::TOTAL][(int)AS::actModes::TOTAL];

	enum class diploStance: uint8_t {WAR, NEUTRAL, TRADE, ALLY, ALLY_WITH_TRADE, TOTAL_STANCES};

	typedef struct {
		diploStance diplomaticStanceToNeighbors[MAX_LA_NEIGHBOURS];
		float dispositionToNeighbors[MAX_LA_NEIGHBOURS];
		float dispositionToNeighborsLastStep[MAX_LA_NEIGHBOURS];

		enum class fields { STANCE, DISPOSITION, DISPOSITION_LAST_STEP,
						    TOTAL_LA_RELATION_PARAMETERS };
	} AS_API LAneighborRelations_t;

	typedef struct {
		diploStance diplomaticStanceToNeighbors[MAX_GA_QUANTITY];
		float dispositionToNeighbors[MAX_GA_QUANTITY];
		float dispositionToNeighborsLastStep[MAX_GA_QUANTITY];

		enum class fields { STANCE, DISPOSITION, DISPOSITION_LAST_STEP,
							TOTAL_GA_RELATION_PARAMETERS };
	} AS_API GAneighborRelations_t;

	typedef	float AS_API LAinfiltrationOnNeighbors_t[MAX_LA_NEIGHBOURS];
	typedef	float AS_API GAinfiltrationOnNeighbors_t[MAX_GA_QUANTITY];

	typedef int AS_API GApersonality[GA_PERSONALITY_TRAITS];

	typedef AZ::FlagField128 LAflagField_t;
	typedef AZ::FlagField32 GAflagField_t;

	//From these, we build:
	//TODO: some of these would benefit from having built-in initialization logic and etc
	typedef struct {
		resources_t resources;
		strenght_t strenght;

		enum class fields { RESOURCES, STRENGHT,
							TOTAL_PARAMETERS_FIELDS };
	} AS_API LAparameters_t;

	typedef struct {
		pos_t position;
		int firstConnectedNeighborId;
		int numberConnectedNeighbors;
		LAflagField_t connectedNeighbors;
		int neighbourIDs[MAX_LA_NEIGHBOURS];

		enum class fields { POSITION, ID_FIRST_CONNECTED, NUMBER_CONNECTIONS,
						    NEIGHBOURS, NEIGHBOUR_IDS,
							TOTAL_LOCATION_FIELDS };
	} AS_API LAlocationAndConnectionData_t;

	typedef struct {
		LAdecisionOffsets_t incentivesAndConstraintsFromGA;
		LAdecisionOffsets_t personality;

		enum class fields { LA_PERSONALITY, LA_OFFSETS_FROM_GA,
			                TOTAL_LA_OFFSET_FIELDS };
	} AS_API LApersonalityAndGAinfluence_t;

	typedef struct {
		resources_t LAesourceTotals;
		float LAstrenghtTotal;
		float GAresources;

		enum class fields { RESOURCES_LAS, STRENGHT_LAS, GA_RESOURCES,
			                TOTAL_GA_PARAMETER_FIELDS };
	} AS_API GAparameterTotals_t;
}

//And finally:

namespace LA {
	
	typedef struct {
		AS::agentName_t name;
		unsigned id;

		enum class fields { NAME, ID,
			                TOTAL_LA_COLD_FIELDS };
	} AS_API coldData_t;

	typedef struct {
		bool onOff;
		AS::LAneighborRelations_t relations;
		AS::LAlocationAndConnectionData_t locationAndConnections;
		AS::LAparameters_t parameters;
		unsigned GAid;

		enum class fields { ACTIVE, RELATIONS, LOCATION, PARAMETERS, GA_ID,
			                TOTAL_LA_STATE_FIELDS };
	} AS_API stateData_t;

	typedef struct {
		AS::LAinfiltrationOnNeighbors_t infiltration;
		AS::LApersonalityAndGAinfluence_t offsets;

		//TODO: -A list of the expected values, for each neighbor, of:
		//resources, income, strenght and relation to this LA.
			
		//TODO : -Desires, Impediments and Potential Actions Data;
		//TODO: Substitute for the concept of Notions;
		//Maybe on notions use abs(infiltration) as "low"/"high": -1 believes is 1;

		enum class fields { INFILTRATION, OFFSETS,
			                TOTAL_GA_DECISION_FIELDS };
	} AS_API decisionData_t;
}

namespace GA {

	typedef struct {
		AS::agentName_t name;
		unsigned id;

		enum class fields { NAME, ID,
			                TOTAL_GA_COLD_FIELDS };
	} AS_API coldData_t;

	typedef struct {
		bool onOff;
		AS::GAflagField_t connectedGAs;
		int neighbourIDs[MAX_GA_QUANTITY];
		AS::GAneighborRelations_t relations;
		AS::LAflagField_t localAgentsBelongingToThis;
		int laIDs[MAX_LA_QUANTITY];
		AS::GAparameterTotals_t parameters;		

		enum class fields { ONOFF, RELATIONS, LOCAL_AGENTS, PARAMETERS, CONNECTED_GAS,
			                TOTAL_GA_STATE_FIELDS };
	} AS_API stateData_t;

	typedef struct {
		AS::GAinfiltrationOnNeighbors_t infiltration;
		AS::GApersonality personality;

		//TODO: -A list of the expected values, for each neighbor, of:
		//GA resources, totals and relation to this AI.

		//TODO: -Desires, Impediments and Potential Actions Data;
		//TODO: Substitute for the concept of Notions;
		//Maybe on notions use abs(infiltration) as "low"/"high": -1 believes is 1;

		enum class fields { INFILTRATION, PERSONALITY,
			                TOTAL_LA_DECISION_FIELDS };
	} AS_API decisionData_t;	
}