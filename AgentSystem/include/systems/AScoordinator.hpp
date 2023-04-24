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
#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/PRNserver.hpp"

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
	void mainLoop(bool fixedTimeStep = false);

	void stepActions(ActionSystem* actionSystem_ptr, 
		             int numberLAs, int numberGAs, float timeMultiplier,
		             WarningsAndErrorsCounter* errorsCounter_ptr, AS::PRNserver* prnServer_ptr);
	
	void stepAgents(int LAdecisionsToTakeThisChop, int GAdecisionsToTakeThisChop,
		                         dataControllerPointers_t* agentDataPointers_ptr,
		                    ActionSystem* actionSystem_ptr, float timeMultiplier, 
										   int numberLAs, int numberEffectiveGAs,
		                             WarningsAndErrorsCounter* errorsCounter_ptr, 
							    bool makeDecisions, AS::PRNserver* prnServer_ptr,
		                       float secondsSinceLastDecisionStep, uint32_t tick);

	struct chopControl_st {
		int chopIndex = 0;
		int totalChops = AS_TOTAL_CHOPS;

		int totalPRNsNeeded = 0;

		int LAdecisionsToMake = 0;
		int GAdecisionsToMake = 0;
		int LAdecisionsMade = 0;
		int GAdecisionsMade = 0;

		int quantityLAs = 0;
		int quantityEffectiveGAs = 0;
	};
}