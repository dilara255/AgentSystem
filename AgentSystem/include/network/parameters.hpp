#pragma once

//TODO: separate DEFAULTS and REFERENCE_VALUES

/*
This file holds:
- Defines for:
-- Fixed parameters which ALL networks will have to follow;
--- Most can easily be refactored to be received as inputs, except for the Network size ones;
-- Defaults for agent and action data, used when creating new files and tests;
-- Test-related defines.

- And also some stuff which TODO: SHOULDN'T BE HERE ANYMORE:
-- Data structure to hold parameters of a network (networkParameters_t);
--- But which ended up also holding other, dynamic stuff;
---> TODO: move this out of here and rename this to "fixedParameters";
----> Try to actually contain all of those in here, to help future refactors.

//TODO: review the description below an update all of this when networkParameters_t is moved.

networkParameters_t includes:
- number of local and global agents;
- a "network name", which can be used with the load/save system;
- an optional "comment" (which can also be used with the load/save system);
- the actual maximum neighbours per LA;
- the actual maximum actions per agent;
- how many ticks the network has ran.

The fixed parameters represent a "class" of network. These are essentially 
constraints for any network to be loaded. They help make things predictable 
and data sizes static.
*/

#include "core.hpp"
#include "flagFields.hpp"
#include "prng.hpp"
#include "miscDefines.hpp"

//--> FIXED parameters, relating to:

//Network size (these will change array sizes):
#define MAX_LA_NEIGHBOURS 10
#define MAX_GA_QUANTITY 16 //NOTE: last one is reserved to "belongs to no GA"
#define MAX_NEIGHBORS (std::max(MAX_LA_NEIGHBOURS, MAX_GA_QUANTITY))
#define MAX_LA_QUANTITY 128
#define MAX_AGENTS (MAX_GA_QUANTITY + MAX_LA_QUANTITY)
#define MAX_ACTIONS_PER_AGENT 10
#define NAME_LENGHT 30
#define COMMENT_LENGHT 250

//TODO: The following parameters can sort-of-easily be turned into load-time parameters
//TODO: A few of the following parameters are really other systems' limits : split them

//System flow:
//Note: the AS uses naive integration in some spots, so high pace and period may break stuff
#define AS_GENERAL_PACE (1.0f) //will change all timeMultipliers
#define AS_MILLISECONDS_PER_STEP (4)
#define MAX_PROPORTIONAL_STEP_DURATION_ERROR (5) 
#define AS_MILLISECONDS_PER_DECISION_ROUND (40) //will be floored to multiple of step time
#define AS_TOTAL_CHOPS (AS_MILLISECONDS_PER_DECISION_ROUND/AS_MILLISECONDS_PER_STEP)
#define SLEEP_TIME_WAITING_MUTEX_MICROS 10
#define MAX_MUTEX_TRY_LOCKS 500 //will try at least twice anyway
#define SECONDS_PER_ERROR_DISPLAY (5) //will be floored to multiple of step time in ms

//Economy:
#define EXTERNAL_GUARD_UPKEEP_RATIO_BY_DEFENDED (0.5)
#define EXTERNAL_GUARD_UPKEEP_RATIO_BY_DEFENDER (1-EXTERNAL_GUARD_UPKEEP_RATIO_BY_DEFENDED)
#define TRADE_SATURATION_MULTIPLIER (1.0)
#define TRADE_SATURATION_FROM_WAR 3
#define TRADE_SATURATION_FROM_NEUTRAL 0
#define TRADE_SATURATION_FROM_TRADE 1
#define TRADE_SATURATION_FROM_ALLY 1
#define TRADE_SATURATION_FROM_ALLY_WITH_TRADE 2
#define LA_UPKEEP_PER_EXCESS_STRENGHT (0.5)
#define TRADE_FACTOR_LA_PER_SECOND (0.5f) //applies to liquid income rate (LAs)
#define TRADE_FACTOR_GA (0.5f) //applied to last tax income (already takes time in account)
#define ATTRITION_FACTOR_PER_SECOND (0.001)
#define GA_TAX_RATE_PER_SECOND (0.001f)

