#pragma once

#include "systems/actionSystem.hpp"
#include "data/agentDataControllers.hpp"

#include "data/agentDataStructures.hpp"

#define BAD_AMBITION DEFAULT_AWFUL_SCORE
#define BAD_WORRY (- BAD_AMBITION)

namespace PURE_LA = LA;
namespace PURE_GA = GA;

namespace AS::Decisions{

	float calculateNotionBaseSelfLA(notionsSelf notion, int agentID, 
								  AS::dataControllerPointers_t* dp,
			                      PURE_LA::readsOnNeighbor_t* refReads_ptr);

	float calculateNotionBaseNeighborLA(notionsNeighbor notion, int neighbor,
								      int agentID, AS::dataControllerPointers_t* dp,
			                          PURE_LA::readsOnNeighbor_t* refReads_ptr);
	float calculateNotionBaseSelfGA(notionsSelf notion, int agentID, 
								  AS::dataControllerPointers_t* dp,
			                      PURE_GA::readsOnNeighbor_t* refReads_ptr);

	float calculateNotionBaseNeighborGA(notionsNeighbor notion, int neighbor,
								      int agentID, AS::dataControllerPointers_t* dp,
			                          PURE_GA::readsOnNeighbor_t* refReads_ptr);

	//Maps baseNotion == 0 to 0 and baseNotion >= effectiveMaxBase to 1.
	//The smallExponent makes this mapping non-linear, emphasizing high values. 
	//Has parameter-based defaults for effectiveMaxBase and smallExponent.
	// 
	//NOTE: baseNotion is *assumed* to be >= 0;
	//NOTE: effectiveMaxBase is *assumed* to be > 0;
	//NOTE: If smallExponent == 0, all values will be cast to 1;
	// 
	//WARNING: Large *small*Exponents may break things : )
	float delinearizeAndClampNotion(float baseNotion, 
								float effectiveMaxBase = NTN_STD_MAX_EFFECTIVE_NOTION_BASE, 
								float smallExponent = NTN_STD_DELINEARIZATION_EXPONENT);
}