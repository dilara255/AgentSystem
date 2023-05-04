//TODO: Some of these functions should be methods on ActionDataController
//TODO: Possibly add the other ones to API, as a pack of helper funtionality (on this file)
#pragma once

#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "data/agentDataControllers.hpp"

#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/actionSystem.hpp"

namespace AS {

	inline const std::vector<AS::actionData_t>* getScopeIndependentActionDataCptr(
							            const ActionSystem* asp, AS::scope scope, int agent, 
							                AS::WarningsAndErrorsCounter* errorsCounter_ptr);

	//Populates activeActions_ptr with the agents active actions (from the start).
	//Returns total active actions, or NATURAL_RETURN_ERROR (-1) on error.
	//NOTE: ignores BOTH action slots wich are marked as occupied but innactive
	//AND slots marked as not occupied (regardless of active/innactive).
	void populateAgentsActiveActions(const ActionSystem* asp, AS::scope scope, int agent,
		                              AS::Decisions::agentsActions_t* activeActions_ptr,
		                                AS::WarningsAndErrorsCounter* errorsCounter_ptr);

	float actionCostFromIntensity(AS::actionData_t action);
	float nextActionsCost(int currentActions);
	int getQuantityOfCurrentActions(scope scope, int agentID, ActionSystem const * asp,
		                               AS::WarningsAndErrorsCounter* errorsCounter_ptr);
}