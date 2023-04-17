#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/actionSystem.hpp"

namespace AS{
	
	//TODO: test
	int getQuantityOfCurrentActions(scope scope, int agentID, ActionSystem const * asp,
		                               AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

		auto actionController_cptr = asp->getDataDirectPointer();

		const std::vector<AS::actionData_t> * actionDataVec_cptr = NULL;

		if (scope == scope::LOCAL) {
			actionDataVec_cptr = actionController_cptr->getActionsLAsCptr();
		}
		else if (scope == scope::GLOBAL) {
			actionDataVec_cptr = actionController_cptr->getActionsGAsCptr();
		}
		if (actionDataVec_cptr == NULL) {
			if (errorsCounter_ptr == NULL) {
				LOG_ERROR("Couldn't get action data constant pointer");
			}
			else {
				errorsCounter_ptr->incrementError(errors::AS_COULDNT_GET_ACTIONS_CPTR);
			}

			return NATURAL_RETURN_ERROR;
		}
		
		int startingIndexOnActionsVector = agentID * MAX_ACTIONS_PER_AGENT;
		int startingIndexNextAgent = (agentID + 1) * MAX_ACTIONS_PER_AGENT;

		int currentActions = 0;
		for(int i = startingIndexOnActionsVector; i < startingIndexNextAgent; i++){
		
			currentActions += actionDataVec_cptr->at(i).ids.active;
		}

		return currentActions;
	}

	float nextActionsCost(int currentActions) {

		float multiplier = currentActions
			 + ACT_SUPERLINEAR_WEIGHT * powf( (float)(currentActions - 1), (float)ACT_SUPERLINEAR_EXPO);
		
		return multiplier * BASE_ACT_COST;
	}
}