#pragma once

#include "systems/actionSystem.hpp"
#include "data/agentDataControllers.hpp"

#include "data/agentDataStructures.hpp"

#define BAD_AMBITION DEFAULT_AWFUL_SCORE
#define BAD_WORRY (- BAD_AMBITION)

namespace PURE_LA = LA;
namespace PURE_GA = GA;

namespace AS::Decisions{

	float calculateNotionSelfLA(notionsSelf notion, int agentID, 
								  AS::dataControllerPointers_t* dp,
			                      PURE_LA::readsOnNeighbor_t* refReads_ptr);

	float calculateNotionNeighborLA(notionsNeighbor notion, int neighbor,
								      int agentID, AS::dataControllerPointers_t* dp,
			                          PURE_LA::readsOnNeighbor_t* refReads_ptr);
	float calculateNotionSelfGA(notionsSelf notion, int agentID, 
								  AS::dataControllerPointers_t* dp,
			                      PURE_GA::readsOnNeighbor_t* refReads_ptr);

	float calculateNotionNeighborGA(notionsNeighbor notion, int neighbor,
								      int agentID, AS::dataControllerPointers_t* dp,
			                          PURE_GA::readsOnNeighbor_t* refReads_ptr);
}