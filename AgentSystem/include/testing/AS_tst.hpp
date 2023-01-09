#pragma once

#include "data/agentDataControllers.hpp"

namespace AS {
	void transferData(int* CLtestArray_ptr);
	void initTstArray();
	void testDataContainerCapacity();
}

#define TST_ARRAY_SIZE 2
#define TST_NUMBER_LAS 15
#define TST_NUMBER_GAS 5