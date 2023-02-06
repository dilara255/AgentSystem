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
#include "systems/prnsServer.hpp"

namespace AS {
	extern const networkParameters_t* currentNetworkParams_cptr;
	extern const dataControllerPointers_t* agentDataControllers_cptr;
	extern const ActionDataController* actionDataController_cptr;

	bool initMainLoopControl(bool* shouldMainLoopBeRunning_ptr,
							 std::thread::id* mainLoopId_ptr,
							 std::thread* mainLoopThread_ptr,
							 ActionSystem* actionSystem_ptr, 
							 dataControllerPointers_t* agentDataControllerPtrs_ptr,
							 networkParameters_t* currentNetworkParams_ptr,
	                         AS::PRNserver* prnServer_ptr);
	bool sendReplacementDataToCL(bool silent);
	bool run();
	bool stop();
	void mainLoop();

	class PRNserver;
	void stepActions(ActionSystem* actionSystem_ptr, 
		             int numberLAs, int numberGAs, float timeMultiplier);
	void stepAgents(int LAdecisionsToTakeThisChop, int GAdecisionsToTakeThisChop,
		                         dataControllerPointers_t* agentDataPointers_ptr,
		                      float timeMultiplier, int numberLAs, int numberGAs);

	struct chopControl_st {
		int chopIndex = 0;

		int totalPRNsNeeded = 0;
		int PRNsToDrawThisChop = 0;

		int LAdecisionsToMake = 0;
		int GAdecisionsToMake = 0;

		int numberLAs = 0;
		int numberGAs = 0;
	};
}