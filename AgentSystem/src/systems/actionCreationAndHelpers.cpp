#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "data/agentDataControllers.hpp"

#include "systems/actionSystem.hpp"
#include "systems/warningsAndErrorsCounter.hpp"

namespace AS{

	//TODO: test
	//TODO: Make this into a method on ActionDataController
	inline const std::vector<AS::actionData_t>* getScopeIndependentActionDataCptr(
							            const ActionSystem* asp, AS::scope scope, int agent, 
							                AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

		//All entry points to the functions should have checked their pointers:
		assert(errorsCounter_ptr != NULL); 
		assert(asp != NULL); 

		auto actionController_cptr = asp->getDataDirectConstPointer();

		const std::vector<AS::actionData_t> * actionDataVec_cptr = NULL;

		if (scope == scope::LOCAL) {
			actionDataVec_cptr = actionController_cptr->getActionsLAsCptr();
		}
		else if (scope == scope::GLOBAL) {
			actionDataVec_cptr = actionController_cptr->getActionsGAsCptr();
		}
		
		return actionDataVec_cptr;
	}

	//TODO: test
	//TODO: Make this into a method on ActionDataController
	int populateAgentsActiveActions(const ActionSystem* asp, AS::scope scope, int agent,
		                              AS::Decisions::agentsActions_t* activeActions_ptr,
		                                AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

		if (errorsCounter_ptr == NULL) {
			LOG_CRITICAL("populateAgentsActiveActions received bad errorsCounter_ptr");
			return NATURAL_RETURN_ERROR;
		}

		if (asp == NULL) {
			errorsCounter_ptr->incrementError(AS::errors::AC_COULDNT_GET_ACTIONS_CPTR);
			return NATURAL_RETURN_ERROR;
		}

		auto actionController_cptr = asp->getDataDirectConstPointer();

		const std::vector<AS::actionData_t> * actionDataVec_cptr = 
				getScopeIndependentActionDataCptr(asp, scope, agent, errorsCounter_ptr);

		if (actionDataVec_cptr == NULL) {
			errorsCounter_ptr->incrementError(errors::AC_COULDNT_GET_ACTIONS_CPTR);
			return NATURAL_RETURN_ERROR;
		}

		int startingIndexOnActionsVector = agent * MAX_ACTIONS_PER_AGENT;
		int startingIndexNextAgent = (agent + 1) * MAX_ACTIONS_PER_AGENT;

		int activeActionsFound = 0;
		for(int i = startingIndexOnActionsVector; i < startingIndexNextAgent; i++){

			auto action_ptr = &(actionDataVec_cptr->at(i));

			if (action_ptr->ids.slotIsUsed && action_ptr->ids.active) {

				assert(activeActionsFound <= activeActions_ptr->totalElements);

				activeActions_ptr->actions[activeActionsFound].actCategory = 
															action_ptr->ids.category;
				activeActions_ptr->actions[activeActionsFound].actMode = 
															action_ptr->ids.mode;
				activeActions_ptr->actions[activeActionsFound].neighbor = 
															action_ptr->ids.target;

				activeActionsFound++;
			}		
		}

		return activeActionsFound;
	}

	//TODO: Test
	//TODO: Make this into a method on ActionDataController
	int getQuantityOfCurrentActions(scope scope, int agentID, ActionSystem const * asp,
		                               AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

		if (errorsCounter_ptr == NULL) {
			LOG_CRITICAL("populateAgentsActiveActions received bad errorsCounter_ptr");
			return NATURAL_RETURN_ERROR;
		}

		auto actionController_cptr = asp->getDataDirectConstPointer();

		const std::vector<AS::actionData_t> * actionDataVec_cptr = 
				getScopeIndependentActionDataCptr(asp, scope, agentID, errorsCounter_ptr);

		if (actionDataVec_cptr == NULL) {
			errorsCounter_ptr->incrementError(errors::AC_COULDNT_GET_ACTIONS_CPTR);
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