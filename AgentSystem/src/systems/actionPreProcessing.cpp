#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "data/agentDataControllers.hpp"

#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/actionSystem.hpp"

#include "data/dataMisc.hpp"

namespace AS{
		
	float calculateDesiredIntensityMultiplier(float score, float whyBother, float JustDoIt);
	
	//This is the dispatcher: pretty much just a big switch : )
	//Takes an action and dispatches it to the appropriate function to set it's details
	//Those will set: phaseTime.total, details.intensity and details.aux, as needed
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

	
	void setChoiceDetails(float score, float whyBother, float JustDoIt,
		                  AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
							                 WarningsAndErrorsCounter* errorsCounter_ptr) {
		
		//We're creating the action, so:
		action_ptr->ids.phase = 0;
		action_ptr->phaseTiming.elapsed = 0;
		//phase total will be set via the call to dispatchActionDetailSetting further down

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

		//Now we'll set the actual details, as well as the first phases duration.
		//For this, we first calculate an intensity multiplier:
		float desiredIntensityMultiplier =
			calculateDesiredIntensityMultiplier(score, whyBother, JustDoIt);

		//And then dispatch the action to the proper function to the rest of the work:
		dispatchActionDetailSetting(desiredIntensityMultiplier, action_ptr, dp, 
			                                                 errorsCounter_ptr);		
	}

	//Todo: test
	float calculateDesiredIntensityMultiplier(float score, float whyBother, float JustDoIt) {
		
		//first we make sure that whyBother and JustDoIt are on a valid range:
		whyBother = 
			std::clamp(whyBother, MIN_ACT_WHY_BOTHER_THRESOLD, MAX_ACT_WHY_BOTHER_THRESOLD);


		float minimumJustDoIt = std::max(MIN_ACT_JUST_DO_IT_THRESOLD,
							             (whyBother + MIN_DECISION_THRESHOLD_SEPARATIONS) );
			
		JustDoIt = std::clamp(JustDoIt, minimumJustDoIt, MAX_ACT_JUST_DO_IT_THRESOLD);
		
		//then we calculate the desiredIntensityMultiplier:
		float opinionWidth = JustDoIt - whyBother;

		//The ranges are:

		//[0, ACT_INTENSITY_WHY_BOTHER], for score <= whyBother
		//(ACT_INTENSITY_WHY_BOTHER, ACT_INTENSITY_JUST_DO_IT), for score in-between
		//[ACT_INTENSITY_JUST_DO_IT, ACT_INTENSITY_SCORE_1+), for score above JustDoIt

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

		assert(false); //we should never get here but the compiler was complaining : p
		return -1;
	}


	/***************************************************************************************
	*                             DETAIL SETTING
	****************************************************************************************/


	/****************************************************************************************
	//                                LOCAL:
	// 
	//            Bellow we define the functions to set the action details 
	//                      for each LOCAL variation (WIP):
	*****************************************************************************************/


	//This decides how much we want to invest in making more troops;
	//The larger the desiredIntensityMultiplier, the more we'll try to invest;
	//This will be proportional to our current strenght;
	//Preparation time will depend on the EXTRA troops being made (sublinear);
	//Aux is set to how much the agent will have to pay for the production;
	//(in case they don't have enough, they'll have to pay during the action, wich
	//will affect it's progress.
	void setActionDetails_STR_S_L(float desiredIntensityMultiplier,
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
		                                    AS::WarningsAndErrorsCounter* errorsCounter_pt) {
	
		int agent = action_ptr->ids.origin;
		auto agentParams_ptr = &(dp->LAstate_ptr->getDataCptr()->at(agent).parameters);

		//We want extra troops proportional to our current ones and desiredIntensityMultiplier:

		float strenght = agentParams_ptr->strenght.current;
		float effectiveStrenght = std::max(strenght, ACT_STR_S_L_REF_SMALL_STR);

		float newTroops = 
			effectiveStrenght * ACT_STR_S_L_REF_PROPORTION_OF_STR * desiredIntensityMultiplier;

		//This is the funding necessary to raise these troops:
		action_ptr->details.processingAux = 
					(newTroops / ACT_REF_STRENGHT) * ACT_STR_S_L_COST_PER_REF_STR;

		//And now for the preparation time:
		double effectiveNewTroopsForTiming = std::sqrt(newTroops / ACT_REF_STRENGHT);

		double preparationTime = 
			ACT_STR_S_L_BASE_PREP_TENTHS_OF_MS_PER_REF_STR * effectiveNewTroopsForTiming;

		action_ptr->phaseTiming.total = (uint32_t)preparationTime;
	}

