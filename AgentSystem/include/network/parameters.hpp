#pragma once

/*
This file holds:
- Data structure to hold parameters of a network (networkParameters_t);
- Some enums and typedefs of agent data;

- Fixed parameters which ALL networks will have to follow;
- Defaults for agent and action data, used when creating new files and tests;
- Test-related defines.

//TODO: this should be split in a couple to a few different files

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

//FIXED parameters:
#define NAME_LENGHT 30
#define COMMENT_LENGHT 250
#define DIPLOMATIC_STANCES 5 //TODO: this would be better left to the enum class diploStances
#define MAX_LA_NEIGHBOURS 10
#define MAX_GA_QUANTITY 16 //NOTE: last one is reserved to "belongs to no GA"
#define MAX_LA_QUANTITY 128
#define MAX_AGENTS (MAX_GA_QUANTITY + MAX_LA_QUANTITY)
#define MAX_ACTIONS_PER_AGENT 10
#define NUMBER_LA_OFFSETS (MAX_ACTIONS_PER_AGENT + (DIPLOMATIC_STANCES - 1))
#define GA_PERSONALITY_TRAITS 4
#define SLEEP_TIME_WAITING_MUTEX_MICROS 10
#define MAX_MUTEX_TRY_LOCKS 500 //will try at least twice anyway
#define AS_MILLISECONDS_PER_STEP (4)
#define AS_MILLISECONDS_PER_DECISION_ROUND (40) //wil be floored to multiple of step time
#define AS_TOTAL_CHOPS (AS_MILLISECONDS_PER_DECISION_ROUND/AS_MILLISECONDS_PER_STEP)
#define PRNS_PER_ACT 5
#define LA_FIELDS_TO_DEDUCE_EXPECTED 10
#define GA_FIELDS_TO_DEDUCE_EXPECTED 10
#define PRNS_PER_FIELD_DEDUCED 2
#define PRNS_TO_CHOOSE_ACTION 1
#define DRAW_WIDTH 4
#define EXTERNAL_GUARD_UPKEEP_RATIO_BY_DEFENDED (0.5)
#define EXTERNAL_GUARD_UPKEEP_RATIO_BY_DEFENDER (1-EXTERNAL_GUARD_UPKEEP_RATIO_BY_DEFENDED)
#define TRADE_SATURATION_MULTIPLIER (1.0)
#define TRADE_SATURATION_FROM_WAR 3
#define TRADE_SATURATION_FROM_NEUTRAL 0
#define TRADE_SATURATION_FROM_TRADE 1
#define TRADE_SATURATION_FROM_ALLY 1
#define TRADE_SATURATION_FROM_ALLY_WITH_TRADE 2
#define LA_UPKEEP_PER_EXCESS_STRENGHT (0.5)
#define MAX_PROPORTIONAL_STEP_DURATION_ERROR (5) 
#define TRADE_FACTOR_PER_SECOND (0.5)
#define ATTRITION_FACTOR_PER_SECOND (0.001)
#define GA_TAX_RATE_PER_SECOND (0.001)

//DEFAULT values for new network creation:
#define DEFAULT_ONOFF (true)
#define DEFAULT_GA_RESOURCES (0.0)
#define DEFAULT_GA_STANCE 2
#define DEFAULT_GA_DISPOSITION (0.0)
#define DEFAULT_LA_STRENGHT (10.0)
#define DEFAULT_LA_STR_THRESHOLD_FOR_UPKEEP (100.0)
#define DEFAULT_LA_RESOURCES (100.0)
#define DEFAULT_LA_INCOME (0.5)
#define DEFAULT_LA_STANCE 2
#define DEFAULT_LA_DISPOSITION (0.0)
#define DEFAULT_LA_DISTANCE 10
#define DEFAULT_LAs_PER_LINE 3
#define DEFAULT_LA_NEIGHBOUR_QUOTIENT 2 //Default connections = max/this
#define DEFAULT_GA_INFILTRATION (0.0)
#define DEFAULT_LA_INFILTRATION (0.0)
#define DEFAULT_GA_PERSONA_0 (AS::GA_PERS_0)
#define DEFAULT_GA_PERSONA_1 (AS::GA_PERS_1)
#define DEFAULT_GA_PERSONA_2 (AS::GA_PERS_2)
#define DEFAULT_GA_PERSONA_3 (AS::GA_PERS_3)
#define DEFAULT_LA_OFFSET (-0.1)
#define DEFAULT_REINFORCEMENT (0.1)
#define DEFAULT_ACTION_ID 0
#define DEFAULT_FIRST_TICK 853
#define DEFAULT_LAST_TICK 1
#define DEFAULT_INTENSITY (-3.0)
#define DEFAULT_ACTION_AUX (99.0)
#define DEFAULT_PRNG_SEED0 0x2545f4914f6cdd1d
#define DEFAULT_PRNG_SEED1 (0x2545f4914f6cdd1d + 1595721457336359713)
#define DEFAULT_PRNG_SEED2 (0x2545f4914f6cdd1d*3)
#define DEFAULT_PRNG_SEED3 1595721457336359713

//TEST-related defines:
#define TST_NUMBER_LAS 15
#define TST_NUMBER_GAS 5
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
#define TST_TA_QUERY_FREQUENCY_MS ((unsigned)(TST_TA_QUERY_MULTIPLIER*TST_MAINLOOP_FREQUENCY_MS))
#define TST_TIMES_TO_QUERRY_TICK 25
#define TST_TICK_COUNT_SAFETY_FACTOR (50.0)
#define TST_RES_CHANGE (89134.4831)

namespace AS {

	typedef struct {
		bool isNetworkInitialized;
		uint64_t mainLoopTicks;
		uint64_t lastMainLoopStartingTick;
		std::chrono::microseconds lastStepTimeMicros;
		int numberLAs;
		int numberGAs;
		int maxLAneighbours;
		int maxActions;
		char name[NAME_LENGHT];
		char comment[COMMENT_LENGHT];
		uint64_t seeds[DRAW_WIDTH];
	} AS_API networkParameters_t;

	enum gaPersonalityTraits {
		GA_PERS_0, GA_PERS_1, GA_PERS_2, GA_PERS_3,
		GA_PERS_4, GA_PERS_5, GA_PERS_6, GA_PERS_7,
		TOTAL_GA_PERS};
}

