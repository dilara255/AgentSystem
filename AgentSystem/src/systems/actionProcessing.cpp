/***********************************************************************
* This file is renponsible for the ACTUAL PROCESSING OF ACTIONS
* For the spawining and initial detail setting of actions:
*		-> See actionPreProcessing.cpp
* 
* Other than the SPAWN phase, each phase of each action variaton has two related functions:
* - tick, which updates the remaining phase time and applies any other continuous change;
* -- the tick functions receive the time to be processed and return how much of that was used;
* - endPhase, which applies any end-of-phase effects and sets the action to the next phase.
* For the SPAWN phase of each variation, there's an onSpawn function.
* All of these have default versions, and the specific ones will be gradually implemented 
* at the end of this file and added to the forward declaration block bellow.
* 
* -> g_processingFunctions holds which funtions will be used for each phase of each variation.
* --> THIS CORRESPONDENCE IS SET ON initializeProcessingFunctions() !
* 
* -> dispatchActionProcessing should remain the ONLY ENTRY POINT for this file;
* --> it first makes sure g_processingFunctions is initialized and sets points to the system,
* then it dispatches the action to the appropriate processing function (also defined here)
/***********************************************************************/

#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "systems/actionSystem.hpp"
#include "systems/actionHelpers.hpp"

#include "data/agentDataStructures.hpp"
#include "data/agentDataControllers.hpp"

#include "data/dataMisc.hpp"

#include "systems/PRNserver.hpp"
#include "systems/warningsAndErrorsCounter.hpp"

namespace AS {
	
	//Initialization of the structure holding function correspondences is done by these:
	void initializeProcessingFunctions();
	void setProcessingFunctionsToDefaults();

	//These are the defaulf Processing functions:
	void defaultOnSpawn(actionData_t* action_ptr);
	//The default tick ticks for tickTenthsOfMs or until the phase ends. 
	//Either way, returns the time effectively ticked.
	uint32_t defaultTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr);
	void defaultPhaseEnd(actionData_t* action_ptr);

	void defaultPrepEnd(actionData_t* action_ptr);
	void defaultTravelEnd(actionData_t* action_ptr);
	void defaultEffectEnd(actionData_t* action_ptr);
	void defaultReturnEnd(actionData_t* action_ptr);
	void defaultConclusionEnd(actionData_t* action_ptr);
	
	uint32_t passtroughTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr);
	uint32_t justEndThePhase(uint32_t tickTenthsOfMs, actionData_t* action_ptr);
	uint32_t chargingTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr);
	
	void intensityToRatePrepEnd(actionData_t* action_ptr);
	void postergatePhaseEnd(actionData_t* action_ptr);
	void refundingConclusionEnd(actionData_t* action_ptr);

	//These are not swappable in as the initial call, but can be called from a standard
	//function and will do their thing and reroute to another standard one to finish:
	uint32_t partialTickIncreasingTotal(uint32_t effectiveTick, uint32_t fullTickTime, 
		                                                     actionData_t* action_ptr);

	//And these are specific variation processing functions:

						//LOCAL:
						//SELF: 
	//STR_S_L:
	void STR_S_L_onSpawn(actionData_t* action_ptr);
	void STR_S_L_PrepEnd(actionData_t* action_ptr);
	uint32_t STR_S_L_effectTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr);

	//RES_S_L:
	void RES_S_L_PrepEnd(actionData_t* action_ptr);
	uint32_t RES_S_L_effectTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr);
	
					  //IMMEDIATE:
	//ATT_I_L:
	void ATT_I_L_onSpawn(actionData_t* action_ptr);
	void ATT_I_L_PrepEnd(actionData_t* action_ptr);
	void ATT_I_L_travelEnd(actionData_t* action_ptr);
	void ATT_I_L_fleePhaseEnd(actionData_t* action_ptr);
	//ATT_I_L_effectTick uses a random number
	uint32_t ATT_I_L_effectTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr);
	void ATT_I_L_EffectEnd(actionData_t* action_ptr);
	void ATT_I_L_ReturnEnd(actionData_t* action_ptr);
	void ATT_I_L_ConclusionEnd(actionData_t* action_ptr);
}

//Here's the meat of this file: entry-point, initialization and processing actions
namespace AS {
	
	static ActionVariations::actionProcessingFunctions_t g_processingFunctions[(int)AS::scope::TOTAL];
	static bool g_processingFunctionsInitialized = false;

	static dataControllerPointers_t* g_agentDataControllers_ptr = NULL;
	static ActionSystem* g_actionSystem_ptr = NULL; 
	static WarningsAndErrorsCounter* g_errors_ptr = NULL;
	static PRNserver* g_prnServer_ptr = NULL;

	//This is the ONLY entrypoint for this file.
	//Here we set the pointers to the system's data (globals on this file and scope).
	//After that, we forward the action to the appropriate function.
	//In case the processing actions structure is not initialized, this also initializes it.
	void dispatchActionProcessing(actionData_t* action_ptr, float timeMultiplier,
								  dataControllerPointers_t* agentDataControllers_ptr,
								  ActionSystem* actionSystem_ptr, PRNserver* prnServer_ptr,
								  WarningsAndErrorsCounter* errors_ptr) {

		if (action_ptr == NULL) {
			errors_ptr->incrementError(AS::errors::AC_RECEIVED_BAD_ACTION_PTR);
			return;
		}

		if ( (action_ptr->ids.active == 0) || (action_ptr->ids.slotIsUsed == 0) ) {
			//nothing to do here:
			return;
		}

		//stepActions checks these before we get here, so we trust the system pointers:
		g_agentDataControllers_ptr = agentDataControllers_ptr;
		g_actionSystem_ptr = actionSystem_ptr;
		g_prnServer_ptr = prnServer_ptr;
		g_errors_ptr = errors_ptr;		

		//But there's no harm in asserting our trust:
		assert(g_agentDataControllers_ptr != NULL);
		assert(g_actionSystem_ptr != NULL);
		assert(g_prnServer_ptr != NULL);
		assert(g_errors_ptr != NULL);

		//We do need to make sure the processing functions are initialized:
		if (!g_processingFunctionsInitialized) {

			initializeProcessingFunctions();
			if (!g_processingFunctionsInitialized) {
				return;
			}
		}

		//Let's also check if the action just spawned and, if so, call it's onSpawn:
		if (action_ptr->ids.phase == (int)actPhases::SPAWN) {
			int cat = action_ptr->ids.category;
			int mode = action_ptr->ids.mode;
			int scope = action_ptr->ids.scope;

			g_processingFunctions[scope][cat][mode].onSpawn(action_ptr);
		}

		//timeMultiplier is in seconds, and has to be changed into a uint32_tenthsOfMilli_t:
		double tenthsOfMillisThisStep = 
							(double)timeMultiplier * (double)TENTHS_OF_MS_IN_A_SECOND;
		
		uint32_tenthsOfMilli_t timeRemainingToProcess = (uint32_t)tenthsOfMillisThisStep;	
		
		//Finally, we dispatch the action processing:
		//Since a phase may end and leave time for the next, this is done in a loop:
		while ( (timeRemainingToProcess > 0)
			     && (action_ptr->ids.active == 1) && (action_ptr->ids.slotIsUsed == 1) ) {

			assert(action_ptr->ids.phase < (int)actPhases::TOTAL);

			//We get these here because an action might "morph" into another as part
			//of its processing
			int cat = action_ptr->ids.category;
			int mode = action_ptr->ids.mode;
			int scope = action_ptr->ids.scope;
			//phases also naturally progress during processing
			int phase = action_ptr->ids.phase;

			//First we tick the action, receive the time progressed and calculate the remaining
			timeRemainingToProcess -= 
				g_processingFunctions[scope][cat][mode].onTick[phase](timeRemainingToProcess, 
					                                                              action_ptr);
			
			//If that is larger than zero, we didn't process as much time as we'd like:
			//that's because we've reached a phase end, so we must process that:
			if (timeRemainingToProcess > 0) {
				
				//onEnd will do any end-of-phase effect and advance phase or deactivate action:
				g_processingFunctions[scope][cat][mode].onEnd[phase](action_ptr);

				//Since there's still timeRemainingToProcess, we'll loop after this
				//and try to tick the timeRemainingToProcess. 
				//We might even end another phase and get back here.
			}
		}	
		//Done, the action was processed until it consumed all the step time or was deactivated
	}