	void setActionDetails_RES_S_L(float desiredIntensityMultiplier,
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
		                                    AS::WarningsAndErrorsCounter* errorsCounter_pt) {
	
		setActionDetails_STR_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
	}

	//This decides the attack strenght and sets action intensity accordingly.
	//The action's phaseTiming.total is also set to an intensity-dependent preparation time.
	//The final intensity depends on the desied intensity (stored in the action's intensity)
	//and relative strenghts.
	//Aux is just set to zero.
	//TODO: this a placeholder / stub, mostly for load testing and some initial exploration
	void setActionDetails_ATT_I_L(float desiredIntensityMultiplier,
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
		                                    AS::WarningsAndErrorsCounter* errorsCounter_pt) {

		//nothing to hold on aux:
		action_ptr->details.processingAux = 0;

		int agent = action_ptr->ids.origin;
		int scope = action_ptr->ids.scope;

		//how strong are we?
		const auto state_ptr = &(dp->LAstate_ptr->getDataCptr()->at(agent));
		float agentStrenght = state_ptr->parameters.strenght.current;
			
		//and what's the target's strenght?

		int targetsIndexOnAgent = 
				getNeighborsIndexOnLA(action_ptr->ids.target, state_ptr);

		assert(targetsIndexOnAgent != NATURAL_RETURN_ERROR); //bad target for this variation

		const auto& decision_ptr = dp->LAdecision_ptr->getDataCptr()->at(agent);
		int strenghtIndex = (int)LA::readsOnNeighbor_t::fields::STRENGHT;

		float enemyStrenght = 
				decision_ptr.reads[targetsIndexOnAgent].readOf[strenghtIndex];
				
		//We'll set the action's intensty to be the attack size
		float attackSize;

		//There are two cases: either we are stronger, or not:

		if (agentStrenght > enemyStrenght) {

			float attackExtraProportionalMargin = 
					ACT_INTENS_ATTACK_MARGIN_PROPORTION * desiredIntensityMultiplier;

			//We are stronger, so we will attack with more strenght than they have,
			//and the more we want to attack, the more extra forces we'll commit:
			attackSize = enemyStrenght * (1 + attackExtraProportionalMargin);
		}
		else {

			//We're not stronger. We'll commit a portion of our strenght proportional
			//to how high our desiredIntensityMultiplier was compared to what it would
			//be if we had made this choice with a score of 1.
			//IE: if our desire is the same as the desire of a score 1 action,
			//then we throw all our forces on this attack:
			attackSize = 
				agentStrenght * (desiredIntensityMultiplier / ACT_INTENSITY_SCORE_1);
		}
		
		//In case we got carried away and chose an attack larger than our strenght:
		attackSize = std::clamp(attackSize, 0.0f, agentStrenght);

		//We can now set the intensity:
		action_ptr->details.intensity = attackSize;

		//Also, more troops = longer preparation phase (but not linearly)
		double effectiveAttackSize = sqrt(attackSize / ACT_REF_STRENGHT);

		double preparationTime = effectiveAttackSize *
									ACT_ATT_I_L_BASE_PREP_TENTHS_OF_MS_PER_REF_STR;			                     

		//Finally we can set how many tenths of MS the preparation phase will take:
		action_ptr->phaseTiming.total = (uint32_t)std::round(preparationTime);
	}


	/****************************************************************************************
	//                                 GLOBAL:
	// 
	//            Bellow we define the functions to set the action details 
	//                      for each GLOBAL variation (WIP):
	*****************************************************************************************/


