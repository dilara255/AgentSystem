#pragma once

/*
This file holds:
- Parameters from the current, active network (if any);
- Fixed parameters which ALL networks will have to follow.

Parameters from the current network include: (TO DO)
- number of local and global agents;
- a "network name", which can be used with the load/save system;
- an optional "comment" (which can also be used with the load/save system);
- the actual maximum neighbours per LA;
- the actual maximum actions per agent.

The fixed parameters represent a "kind" of network. These are essentially 
constraints for any network to be loaded. They help make things predictable 
and sizes static.
*/

#include "flagFields.hpp"

#define NAME_LENGHT 30
#define COMMENT_LENGHT 250
#define NUMBER_LA_ACTIONS 18
#define DIPLOMATIC_STANCES 5
#define NUMBER_LA_OFFSETS (NUMBER_LA_ACTIONS + DIPLOMATIC_STANCES - 1)
#define MAX_LA_NEIGHBOURS 10
#define MAX_GA_QUANTITY 16 //NOTE: last one is reserved to "belong to no GA"
#define MAX_LA_QUANTITY 128
#define MAX_ACTIONS_PER_AGENT 10

//Default values for new network creation:
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

namespace AS {
	typedef AZ::FlagField128 LAflagField_t;
	typedef AZ::FlagField32 GAflagField_t;

	typedef struct {
		bool isNetworkInitialized;
		int numberLAs;
		int numberGAs;
		int maxLAneighbours;
		int maxActions;
		char name[NAME_LENGHT];
		char comment[COMMENT_LENGHT];
	} networkParameters_t;
}

extern AS::networkParameters_t currentNetworkParams;

enum gaPersonalityTraits {/*add some*/ };