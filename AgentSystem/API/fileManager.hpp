#pragma once

#include "miscStdHeaders.h"
#include "data/agentDataControllers.hpp"
#include "network/parameters.hpp"

namespace AS{
    int createEmptyNetworkFile(std::string fileName, std::string comment, int numberLAs,
                               int numberGAs, int maxNeighbors, int maxActions,
                               bool setDefaults);

    bool loadNetworkFromFileToDataControllers(FILE* fp, 
                                           dataControllerPointers_t agentDataControllers,
                                           networkParameters_t currentNetworkParams);

    bool fileIsCompatible(FILE* fp);

    FILE* acquireFilePointerToLoad(std::string name);
}