	//This is where the functions used for each phase of each action variation
	//are selected. Changing this will change their behavior.
	void initializeProcessingFunctions() {

		setProcessingFunctionsToDefaults();		

		int local = (int)AS::scope::LOCAL;
		int global = (int)AS::scope::GLOBAL;
		int self = (int)AS::actModes::SELF;
		int immediate = (int)AS::actModes::IMMEDIATE;
		int request = (int)AS::actModes::REQUEST;
		int prep = (int)AS::actPhases::PREPARATION;
		int travel = (int)AS::actPhases::TRAVEL;
		int effect = (int)AS::actPhases::EFFECT;
		int ret = (int)AS::actPhases::RETURN;
		int conclusion = (int)AS::actPhases::CONCLUSION;

							//SPECIFIC BEHAVIOR:

								  //LOCAL:
		
								  //SELF:
		//STRENGHT:
		int str = (int)AS::actCategories::STRENGHT;
		g_processingFunctions[local][str][self].onSpawn = STR_S_L_onSpawn;
		g_processingFunctions[local][str][self].onTick[prep] = chargingTick;
		g_processingFunctions[local][str][self].onEnd[prep] = STR_S_L_PrepEnd;
		g_processingFunctions[local][str][self].onTick[travel] = justEndThePhase;
		g_processingFunctions[local][str][self].onTick[effect] = STR_S_L_effectTick;
		g_processingFunctions[local][str][self].onTick[ret] = justEndThePhase;
		g_processingFunctions[local][str][self].onTick[conclusion] = justEndThePhase;
		
		//RESOURCES:
		int res = (int)AS::actCategories::RESOURCES;
		g_processingFunctions[local][res][self].onTick[prep] = chargingTick;
		g_processingFunctions[local][res][self].onEnd[prep] = RES_S_L_PrepEnd;
		g_processingFunctions[local][res][self].onTick[travel] = justEndThePhase;
		g_processingFunctions[local][res][self].onTick[effect] = RES_S_L_effectTick;
		g_processingFunctions[local][res][self].onTick[ret] = justEndThePhase;
		g_processingFunctions[local][res][self].onTick[conclusion] = justEndThePhase;

						    //IMMEDIATE:
		//ATTACK:
		int att = (int)AS::actCategories::ATTACK;
		g_processingFunctions[local][att][immediate].onSpawn = ATT_I_L_onSpawn;
		g_processingFunctions[local][att][immediate].onEnd[prep] = ATT_I_L_PrepEnd;
		g_processingFunctions[local][att][immediate].onEnd[travel] = ATT_I_L_travelEnd;
		g_processingFunctions[local][att][immediate].onTick[effect] = ATT_I_L_effectTick;
		g_processingFunctions[local][att][immediate].onEnd[effect] = ATT_I_L_EffectEnd;
		g_processingFunctions[local][att][immediate].onEnd[ret] = ATT_I_L_ReturnEnd;
		g_processingFunctions[local][att][immediate].onEnd[conclusion] = ATT_I_L_ConclusionEnd;

							   //DONE

		g_processingFunctionsInitialized = true;
	}

	//Sets all processing functions to default versions
	int local = (int)AS::scope::LOCAL;
	int global = (int)AS::scope::GLOBAL;

	void setProcessingFunctionsToDefaults(){
		for (int cat = 0; cat < (int)actCategories::TOTAL; cat++) {
			for (int mode = 0; mode < (int)actModes::TOTAL; mode++) {

				g_processingFunctions[local][cat][mode].onSpawn = defaultOnSpawn;
				g_processingFunctions[global][cat][mode].onSpawn = defaultOnSpawn;

				for (int phase = 0; phase < (int)actPhases::TOTAL; phase++) {
					
					g_processingFunctions[local][cat][mode].onTick[phase] = defaultTick;
					g_processingFunctions[global][cat][mode].onTick[phase] = defaultTick;
				}

				g_processingFunctions[local][cat][mode].onEnd[(int)actPhases::PREPARATION] = 
																		defaultPrepEnd;
				g_processingFunctions[global][cat][mode].onEnd[(int)actPhases::PREPARATION] = 
																		defaultPrepEnd;
				g_processingFunctions[local][cat][mode].onEnd[(int)actPhases::TRAVEL] = 
																		defaultTravelEnd;
				g_processingFunctions[global][cat][mode].onEnd[(int)actPhases::TRAVEL] = 
																		defaultTravelEnd;
				g_processingFunctions[local][cat][mode].onEnd[(int)actPhases::EFFECT] = 
																		defaultEffectEnd;
				g_processingFunctions[global][cat][mode].onEnd[(int)actPhases::EFFECT] = 
																		defaultEffectEnd;
				g_processingFunctions[local][cat][mode].onEnd[(int)actPhases::RETURN] = 
																		defaultReturnEnd;
				g_processingFunctions[global][cat][mode].onEnd[(int)actPhases::RETURN] = 
																		defaultReturnEnd;
				g_processingFunctions[local][cat][mode].onEnd[(int)actPhases::CONCLUSION] = 
																		defaultConclusionEnd;
				g_processingFunctions[global][cat][mode].onEnd[(int)actPhases::CONCLUSION] = 
																		defaultConclusionEnd;
			}
		}
	}

