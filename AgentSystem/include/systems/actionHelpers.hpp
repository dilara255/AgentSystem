#pragma once

#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/actionSystem.hpp"

namespace AS {

	//Sets neighbor ids.target and details (based on desired intensity from score)
	void setActionDetails(float score, float whyBother, float JustDoIt, 
		                  AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp);

	void chargeForAndSpawnAction(actionData_t action, AS::dataControllerPointers_t* dp,
							               WarningsAndErrorsCounter* errorsCounter_ptr);	
	bool spawnAction(actionData_t action);

	float nextActionsCost(int currentActions);
	int getQuantityOfCurrentActions(scope scope, int agentID, ActionSystem const * asp,
		                            AS::WarningsAndErrorsCounter* errorsCounter_ptr);
}