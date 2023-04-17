#pragma once

#include "systems/actionSystem.hpp"
#include "data/agentDataControllers.hpp"

namespace AS::Decisions{
	
	float calculateNotion(notionsSelf notion, scope scope, int agentID, 
		                                  AS::dataControllerPointers_t* dp);
	float calculateNotion(notionsNeighbor notion, scope scope, int agentID, 
		                                      AS::dataControllerPointers_t* dp);
}