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

#include "miscDefines.hpp"
#include "miscStdHeaders.h"

namespace AS {

	enum class actCategories { STRENGHT, RESOURCES, ATTACK, GUARD,
						       SPY, SABOTAGE, DIPLOMACY, CONQUEST,
							   TOTAL };
	//TODO: brief description of each

	enum class actModes { SELF, IMMEDIATE, REQUEST,
		                  TOTAL };
	//TODO: brief description of each

	enum class scope { LOCAL, GLOBAL,
		               TOTAL};

	//Wether action variation - ie: strenght, immediate, local - exists, and it's "kind":
    //NOT: "no". SPC: "yes, and is specific to this action". STD: "yes, and is the standard".
	enum class actExists { NOT = 0, SPC = 1, STD = -1 };

	//TODO-CRITICAL: WARNING: (possible) BUG: MAY NOT BE PORTABLE?
	typedef struct {
		uint32_t active : 1;
		uint32_t origin : 11;
		uint32_t target : 11;
		uint32_t category : 3;
		uint32_t scope : 1;
		uint32_t mode : 2;
		uint32_t phase : 2;

		enum class fields { ACTIVE, ORIGIN, TARGET, CATEGORY, SCOPE, MODE, PHASE,
			                TOTAL_ACTION_FIELDS };
	} AS_API ids_t;

	//TODO: WARNING: MAY NOT BE PORTABLE?
	//Used to pack scope, agent and action number in a uint32_t for transfering from client
	//NOTE: this is *NOT* meant to be used with ids_t
	enum class actionIDtoUnsigned: uint32_t {SCOPE_SHIFT = 31, SCOPE_MASK = 2147483648, 
										     AGENT_SHIFT = 15, AGENT_MASK = 2147450880,
											 ACTION_SHIFT = 0, ACTION_MASK = 32767 };

	typedef uint32_t uint32_tenthsOfMilli_t;

	typedef struct {
		uint32_tenthsOfMilli_t elapsed;
		uint32_tenthsOfMilli_t total;

		enum class fields { INITIAL, LAST_PROCESSED,
			                TOTAL_ACTION_FIELDS };
	} AS_API timeInfo_t;

	typedef struct {
		float intensity;
		float processingAux;

		enum class fields { INTENSITY, AUX,
			                TOTAL_ACTION_FIELDS	};
	} AS_API details_t;

	typedef struct {
		ids_t ids;
		timeInfo_t phaseTiming;
		details_t details;

		enum class fields { IDS, TICKS, DETAILS,
			                TOTAL_ACTION_FIELDS };
	} AS_API actionData_t;
}
