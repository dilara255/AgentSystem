#pragma once


//
#include "../include/data/agentDataStructures.hpp"
#include "../include/data/actionData.hpp"
#include "../include/network/parameters.hpp"

//TODO: THESE SHOULD BE RENAMED AND MOVED
namespace AS{
	FILE* acquireFilePointerToSave(std::string name, bool shouldOverwrite = false, 
		                                                std::string filePath = "");
	FILE* acquireFilePointerToLoad(std::string name, std::string filePath = "");
}
