#pragma once

#include "AS_internal.hpp"

namespace AS {
	AS_API int getGAsIDonNeighbor(int agent, int neighborID, 
		                      const GA::stateData_t* partnerState_ptr);

	AS_API int getLAsIDonNeighbor(int agent, int neighborID, 
		                      const LA::stateData_t* partnerState_ptr);

	AS_API bool copyNetworkParameters(networkParameters_t* destination,
		                            const networkParameters_t * source);

	AS_API bool defaultNetworkParameters(networkParameters_t* destination);

	/*
	typedef struct {
		bool* shouldMainLoopBeRunning_ptr;
		std::thread::id* mainLoopId_ptr;
		std::thread* mainLoopThread_ptr;

	    networkParameters_t* currentNetworkParams_ptr;

	    ActionSystem* actionSystem_ptr; 
		dataControllerPointers_t* agentDataControllerPtrs_ptr;
	    networkParameters_t** currentNetworkParams_ptr_ptr;

		const ActionSystem** actionSystem_cptr_ptr;
		const ActionDataController** actionDataController_cptr_ptr;
		const networkParameters_t** currentNetworkParams_cptr_ptr;
		const dataControllerPointers_t** agentDataControllers_cptr_ptr;
	} ASbaseControllersPtrs_t;
	*/
}