//Diplomacy:
#define DIPLOMATIC_STANCES 5 //TODO: would this be better left to the enum class diploStances?
#define TOTAL_LA_INFO_RELATION_WEIGHT_FOR_GA_PER_SECOND (0.001f)
#define MAX_DISPOSITION_RAISE_FROM_TRADE_PER_SECOND (0.001f)
#define MAX_INFILTRATION_RAISE_FROM_TRADE_PER_SECOND (0.001f)
#define DISPOSITION_RAISE_FROM_ALLIANCE_PER_SECOND (MAX_DISPOSITION_RAISE_FROM_TRADE_PER_SECOND/4)
#define INFILTRATION_RAISE_FROM_ALLIANCE_PER_SECOND (MAX_INFILTRATION_RAISE_FROM_TRADE_PER_SECOND/4)
#define INFILTRATION_CHANGE_FROM_NEIGHBOR_DISPOSITION_PER_SECOND (0.001f)
#define MIN_DISPOSITION (-1.0f)
#define MAX_DISPOSITION (1.0f)
#define MIN_INFILTRATION (-1.0f)
#define MAX_INFILTRATION (1.0f)

//Expected-value reading parameters:
#define LA_FIELDS_TO_READ 4 //TODO: use the enum from the data structure
#define GA_FIELDS_TO_READ 5 //TODO: use the enum from the data structure
#define PRNS_PER_FIELD_DEDUCED 2 //TODO: shouldn't this be 1?
#define NEIGHBORS_RELATIVE_WEIGHT_FOR_REF_EXPECTATIONS (1.0f)
#define TOTAL_WEIGHT_FOR_REF_EXPECTATIONS (1.0f + NEIGHBORS_RELATIVE_WEIGHT_FOR_REF_EXPECTATIONS)
#define EXPC_MAX_PROPORTIONAL_CHANGE_PER_SECOND (0.1f)
#define EXPC_INFILTRATION_EQUIVALENT_TO_MAXIMUM_NOISE (2.0f)
#define EXPC_EFFECTIVE_MIN_PROPORTIONAL_DIFF (0.01f)
#define EXPC_MIN_PROPORTIONAL_ERROR_TO_CORRECT (1.0f)
#define EXPC_PROPORTIONAL_ERROR_OVER_MINIMUM_FOR_EQUILIBRIUM (2.0f)
#define EXPC_PROPORTIONAL_ERROR_OVER_EQUILIBRIUM_FOR_MAX_CORRECTION (2.0f)
#define EXPC_PROPORTIONAL_ERROR_OVER_MINIMUM_FOR_MAX_CORRECTION (EXPC_PROPORTIONAL_ERROR_OVER_MINIMUM_FOR_EQUILIBRIUM + EXPC_PROPORTIONAL_ERROR_OVER_EQUILIBRIUM_FOR_MAX_CORRECTION)
#define EXPC_PROPORTIONAL_ERROR_FOR_MAX_CORRECTION (EXPC_PROPORTIONAL_ERROR_OVER_MINIMUM_FOR_MAX_CORRECTION + EXPC_MIN_PROPORTIONAL_ERROR_TO_CORRECT)
#define EXPC_ERROR_CORRECTION_SHARPNESS (4.0f)

