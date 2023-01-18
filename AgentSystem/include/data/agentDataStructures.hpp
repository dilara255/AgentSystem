#pragma once

/*
* WARNING: THIS FILE WILL BE EXPOSED TO THE APPLICATION!
* - TO DO: MOVE TO API/AS_dataTypes
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
	} AS_API resources_t;

	enum class ResourcesField { CURRENT, INCOME, 
		                        TOTAL_RESOURCE_FIELDS };


	typedef struct {
		float current;
		float externalGuard;
		float thresholdToCostUpkeep;
		float currentUpkeep;
	} AS_API strenght_t;

	enum class StrenghtField { CURRENT, GUARDING, THRESHOLD, UPKEEP, 
		                       TOTAL_STRENGHT_FIELDS };


	typedef struct {
		float x;
		float y;
	} AS_API pos_t;

	enum class PositionField { X, Y,
		                       TOTAL_POSITION_FIELDS };


	typedef float AS_API LAdecisionOffsets_t[AS::TOTAL_CATEGORIES][AS::TOTAL_MODES];


	typedef struct {
		int diplomaticStanceToNeighbors[MAX_LA_NEIGHBOURS];
		float dispositionToNeighbors[MAX_LA_NEIGHBOURS];
		float dispositionToNeighborsLastStep[MAX_LA_NEIGHBOURS];
	} AS_API LAneighborRelations_t;

	enum class LArelations { STANCE, DISPOSITION, DISPOSITION_LAST_STEP,
		                     TOTAL_GA_RELATION_PARAMETERS };


	typedef struct {
		int diplomaticStanceToNeighbors[MAX_GA_QUANTITY];
		float dispositionToNeighbors[MAX_GA_QUANTITY];
		float dispositionToNeighborsLastStep[MAX_GA_QUANTITY];
	} AS_API GAneighborRelations_t;

	enum class GArelationsField { STANCE, DISPOSITION, DISPOSITION_LAST_STEP,
		                          TOTAL_GA_RELATION_PARAMETERS };


	typedef	float AS_API LAinfiltrationOnNeighbors_t[MAX_LA_NEIGHBOURS];
	typedef	float AS_API GAinfiltrationOnNeighbors_t[MAX_GA_QUANTITY];


	typedef int AS_API GApersonality[GA_PERSONALITY_TRAITS];


	typedef AZ::FlagField128 LAflagField_t;
	typedef AZ::FlagField32 GAflagField_t;

	//From these, we build:

	typedef struct {
		resources_t resources;
		strenght_t strenght;
	} AS_API LAparameters_t;

	enum class LAparametersField { RESOURCES, STRENGHT, BETERRABA,
		                           TOTAL_PARAMETERS_FIELDS };
	

	typedef struct {
		pos_t position;
		int firstConnectedNeighborId;
		int numberConnectedNeighbors;
		LAflagField_t connectedNeighbors;
		int neighbourIDs[MAX_LA_NEIGHBOURS];
	} AS_API LAlocationAndConnectionData_t;

	enum class LAlocationField { POSITION, ID_FIRST_CONNECTED, NUMBER_CONNECTIONS,
				     			 NEIGHBOURS, NEIGHBOUR_IDS, 
		                         TOTAL_LOCATION_FIELDS };


	typedef struct {
		LAdecisionOffsets_t incentivesAndConstraintsFromGA;
		LAdecisionOffsets_t personality;
	} AS_API LApersonalityAndGAinfluence_t;

	enum class LAdpersonalityField { LA_PERSONALITY, LA_OFFSETS_FROM_GA, 
		                              TOTAL_LA_OFFSET_FIELDS };


	typedef struct {
		resources_t LAesourceTotals;
		float LAstrenghtTotal;
		float GAresources;
	} AS_API GAparameterTotals_t;

	enum class GAparamentersField { RESOURCES_LAS, STRENGHT_LAS, GA_RESOURCES, 
		                            TOTAL_GA_PARAMETER_FIELDS };
}

//And finally:

namespace LA {
	
	typedef struct {
		AS::agentName_t name;
		unsigned id;
	} AS_API coldData_t;

	enum class ColdDataField { NAME, ID, 
		                   TOTAL_LA_COLD_FIELDS };


	typedef struct {
		bool onOff;
		AS::LAneighborRelations_t relations;
		AS::LAlocationAndConnectionData_t locationAndConnections;
		AS::LAparameters_t parameters;
		unsigned GAid;
	} AS_API stateData_t;

	enum class StateField { ACTIVE, RELATIONS, LOCATION, PARAMETERS, GA_ID, 
		                    TOTAL_LA_STATE_FIELDS };


	typedef struct {
		AS::LAinfiltrationOnNeighbors_t infiltration;
		AS::LApersonalityAndGAinfluence_t offsets;

		//TO DO: -A list of the expected values, for each neighbor, of:
		//resources, income, strenght, diplomacyand relations to this LA.
			
		//TO DO : -Desires, Impediments and Potential Actions Data;
	} AS_API decisionData_t;

	enum class DecisionField { INFILTRATION, OFFSETS, 
		                       TOTAL_GA_DECISION_FIELDS };
}

namespace GA {

	typedef struct {
		AS::agentName_t name;
		unsigned id;
	} AS_API coldData_t;

	enum class ColdField { NAME, ID, 
		                   TOTAL_GA_COLD_FIELDS };


	typedef struct {
		bool onOff;
		AS::GAneighborRelations_t relations;
		AS::LAflagField_t localAgentsBelongingToThis;
		AS::GAparameterTotals_t parameters;
		AS::GAflagField_t connectedGAs;
	} AS_API stateData_t;

	enum class StateField { ONOFF, RELATIONS, LOCAL_AGENTS, PARAMETERS, CONNECTED_GAS,
						    TOTAL_GA_STATE_FIELDS };


	typedef struct {
		AS::GAinfiltrationOnNeighbors_t infiltration;
		AS::GApersonality personality;

		//TO DO: -A list of the expected values, for each neighbor, of:
		//GA resources, totals, diplomacy and relations.

		//TO DO : -Desires, Impediments and Potential Actions Data;
	} AS_API decisionData_t;

	enum class DecisionField { INFILTRATION, PERSONALITY, 
		                       TOTAL_LA_DECISION_FIELDS };
}