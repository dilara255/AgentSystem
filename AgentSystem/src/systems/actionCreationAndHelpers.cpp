#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "data/agentDataControllers.hpp"

#include "systems/actionSystem.hpp"
#include "systems/warningsAndErrorsCounter.hpp"

namespace AS{
	//TODO: Test
	//TODO: Make this into a method on ActionDataController
	int getQuantityOfCurrentActions(scope scope, int agentID, ActionSystem const * asp,
		                               AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

		auto actionController_cptr = asp->getDataDirectConstPointer();

		const std::vector<AS::actionData_t> * actionDataVec_cptr = NULL;

		if (scope == scope::LOCAL) {
			actionDataVec_cptr = actionController_cptr->getActionsLAsCptr();
		}
		else if (scope == scope::GLOBAL) {
			actionDataVec_cptr = actionController_cptr->getActionsGAsCptr();
		}
		if (actionDataVec_cptr == NULL) {
			if (errorsCounter_ptr == NULL) {
				LOG_ERROR("Couldn't get action data constant pointer (nor find the error counter)");
			}
			else {
				errorsCounter_ptr->incrementError(errors::AC_COULDNT_GET_ACTIONS_CPTR);
			}
			
			return NATURAL_RETURN_ERROR;
		}
		
		int startingIndexOnActionsVector = agentID * MAX_ACTIONS_PER_AGENT;
		int startingIndexNextAgent = (agentID + 1) * MAX_ACTIONS_PER_AGENT;

		int currentActions = 0;
		for(int i = startingIndexOnActionsVector; i < startingIndexNextAgent; i++){
		
			currentActions += actionDataVec_cptr->at(i).ids.slotIsUsed;
		}

		return currentActions;
	}

	//TODO: document math
	float nextActionsCost(int currentActions) {

		float multiplier = currentActions
			 + ACT_SUPERLINEAR_WEIGHT * powf( (float)(currentActions - 1), (float)ACT_SUPERLINEAR_EXPO);
		
		return multiplier * BASE_ACT_COST;
	}

	//TODO: review scaling (ie: size of agent should matter too?)
	//TODO: should take category in account too? Mode?
	float actionCostFromIntensity(AS::actionData_t action) {

		return ACT_INTENSITY_COST_MULTIPLIER * action.details.intensity;
	}

	bool spawnAction(actionData_t action, ActionSystem* actionSystem_ptr, uint32_t tick) {		

		return actionSystem_ptr->getDataDirectPointer()->addActionData(action);
	}

	void chargeForAndSpawnAction(actionData_t action, AS::dataControllerPointers_t* dp,
							             ActionSystem* actionSystem_ptr, uint32_t tick, 
										   WarningsAndErrorsCounter* errorsCounter_ptr) {
		
		bool spawned = spawnAction(action, actionSystem_ptr, tick);

		if (spawned) {
			AS::scope scope = (AS::scope)action.ids.scope;
			int agentID = action.ids.origin;

			int currentActions = 
				getQuantityOfCurrentActions(scope, agentID, actionSystem_ptr, 
					                                       errorsCounter_ptr);
			
			float cost = nextActionsCost(currentActions - 1) + actionCostFromIntensity(action);
			
			float* currency_ptr = NULL;
			if(scope == AS::scope::LOCAL){
				auto agent_ptr = &(dp->LAstate_ptr->getDirectDataPtr()->at(agentID));
				currency_ptr = &(agent_ptr->parameters.resources.current);
			}
			else if(scope == AS::scope::GLOBAL){
				auto agent_ptr = &(dp->GAstate_ptr->getDirectDataPtr()->at(agentID));
				currency_ptr = &(agent_ptr->parameters.GAresources);
			}
			assert(currency_ptr != NULL);
			
			*currency_ptr -= cost;
		}
	}
}