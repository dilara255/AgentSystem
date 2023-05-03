#pragma once

#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/actionSystem.hpp"

namespace AS {

	//Sets neighbor ids.target and details (based on desired intensity from score)
	void setActionDetails(float score, float whyBother, float JustDoIt, 
		                  AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
							                 WarningsAndErrorsCounter* errorsCounter_ptr);

	void chargeForAndSpawnAction(actionData_t action, AS::dataControllerPointers_t* dp,
								         ActionSystem* actionSystem_ptr, uint32_t tick,
							               WarningsAndErrorsCounter* errorsCounter_ptr);	
	
	//Sets initial and last tick, and phase = 0, and tries to add the action (true if so)
	bool spawnAction(actionData_t action, ActionSystem* actionSystem_ptr, uint32_t tick);

	//TODO: Split these into another file so I can use it to check stuff
	//TODO: Possibly add to API as well, as a pack of helper funtionality
	float actionCostFromIntensity(AS::actionData_t action);
	float nextActionsCost(int currentActions);
	int getQuantityOfCurrentActions(scope scope, int agentID, ActionSystem const * asp,
		                               AS::WarningsAndErrorsCounter* errorsCounter_ptr);
}