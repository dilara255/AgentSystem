#pragma once

#include "data/agentDataControllers.hpp"

namespace AS {
	void transferData(int* CLtestArray_ptr);
	void initTstArray();
	bool testDataContainerCapacity(const dataControllerPointers_t* agentDataControllers_cptr);
}

namespace AS_TST {
	void updateReadTest(float* read, float real, float reference, float infiltration, 
							                 float prnFrom0to1, float timeMultiplier);
}

#define TST_ARRAY_SIZE 2