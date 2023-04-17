#pragma once

#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/actionSystem.hpp"

namespace AS {

	float nextActionsCost(int currentActions);
	int getQuantityOfCurrentActions(scope scope, int agentID, ActionSystem const * asp,
		                            AS::WarningsAndErrorsCounter* errorsCounter_ptr);

}