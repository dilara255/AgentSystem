#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "data/agentDataControllers.hpp"

#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/actionSystem.hpp"

namespace AS{
		
	float calculateDesiredIntensityMultiplier(float score, float whyBother, float JustDoIt);
	void dispatchActionDetailSetting(float desiredIntensityMultiplier, 
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp);

	//TODO: test
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
		
			currentActions += actionDataVec_cptr->at(i).ids.active;
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

	bool spawnAction(actionData_t action, ActionSystem* actionSystem_ptr) {

		return actionSystem_ptr->getDataDirectPointer()->addActionData(action);
	}

	void chargeForAndSpawnAction(actionData_t action, AS::dataControllerPointers_t* dp,
											            ActionSystem* actionSystem_ptr, 
										   WarningsAndErrorsCounter* errorsCounter_ptr) {
		
		bool spawned = spawnAction(action, actionSystem_ptr);

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

	void setActionDetails(float score, float whyBother, float JustDoIt,
		                  AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp) {
	
		//First we change the target info so it stores the target's actual ID:
		int agent = action_ptr->ids.origin;
		int neighborIndexOnAgent = action_ptr->ids.target;

		if (neighborIndexOnAgent == NEIGHBOR_ID_FOR_SELF) {
			action_ptr->ids.target = agent;
		}
		else {
			if (action_ptr->ids.scope == (int)AS::scope::LOCAL) {

				auto agent_ptr = &(dp->LAstate_ptr->getDataCptr()->at(agent));

				action_ptr->ids.target = 
						agent_ptr->locationAndConnections.neighbourIDs[neighborIndexOnAgent];
			}
			else if (action_ptr->ids.scope == (int)AS::scope::GLOBAL) {

				auto agent_ptr = &(dp->GAstate_ptr->getDataCptr()->at(agent));

				action_ptr->ids.target = agent_ptr->neighbourIDs[neighborIndexOnAgent];
			}
		}

		float desiredIntensityMultiplier =
			calculateDesiredIntensityMultiplier(score, whyBother, JustDoIt);

		dispatchActionDetailSetting(desiredIntensityMultiplier, action_ptr, dp);		
	}

	//Todo: test
	float calculateDesiredIntensityMultiplier(float score, float whyBother, float JustDoIt) {
		//first we make sure that whyBother and JustDoIt are on a valid range:
		whyBother = 
			std::clamp(whyBother, MIN_ACT_WHY_BOTHER_THRESOLD, MAX_ACT_WHY_BOTHER_THRESOLD);

		float minimumJustDoIt = 
			std::max(whyBother + MIN_DECISION_THRESHOLD_SEPARATIONS, 
				                        MIN_ACT_JUST_DO_IT_THRESOLD);
		JustDoIt = std::clamp(JustDoIt, minimumJustDoIt, MAX_ACT_JUST_DO_IT_THRESOLD);
		
		//then we calculate the desiredIntensityMultiplier:
		float opinionWidth = JustDoIt - whyBother;

		//The ranges are:
		//[0, ACT_INTENSITY_WHY_BOTHER], score <= whyBother
		//(ACT_INTENSITY_WHY_BOTHER, ACT_INTENSITY_JUST_DO_IT), score in-between
		//[ACT_INTENSITY_JUST_DO_IT, ACT_INTENSITY_SCORE_1+), score above JustDoIt
		//NOTE: if score > 1, intensity > ACT_INTENSITY_SCORE_1 (expected)
		if (score <= whyBother) {
			float proportionOnRange = (score/whyBother);
			return ACT_INTENSITY_WHY_BOTHER * proportionOnRange;
		}
		else if (score < JustDoIt) {
			float proportionOnRange = (score - whyBother)/opinionWidth;
			return ACT_INTENSITY_WHY_BOTHER + 
					(ACT_INTENSITY_DIFFERENCE_TO_JUST_DO_IT * proportionOnRange);
		}
		else if (score >= JustDoIt) {
			float proportionOnRange = (score - JustDoIt)/(1 - JustDoIt);
			return ACT_INTENSITY_JUST_DO_IT + 
					(ACT_INTENSITY_DIFFERENCE_TO_SCORE_1 * proportionOnRange);
		}	

		//we should never get here, so:
		float wtf = 0;
		assert(wtf != 0);
		return wtf;
	}

	//TODO: this is temporary. We'll have these for different variations, elsewehere
	void setActionDetailsVarSTUB(float desiredIntensityMultiplier,
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp) {

		//Temporary way of setting the details of an action:


	}

	void dispatchActionDetailSetting(float desiredIntensityMultiplier,
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp) {

		//We'll need a different dispatcher for each unique variation : )
		
		//BUT for now we set all actions details with a single temp function:
		setActionDetailsVarSTUB(desiredIntensityMultiplier, action_ptr, dp);
	}
}