//Decision-making parameters:
#define PRNS_TO_CHOOSE_ACTION 0
#define CONSTRAINT_CHECK_ROUNDS 1
#define NOTIONS_ABOUT_SELF 8
#define NOTIONS_ABOUT_NEIGHBOR 12
#define NUMBER_LA_OFFSETS (MAX_ACTIONS_PER_AGENT + (DIPLOMATIC_STANCES - 1))
#define GA_PERSONALITY_TRAITS 4
#define MAX_MITIGATION_ROUNDS 3
#define MAX_ACT_GOAL_SHORTLIST_SIZE 10
#define MAX_ACT_CHOICE_SHORTLIST_SIZE 20
#define MIN_DECISION_THRESHOLD_SEPARATIONS (0.1f)
#define MIN_ACT_WHY_BOTHER_THRESOLD (MIN_DECISION_THRESHOLD_SEPARATIONS)
#define MIN_ACT_JUST_DO_IT_THRESOLD (MIN_ACT_WHY_BOTHER_THRESOLD - MIN_DECISION_THRESHOLD_SEPARATIONS)
#define MAX_ACT_JUST_DO_IT_THRESOLD (1.0f - MIN_DECISION_THRESHOLD_SEPARATIONS)
#define MAX_ACT_WHY_BOTHER_THRESOLD (MAX_ACT_JUST_DO_IT_THRESOLD - MIN_DECISION_THRESHOLD_SEPARATIONS)
#define ACT_INTENSITY_WHY_BOTHER (0.5f)
#define ACT_INTENSITY_JUST_DO_IT (1.0f)
#define ACT_INTENSITY_SCORE_1 (3.0f)
#define ACT_INTENSITY_DIFFERENCE_TO_JUST_DO_IT (ACT_INTENSITY_JUST_DO_IT - ACT_INTENSITY_WHY_BOTHER)
#define ACT_INTENSITY_DIFFERENCE_TO_SCORE_1 (ACT_INTENSITY_SCORE_1 - ACT_INTENSITY_JUST_DO_IT)
//** TODO: make these a part of agent's personalities (and add defaults) **
#define ACT_EXTRA_SCORE_CONTRIBUTION_MITIGATION_WEIGHTS (0.5f)
#define ACT_EXTRA_SCORE_MULTIPLIER (1.0f)
#define ACT_SUCESSIVE_MITIGATION_DAMPENNING_MULTIPLIER (0.8f) //will use this ^ rounds_done
#define ACT_MITIGATION_ROUNDS 2
#define ACT_GOAL_SHORTLIST_SIZE 4
#define ACT_CHOICE_SHORTLIST_SIZE 10
#define ACT_WHY_BOTHER_THRESOLD (0.2f)
#define ACT_JUST_DO_IT_THRESOLD (0.8f)

//Notion Calculation:
#define NOTION_UPKEEP_TO_BASE_INCOME_RATIO_TO_WORRY (0.8f)

//Actions (general):
#define PRNS_PER_ACT 5
#define MAX_ACT_PRNS_PER_AGENT (PRNS_PER_ACT * MAX_ACTIONS_PER_AGENT)
#define ACT_COST_MULTIPLIER_FROM_REFERENCE (0.5f)
#define ACT_REFERENCE_RESOURCES (100.0f)
#define BASE_ACT_COST (ACT_COST_MULTIPLIER_FROM_REFERENCE * ACT_REFERENCE_RESOURCES)
#define ACT_INTENSITY_COST_MULTIPLIER (5.0f)
#define ACT_SUPERLINEAR_WEIGHT (1.0f)
#define ACT_SUPERLINEAR_EXPO (2)

//Spefic Action details:
#define ACT_INTENS_ATTACK_MARGIN_PROPORTION (0.2f)
#define ACT_MAX_SUGESTION_INTENSITY (3.0f)
#define ACT_BASE_ATTACK_L_I_PREP_SECS_PER_DEFAULT_STR (10.0f)
#define ACT_ATTACK_G_S_SUGESTION_PREP_SECS_PER_INTENSITY (10.0f)

//Spefic Action details (calculated from the above):
#define ACT_BASE_ATTACK_L_I_PREP_TENTHS_OF_MS_PER_DEFAULT_STR (ACT_BASE_ATTACK_L_I_PREP_SECS_PER_DEFAULT_STR * TENTHS_OF_MS_IN_A_SECOND)
#define ACT_ATTACK_G_S_SUGESTION_PREP_TENTHS_OF_MS_PER_INTENSITY (ACT_ATTACK_G_S_SUGESTION_PREP_SECS_PER_INTENSITY * TENTHS_OF_MS_IN_A_SECOND)
//*************************************************************************

//--> DEFAULT values for new network creation:

