#pragma once

#include "miscStdHeaders.h"
//TODO: Possibly add this to API, as a pack of helper funtionality

#include "miscDefines.hpp"

#include "data/agentDataControllers.hpp"

#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/actionSystem.hpp"

namespace AS {

	float actionCostFromIntensity(AS::actionData_t action);
	float nextActionsCost(int currentActions);
	int getQuantityOfCurrentActions(scope scope, int agentID, ActionSystem const * asp,
		                               AS::WarningsAndErrorsCounter* errorsCounter_ptr);
}