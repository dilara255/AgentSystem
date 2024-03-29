//TODO: Much of the detail setting will be based around some core ideas: Extract that

#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "data/agentDataControllers.hpp"

#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/actionSystem.hpp"
#include "systems/actionHelpers.hpp"

#include "data/dataMisc.hpp"

namespace AS{
		
	float calculateDesiredIntensityMultiplier(float score, float whyBother, float iGuess, 
		                                                                  float justDoIt);
	
	//This is the dispatcher: pretty much just a big switch : )
	//Takes an action and dispatches it to the appropriate function to set it's details
	//Those will set: phaseTime.total and details' intensity and processingAux, as needed
	//NOTE: details.processingAux will hold costs to be charged onSpawn or paid later
	void dispatchActionDetailSetting(float desiredIntensityMultiplier, 
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
							                   WarningsAndErrorsCounter* errorsCounter_ptr);
	
	//Sets phase and elapsed time to 0, stes the target, calculates an intensity multiplier
	//and them dispatches the action to have it's other details set: 
	//intensity, processingAux and total phase time for the preparation phase
	void setChoiceDetails(float score, float whyBother, float iGuess, float JustDoIt,
		                  AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
							                 WarningsAndErrorsCounter* errorsCounter_ptr) {
		
		assert(score > whyBother);

		//We're creating the action, so:
		action_ptr->ids.phase = (int)actPhases::SPAWN;
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
			calculateDesiredIntensityMultiplier(score, whyBother, iGuess, JustDoIt);

		assert(isfinite(desiredIntensityMultiplier));

		//And then dispatch the action to the proper function to the rest of the work:
		dispatchActionDetailSetting(desiredIntensityMultiplier, action_ptr, dp, 
			                                                 errorsCounter_ptr);

		return;
	}

	//Makes sure the delimiters are consistent and calculates desired intensity from score
	float calculateDesiredIntensityMultiplier(float score, float whyBother, float iGuess, 
																	      float justDoIt) {
		
		//first we make sure that whyBother, iGuess and justDoIt are on valid ranges:
		whyBother = 
			std::clamp(whyBother, MIN_ACT_WHY_BOTHER_THRESOLD, MAX_ACT_WHY_BOTHER_THRESOLD);

		float minimumIGuess = std::max(MIN_ACT_I_GUESS_THRESOLD,
							             (whyBother + MIN_DECISION_THRESHOLD_SEPARATIONS) );

		iGuess = 
			std::clamp(iGuess, minimumIGuess, MAX_ACT_I_GUESS_THRESOLD);	

		float minimumJustDoIt = std::max(MIN_ACT_JUST_DO_IT_THRESOLD,
							             (iGuess + MIN_DECISION_THRESHOLD_SEPARATIONS) );
			
		justDoIt = std::clamp(justDoIt, minimumJustDoIt, MAX_ACT_JUST_DO_IT_THRESOLD);
		
		//then we calculate the desiredIntensityMultiplier:

		//The final score ranges are:

		// ACT_MIN_INTENSITY, for score in [whyBother, iGuess]
		//(ACT_INTENSITY_I_GUESS, ACT_INTENSITY_JUST_DO_IT), for score in-between
		//[ACT_INTENSITY_JUST_DO_IT, ACT_INTENSITY_SCORE_1+), for score above JustDoIt

		//NOTE: if score > 1, intensity > ACT_INTENSITY_SCORE_1 (expected)
		//NOTE: if score < why bother, the action shouldn't even have been chosen

		assert(score >= whyBother);

		if (score <= iGuess) {
			float rangeWidth = iGuess - whyBother;
			float proportionOnRange = (score - whyBother/rangeWidth);

			return ACT_MIN_INTENSITY 
				   + (ACT_INTENSITY_DIFFERENCE_TO_I_GUESS * proportionOnRange);
		}
		else if (score < justDoIt) {
			float rangeWidth = justDoIt - iGuess;
			float proportionOnRange = (score - iGuess)/rangeWidth;
			return ACT_INTENSITY_I_GUESS
				   + (ACT_INTENSITY_DIFFERENCE_TO_JUST_DO_IT * proportionOnRange);
		}
		else if (score >= justDoIt) {
			float proportionOnRange = (score - justDoIt)/(1.0f - justDoIt);
			return ACT_INTENSITY_JUST_DO_IT 
				   + (ACT_INTENSITY_DIFFERENCE_TO_SCORE_1 * proportionOnRange);
		}	

		assert(false); //we should never get here but the compiler was complaining : p
		return -1;
	}


