#pragma once

/*
This file holds:
- Data structure to hold parameters of a network (networkParameters_t);
- Some enums and typedefs of agent data;

- Fixed parameters which ALL networks will have to follow;
- Defaults for agent and action data, used when creating new files and tests;
- Test-related defines.

//TO DO: this should be split in a couple to a few different files

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
#define NUMBER_LA_ACTIONS 18 //deprecated
#define DIPLOMATIC_STANCES 5
#define NUMBER_LA_OFFSETS (NUMBER_LA_ACTIONS + DIPLOMATIC_STANCES - 1)
#define MAX_LA_NEIGHBOURS 10
#define MAX_GA_QUANTITY 16 //NOTE: last one is reserved to "belong to no GA"
#define MAX_LA_QUANTITY 128
#define MAX_ACTIONS_PER_AGENT 10
#define GA_PERSONALITY_TRAITS 4
#define SLEEP_TIME_WAITING_MUTEX_MICROS 50
#define MAX_MUTEX_TRY_LOCKS 200

//DEFAULT values for new network creation:
#define DEFAULT_ONOFF (true)
#define DEFAULT_GA_RESOURCES 0
#define DEFAULT_GA_STANCE 2
#define DEFAULT_GA_DISPOSITION (0.0)
#define DEFAULT_LA_STRENGHT (10.0)
#define DEFAULT_LA_UPKEEP_PER_STRENGHT (0.5)
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
	#define TST_MAINLOOP_FREQUENCY_MS (100)
#else
	#define TST_MAINLOOP_FREQUENCY_MS 10
#endif// AS_DEBUG
#define TST_TA_QUERY_MULTIPLIER (1.5)
#define TST_TA_QUERY_FREQUENCY_MS ((unsigned)(TST_TA_QUERY_MULTIPLIER*TST_MAINLOOP_FREQUENCY_MS))
#define TST_TIMES_TO_QUERRY_TICK 10
#define TST_TICK_COUNT_SAFETY_FACTOR (10.0)
#define TST_RES_CHANGE (89134.4831)

namespace AS {

	typedef struct {
		bool isNetworkInitialized;
		uint64_t mainLoopTicks;
		uint64_t lastMainLoopStartingTick;
		int numberLAs;
		int numberGAs;
		int maxLAneighbours;
		int maxActions;
		char name[NAME_LENGHT];
		char comment[COMMENT_LENGHT];
	} AS_API networkParameters_t;

	enum gaPersonalityTraits {
		GA_PERS_0, GA_PERS_1, GA_PERS_2, GA_PERS_3,
		GA_PERS_4, GA_PERS_5, GA_PERS_6, GA_PERS_7,
		TOTAL_GA_PERS};
}

