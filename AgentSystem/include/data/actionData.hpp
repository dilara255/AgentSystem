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

	enum class scope { LOCAL, GLOBAL,
		               TOTAL};

	enum class actCategories { STRENGHT, RESOURCES, ATTACK, GUARD,
						       SPY, SABOTAGE, DIPLOMACY, CONQUEST,
							   TOTAL };
	//TODO: brief description of each

	enum class actModes { SELF, IMMEDIATE, REQUEST,
		                  TOTAL };
	//TODO: brief description of each

	//NOTE: SPAWN comes after total because it's processed different from the others
	enum class actPhases { PREPARATION, TRAVEL, EFFECT, RETURN, CONCLUSION,
		                   TOTAL, SPAWN };
	//TODO: brief description of each

	//Wether action variation - ie: strenght, immediate, local - exists, and it's "kind":
    //NOT: "no". SPC: "yes, and is specific to this action". STD: "yes, and is the standard".
	enum class actExists { NOT = 0, SPC = 1, STD = -1 };

	//Should be always guaranteed to be 32 bits wide.
	//TODO: a cast to uint32_t and a enum with offsets
	//TODO-CRITICAL: WARNING: (possible) BUG: MAY NOT BE PORTABLE?
	typedef struct ids_st {
		enum class fieldSizes { USED = 1, ACTIVE = 1, ORIGIN = 10, TARGET = 10, 
			                    SCOPE = 1, CATEGORY = 4, MODE = 2, PHASE = 3,
			                    TOTAL_BITS = 
									USED + ACTIVE + ORIGIN + TARGET + SCOPE
									+ CATEGORY + MODE + PHASE};

		static_assert((int)ids_st::fieldSizes::TOTAL_BITS == 32);

		uint32_t slotIsUsed : fieldSizes::USED;
		uint32_t active : fieldSizes::ACTIVE;
		uint32_t origin : fieldSizes::ORIGIN; //max LAs <= 1024
		uint32_t target : fieldSizes::TARGET;
		uint32_t scope : fieldSizes::SCOPE;
		uint32_t category : fieldSizes::CATEGORY;
		uint32_t mode : fieldSizes::MODE;
		uint32_t phase : fieldSizes::PHASE;

		bool operator==(ids_st& rhs) {
			return	(rhs.slotIsUsed == slotIsUsed)
					&& (rhs.active == active)
					&& (rhs.origin == origin)
					&& (rhs.target == target)
					&& (rhs.scope == scope)
					&& (rhs.category == category)
					&& (rhs.mode == mode)
					&& (rhs.phase == phase);
		}

		bool operator!=(ids_st& rhs) {
			return	!(*this == rhs);
		}
	} AS_API ids_t;

	//TODO: WARNING: MAY NOT BE PORTABLE?
	//Used to pack scope, agentID and action number in a uint32_t for transfering from client
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

	namespace Decisions {



	}
}