	//DEFAULT ACTION PROCESSING FUNCTIONS:

	void defaultOnSpawn(actionData_t* action_ptr) {

		action_ptr->ids.phase = (int)AS::actPhases::PREPARATION;
		return;
	}

	uint32_t defaultTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

		assert(action_ptr->phaseTiming.elapsed <= action_ptr->phaseTiming.total);

		uint32_tenthsOfMilli_t timeRemainingOnPhase = 
			action_ptr->phaseTiming.total - action_ptr->phaseTiming.elapsed;

		if (timeRemainingOnPhase >= tickTenthsOfMs) {
			action_ptr->phaseTiming.elapsed += tickTenthsOfMs;
			return tickTenthsOfMs; //all time consumed
		}
		else { //phase has less than a full tick remaining
			action_ptr->phaseTiming.elapsed += timeRemainingOnPhase;
			return timeRemainingOnPhase;
		}
	}

	void defaultPhaseEnd(actionData_t* action_ptr) {

		action_ptr->ids.phase++;
		action_ptr->phaseTiming.elapsed = 0;
		
		return;
	}

	void defaultPrepEnd(actionData_t* action_ptr) {

		defaultPhaseEnd(action_ptr);
	}

	void defaultTravelEnd(actionData_t* action_ptr) {

		defaultPhaseEnd(action_ptr);
	}

	void defaultEffectEnd(actionData_t* action_ptr) {

		defaultPhaseEnd(action_ptr);
	}

	void defaultReturnEnd(actionData_t* action_ptr) {

		defaultPhaseEnd(action_ptr);
	}

	void defaultConclusionEnd(actionData_t* action_ptr) {

		//Just invalidades the action:
		action_ptr->ids.active = 0;
		action_ptr->ids.slotIsUsed = 0;
	}

	uint32_t passtroughTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

		return tickTenthsOfMs;
	}

	uint32_t justEndThePhase(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

		return 0;
	}

	uint32_t partialTickIncreasingTotal(uint32_t effectiveTick, uint32_t fullTickTime, 
		                                                     actionData_t* action_ptr) {

		if (action_ptr->phaseTiming.elapsed > action_ptr->phaseTiming.total) {
			return justEndThePhase(fullTickTime, action_ptr);
		}

		uint32_tenthsOfMilli_t timeRemainingOnPhase = 
				action_ptr->phaseTiming.total - action_ptr->phaseTiming.elapsed;			

		if (timeRemainingOnPhase >= effectiveTick) { 
			//consume all time, pretend was intended time, add difference to total
			uint32_tenthsOfMilli_t effectiveFullTime = 
						std::min(fullTickTime, timeRemainingOnPhase);
			uint32_tenthsOfMilli_t timeNotReallyTicked = effectiveFullTime - effectiveTick;

			action_ptr->phaseTiming.total += timeNotReallyTicked;
			action_ptr->phaseTiming.elapsed += fullTickTime;
			return passtroughTick(fullTickTime, action_ptr);
		}
		else { //phase has less than a full tick remaining
			action_ptr->phaseTiming.elapsed += timeRemainingOnPhase;
			return justEndThePhase(fullTickTime, action_ptr);
		}
	}

	uint32_t chargingTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

		//tries to pay wathever funding is left of the desired funding, then tick as usual

		int agent = action_ptr->ids.origin;

		assert(g_agentDataControllers_ptr != NULL);

		float* resources_ptr =
			&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(agent).parameters.resources.current);
		
		assert(isfinite(*resources_ptr));

		if ((*resources_ptr) >= action_ptr->details.processingAux) {

			*resources_ptr -= action_ptr->details.processingAux;
			action_ptr->details.processingAux = 0;
		}
		else if ((*resources_ptr) > 0) {
			action_ptr->details.processingAux -= *resources_ptr;
			*resources_ptr = 0;			
		}

		return defaultTick(tickTenthsOfMs, action_ptr);
	}

	//Doesn't change time ellapsed or advance phase. Instead, add to the total time.
	void postergatePhaseEnd(actionData_t* action_ptr){

		action_ptr->phaseTiming.total += 
			MAX_PROPORTIONAL_STEP_DURATION_ERROR *
					AS_MILLISECONDS_PER_STEP * TENTHS_OF_MS_IN_A_MILLI;

		return;
	}

	void intensityToRatePrepEnd(actionData_t* action_ptr) {

		if (action_ptr->phaseTiming.total == 0) {
			action_ptr->details.intensity = 0;
		}
		else {
			//The intensity is divided by the next phase's time, becoming a rate
			action_ptr->details.intensity /= action_ptr->phaseTiming.total;
		}

		//Then the usual stuff is done:
		defaultPrepEnd(action_ptr);

		return;
	}

	//To be used when aborting onSpawns: refunds the agent and kills the action
	void refundingConclusionEnd(actionData_t* action_ptr) {

		int agent = action_ptr->ids.origin;
		int actionsCountingThis = 
			AS::getQuantityOfCurrentActions((AS::scope)action_ptr->ids.scope, agent,
													g_actionSystem_ptr, g_errors_ptr);
		assert(actionsCountingThis >= 1);
			
		float costForRefund = AS::nextActionsCost(actionsCountingThis - 1);

		if (costForRefund > 0) {
			auto agent_ptr =
				&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(agent));
				
			agent_ptr->parameters.resources.current += costForRefund;

			assert(isfinite(agent_ptr->parameters.resources.current));
		}

		defaultConclusionEnd(action_ptr);
		return;
	}

				//SPECIFIC ACTION PROCESSING FUNCIONS:

							//LOCAL:

							//SELF: 	
	//STRENGHT:
	
	//We check if raising the desired troops is economically feasible:
	void STR_S_L_onSpawn(actionData_t* action_ptr) {

		assert(g_agentDataControllers_ptr != NULL);

		int agent = action_ptr->ids.origin;
		auto agentState_ptr = 
				&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(agent));

		float finalIntensity = 
			AS::getResponsibleTroopsToRaise(agentState_ptr, action_ptr->details.intensity);

		action_ptr->details.intensity = finalIntensity;

		assert(finalIntensity >= 0);

		if (finalIntensity <= 0) {
			
			//This is not worth the trouble
			//Did the agent actually pay anything for this? If so, let's refund that:
			refundingConclusionEnd(action_ptr);
			return;
		}	
		else {
			defaultOnSpawn(action_ptr);
			return;
		}

	}

	void STR_S_L_PrepEnd(actionData_t* action_ptr) {

		//Aux was set to hold how many funds haven't been paid yet
		//This changes that to how much HAS BEEN ALREADY paid

		float totalFundsNeeded = STR_S_L_necessaryFunding(action_ptr->details.intensity);
		action_ptr->details.processingAux = 
							totalFundsNeeded - action_ptr->details.processingAux;

		//Then the time for the next phase is set to be the same from the preparation phase
		//times a parametrized multiplier:
		action_ptr->phaseTiming.total *= 
						(uint32_tenthsOfMilli_t)ACT_STR_S_L_EFFECT_TIME_MULTIPLIER;
		
		//Finally, the intensity is divided by the next phase's time, becoming a rate
		//(and the phase is advanced as usual):
		intensityToRatePrepEnd(action_ptr);

		return;
	}

	uint32_t STR_S_L_effectTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

		int agent = action_ptr->ids.origin;
		auto params_ptr = 
			&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(agent).parameters);

		//Try to pay as much as possible of the remaining funding:
		uint32_t timeLeft = action_ptr->phaseTiming.total - action_ptr->phaseTiming.elapsed;
		//How much does the rate times the time left cost?
		float resourcesLeftToPay = 
			STR_S_L_necessaryFunding(action_ptr->details.intensity * timeLeft);
		
		float* funds_ptr = &(action_ptr->details.processingAux);

		float fundingPending = std::max(0.0f, resourcesLeftToPay - (*funds_ptr));

		//Calculate how much STR we should get this tick if fully funded:
		float idealGain = action_ptr->details.intensity * tickTenthsOfMs;

		//Then we calculate how much that would cost in fundings:
		float fundsForIdealGain = STR_S_L_necessaryFunding(idealGain);

		//How much of that can we pay now?
		float* resources_ptr = &(params_ptr->resources.current);	

		assert(isfinite(*resources_ptr));

		float newFunds = 0;
		if ( (*resources_ptr) >= fundingPending) {
			newFunds = fundingPending;
		}
		else if ( (*resources_ptr) > 0 ) {
			newFunds = (*resources_ptr);
		}
		else if ( (*resources_ptr) > AS::getMaxDebt(params_ptr->resources.updateRate) ) {
			newFunds = fundsForIdealGain * ACT_STR_S_L_OVERDRAFT_FUNDS_PROPORTION;
		}
		else { //we're in too much debt, so just let the time pass:
			return defaultTick(tickTenthsOfMs, action_ptr);
		}

		//Charge for the funds:
		*funds_ptr += newFunds;
		*resources_ptr -= newFunds;

		//Of course we won't try to pay over our intended maximum:
		float effectiveMaximumExpenseThisTick = 
						std::min(fundsForIdealGain, resourcesLeftToPay);

		//How much of that can we pay for from our funding?
		float ratioToGain = std::min(1.0f, ((*funds_ptr) / effectiveMaximumExpenseThisTick) );
		 
		//Now we pay for that that and raise the str:
		*funds_ptr -= (ratioToGain * effectiveMaximumExpenseThisTick);
		*funds_ptr = std::max(0.0f, (*funds_ptr) ); //to avoid any floating point weirdness

		//Gain STR:
		params_ptr->strenght.current += (ratioToGain * idealGain);
		
		//We want to tick normally, but since the effect may be partial,
		//we offset that by an increase in total time proportional to the gain in STR:
		if (ratioToGain < 1.0f) {
			//We use floor to make sure this ends, even if with *slightly* less troops
			action_ptr->phaseTiming.total +=
					(uint32_t)floor((1 - (double)ratioToGain)*tickTenthsOfMs);
		}
		
		return defaultTick(tickTenthsOfMs, action_ptr);
	}

	//RESOURCES:

	void RES_S_L_PrepEnd(actionData_t* action_ptr) {
		
		assert(isfinite(action_ptr->details.intensity));

		//Aux was set to hold how many funds haven't been paid yet
		//This changes that to how much HAS BEEN ALREADY paid

		float totalFundsNeeded = RES_S_L_necessaryFunding(action_ptr->details.intensity);
		action_ptr->details.processingAux = 
							totalFundsNeeded - action_ptr->details.processingAux;

		//Then the time for the next phase is set to be the same from the preparation phase
		//times a parametrized multiplier:
		action_ptr->phaseTiming.total *= 
						(uint32_tenthsOfMilli_t)ACT_RES_S_L_EFFECT_TIME_MULTIPLIER;

		//Finally, the intensity is divided by the next phase's time, becoming a rate
		//(and the phase is advanced as usual):
		intensityToRatePrepEnd(action_ptr);

		assert(isfinite(action_ptr->details.intensity));

		return;
	}

	uint32_t RES_S_L_effectTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

		int agent = action_ptr->ids.origin;
		auto params_ptr = 
			&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(agent).parameters);

		assert(isfinite(params_ptr->resources.updateRate));
		assert(isfinite(action_ptr->details.intensity));

		//Try to pay as much as possible of the remaining funding:
		uint32_t timeLeft = action_ptr->phaseTiming.total - action_ptr->phaseTiming.elapsed;
		//How much does the rate times the time left cost?
		float resourcesLeftToPay = 
			RES_S_L_necessaryFunding(action_ptr->details.intensity * timeLeft);
		
		float* funds_ptr = &(action_ptr->details.processingAux);

		float fundingPending = std::max(0.0f, resourcesLeftToPay - (*funds_ptr));

		//Calculate how much INC we should get this tick if fully funded:
		float idealGain = action_ptr->details.intensity * tickTenthsOfMs;
		assert(isfinite(idealGain));

		//Then we calculate how much that would cost in fundings:
		float fundsForIdealGain = RES_S_L_necessaryFunding(idealGain);

		//How much of that can we pay now?
		float* resources_ptr = &(params_ptr->resources.current);

		assert(isfinite(*resources_ptr));

		float newFunds = 0;
		if ( (*resources_ptr) >= fundingPending) {
			newFunds = fundingPending;
		}
		else if( (*resources_ptr) > 0 ) {
			newFunds = (*resources_ptr);
		}
		else if( (*resources_ptr) > AS::getMaxDebt(params_ptr->resources.updateRate) ) {
			newFunds = fundsForIdealGain * ACT_RES_S_L_OVERDRAFT_FUNDS_PROPORTION;
		}
		else { //We're in too much debt, but some income increase will come for free:
			newFunds = fundsForIdealGain * ACT_RES_S_L_PITTY_FUNDS_PROPORTION;
		}

		//Gain the funds:
		*funds_ptr += newFunds;
		//And charge for them, but only if we're above the max debt line:
		if ((*resources_ptr) > AS::getMaxDebt(params_ptr->resources.updateRate)) {
			*resources_ptr -= newFunds;
		}

		//Of course we won't try to pay over our intended maximum:
		float effectiveMaximumExpenseThisTick = 
						std::min(fundsForIdealGain, resourcesLeftToPay);

		//How much of that can we pay for from our funding?
		float ratioToGain = 1.0f;
		if (effectiveMaximumExpenseThisTick > 0) {
			ratioToGain = std::min(1.0f, ((*funds_ptr) / effectiveMaximumExpenseThisTick) );
			assert(isfinite(ratioToGain));
		}			
		 
		//Now we pay for that and raise the str:
		*funds_ptr -= (ratioToGain * effectiveMaximumExpenseThisTick);
		*funds_ptr = std::max(0.0f, (*funds_ptr) ); //to avoid any floating point weirdness

		//Gain INC:
		params_ptr->resources.updateRate += (ratioToGain * idealGain);

		assert(isfinite(params_ptr->resources.updateRate));
		
		//We want to tick normally, but since the effect may be partial,
		//we offset that by an increase in total time proportional to the gain in STR:
		if (ratioToGain < 1.0f) {
			//We use floor to make sure this ends, even if with *slightly* less troops
			action_ptr->phaseTiming.total +=
					(uint32_t)floor((1 - (double)ratioToGain)*tickTenthsOfMs);
		}

		assert(isfinite(*resources_ptr));
		
		return defaultTick(tickTenthsOfMs, action_ptr);
	}
	
					  //IMMEDIATE:
	//ATTACK:
	
	//The attack intensity becomes guard (so it can't be used on other attacks)
	//TODO: This has the side-effect of lowering upkeep. Is that a problem?
	void ATT_I_L_onSpawn(actionData_t* action_ptr) {

		assert(g_agentDataControllers_ptr != NULL);

		float intensity = action_ptr->details.intensity;

		if (intensity >= ACT_ATT_I_L_MINIMUM_ATTACK_INTENSITY) {

			int agent = action_ptr->ids.origin;
			auto agentState_ptr = 
				&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(agent));
		
			auto agentStrenght_ptr = &(agentState_ptr->parameters.strenght);
			assert(agentStrenght_ptr != NULL);

			agentStrenght_ptr->current -= intensity;
			agentStrenght_ptr->externalGuard += intensity;

			action_ptr->details.longTermAux = intensity;
			
			defaultOnSpawn(action_ptr);
			return;
		}
		else {
			//This is not worth the trouble
			//Did the agent actually pay anything for this? If so, let's refund that:
			refundingConclusionEnd(action_ptr);
			return;
		}				
	}

	void ATT_I_L_PrepEnd(actionData_t* action_ptr) {

		int agent = action_ptr->ids.origin;
		auto ourState_ptr = 
			&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(agent));

		if (ourState_ptr->underAttack > 0) { //Don't actually end the preparation phase
			postergatePhaseEnd(action_ptr);
			return;
		}

		//If we're not under attack, let's get on our way

		int neighbor = action_ptr->ids.target;
		auto enemyState_ptr = 
			&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(neighbor));

		//First, we take away the troops which were sent to the guard in the spawn:
		float* externalGuard_ptr = &(ourState_ptr->parameters.strenght.externalGuard);
		float* intensity_ptr = &(action_ptr->details.intensity);

		if (*externalGuard_ptr >= *intensity_ptr) {
			*externalGuard_ptr -= action_ptr->details.intensity;	
		}
		else {
			float* strenght_ptr = &(ourState_ptr->parameters.strenght.current);

			float troopsMissing = *intensity_ptr - *externalGuard_ptr;
			*externalGuard_ptr = 0;

			if (*strenght_ptr >= troopsMissing) {
				*strenght_ptr -= troopsMissing;
				troopsMissing = 0;
			}
			else {
				troopsMissing -= *strenght_ptr;
				*strenght_ptr = 0;
			}
			
			//The attack will be launched as well as it can be:
			*intensity_ptr -= troopsMissing;

			//That is, if there is still an attack to launch at all:
			if ((*intensity_ptr) < ACT_ATT_I_L_MINIMUM_SIZE_FOR_REDUCED_ATTACK) {
				invalidateAction(action_ptr);
				return;
			}
		}
		
		//We record our initial forces, both on the agent and the action side:
		AS::incrementTroopsOnAttack(&ourState_ptr->parameters.strenght.onAttacks, *intensity_ptr);
		action_ptr->details.longTermAux = (*intensity_ptr);

		//Then, we calculate a base travel time according to distance and a parameter:
		pos_t ourPosition = ourState_ptr->locationAndConnections.position;
		pos_t enemyPosition = enemyState_ptr->locationAndConnections.position;

		action_ptr->phaseTiming.total =
			ATT_I_L_travelTimeFromDistanceAndTroops(ourPosition, enemyPosition, 
													            *intensity_ptr);

		//Now we just need to do the usual phase end stuff, so:
		defaultPhaseEnd(action_ptr);
	}

	void ATT_I_L_travelEnd(actionData_t* action_ptr) {

		//Know thy enemy:

		int attacker = action_ptr->ids.origin;
		int defender = action_ptr->ids.target;

		auto defendersDecision_ptr = 
			&(g_agentDataControllers_ptr->LAdecision_ptr->getDirectDataPtr()->at(defender));

		auto defendersState_ptr =
			&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(defender));

		int attackerIDonDefender = AS::getNeighborsIndexOnLA(attacker, defendersState_ptr);
		
		//The attack has begun! Our foes grit their teeth and swear our names:
		defendersState_ptr->relations.dispositionToNeighbors[attackerIDonDefender] -=
														ACT_ATT_I_L_DISPO_LOSS_FROM_ATTACK;

		//They realize their errors about our forces:
		int strenghtField = (int)LA::readsOnNeighbor_t::fields::STRENGHT;
		float* defendersReadOnAttackersStrenght_ptr =
			&(defendersDecision_ptr->reads[attackerIDonDefender].of[strenghtField].read);

		float attackSize = action_ptr->details.intensity;

		assert (attackSize > 0);

		//Do they think we've attacked with all our might?
		float attackToReadNormalizedDifference =
			std::sqrt(std::abs(attackSize - *defendersReadOnAttackersStrenght_ptr) / attackSize);

		*defendersReadOnAttackersStrenght_ptr -= attackSize;

		float defendersDefences = defendersState_ptr->parameters.strenght.current
								  + defendersState_ptr->parameters.strenght.externalGuard;

		//Are they impressed?
		float attackToDefenceNormalizedDifference = 
									(attackSize - defendersDefences) / attackSize;

		//Will they learn to fear our forces?
		float STRcorrectionMultiplier = 
			attackToReadNormalizedDifference * attackToDefenceNormalizedDifference;

		STRcorrectionMultiplier = 
			std::clamp(STRcorrectionMultiplier, -ACT_ATT_I_L_MAX_DEFENDER_STR_READ_MULT, 
										         ACT_ATT_I_L_MAX_DEFENDER_STR_READ_MULT);
		
		*defendersReadOnAttackersStrenght_ptr += 
				ACT_ATT_I_L_DEFENDER_STR_READ_BASE * STRcorrectionMultiplier 
											       * std::sqrt(attackSize);

		*defendersReadOnAttackersStrenght_ptr =
					std::max(*defendersReadOnAttackersStrenght_ptr, 0.0f);

		//What do the seers say?
		action_ptr->details.shortTermAux = g_prnServer_ptr->getNext();

		//Are we too feeble? Should we just gather whatever intel we can and flee?
		if ( (attackSize / defendersDefences) < ACT_ATT_I_L_THRESHOLD_FOR_SCOUTING) {
			//ALAS - can we even make the run for it? The fewer we are, the lightr our feet!
			if (action_ptr->details.shortTermAux > attackToDefenceNormalizedDifference) {
				//As we flee, we gather our memories of our formidable foes:
				action_ptr->details.shortTermAux = defendersDefences;

				action_ptr->details.processingAux = 0.0f; //no time for loot! : /
				action_ptr->details.longTermAux = ACT_ATT_I_L_LONG_TERM_AUX_ON_SCOUTING_RUN;
				
				ATT_I_L_fleePhaseEnd(action_ptr);
				return;
			}			
		}

		//The battle is about to begin, but already our men wonder: how long can it last?
		action_ptr->phaseTiming.total = AS::ATT_I_L_attackTime(attackSize);
		defendersState_ptr->underAttack++;

		//Aanyway:
		defaultPhaseEnd(action_ptr);
	}

	void ATT_I_L_fleePhaseEnd(actionData_t* action_ptr) {
		//Fleeing is a return trip, as fast as the original travel: same total time, but:
		action_ptr->phaseTiming.elapsed = 0;
		action_ptr->ids.phase = (int)AS::actPhases::RETURN;
		return;
	}

	uint32_t ATT_I_L_effectTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

		//This makes the attack happen and builds up an "score" of the fight in processingAux. 
		//The tides of the battle depend on a random factor, the forces and the score;
		//The score is used as a "morale", and dictates the pace of the fight (time advanced).
		//If the fight's time ends, whoever the score favours wins (dealt with on effectEnd).
		//If either side goes out of troops, they loose:
		//The score is changed to reflect that and we tick for zero time to end the phase.
		//The score will also later be used to calculate loot, in case the attacker wins.		

		assert(action_ptr->details.intensity > 0); //should have ended otherwise

		int defender = action_ptr->ids.target;
		auto defendersStrData_ptr = 
			&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(defender).parameters.strenght);

		float defences = defendersStrData_ptr->current + defendersStrData_ptr->externalGuard;

		assert(defences >= 0); //first turn around, zero is valid

		//How do the forces compare, effectively?
		//The score is stored in aux. Positive score: attacker winning. 
		//It's also a morale, so it changes effective defense and attack.
		
		//We also take the sqrt of the force sizes: a bunch of guys can't fight a few together
		//The signs on the multipliers preserve the meaning (positive: attacker ahead)

		float moraleFactor = ACT_ATT_I_L_MORALE_IMPACT_FACTOR * action_ptr->details.processingAux;

		float effectiveDefences = std::sqrt (defences ) * (1 - moraleFactor);
		
		float* intensity_ptr = &(action_ptr->details.intensity);
		float effectiveAttack = std::sqrt ( (*intensity_ptr) ) * (1 + moraleFactor);

		//Each tick we calculate the score for this round of fighting:
		//First, we need to know how the balance of the forces is:
		float balancePoint = effectiveAttack / (effectiveAttack + effectiveDefences);
		//Two prns are used to make the distribution a triangle centered on 0.5:
		
		float* draw_ptr = &(action_ptr->details.shortTermAux);
		float period = ((float)tickTenthsOfMs / TENTHS_OF_MS_IN_A_SECOND);

		*draw_ptr = g_prnServer_ptr->getRedNextGivenTime((*draw_ptr), period, 
						      ACT_ATT_I_L_BATTLE_RED_PRN_STD_DEV_AT_REF_TIME,
				                         NOTIONS_AND_ACTIONS_REF_PERIOD_SECS);
		float draw = *draw_ptr;

		//The score change is proportional to the timestep (so high frequency doesn't blow it).
		//Also, larger battles should tend to take longer: we use a reference time for that
		//The order of balancePoint - draw is important to preserve the meaning (att vs def)
		//Note that the more unbalanced the balance point, te harder it is to recover
		
		float totalForces = defences + (*intensity_ptr);
		uint32_t referenceTime = AS::ATT_I_L_prepTime(totalForces);

		float scoreChange = (balancePoint - draw) * ((float)tickTenthsOfMs / referenceTime);
		action_ptr->details.processingAux += scoreChange;

		//How many troops were lost?		
		float totalLosses = totalForces * std::abs(scoreChange);
	
		//How will that be distributed?
		//Both sides suffer losses, but the distribution depends on the balance and the draw:

		float drawOffset = (draw - balancePoint)/2; //the division is because we'll apply to both
		float drawWidht = balancePoint;
		if(drawOffset > 0) { drawWidht = (1 - balancePoint); }
		if(drawWidht != 0){
			drawWidht *= 2; //it's as if there was this much room both sides of the balancePoint
			drawOffset /= drawWidht; //relative to wich side of the balancePoint the draw fell in
		}
		else { drawOffset = std::copysign(0.5f, drawOffset); }

		float attackerShareOfLosses = 0.5f + drawOffset;
		float deffenderShareOfLosses = 0.5f - drawOffset;

		//For the attacker, the losses come from the action intensity:
		*intensity_ptr -= totalLosses * attackerShareOfLosses;

		//For the defender, we take away the guard first, then the strenght:
		float defendersLosses = totalLosses * deffenderShareOfLosses;
				
		if (defendersStrData_ptr->externalGuard >= defendersLosses) {
			defendersStrData_ptr->externalGuard -= defendersLosses;
		}
		else {
			float defendersLossesFromStr = defendersLosses - defendersStrData_ptr->externalGuard;
			defendersStrData_ptr->externalGuard = 0;
			defendersStrData_ptr->current -= defendersLossesFromStr;
			defendersStrData_ptr->current = std::max(0.0f, defendersStrData_ptr->current);
		}
		defences = defendersStrData_ptr->externalGuard + defendersStrData_ptr->current;


		//Now we check if both sides still have troops:
		float irrelevantForce = 0.1f;
		bool keepFighting = true;

		if ((defences < irrelevantForce) && ((*intensity_ptr) < irrelevantForce)) {//both dead

			defendersStrData_ptr->current = 0;
			*intensity_ptr = 0;

			action_ptr->details.processingAux = 0;

			keepFighting = false;
		}
		else if (defences < irrelevantForce) { //defenders dead
			
			defendersStrData_ptr->current = 0;
			
			//No defenses leave time to recover and regroup (and maybe loot):
			action_ptr->details.processingAux +=
				ACT_ATT_I_L_ATTACKER_SCORE_BONUS_IF_NO_DEFENDERS;

			//Only score >= 0 makes sense, so:
			action_ptr->details.processingAux = 
				std::max(0.0f, action_ptr->details.processingAux);
			//Score == 0: "attackers were retreating and beat defenders"

			keepFighting = false;
		}
		else if ( (*intensity_ptr) < irrelevantForce) { //attackers dead
			
			*intensity_ptr = 0;

			//Same idea, but in reverse:

			//No attackers able to fight means more opportunities to make prisioners:
			action_ptr->details.processingAux +=
				ACT_ATT_I_L_ATTACKER_SCORE_BONUS_IF_NO_DEFENDERS;

			action_ptr->details.processingAux = 
				std::min(0.0f, action_ptr->details.processingAux);

			keepFighting = false;
		}

		bool scoreEnd = 
			std::abs(action_ptr->details.processingAux) >= ACT_ATT_I_L_ABS_SCORE_TO_END_FIGHT;

		if(keepFighting && scoreEnd) {
			keepFighting = false;
		}
		
		//Now we deal with ticking (and possibly ending the phase):
		if (keepFighting) {
			
			//The more one-sided the fight is, the less we expect it to last. So:
			uint32_t effectiveTick = 
				(uint32_t)( (double)tickTenthsOfMs * std::abs(action_ptr->details.processingAux) );

			return partialTickIncreasingTotal(effectiveTick, tickTenthsOfMs, action_ptr);
		}
		else {
			return justEndThePhase(0, action_ptr); //the fighting is over
		}

	}

	void ATT_I_L_EffectEnd(actionData_t* action_ptr) {
		
		//First we deal with the defender:
		int defender = action_ptr->ids.target;
		auto defendersDecision_ptr = 
			&(g_agentDataControllers_ptr->LAdecision_ptr->getDirectDataPtr()->at(defender));

		auto defendersState_ptr = 
			&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(defender));

		defendersState_ptr->underAttack--;

		int attackerID = action_ptr->ids.origin;
		int attackerIndexOnDefender = getNeighborsIndexOnLA(attackerID, defendersState_ptr);

		//They get some general information according to the score, if negative (defender win)
		float*  aux_ptr = &(action_ptr->details.processingAux);
		float infiltrationGain = 
						- std::min(0.0f, (*aux_ptr) ) * ACT_ATT_I_L_BASE_DEFENDER_INFO_GAIN;

		defendersDecision_ptr->infiltration[attackerIndexOnDefender] +=
			                std::min(infiltrationGain, ACT_ATT_I_L_MAX_DEFENDER_INFO_GAIN);

		//And add any routing/returning troops to their expectations:
		int str = (int)LA::readsOnNeighbor_t::fields::STRENGHT;

		float* intensity_ptr = &(action_ptr->details.intensity);
		defendersDecision_ptr->reads[attackerIndexOnDefender].of[str].read += (*intensity_ptr);

		//Now we deal with the returnees:
		
		//In order to set the return time, we will need:
		uint32_t* phaseTotal_ptr = &(action_ptr->phaseTiming.total);
		auto attackerState_ptr =
				&(g_agentDataControllers_ptr->LAstate_ptr->getDataCptr()->at(attackerID));
		
		AS::pos_t attackerPos = attackerState_ptr->locationAndConnections.position;
		AS::pos_t defenderPos = defendersState_ptr->locationAndConnections.position;

		//has anyone even survived?
		if ((*intensity_ptr) == 0) {
			//nope. Clear aux and set a time for the agent to realize their loss
			*aux_ptr = 0; //no survivors, no loot
			*phaseTotal_ptr = AS::ATT_I_L_travelTimeFromDistanceAndTroops(attackerPos,
														defenderPos, ACT_REF_STRENGHT);
		}
		else { //We have people alive
			//How long will their travel back take? 
			//Less people travel faster, but also the worse the losses, the slower we go
			
			*phaseTotal_ptr = ATT_I_L_returnTime(attackerPos, defenderPos, (*intensity_ptr),
				                                                         (*phaseTotal_ptr) );
			
			//Anyway, how much loot did they grab, losses and all? 
			//(zero on defeat, otherwise calculated from score and defender's resources)
			if ((*aux_ptr <= 0) || (*intensity_ptr <= 0)) {
				*aux_ptr = 0; //now storing the (non-existent) loot
			}
			else { //attacker won: there will be loot!
				//The better the score, the more loot each attacker manages to get,
				//and the more surviving attackers, the more hands to loot!

				auto defendersResourceInfo_ptr = &(defendersState_ptr->parameters.resources);

				assert(isfinite(defendersResourceInfo_ptr->current));
				assert(isfinite(defendersResourceInfo_ptr->updateRate));

				float resources = defendersResourceInfo_ptr->current;

				float effectiveDefendersResources = 
						resources * (1 - ACT_ATT_I_L_MIN_PROPORTIONAL_RESOURCE_RESERVE_FROM_LOOT);

				effectiveDefendersResources = std::max(0.0f, effectiveDefendersResources);
				
				float lootEffectiveness =
						std::min((*aux_ptr)*(*aux_ptr), ACT_ATT_I_L_MAX_LOOT_PER_HAND);

				float totalCarryCapactiy = ACT_ATT_I_L_MAX_LOOT_PER_HAND
								           * (*intensity_ptr) * lootEffectiveness * 2;
				
				//This is the loot:
				*aux_ptr = std::min(effectiveDefendersResources, totalCarryCapactiy);
				defendersResourceInfo_ptr->current -= *aux_ptr;

				assert(isfinite(defendersResourceInfo_ptr->current));

				//Looting damages infrastructure, which lowers the lootee's income.
				//(proportional to loot/resources, to the original income, and to a parameter)

				//In case there are no resources to loot, we have looted as much as possible:
				float lootProportion = 1;
				//Otehrwise:
				if(resources > 0) {
					lootProportion = *aux_ptr / resources; //resources is pre-loot value
				}
			
				float unprotectedIncome = 
							defendersResourceInfo_ptr->updateRate
									* ACT_ATT_I_L_INCOME_PROPORTION_NOT_PROTECTED_FROM_LOOT;

				defendersResourceInfo_ptr->updateRate -= unprotectedIncome * lootProportion;

				assert(isfinite(defendersResourceInfo_ptr->updateRate));
			}
		}

		//Let the troops (or their ghosts) travel back home:
		defaultPhaseEnd(action_ptr);
		return;
	}

	void ATT_I_L_ReturnEnd(actionData_t* action_ptr) {

		assert(action_ptr->details.intensity >= 0);

		int agent = action_ptr->ids.origin;
		auto agentState_ptr = &(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(agent));

		//The agent no longer counts the sent troops as being on attack:
		AS::decrementTroopsOnAttack(&agentState_ptr->parameters.strenght.onAttacks, 
			                                       action_ptr->details.longTermAux);
			
		//But how many of them actually returned? DO they have tales to tell about our enemies?
		float returnees = action_ptr->details.intensity;

		int neighbor = action_ptr->ids.target;
		int neighborIndexOnAgent = getNeighborsIndexOnLA(neighbor, agentState_ptr);
		auto decision_ptr = 
			&(g_agentDataControllers_ptr->LAdecision_ptr->getDirectDataPtr()->at(agent));
		int str = (int)LA::readsOnNeighbor_t::fields::STRENGHT;
		int grd = (int)LA::readsOnNeighbor_t::fields::GUARD;

		if (action_ptr->details.longTermAux == ACT_ATT_I_L_LONG_TERM_AUX_ON_SCOUTING_RUN) {
			//This will be considered a scouting run. 
			//We'll get info as a returning attack, bellow, but also some extra as a baseline:

			float readDefences = 
				decision_ptr->reads->of[str].read + decision_ptr->reads->of[grd].read;
			float defencesFound = action_ptr->details.shortTermAux;
			float differenceInDefences = action_ptr->details.shortTermAux - readDefences;

			float trustInReturneesTales = returnees / defencesFound;
			assert(trustInReturneesTales < 1);

			//Our guys couldn't tell who was a native or a reinforcement, so:
			float strToAddToEachField = 
				std::sqrt(trustInReturneesTales) * differenceInDefences / 2.0f;

			decision_ptr->reads->of[str].read += strToAddToEachField;
			decision_ptr->reads->of[grd].read += strToAddToEachField;
		}
		if(returnees > 0) { 
						
			//First of all, take any loot in:
			agentState_ptr->parameters.resources.current += 
									ACT_ATT_I_L_PROPORTION_OF_LOOT_ACTUALLY_PROFITED 
									* action_ptr->details.processingAux;
		
			assert(isfinite(agentState_ptr->parameters.resources.current));

			//The more people returned compared to the original size and the faster the fight,
			//the more info they'll have been able to get:

			auto attackerState_ptr = 
				&(g_agentDataControllers_ptr->LAstate_ptr->getDataCptr()->at(agent));

			int defender = action_ptr->ids.target;
			auto defenderState_ptr = 
				&(g_agentDataControllers_ptr->LAstate_ptr->getDataCptr()->at(defender));

			AS::pos_t attackerPos = attackerState_ptr->locationAndConnections.position;
			AS::pos_t defenderPos = defenderState_ptr->locationAndConnections.position;

			float effectiveReturneeRatio = 
				ATT_I_L_effectiveReturneeRatio(attackerPos, defenderPos,
					     action_ptr->phaseTiming.total, returnees);
		
			effectiveReturneeRatio = std::min(1.0f, effectiveReturneeRatio);

			//Then we gather information:

			//We get infiltration points proportional to the effectiveReturneeRatio:

			decision_ptr->infiltration[neighborIndexOnAgent] +=
								effectiveReturneeRatio * ACT_ATT_I_L_BASE_INFO_FROM_RETURNEES;
			
			decision_ptr->infiltration[neighborIndexOnAgent] =
					std::clamp(decision_ptr->infiltration[neighborIndexOnAgent], 
						                     MIN_INFILTRATION, MAX_INFILTRATION);

			//And information specifically about their troops:
			//We'll interpolate our read with the real value using returneesFactor as weight:

			auto neighborStrenght_ptr = 
				&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(neighbor).parameters.strenght);

			decision_ptr->reads[neighborIndexOnAgent].of[str].read *= (1 - effectiveReturneeRatio);
			decision_ptr->reads[neighborIndexOnAgent].of[str].read += 
										   effectiveReturneeRatio * neighborStrenght_ptr->current;
			
			decision_ptr->reads[neighborIndexOnAgent].of[grd].read *= (1 - effectiveReturneeRatio);
			decision_ptr->reads[neighborIndexOnAgent].of[grd].read += 
									 effectiveReturneeRatio * neighborStrenght_ptr->externalGuard;
			
			//Finally we accept out troops back in the GUARD (to recover):
			agentState_ptr->parameters.strenght.externalGuard += returnees;

			//and set a recover time:
			action_ptr->phaseTiming.total = AS::ATT_I_L_prepTime(returnees);
		}
		else { //in case everyone's dead:
			//Oh well. Set next phases total time to zero and proceed:
			action_ptr->phaseTiming.total = 0;
		}
		

		//Then we advance the phase normally:
		defaultPhaseEnd(action_ptr);
	}

	void ATT_I_L_ConclusionEnd(actionData_t* action_ptr) {

		//Our returned troops are ready to go back from the guard to the strenght:
		int agent = action_ptr->ids.origin;

		auto strenght_ptr = 
			&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(agent).parameters.strenght);

		if (strenght_ptr->externalGuard >= action_ptr->details.intensity) {

			strenght_ptr->externalGuard -= action_ptr->details.intensity;
			strenght_ptr->current += action_ptr->details.intensity;
		}
		else { //maybe they died?
			strenght_ptr->current += strenght_ptr->externalGuard;		
			strenght_ptr->externalGuard = 0;
		}

		defaultConclusionEnd(action_ptr);
	}
}