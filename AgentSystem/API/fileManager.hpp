#pragma once

#include "miscStdHeaders.h"

int createEmptyNetworkFile(std::string name, std::string comment, int numberLAs,
    int numberGAs, int maxNeighbors, int maxActions,
    bool setDefaults);