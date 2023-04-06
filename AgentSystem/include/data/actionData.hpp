#pragma once

//TODO-CRITICAL: move to API/AS_actionDataTypes
//TODO-CRITICAL: assignment and copy operators and initialization to default values
//TODO-CRITICAL: handling: ASmirror and ClientDataHandler
//TODO: review namespaces?

/*
This file:
- Describes the data structure of actions;
- Has enums for action categories, modes, scopes and availability;
*/

#include "core.hpp"

#include "miscStdHeaders.h"

namespace AS {

	enum class actCategories { STRENGHT, RESOURCES, ATTACK, GUARD,
						          SPY, SABOTAGE, DIPLOMACY, CONQUEST,
							      TOTAL };
	//TODO: brief description of each

	enum class actModes { IMMEDIATE, REQUEST, SELF,
		                     TOTAL };
	//TODO: brief description of each

	enum class scope { LOCAL, GLOBAL,
		               TOTAL};

	//Wether action variation - ie: strenght, immediate, local - exists, and it's "kind":
    //NOT: "no". SPC: "yes, and is specific to this action". STD: "yes, and is the standard".
	enum class actExists { NOT = 0, SPC = 1, STD = -1 };

	//TODO: WARNING: MAY NOT BE PORTABLE?
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

	//TODO: WARNING: MAY NOT BE PORTABLE?
	enum class actionIDtoUnsigned: uint32_t {SCOPE_SHIFT = 31, SCOPE_MASK = 2147483648, 
										     AGENT_SHIFT = 15, AGENT_MASK = 2147450880,
											 ACTION_SHIFT = 0, ACTION_MASK = 32767 };

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
