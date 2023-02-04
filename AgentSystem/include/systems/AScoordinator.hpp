#pragma once

/*
Classes and functions declared on this file are responsible for the 
coordination of the AS.
This includes:
- Coordinating initialization and termination;
- Main loop; and
- Access/references to the different systems (right now through externs : p).
*/

#include "data/agentDataControllers.hpp"

namespace AS {
	extern const networkParameters_t* currentNetworkParams_cptr;
	extern const dataControllerPointers_t* agentDataControllers_cptr;
	extern const ActionDataController* actionDataController_cptr;

	bool initMainLoopControl(bool* shouldMainLoopBeRunning_ptr,
							 std::thread::id* mainLoopId_ptr,
							 std::thread* mainLoopThread_ptr,
							 ActionSystem* actionSystem_ptr, 
							 dataControllerPointers_t* agentDataControllerPtrs_ptr,
							 networkParameters_t* currentNetworkParams_ptr);
	bool sendReplacementDataToCL(bool silent);
	bool run();
	bool stop();
	void mainLoop();

	void drawPRNs(bool willAgentsMakeDecisions);
	void stepActions();
	void stepAgents(bool shouldMakeDecisions);
}