	//This decides how aggresivelly the GA will sugest LAs to attack the targeted GA:
	//this will be set as the intensity.
	//The action's phaseTiming.total is also set to an intensity-dependent preparation time.
	//The final intensity depends on the desied intensity (stored in the action's intensity)
	//and relative strenghts.
	//Aux is just set to zero.
	//TODO: this a placeholder / stub, mostly for load testing and some initial exploration
	void setActionDetails_ATT_S_G(float desiredIntensityMultiplier,
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
		                                    AS::WarningsAndErrorsCounter* errorsCounter_pt) {
		
		action_ptr->details.processingAux = 0;

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
		
		float attackMarginProportion = 
			ACT_INTENS_ATTACK_MARGIN_PROPORTION * desiredIntensityMultiplier;

		float suggestionIntensity;

		if (agentStrenght > enemyStrenght) {
			suggestionIntensity = 1 + attackMarginProportion;
		}
		else {
			//since the agent is not as strong as the enemy:
			suggestionIntensity = desiredIntensityMultiplier / ACT_INTENSITY_SCORE_1;
		}
		
		suggestionIntensity = std::clamp(suggestionIntensity, 0.0f, 
			                             ACT_MAX_SUGESTION_INTENSITY);

		action_ptr->details.intensity = suggestionIntensity;

		double preparationTime = (double)suggestionIntensity *
									ACT_ATT_S_G_SUGESTION_PREP_TENTHS_OF_MS_PER_INTENSITY;

		action_ptr->phaseTiming.total = (uint32_t)std::round(preparationTime);
	}

	void dispatchActionDetailSetting(float desiredIntensityMultiplier,
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
		                                    AS::WarningsAndErrorsCounter* errorsCounter_pt) {
	
		int cat = action_ptr->ids.category;
		int mode = action_ptr->ids.mode;
		int scope = action_ptr->ids.scope;

		//The validity of the variation should have been checked before getting here, so:
		assert(AS::ActionVariations::isValid(cat, mode, scope)); 

		//This is really just rerouting the call to the appropriate function and returning:
		if (scope == (int)AS::scope::LOCAL) {

			if (mode == (int)AS::actModes::SELF) {
				switch (cat)
				{
				case (int)AS::actCategories::STRENGHT:
					setActionDetails_STR_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::RESOURCES:
					setActionDetails_RES_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::ATTACK:
					setActionDetails_STR_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::GUARD:
					setActionDetails_STR_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::SPY:
					setActionDetails_STR_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::SABOTAGE:
					setActionDetails_STR_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::DIPLOMACY:
					setActionDetails_STR_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::CONQUEST:
					setActionDetails_STR_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				default:
					assert(false); //all expected categories are already described
				}
			}
			else if (mode == (int)AS::actModes::IMMEDIATE) {
				switch (cat)
				{
				case (int)AS::actCategories::STRENGHT:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::RESOURCES:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::ATTACK:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::GUARD:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::SPY:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::SABOTAGE:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::DIPLOMACY:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::CONQUEST:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				default:
					assert(false); //all expected categories are already described
				}
			}
			else if (mode == (int)AS::actModes::REQUEST) {
				switch (cat)
				{
				case (int)AS::actCategories::STRENGHT:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::RESOURCES:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::ATTACK:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::GUARD:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::SPY:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::SABOTAGE:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::DIPLOMACY:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::CONQUEST:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				default:
					assert(false); //all expected categories are already described
				}
			}
			else { assert(false); } //there really shouldn't be other modes
		}
		else if (scope == (int)AS::scope::GLOBAL) {

			if (mode == (int)AS::actModes::SELF) {
				switch (cat)
				{
				case (int)AS::actCategories::STRENGHT:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::RESOURCES:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::ATTACK:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::GUARD:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::SPY:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::SABOTAGE:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::DIPLOMACY:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::CONQUEST:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				default:
					assert(false); //all expected categories are already described
				}
			}
			else if (mode == (int)AS::actModes::IMMEDIATE) {
				switch (cat)
				{
				case (int)AS::actCategories::STRENGHT:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::RESOURCES:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::ATTACK:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::GUARD:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::SPY:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::SABOTAGE:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::DIPLOMACY:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::CONQUEST:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				default:
					assert(false); //all expected categories are already described
				}
			}
			else if (mode == (int)AS::actModes::REQUEST) {
				switch (cat)
				{
				case (int)AS::actCategories::STRENGHT:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::RESOURCES:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::ATTACK:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::GUARD:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::SPY:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::SABOTAGE:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				case (int)AS::actCategories::DIPLOMACY:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
					return;
				case (int)AS::actCategories::CONQUEST:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_pt);
				return;
				default:
					assert(false); //all expected categories are already described
				}
			}
			else { assert(false); } //there really shouldn't be other modes
		}
		else { assert(false); } //there really shouldn't be other scopes
	}
}