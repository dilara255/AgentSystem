#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "data/agentDataControllers.hpp"

#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/actionSystem.hpp"

namespace AS{
		
	float calculateDesiredIntensityMultiplier(float score, float whyBother, float JustDoIt);
	void dispatchActionDetailSetting(float desiredIntensityMultiplier, 
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp);

	bool spawnAction(actionData_t action) {

		return false;
	}

	void chargeForAndSpawnAction(actionData_t action, AS::dataControllerPointers_t* dp,
							               WarningsAndErrorsCounter* errorsCounter_ptr) {
		
		bool spawned = spawnAction(action);

		if (spawned) {

			//charge for action
		}
	}

	void setActionDetails(float score, float whyBother, float JustDoIt,
		                  AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp) {
	
		float desiredIntensityMultiplier =
			calculateDesiredIntensityMultiplier(score, whyBother, JustDoIt);
			
		dispatchActionDetailSetting(desiredIntensityMultiplier, action_ptr, dp);		
	}

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
	}

	//TODO: this is temporary. We'll have these for different variations, elsewehere
	void setActionDetailsVarTMP(float desiredIntensityMultiplier,
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp) {

		//Temporary way of setting the details of an action:
	}

	void dispatchActionDetailSetting(float desiredIntensityMultiplier,
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp) {

		//We'll need a different dispatcher for each unique variation : )
		
		//BUT for now we set all actions details with a single temp function:
		setActionDetailsVarTMP(desiredIntensityMultiplier, action_ptr, dp);
	}
}