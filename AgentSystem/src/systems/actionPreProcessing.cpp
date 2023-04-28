#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "data/agentDataControllers.hpp"

#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/actionSystem.hpp"

#include "data/dataMisc.hpp"

namespace AS{
		
	float calculateDesiredIntensityMultiplier(float score, float whyBother, float JustDoIt);
	void dispatchActionDetailSetting(float desiredIntensityMultiplier, 
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
							                   WarningsAndErrorsCounter* errorsCounter_ptr);

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

	
	void setActionDetails(float score, float whyBother, float JustDoIt,
		                  AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
							                 WarningsAndErrorsCounter* errorsCounter_ptr) {
		
		//We're creating the action, so:
		action_ptr->ids.phase = 0;
		action_ptr->phaseTiming.elapsed = 0;
		//the phase total is set through dispatchActionDetailSetting

		//Next, we change the target info so it stores the target's actual ID:
		int agent = action_ptr->ids.origin;
		int neighborIndexOnAgent = action_ptr->ids.target;

		if (action_ptr->ids.mode == (uint32_t)AS::actModes::SELF) {
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

		dispatchActionDetailSetting(desiredIntensityMultiplier, action_ptr, dp, 
			                                                 errorsCounter_ptr);		
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

	//This decides the attack strenght and sets action intensity accordingly.
	//The action's phaseTiming.total is also set to an intensity-dependent preparation time.
	//The final intensity depends on the desied intensity (stored in the action's intensity)
	//and relative strenghts.
	//TODO: this a placeholder / stub, mostly for load testing and some initial exploration
	void setActionDetails_L_I_Attack(float desiredIntensityMultiplier,
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
		                                    AS::WarningsAndErrorsCounter* errorsCounter_pt) {
		
		int agent = action_ptr->ids.origin;
		int scope = action_ptr->ids.scope;

		float agentStrenght;
		float enemyStrenght;

		const auto state_ptr = &(dp->LAstate_ptr->getDataCptr()->at(agent));
		agentStrenght = state_ptr->parameters.strenght.current;
			
		int targetsIndexOnAgent = 
				getNeighborsIndexOnLA(action_ptr->ids.target, state_ptr);

		if (targetsIndexOnAgent == NATURAL_RETURN_ERROR) {
			if (action_ptr->ids.mode != (uint32_t)AS::actModes::SELF) {
				errorsCounter_pt->incrementError(AS::errors::DS_FAILED_TO_FIND_NEIGHBORS_INDEX);
			}
			enemyStrenght = 0;
		}
		else {
			const auto& decision_ptr = dp->LAdecision_ptr->getDataCptr()->at(agent);
			int strenghtIndex = (int)LA::readsOnNeighbor_t::fields::STRENGHT;

			enemyStrenght = 
					decision_ptr.reads[targetsIndexOnAgent].readOf[strenghtIndex];
		}
		
		float desiredIntensity = action_ptr->details.intensity;

		float attackMarginProportion = 
			ACT_INTENS_ATTACK_MARGIN_PROPORTION * desiredIntensity;

		float attackSize;

		if (agentStrenght > enemyStrenght) {
			attackSize = enemyStrenght * (1 + attackMarginProportion);
		}
		else {
			//since the agent is not as strong as the enemy and score is between 0 and 1:
			//the attack size will be (score * strenght). This is accomplished by:
			attackSize = 
				agentStrenght * (desiredIntensity / (ACT_INTENSITY_SCORE_1/2.0f));
		}
		
		attackSize = std::clamp(attackSize, 0.0f, agentStrenght);
		action_ptr->details.intensity = attackSize;

		//More troops = longer preparation phase, which is stored on aux (in ms):
		double effectiveAttackSize = sqrt(attackSize/DEFAULT_LA_STRENGHT);

		double preparationTime = effectiveAttackSize *
									ACT_BASE_ATTACK_L_I_PREP_TENTHS_OF_MS_PER_DEFAULT_STR;			                     

		action_ptr->phaseTiming.total = (uint32_t)std::round(preparationTime);
	}

	//This decides how aggresivelly the GA will sugest LAs to attack the targeted GA:
	//this will be set as the intensity.
	//The action's phaseTiming.total is also set to an intensity-dependent preparation time.
	//The final intensity depends on the desied intensity (stored in the action's intensity)
	//and relative strenghts.
	//TODO: this a placeholder / stub, mostly for load testing and some initial exploration
	void setActionDetails_G_S_Attack(float desiredIntensityMultiplier,
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
		                                    AS::WarningsAndErrorsCounter* errorsCounter_pt) {
		
		int agent = action_ptr->ids.origin;
		int scope = action_ptr->ids.scope;

		float agentStrenght;
		float enemyStrenght;

		const auto state_ptr = &(dp->GAstate_ptr->getDataCptr()->at(agent));		
		agentStrenght = state_ptr->parameters.LAstrenghtTotal;
			
		int targetsIndexOnAgent = 
				getNeighborsIndexOnGA(action_ptr->ids.target, state_ptr);
			
		if (targetsIndexOnAgent == NATURAL_RETURN_ERROR) {
			if (action_ptr->ids.mode != (uint32_t)AS::actModes::SELF) {
				errorsCounter_pt->incrementError(AS::errors::DS_FAILED_TO_FIND_NEIGHBORS_INDEX);
			}
			enemyStrenght = 0;
		}
		else {
			const auto& decision_ptr = dp->GAdecision_ptr->getDataCptr()->at(agent);
			int strenghtIndex = (int)GA::readsOnNeighbor_t::fields::STRENGHT_LAS;

			enemyStrenght = 
					decision_ptr.reads[targetsIndexOnAgent].readOf[strenghtIndex];
		}
		
		float desiredIntensity = action_ptr->details.intensity;

		float attackMarginProportion = 
			ACT_INTENS_ATTACK_MARGIN_PROPORTION * desiredIntensity;

		float suggestionIntensity;

		if (agentStrenght > enemyStrenght) {
			suggestionIntensity = 1 + attackMarginProportion;
		}
		else {
			//since the agent is not as strong as the enemy and score is between 0 and 1:
			//the intensity will be the same as the action's score, which is given by:
			suggestionIntensity = desiredIntensity / (ACT_INTENSITY_SCORE_1/2.0f);
		}
		
		suggestionIntensity = std::clamp(suggestionIntensity, 0.0f, 
			                             ACT_MAX_SUGESTION_INTENSITY);

		action_ptr->details.intensity = suggestionIntensity;

		double preparationTime = suggestionIntensity *
									ACT_ATTACK_G_S_SUGESTION_PREP_TENTHS_OF_MS_PER_INTENSITY;

		action_ptr->phaseTiming.total = (uint32_t)std::round(preparationTime);
	}

	//Takes an action and dispatches it to the appropriate function to set it's details
	//This will set: phaseTime.total, details.intensity and details.aux, as needed
	void dispatchActionDetailSetting(float desiredIntensityMultiplier,
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
		                                    AS::WarningsAndErrorsCounter* errorsCounter_pt) {

		//The idea is for this to be pretty much a dispatcher:
		//it validates that the variation exists (or raises an error), then dispatches
		//to a function which sets intensity and if processing aux for that variation

		//TODO: the validation, plus a couple more stubs (and review the GS_ATTACK stub)

		//FOR NOW: we treat all actions as if they were a L_I or G_S attack:
		if (action_ptr->ids.scope == (uint32_t)AS::scope::LOCAL) {
			setActionDetails_L_I_Attack(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
		}
		else { //GLOBAL
			setActionDetails_G_S_Attack(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
		}
	}
}