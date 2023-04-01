#pragma once

//NOTE: AS_internal can't contain actual functions, only export data definitions,
//since AS's dll is NOT included in CL

#include "../include/data/agentDataStructures.hpp"
#include "../include/data/actionData.hpp"
#include "../include/network/parameters.hpp"

namespace AS{
	FILE* acquireFilePointerToSave(std::string name, bool shouldOverwrite = false, 
		                                                std::string filePath = "");
	FILE* acquireFilePointerToLoad(std::string name, std::string filePath = "");
}
