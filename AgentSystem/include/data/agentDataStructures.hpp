#pragma once

/*
* WARNING: THIS FILE WILL BE EXPOSED TO THE APPLICATION!
* - TO DO: MOVE TO API/AS_dataTypes
* 
* This file declares the basic data structres of agents.
* These structures build up from primitives to "cold", state" and "decision" data types,
* each with a variation for LA's and another for GA's.
* 
* NOTE: we assume all LAs have maxNeighbours, which makes all agent's Data of a same,
* definite size after the network is initialized. We also assume at most maxLAs and maxGAs.
*/

#include "../include/network/parameters.hpp"
#include "../include/systems/actionSystem.hpp"

#include "core.hpp"

#include "flagFields.hpp"

#define AS_MAX_SUB_FIELD_DEPTH 4

namespace AS {
	//General:
	typedef struct {
		float x;
		float y;
	} AS_API pos_t;

	typedef struct {
		float current;
		float updateRate;
	} AS_API resources_t;

	typedef struct {
		float current;
		float externalGuard;
		float thresholdToCostUpkeep;
		float currentUpkeep;
	} AS_API strenght_t;

	typedef char agentName_t[NAME_LENGHT + 1];

	//LA related:
	typedef float AS_API LAdecisionOffsets_t[AS::TOTAL_CATEGORIES][AS::TOTAL_MODES];

	typedef struct {
		resources_t resources;
		strenght_t strenght;
	} AS_API LAparameters_t;

	typedef struct {
		pos_t position;
		int firstConnectedNeighborId;
		int numberConnectedNeighbors;
		LAflagField_t connectedNeighbors;
		int neighbourIDs[MAX_LA_NEIGHBOURS];
	} AS_API LAlocationAndConnectionData_t;

	typedef struct {
		LAdecisionOffsets_t incentivesAndConstraintsFromGA;
		LAdecisionOffsets_t personality;
	} AS_API LApersonalityAndGAinfluence_t;

	typedef struct {
		int diplomaticStanceToNeighbors[MAX_LA_NEIGHBOURS];
		float dispositionToNeighbors[MAX_LA_NEIGHBOURS];
		float dispositionToNeighborsLastStep[MAX_LA_NEIGHBOURS];
	} AS_API LAneighborRelations_t;

	typedef	float AS_API LAinfiltrationOnNeighbors_t[MAX_LA_NEIGHBOURS];

	//GA specific:
	typedef int AS_API GApersonality[GA_PERSONALITY_TRAITS];

	typedef struct {
		int diplomaticStanceToNeighbors[MAX_GA_QUANTITY];
		float dispositionToNeighbors[MAX_GA_QUANTITY];
		float dispositionToNeighborsLastStep[MAX_GA_QUANTITY];
	} AS_API GAneighborRelations_t;

	typedef	float AS_API GAinfiltrationOnNeighbors_t[MAX_GA_QUANTITY];

	typedef struct {
		resources_t LAesourceTotals;
		float LAstrenghtTotal;
		float GAresources;
	} AS_API GAparameterTotals_t;
}

namespace LA {
	
	typedef struct {
		AS::agentName_t name;
		unsigned id;
	} AS_API coldData_t;

	typedef struct {
		bool onOff;
		AS::LAneighborRelations_t relations;
		AS::LAlocationAndConnectionData_t locationAndConnections;
		AS::LAparameters_t parameters;
		unsigned GAid;
	} AS_API stateData_t;

	typedef struct {
		AS::LAinfiltrationOnNeighbors_t infiltration;
		AS::LApersonalityAndGAinfluence_t offsets;

		//TO DO: -A list of the expected values, for each neighbor, of:
		//resources, income, strenght, diplomacyand relations to this LA.
			
		//TO DO : -Desires, Impediments and Potential Actions Data;
	} AS_API decisionData_t;
}

//Data Controller Class declarations
namespace GA {

	typedef struct {
		AS::agentName_t name;
		unsigned id;
	} AS_API coldData_t;

	typedef struct {
		bool onOff;
		AS::GAneighborRelations_t relations;
		AS::LAflagField_t localAgentsBelongingToThis;
		AS::GAparameterTotals_t parameters;
		AS::GAflagField_t connectedGAs;
	} AS_API stateData_t;

	typedef struct {
		AS::GAinfiltrationOnNeighbors_t infiltration;
		AS::GApersonality personality;

		//TO DO: -A list of the expected values, for each neighbor, of:
		//GA resources, totals, diplomacy and relations.

		//TO DO : -Desires, Impediments and Potential Actions Data;
	} AS_API decisionData_t;
}