#pragma once

#include "data/agentDataControllers.hpp"

namespace AS {
	void transferData(int* CLtestArray_ptr);
	void initTstArray();
	bool testDataContainerCapacity(const dataControllerPointers_t* agentDataControllers_cptr);
}

#define TST_ARRAY_SIZE 2
#define TST_NUMBER_LAS 15
#define TST_NUMBER_GAS 5