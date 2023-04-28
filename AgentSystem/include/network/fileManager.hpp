#pragma once

#include "miscStdHeaders.h"
#include "data/agentDataControllers.hpp"
#include "network/parameters.hpp"
#include "AS_internal.hpp" //TODO: remove and fix on other files

namespace AS{
    int createEmptyNetworkFile(std::string fileName, std::string comment, int numberLAs,
                               int numberGAs, int maxNeighbors, int maxActions,
                               bool setDefaults, std::string filePath = "");

    bool createNetworkFileFromData(FILE* fp,
                        const AS::dataControllerPointers_t* agentDataControllers_cptr,
                        const AS::networkParameters_t* currentNetworkParams_cptr,
                        const AS::ActionDataController* actionDataController_cptr);

    bool loadNetworkFromFileToDataControllers(FILE* fp, 
                                           dataControllerPointers_t agentDataControllers,
                                           networkParameters_t* currentNetworkParams_ptr,
                                           ActionDataController* actionDataController_ptr, 
	                                       bool disableDecisions = false, bool blockActions = false);

    bool fileIsCompatible(FILE* fp);
}
