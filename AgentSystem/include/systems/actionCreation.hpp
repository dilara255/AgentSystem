#pragma once

#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "data/agentDataControllers.hpp"

#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/actionSystem.hpp"

namespace AS {

	//Sets neighbor ids.target and details (based on desired intensity from score)
	void setChoiceDetails(float score, float whyBother, float JustDoIt, 
		                  AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
							                 WarningsAndErrorsCounter* errorsCounter_ptr);

	void chargeForAndSpawnAction(actionData_t action, AS::dataControllerPointers_t* dp,
														ActionSystem* actionSystem_ptr, 
									       WarningsAndErrorsCounter* errorsCounter_ptr);	
	
	//Sets initial and last tick, and phase = 0, and tries to add the action (true if so)
	bool spawnAction(actionData_t action, ActionSystem* actionSystem_ptr, uint32_t tick);
}