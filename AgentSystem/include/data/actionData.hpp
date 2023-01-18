#pragma once

/*
* WARNING: THIS FILE WILL BE EXPOSED TO THE APPLICATION!
* - TO DO: MOVE TO API/AS_dataTypes
* - TO DO: Moce into namespace and turn enums into enum classes

This file:
- Describes the data structure of actions;
- Has enums for action categories, modes, scopes and availability;
*/

#include "core.hpp"

#include "miscStdHeaders.h"

namespace AS {

	enum actionCategories { STRENGHT, RESOURCES, ATTACK, GUARD,
						    SPY, SABOTAGE, DIPLOMACY, CONQUEST,
							TOTAL_CATEGORIES };
	//TO DO: brief description of each

	enum actionModes { IMMEDIATE, REQUEST, SELF,
		               TOTAL_MODES };
	//TO DO: brief description of each

	enum actionScopes { LOCAL, GLOBAL,
		                TOTAL_SCOPES };

	enum actionAvailability { NOT_AVAILABE, SPECIFIC = 1, STANDARD = -1 };
	//WARNING: any updates to this should be reflected on the initialization of
	//Actions::availableVariations

	typedef struct {
		uint32_t active : 1;
		uint32_t origin : 13;
		uint32_t target : 13;
		uint32_t category : 3;
		uint32_t scope : 1;
		uint32_t mode : 1;

		enum class fields { ACTIVE, ORIGIN, TARGET, CATEGORY, SCOPE, MODE,
			                TOTAL_ACTION_FIELDS };
	} AS_API ids_t;

	typedef struct {
		uint32_t initial;
		uint32_t lastProcessed;

		enum class fields { INITIAL, LAST_PROCESSED,
			                TOTAL_ACTION_FIELDS };
	} AS_API tickInfo_t;

	typedef struct {
		float intensity;
		float processingAux;

		enum class fields { INTENSITY, AUX,
			                TOTAL_ACTION_FIELDS	};
	} AS_API details_t;

	typedef struct {
		ids_t ids;
		tickInfo_t ticks;
		details_t details;

		enum class fields { IDS, TICKS, DETAILS,
			                TOTAL_ACTION_FIELDS };
	} AS_API actionData_t;
}