#define DEFAULT_ONOFF (true)
#define DEFAULT_NUMBER_LAS 15
#define DEFAULT_NUMBER_GAS 5 //but last one doesn't count
#define DEFAULT_GA_RESOURCES (0.0f)
#define DEFAULT_GA_STANCE 2
#define DEFAULT_GA_DISPOSITION (0.0f)
#define DEFAULT_LA_STRENGHT (10.0f)
#define DEFAULT_LA_STR_THRESHOLD_FOR_UPKEEP (100.0f)
#define DEFAULT_LA_RESOURCES ACT_REFERENCE_RESOURCES
#define DEFAULT_LA_INCOME (0.5f)
#define DEFAULT_LA_STANCE 2
#define DEFAULT_LA_DISPOSITION (0.0f)
#define DEFAULT_LA_DISTANCE 10
#define DEFAULT_LAs_PER_LINE 3
#define DEFAULT_LA_NEIGHBOUR_QUOTIENT 2 //Default connections = max/this
#define DEFAULT_GA_INFILTRATION (0.0f)
#define DEFAULT_LA_INFILTRATION (0.0f)
#define DEFAULT_GA_PERSONA_0 (AS::GA_PERS_0)
#define DEFAULT_GA_PERSONA_1 (AS::GA_PERS_1)
#define DEFAULT_GA_PERSONA_2 (AS::GA_PERS_2)
#define DEFAULT_GA_PERSONA_3 (AS::GA_PERS_3)
#define DEFAULT_LA_OFFSET (-0.1f)
#define DEFAULT_REINFORCEMENT (0.1f)
#define DEFAULT_ACTION_ID 0
#define DEFAULT_PHASE_TOTAL 0
#define DEFAULT_INTENSITY 0
#define DEFAULT_ACTION_AUX 0
#define DEFAULT_REQUESTS (5.5f)
#define DEFAULT_SHOULD_DECIDE 1
#define DEFAULT_SYSTEM_WIDE_MAKE_DECISIONS (true)
#define DEFAULT_SYSTEM_WIDE_PROCESS_ACTIONS (true)

//--> TEST-related defines:
#define TST_NUMBER_LAS DEFAULT_NUMBER_LAS
#define TST_NUMBER_GAS DEFAULT_NUMBER_GAS
#define TST_GA_ID 2
#define TST_GA_CONNECTIONS 2
#define TST_LA_REINFORCEMENT (5.5)
#define TST_LA_OFFSET (-0.896)
#define TST_LAST_ACTION_AUX (-48513.0)
#define TST_COMMENT_LETTER_CHANGE ('X')
#define TST_CHANGED_CATEGORY 7
#define TST_CHANGED_MODE 2
#ifdef AS_DEBUG
	//TODO: SHOULD BE PERIOD
	#define TST_MAINLOOP_FREQUENCY_MS (AS_MILLISECONDS_PER_STEP)
#else
	//TODO: SHOULD BE PERIOD
	#define TST_MAINLOOP_FREQUENCY_MS (AS_MILLISECONDS_PER_STEP)
#endif// AS_DEBUG
#define TST_TA_QUERY_MULTIPLIER (1.75) //~250 / 144 (4 ms vs 144 fps), as a limiting case
#define TST_TA_QUERY_FREQUENCY_MS ((uint32_t)(TST_TA_QUERY_MULTIPLIER*TST_MAINLOOP_FREQUENCY_MS))
#define TST_TIMES_TO_QUERRY_TICK 25
#define TST_TICK_COUNT_SAFETY_FACTOR (50.0)
#define TST_RES_CHANGE (89134.4831)
#define TST_PRNS_TO_DRAW MAX_PRNS
#define TST_PRN_CHOPS 7

//TODO-CRITICAL: None of the following should be here : )
namespace AS {

	//TODO-CRITICAL: WARNING: REFACTOR: this will be part of a DATA project
	//Should then have copy/assignment constructor
	//FOR NOW: WARNING: FIX: any changes here have to be reflected on dataMisc, dataMirror,
	//clientDataHandler, and file format, creation and loading!
	typedef struct {
		bool isNetworkInitialized;
		uint64_t mainLoopTicks;
		uint64_t lastMainLoopStartingTick;
		double accumulatedMultiplier;
		std::chrono::microseconds lastStepTimeMicros;
		std::chrono::microseconds lastStepHotMicros;
		int numberLAs;
		int numberGAs;
		int maxLAneighbours;
		int maxActions;
		char name[NAME_LENGHT];
		char comment[COMMENT_LENGHT];
		uint64_t seeds[DRAW_WIDTH];
		bool makeDecisions;
		bool processActions;
	} AS_API networkParameters_t;
}

