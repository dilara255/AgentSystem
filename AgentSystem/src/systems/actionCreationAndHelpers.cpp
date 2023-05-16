#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "data/agentDataControllers.hpp"
#include "data/dataMisc.hpp"

#include "systems/actionSystem.hpp"
#include "systems/warningsAndErrorsCounter.hpp"

namespace AS{

	void incrementTroopsOnAttack(float* onAttacks_ptr, float intensity) {
		*onAttacks_ptr += std::round(intensity);
	}

	void decrementTroopsOnAttack(float* onAttacks_ptr, float originalIntensity) {
		*onAttacks_ptr -= std::round(originalIntensity);

		assert( (*onAttacks_ptr) >= 0 );
	}

	float getMaxDebt(float currentBaseIncome) {
		float effectiveIncome = std::max(currentBaseIncome, MINIMUM_REF_INCOME);
		return (-currentBaseIncome)*MAX_DEBT_TO_INCOME_RATIO;
	}

	bool isActionValid(const actionData_t* action_ptr) {
		return (action_ptr->ids.active && action_ptr->ids.slotIsUsed);
	}

	void invalidateAction(actionData_t* action_ptr) {
		action_ptr->ids.active = 0;
		action_ptr->ids.slotIsUsed = 0;
		return;
	}

	//TODO: test
	//TODO: Make this into a method on ActionDataController
	const std::vector<AS::actionData_t>* getScopeIndependentActionDataCptr(
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
	void populateAgentsActiveActions(const ActionSystem* asp, AS::scope scope, int agent,
		                               AS::Decisions::agentsActions_t* activeActions_ptr, 
		                                 AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

		if (errorsCounter_ptr == NULL) {
			LOG_CRITICAL("populateAgentsActiveActions received bad errorsCounter_ptr");
			return;
		}

		if (asp == NULL) {
			errorsCounter_ptr->incrementError(AS::errors::AC_COULDNT_GET_ACTIONS_CPTR);
			return;
		}

		auto actionController_cptr = asp->getDataDirectConstPointer();

		const std::vector<AS::actionData_t> * actionDataVec_cptr = 
				getScopeIndependentActionDataCptr(asp, scope, agent, errorsCounter_ptr);

		if (actionDataVec_cptr == NULL) {
			errorsCounter_ptr->incrementError(errors::AC_COULDNT_GET_ACTIONS_CPTR);
			return;
		}

		int maxActionsPerAgent = asp->getMaxActionsPerAgent();

		int startingIndexOnActionsVector = agent * maxActionsPerAgent;
		int startingIndexNextAgent = (agent + 1) * maxActionsPerAgent;

		int activeActionsFound = 0;
		for (int i = startingIndexOnActionsVector; i < startingIndexNextAgent; i++) {

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

		activeActions_ptr->totalActiveActions = activeActionsFound;
		return;
	}

	//TODO: Test
	//TODO: Make this into a method on ActionDataController
	int getQuantityOfCurrentActions(scope scope, int agentID, ActionSystem const* asp,
		                              AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

		if (errorsCounter_ptr == NULL) {
			LOG_CRITICAL("populateAgentsActiveActions received bad errorsCounter_ptr");
			return NATURAL_RETURN_ERROR;
		}

		auto actionController_cptr = asp->getDataDirectConstPointer();

		const std::vector<AS::actionData_t>* actionDataVec_cptr =
			getScopeIndependentActionDataCptr(asp, scope, agentID, errorsCounter_ptr);

		if (actionDataVec_cptr == NULL) {
			errorsCounter_ptr->incrementError(errors::AC_COULDNT_GET_ACTIONS_CPTR);
			return NATURAL_RETURN_ERROR;
		}

		int maxActionsPerAgent = asp->getMaxActionsPerAgent();

		int startingIndexOnActionsVector = agentID * maxActionsPerAgent;
		int startingIndexNextAgent = (agentID + 1) * maxActionsPerAgent;

		int currentActions = 0;
		for (int i = startingIndexOnActionsVector; i < startingIndexNextAgent; i++) {

			currentActions += actionDataVec_cptr->at(i).ids.slotIsUsed;
		}

		return currentActions;
	}

	bool spawnAction(actionData_t action, ActionSystem* actionSystem_ptr) {

		return actionSystem_ptr->getDataDirectPointer()->addActionData(action);
	}

	void chargeForAndSpawnAction(actionData_t action, AS::dataControllerPointers_t* dp,
										                ActionSystem* actionSystem_ptr,
										   WarningsAndErrorsCounter* errorsCounter_ptr) {

		assert(action.ids.phase == (int)AS::actPhases::SPAWN);

		AS::scope scope = (AS::scope)action.ids.scope;
		int agentID = action.ids.origin;

		int currentActions =
			getQuantityOfCurrentActions(scope, agentID, actionSystem_ptr,
				                                       errorsCounter_ptr);

		//The cost is composed of a base cost from the amount current actions plus
		//any costs related to funding (stored at processingAux when details are set):
		float cost = nextActionsCost(currentActions) + action.details.processingAux;

		float* currency_ptr = NULL;
		if (scope == AS::scope::LOCAL) {
			auto agent_ptr = &(dp->LAstate_ptr->getDirectDataPtr()->at(agentID));
			currency_ptr = &(agent_ptr->parameters.resources.current);
		}
		else if (scope == AS::scope::GLOBAL) {
			auto agent_ptr = &(dp->GAstate_ptr->getDirectDataPtr()->at(agentID));
			currency_ptr = &(agent_ptr->parameters.GAresources);
		}
		assert(currency_ptr != NULL);

		//The agent can pay any eventual funding as the action runs, but we'll pay as much
		//as possible now:
		if ((*currency_ptr) >= cost) {
			*currency_ptr -= cost;
			cost = 0;			
		}
		else if ((*currency_ptr) > 0)  {
			cost -= (*currency_ptr);
			*currency_ptr = 0;
		}		

		//Any pending funding will be stored back in the aux:
		action.details.processingAux = cost;

		//finally, we try to spawn the action:
		if (!spawnAction(action, actionSystem_ptr)) {
			errorsCounter_ptr->incrementWarning(AS::warnings::DS_TRIED_TO_SPAWN_TOO_MANY_ACTIONS);
		}

		return;
	}
}