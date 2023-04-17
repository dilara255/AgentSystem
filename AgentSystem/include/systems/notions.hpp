#pragma once

#include "systems/actionSystem.hpp"
#include "data/agentDataControllers.hpp"

namespace AS::Decisions{
	
	float calculateNotionSelf(notionsSelf notion, scope scope, int agentID, 
		                                  AS::dataControllerPointers_t* dp);
	float calculateNotionNeighbor(notionsNeighbor notion, scope scope, int neighbor,
		                              int agentID, AS::dataControllerPointers_t* dp);
}