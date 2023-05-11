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

	typedef struct {
		AS::scope scope;
		AS::actCategories category;
		AS::actModes mode;
	} AS_API actionLabel_t;

	namespace Decisions {

		enum class notionsSelf { LOW_INCOME_TO_STR, LOW_DEFENSE_TO_RESOURCES, LOW_CURRENCY, 
								 S3, S4, S5, S6, S7,
								 TOTAL, NOT_NOTION_SELF };
		typedef	float AS_API notionsSelf_t[(int)notionsSelf::TOTAL];
		
		enum class notionsNeighbor { LOW_DEFENSE_TO_RESOURCES, IS_STRONG, WORRIES_ME, 
			                         I_TRUST_THEM, N4, N5, N6, N7, N8, N9, N10, N11,
		                             TOTAL, NOT_NOTION_NEIGHBOR };
		typedef	float AS_API notionsNeighbor_t[(int)notionsNeighbor::TOTAL];

		constexpr int TOTAL_NOTIONS = (int)notionsSelf::TOTAL + (int)notionsNeighbor::TOTAL;

		//TODO: is this really the best way to label notions?
		typedef struct notionLabel_st {
			notionsSelf self = notionsSelf::NOT_NOTION_SELF;
			notionsNeighbor neighbor = notionsNeighbor::NOT_NOTION_NEIGHBOR;

			void setNotionSelf(notionsSelf notion) {
				self = notion;
				neighbor = notionsNeighbor::NOT_NOTION_NEIGHBOR;
			}

			void setNotionNeighbor(notionsNeighbor notion) {
				neighbor = notion;
				self = notionsSelf::NOT_NOTION_SELF;
			}

			bool isSet() {
				return 
					(neighbor != notionsNeighbor::NOT_NOTION_NEIGHBOR)
					|| (self != notionsSelf::NOT_NOTION_SELF);
			}

			bool isSelf() {
				return self != notionsSelf::NOT_NOTION_SELF;
			}

			bool isNeighbor() {
				return neighbor != notionsNeighbor::NOT_NOTION_NEIGHBOR;
			}
		} AS_API notionLabel_t;

		typedef struct score_st {
			float score;
			actionLabel_t label;
		} score_t;

		typedef struct scoresRecord_st {
			score_t record[SCORES_TO_KEEP_TRACK_EACH_DECISION_STAGE];
			int fieldsUsed = 0;
		} AS_API scoresRecord_t;

		typedef struct notion_st {
			float score;
			notionLabel_t label;
		} notion_t;

		typedef struct notionsRecord_st {
			notion_t record[NOTIONS_TO_KEEP_TRACK_EACH_DECISION_STAGE];
			int fieldsUsed = 0;
		} AS_API notionsRecord_t;
			
		typedef struct mitigationRecord_st {
			notionsRecord_t worries;
			scoresRecord_t helpfulOptions;
			scoresRecord_t newIdeas;

		} AS_API mitigationRecord_t;

		typedef struct decisionRecord_st {
			scoresRecord_t initialAmbitions;
			notionsRecord_t initialNotionsFor;
			mitigationRecord_t mitigationAttempts[MAX_MITIGATION_ROUNDS];
			scoresRecord_t finalOptions;

			int mitigationRounds = 0;
			uint64_t tickLastUpdate = 0;
		} AS_API decisionRecord_t;

		typedef struct networksDecisionsReflection_st {
			std::vector<decisionRecord_t> LAdecisionReflection;
			std::vector<decisionRecord_t> GAdecisionReflection;

			bool initialized = false;

			void clear() {
				LAdecisionReflection.clear();
				GAdecisionReflection.clear();
				initialized = false;
			}

			bool initialize(int numberLAs, int numberEffectiveGAs) {
				if(numberLAs < 0 || numberEffectiveGAs < 0 ){ return false; }
				
				decisionRecord_t emptyRecord;

				LAdecisionReflection.resize(numberLAs);
				GAdecisionReflection.resize(numberEffectiveGAs);

				initialized = true;

				return true;
			}

			bool reinitialize(int numberLAs, int numberEffectiveGAs) {
				clear();

				LAdecisionReflection.shrink_to_fit();
				GAdecisionReflection.shrink_to_fit();

				return initialize(numberLAs, numberEffectiveGAs);
			}
		} AS_API networksDecisionsReflection_t;
	}
}