	/***************************************************************************************
	*                             DETAIL SETTING
	* 
	* NOTE: processingAux should be used ONLY for costs to be payed before spawning
	*    or later as the action runs (it can have other uses later, but not here)
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
	//will affect its progress).
	void setActionDetails_STR_S_L(float desiredIntensityMultiplier,
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
		                                    AS::WarningsAndErrorsCounter* errorsCounter_ptr) {
	
		int agent = action_ptr->ids.origin;
		auto agentParams_ptr = &(dp->LAstate_ptr->getDataCptr()->at(agent).parameters);

		//We want extra troops proportional to our current ones and desiredIntensityMultiplier:

		float strenght = agentParams_ptr->strenght.current;
		float effectiveStrenght = std::max(strenght, ACT_STR_S_L_REF_SMALL_STR);

		float newTroops = 
			AS::STR_S_L_calculateNewTroops(effectiveStrenght, desiredIntensityMultiplier);
			
		assert(isfinite(newTroops));

		newTroops = std::ceil(newTroops);

		action_ptr->details.intensity = newTroops;		

		//This is the funding necessary to raise these troops:
		action_ptr->details.processingAux = AS::STR_S_L_necessaryFunding(newTroops);

		//And now for the preparation time:
		action_ptr->phaseTiming.total = AS::STR_S_L_prepTime(newTroops);
	}

	//Resources will be invested to raise income
	//How much the agent will raise it's income will be proportional to
	//their current income as well as the desiredIntensityMultiplier
	//The income raise is stored as the intensity, while the total
	//investment cost is stored in the processingAux.
	//(in case they don't have enough, they'll have to pay during the action, wich
	//will affect its progress).
	//Preparation time will depend on the increase in income (sublinear);
	void setActionDetails_RES_S_L(float desiredIntensityMultiplier,
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
		                                    AS::WarningsAndErrorsCounter* errorsCounter_ptr) {
	
		assert(desiredIntensityMultiplier > 0);

		int agent = action_ptr->ids.origin;
		auto agentParams_ptr = &(dp->LAstate_ptr->getDataCptr()->at(agent).parameters);

		//We want extra income proportional to our current one and desiredIntensityMultiplier:

		float income = agentParams_ptr->resources.updateRate;
		float effectiveIncome = std::max(income, ACT_RES_S_L_REF_SMALL_INCOME);

		float raise = AS::RES_S_L_calculateRaise(effectiveIncome, desiredIntensityMultiplier);

		assert(isfinite(raise));
		assert(raise > 0);

		action_ptr->details.intensity = raise;

		//This is the funding necessary to raise the income:
		action_ptr->details.processingAux = AS::RES_S_L_necessaryFunding(raise);

		//And now for the preparation time:
		action_ptr->phaseTiming.total = AS::RES_S_L_prepTime(raise);
	}

	//This decides the attack strenght and sets action intensity accordingly.
	//The action's phaseTiming.total is also set to an intensity-dependent preparation time.
	//The final intensity depends on the desied intensity (stored in the action's intensity)
	//and relative strenghts.
	//Aux is not used.
	//TODO: this a placeholder / stub, mostly for load testing and some initial exploration
	void setActionDetails_ATT_I_L(float desiredIntensityMultiplier,
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
		                                    AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

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
				decision_ptr.reads[targetsIndexOnAgent].of[strenghtIndex].read;
				
		//We'll set the action's intensty to be the attack size
		float attackSize = 0;

		//There are two cases: either we are stronger, or not:

		if (agentStrenght > enemyStrenght) {

			float attackExtraProportionalMargin = 
				ACT_INTENS_ATTACK_MARGIN_INTENSITY_MULTIPLIER * desiredIntensityMultiplier;

			//We are stronger, so we will attack with more strenght than they have,
			//and the more we want to attack, the more extra forces we'll commit:
			attackSize = enemyStrenght * (1 + attackExtraProportionalMargin);
		}
		else {

			//We're not stronger. We'll commit a portion of our strenght proportional
			//to how high our desiredIntensityMultiplier was compared to what it would
			//be if we had made this choice with a score of "justDoIt".
			//IE: if our desire is the same as the desire of a score 1 action,
			//then we throw all our forces on this attack:
			attackSize = 
				agentStrenght * (desiredIntensityMultiplier / ACT_INTENSITY_JUST_DO_IT);
		}
		
		//In case we got carried away and chose an attack larger than our strenght:
		attackSize = std::clamp(attackSize, 0.0f, agentStrenght);

		assert(isfinite(attackSize));

		//We can now set the intensity:
		action_ptr->details.intensity = attackSize;

		//Also, more troops = longer preparation phase (but not linearly)
		uint32_t preparationTime = AS::ATT_I_L_prepTime(attackSize);

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
		                                    AS::WarningsAndErrorsCounter* errorsCounter_ptr) {
		
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
				errorsCounter_ptr->incrementError(AS::errors::DS_FAILED_TO_FIND_NEIGHBORS_INDEX);
			}
			enemyStrenght = 0;
		}
		else {
			const auto& decision_ptr = dp->GAdecision_ptr->getDataCptr()->at(agent);
			int strenghtIndex = (int)GA::readsOnNeighbor_t::fields::STRENGHT_LAS;

			enemyStrenght = 
					decision_ptr.reads[targetsIndexOnAgent].of[strenghtIndex].read;
		}
		
		float attackMarginProportion = 
			ACT_INTENS_ATTACK_MARGIN_INTENSITY_MULTIPLIER * desiredIntensityMultiplier;

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

		assert(isfinite(suggestionIntensity));

		action_ptr->details.intensity = suggestionIntensity;

		double preparationTime = (double)suggestionIntensity *
									ACT_ATT_S_G_SUGESTION_PREP_TENTHS_OF_MS_PER_INTENSITY;

		action_ptr->phaseTiming.total = (uint32_t)std::round(preparationTime);
	}

	void dispatchActionDetailSetting(float desiredIntensityMultiplier,
							AS::actionData_t* action_ptr, AS::dataControllerPointers_t* dp,
		                                    AS::WarningsAndErrorsCounter* errorsCounter_ptr) {
	
		int cat = action_ptr->ids.category;
		int mode = action_ptr->ids.mode;
		int scope = action_ptr->ids.scope;

		//For spawning, processingAux holds the intensity-dependent "investment",
		//wich will be charged for when the action is created (not on it's onSpawn)
		//the default is zero, so:
		action_ptr->details.processingAux = 0;

		//The validity of the variation should have been checked before getting here, so:
		assert(AS::ActionVariations::isValid(cat, mode, scope)); 

		//This is really just rerouting the call to the appropriate function and returning:
		if (scope == (int)AS::scope::LOCAL) {

			if (mode == (int)AS::actModes::SELF) {
				switch (cat)
				{
				case (int)AS::actCategories::STRENGHT:
					setActionDetails_STR_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::RESOURCES:
					setActionDetails_RES_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::ATTACK:
					setActionDetails_STR_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::GUARD:
					setActionDetails_STR_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::SPY:
					setActionDetails_STR_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::SABOTAGE:
					setActionDetails_STR_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::DIPLOMACY:
					setActionDetails_STR_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::CONQUEST:
					setActionDetails_STR_S_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
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
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::RESOURCES:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::ATTACK:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::GUARD:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::SPY:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::SABOTAGE:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::DIPLOMACY:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::CONQUEST:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
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
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::RESOURCES:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::ATTACK:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::GUARD:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::SPY:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::SABOTAGE:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::DIPLOMACY:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::CONQUEST:
					setActionDetails_ATT_I_L(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
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
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::RESOURCES:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::ATTACK:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::GUARD:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::SPY:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::SABOTAGE:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::DIPLOMACY:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::CONQUEST:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
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
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::RESOURCES:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::ATTACK:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::GUARD:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::SPY:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::SABOTAGE:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::DIPLOMACY:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::CONQUEST:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
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
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::RESOURCES:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::ATTACK:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::GUARD:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::SPY:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::SABOTAGE:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
				return;
				case (int)AS::actCategories::DIPLOMACY:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
					return;
				case (int)AS::actCategories::CONQUEST:
					setActionDetails_ATT_S_G(desiredIntensityMultiplier, action_ptr, dp, 
			                                                      errorsCounter_ptr);
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