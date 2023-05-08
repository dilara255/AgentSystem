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

	//And these are specific variation processing functions:

						//LOCAL:
						//SELF: 
	//STR_S_L:
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
			action_ptr->ids.phase = (int)AS::actPhases::PREPARATION;
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
		g_processingFunctions[local][res][self].onEnd[travel] = ATT_I_L_travelEnd;
		g_processingFunctions[local][res][self].onTick[travel] = justEndThePhase;
		g_processingFunctions[local][res][self].onTick[effect] = RES_S_L_effectTick;
		g_processingFunctions[local][res][self].onTick[ret] = justEndThePhase;
		g_processingFunctions[local][res][self].onTick[conclusion] = justEndThePhase;

						    //IMMEDIATE:
		//ATTACK:
		int att = (int)AS::actCategories::ATTACK;
		g_processingFunctions[local][att][immediate].onSpawn = ATT_I_L_onSpawn;
		g_processingFunctions[local][att][self].onEnd[prep] = ATT_I_L_PrepEnd;
		g_processingFunctions[local][att][self].onTick[effect] = ATT_I_L_effectTick;
		g_processingFunctions[local][att][self].onEnd[effect] = ATT_I_L_EffectEnd;
		g_processingFunctions[local][att][self].onEnd[effect] = ATT_I_L_ReturnEnd;
		g_processingFunctions[local][att][self].onEnd[effect] = ATT_I_L_ConclusionEnd;

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

		return;
	}

	uint32_t defaultTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

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

	uint32_t routeToPasstroughOrEndOfPhaseTick(uint32_t effectiveTick, uint32_t fullTickTime, 
		                                                            actionData_t* action_ptr) {

		uint32_tenthsOfMilli_t timeRemainingOnPhase = 
			action_ptr->phaseTiming.total - action_ptr->phaseTiming.elapsed;

		if (timeRemainingOnPhase >= effectiveTick) { 
			//consume all time, pretend was intended time
			action_ptr->phaseTiming.elapsed += effectiveTick;
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
		
		if ((*resources_ptr) >= action_ptr->details.processingAux) {

			*resources_ptr -= action_ptr->details.processingAux;
			action_ptr->details.processingAux = 0;
		}
		else {
			action_ptr->details.processingAux -= *resources_ptr;
			*resources_ptr = 0;			
		}

		return defaultTick(tickTenthsOfMs, action_ptr);
	}

	void intensityToRatePrepEnd(actionData_t* action_ptr) {

		//The intensity is divided by the next phase's time, becoming a rate
		action_ptr->details.intensity /= action_ptr->phaseTiming.total;

		//Then the usual stuff is done:
		defaultPrepEnd(action_ptr);

		return;
	}

				//SPECIFIC ACTION PROCESSING FUNCIONS:

							//LOCAL:

							//SELF: 	
	//STRENGHT:
	
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

		//How much of that can we pay now?
		float* resources_ptr = &(params_ptr->resources.current);		

		float newFunds = 0;
		if ( (*resources_ptr) >= fundingPending) {
			newFunds = fundingPending;
		}
		else {
			newFunds = (*resources_ptr);
		}

		*funds_ptr += newFunds;
		*resources_ptr -= newFunds;
		
		//Calculate how much str we should get this tick if fully funded:
		float idealGain = action_ptr->details.intensity * tickTenthsOfMs;

		//Then we calculate how much that would cost in fundings:
		float fundsForIdealGain = STR_S_L_necessaryFunding(idealGain);

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

		return;
	}

	uint32_t RES_S_L_effectTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

		int agent = action_ptr->ids.origin;
		auto params_ptr = 
			&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(agent).parameters);

		//Try to pay as much as possible from the pending funding:
		float* resources_ptr = &(params_ptr->resources.current);
		float totalDesiredFunding = STR_S_L_necessaryFunding(action_ptr->details.intensity);

		float* funds_ptr = &(action_ptr->details.processingAux);

		float leftToPay = totalDesiredFunding - (*funds_ptr);

		float newFunds = 0;
		if ( (*resources_ptr) >= leftToPay) {
			newFunds = leftToPay;
		}
		else {
			newFunds = (*resources_ptr);
		}

		*funds_ptr += newFunds;
		*resources_ptr -= newFunds;
		 
		//Calculate how much str we should get this tick if fully funded:
		float idealGain = action_ptr->details.intensity * tickTenthsOfMs;

		//Then we calculate how much that would cost in fundings:
		float fundsForIdealGain = RES_S_L_necessaryFunding(idealGain);
		
		//How much of that can we pay for from our funding?
		float ratioToGain = std::min(1.0f, ((*funds_ptr) / fundsForIdealGain) );
		 
		//Now we pay for that that and raise the str:
		*funds_ptr -= (ratioToGain * fundsForIdealGain);
		*funds_ptr = std::max(0.0f, (*funds_ptr) ); //to avoid any floating point weirdness

		params_ptr->resources.updateRate += (ratioToGain * idealGain);
		
		//Then we do the usual ticking, but advance time proportionally to our gain:
		uint32_t timeToTick = (uint32_t)round( (double)tickTenthsOfMs * (double)ratioToGain );

		return defaultTick(timeToTick, action_ptr);
	}
	
					  //IMMEDIATE:
	//ATTACK:
	
	//The attack intensity becomes guard (so it can't be used on other attacks)
	//TODO: This has the side-effect of lowering upkeep. Is that a problem?
	void ATT_I_L_onSpawn(actionData_t* action_ptr) {

		int agent = action_ptr->ids.origin;
		float intensity = action_ptr->details.intensity;

		assert(g_agentDataControllers_ptr != NULL);

		auto agentStrenght_ptr =
			&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(agent).parameters.strenght);

		assert(agentStrenght_ptr != NULL);

		agentStrenght_ptr->current -= intensity;
		agentStrenght_ptr->externalGuard += intensity;
		
		return;
	}

	void ATT_I_L_PrepEnd(actionData_t* action_ptr) {

		int agent = action_ptr->ids.origin;
		auto ourState_ptr = 
			&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(agent));

		int neighbor = action_ptr->ids.target;
		auto enemyState_ptr = 
			&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(neighbor));

		//First, we take away the troops which were sent to the guard in the spawn:
		ourState_ptr->parameters.strenght.externalGuard -= action_ptr->details.intensity;

		ourState_ptr->parameters.strenght.externalGuard =
						std::max(0.0f, ourState_ptr->parameters.strenght.externalGuard);

		//Then, we calculate a base travel time according to distance and a parameter:
		pos_t ourPosition = ourState_ptr->locationAndConnections.position;
		pos_t enemyPosition = enemyState_ptr->locationAndConnections.position;

		action_ptr->phaseTiming.total =
			ATT_I_L_travelTimeFromDistanceAndTroops(ourPosition, enemyPosition, 
													action_ptr->details.intensity);

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
		int strenghtField = (int)LA::requestExpectations_t::fields::STRENGHT;
		float* defendersReadOnAttackersStrenght_ptr =
			&(defendersDecision_ptr->requestsForNeighbors[attackerIDonDefender].expected[strenghtField]);

		float attackSize = action_ptr->details.intensity;

		float attackToReadNormalizedDifference =
			std::abs(attackSize - *defendersReadOnAttackersStrenght_ptr) / attackSize;

		*defendersReadOnAttackersStrenght_ptr -= attackSize;

		float defendersDefences = defendersState_ptr->parameters.strenght.current
								  + defendersState_ptr->parameters.strenght.externalGuard;

		float attackToDefenceNormalizedDifference = 
									(attackSize - defendersDefences) / attackSize;

		float STRcorrectionMultiplier = 
			attackToReadNormalizedDifference * attackToDefenceNormalizedDifference;

		STRcorrectionMultiplier = 
			std::clamp(STRcorrectionMultiplier, 0.0f, ACT_ATT_I_L_MAX_DEFENCE_STR_READ_MULT);

		*defendersReadOnAttackersStrenght_ptr -= 
				ACT_ATT_I_L_BASE_DEFENCE_STR_READ_BASE * STRcorrectionMultiplier;

		*defendersReadOnAttackersStrenght_ptr =
					std::max(*defendersReadOnAttackersStrenght_ptr, 0.0f);

		//The fight has just begun, but already our men wonder: how long can this battle last?
		action_ptr->phaseTiming.total = AS::ATT_I_L_attackTime(attackSize);

		//Other than that, it's all done as usual:
		defaultPhaseEnd(action_ptr);
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
		
		assert(defences > 0); //should have ended otherwise

		//The score is stored in aux. Positive score: attacker winning. It's also a morale:
		float effectiveDefences = defences * (1 - action_ptr->details.processingAux);
		
		float* intensity_ptr = &(action_ptr->details.intensity);
		
		float effectiveAttack = (*intensity_ptr) * (1 + action_ptr->details.processingAux);

		//each tick we calculate the score for this round of fighting:
		float balancePoint = effectiveAttack / (effectiveAttack + effectiveDefences);
		//two prns are used to make the distribution a triangle centered on 0.5:
		float draw = (g_prnServer_ptr->getNext() + g_prnServer_ptr->getNext()) / 2;

		float scoreChange = balancePoint - draw; //this order preserves the meaning of score
		action_ptr->details.processingAux += scoreChange;

		//The, how many troops each side lost:
		float totalForces = defences + (*intensity_ptr);
		
		uint32_t referenceTime = AS::ATT_I_L_prepTime(totalForces);
		float timeImpact = (float)( (double)tickTenthsOfMs / (double)referenceTime );

		float totalLosses = totalForces * std::abs(scoreChange) * timeImpact;
	
		*intensity_ptr -= totalLosses * (draw / balancePoint);
		float defendersLosses = totalLosses * ( (1 - draw)/(1 - balancePoint) );

		//For the defender, we take away the guard first:
		if (defendersStrData_ptr->externalGuard >= defendersLosses) {
			defendersStrData_ptr->externalGuard -= defendersLosses;
		}
		else {
			defendersLosses -= defendersStrData_ptr->externalGuard;
			defendersStrData_ptr->externalGuard = 0;
			defendersStrData_ptr->current -= defendersLosses;
		}

		//Now we check if both sides still have troops:
		bool keepFighting = true;
		if ((defendersStrData_ptr->current < 0) || ( (*intensity_ptr) < 0)) { //both alive
			if ((defendersStrData_ptr->current < 0) && ((*intensity_ptr) < 0)) {//both dead

				action_ptr->details.processingAux = 0;
				keepFighting = false;
			}
			else if (defendersStrData_ptr->current < 0) { //defenders dead
				//Only score >= 0 makes sense, so:
				action_ptr->details.processingAux = 
					std::max(0.0f, action_ptr->details.processingAux);
				//Score == 0: "attackers were retreating and beat defenders"
			}
			else { //attackers dead
				//Same idea, but in reverse:
				action_ptr->details.processingAux = 
					std::min(0.0f, action_ptr->details.processingAux);
			}
		}

		//We can now deal with ticking:
		uint32_t tick = 
			(uint32_t)(keepFighting * tickTenthsOfMs * action_ptr->details.processingAux);
		
		if (keepFighting) {
			
			uint32_t timeAdvanced = defaultTick(tick, action_ptr);

			if (timeAdvanced == tick) { //the action still has time
				return tickTenthsOfMs; //so we let the control loop know that;
			}
		}
		
		//If we get here, either the figh has ended or the time is up. Either way:
		return 0; //to trigger the phase's onEnd.
	}

	void ATT_I_L_EffectEnd(actionData_t* action_ptr) {
		
		//First we deal with the defender:
		int defender = action_ptr->ids.target;
		auto defendersDecision_ptr = 
			&(g_agentDataControllers_ptr->LAdecision_ptr->getDirectDataPtr()->at(defender));

		int attackerID = action_ptr->ids.origin;
		auto defenderState_ptr = 
			&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(defender));

		int attackerIndexOnDefender = getNeighborsIndexOnLA(attackerID, defenderState_ptr);

		//They will get some general information according to the score, if positive
		float*  aux_ptr = &(action_ptr->details.processingAux);
		float infiltrationGain = 
							std::max(0.0f, (*aux_ptr) ) * ACT_ATT_I_L_BASE_DEFENDER_INFO_GAIN;

		defendersDecision_ptr->infiltration[attackerIndexOnDefender] +=
			                std::min(infiltrationGain, ACT_ATT_I_L_MAX_DEFENDER_INFO_GAIN);

		//And add any routing/returning troops to their expectations:
		int str = (int)LA::readsOnNeighbor_t::fields::STRENGHT;

		float* intensity_ptr = &(action_ptr->details.intensity);
		defendersDecision_ptr->reads[attackerIndexOnDefender].readOf[str] += (*intensity_ptr);

		//In order to set the return time, we will need:
		uint32_t* phaseTotal_ptr = &(action_ptr->phaseTiming.total);
		auto attackerState_ptr =
				&(g_agentDataControllers_ptr->LAstate_ptr->getDataCptr()->at(attackerID));
		
		AS::pos_t attackerPos = attackerState_ptr->locationAndConnections.position;
		AS::pos_t defenderPos = defenderState_ptr->locationAndConnections.position;

		//Then the attacker: has anyone survived?
		if ((*intensity_ptr) == 0) {
			//nope. Clear aux and set a time for the agent to realize their loss
			*aux_ptr = 0; //no survivors, no loot
			*phaseTotal_ptr = AS::ATT_I_L_travelTimeFromDistanceAndTroops(attackerPos,
														defenderPos, ACT_REF_STRENGHT);
		}
		else { //We have people alive
			//How long will their travel back take? 
			//Less people travel faster, but also the worse the losses, the slower we go
			
			//So, as an indicator of our losses, we use:
			*phaseTotal_ptr = ATT_I_L_returnTime(attackerPos, defenderPos, (*intensity_ptr),
				                                                         (*phaseTotal_ptr) );
			
			//Anyway, how much loot did they grab, losses and all? 
			//(zero on defeat, otherwise calculated from score and defender's resources)
			if ((*aux_ptr <= 0) || (*intensity_ptr <= 0)) {
				*aux_ptr = 0; //now storing the (non-existent) loot
			}
			else { //attacker won: there will be loot!
				//The better the score, the more loot each attacker manages to get
				//The more surviving attackers, the more hands o loot

				float resources = defenderState_ptr->parameters.resources.current;
				float effectiveDefendersResources = 
						resources * ACT_ATT_I_L_MIN_PROPORTIONAL_RESOURCE_RESERVE_FROM_LOOT;
				
				float carryCapactiy = 
					ACT_ATT_I_L_BASE_LOOT_PER_STR
								* (*intensity_ptr)
									* std::max((*aux_ptr), ACT_ATT_I_L_MAX_CARRY_MULTIPLIER);
																	
				*aux_ptr = std::min(effectiveDefendersResources, carryCapactiy);

				defenderState_ptr->parameters.resources.current -= (*aux_ptr);
			}

			//Looting damages stuff: lower their income.
			//(proportional to loot/defenders_resources, defenders_income, and a parameter)
			float originalResources = 
				defenderState_ptr->parameters.resources.current + (*aux_ptr);

			float lootProportion = (*aux_ptr) / originalResources;
			
			float unprotectedIncome = 
						defenderState_ptr->parameters.resources.updateRate
								* ACT_ATT_I_L_INCOME_PROPORTION_PROTECTED_FROM_LOOT;

			defenderState_ptr->parameters.resources.updateRate -=
														unprotectedIncome * lootProportion;
		}

		//Let the troops (or their ghosts) travel back home:
		defaultPhaseEnd(action_ptr);
		return;
	}

	void ATT_I_L_ReturnEnd(actionData_t* action_ptr) {

		//Did anyone survive? 
		if(action_ptr->details.intensity == 0) {

			//If not, just set next phases total time to zero and proceed
			action_ptr->phaseTiming.total = 0;
		}
		else { //In case there are survivors:
			
			float returnees = action_ptr->details.intensity;
			int agent = action_ptr->ids.origin;
			
			auto param_ptr = 
				&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(agent).parameters);
			
			//First of all, take any loot in:
			param_ptr->resources.current += action_ptr->details.processingAux;
		
			//Then we try to remember roughly how many people we had sent on the attack:
			auto attackerState_ptr = 
				&(g_agentDataControllers_ptr->LAstate_ptr->getDataCptr()->at(agent));

			int defender = action_ptr->ids.target;
			auto defenderState_ptr = 
				&(g_agentDataControllers_ptr->LAstate_ptr->getDataCptr()->at(defender));

			AS::pos_t attackerPos = attackerState_ptr->locationAndConnections.position;
			AS::pos_t defenderPos = defenderState_ptr->locationAndConnections.position;

			float estimatedAttackSize = 
				ATT_I_L_attackSizeFromIntensityAndReturnTime(attackerPos, defenderPos,
					     action_ptr->phaseTiming.total, action_ptr->details.intensity);
		
			//Then we gather information:
			//We get infiltration points proportional to the sqrt of the ratio of returnees:

			int neighbor = action_ptr->ids.target;
			auto neighborState_ptr = 
				&(g_agentDataControllers_ptr->LAstate_ptr->getDirectDataPtr()->at(neighbor));

			int neighborIDonAgent = getNeighborsIndexOnLA(agent, neighborState_ptr);

			auto decision_ptr = 
				&(g_agentDataControllers_ptr->LAdecision_ptr->getDirectDataPtr()->at(agent));

			float returneesFactor = sqrt(returnees / estimatedAttackSize);

			decision_ptr->infiltration[neighborIDonAgent] +=
								returneesFactor * ACT_ATT_I_L_BASE_INFO_FROM_RETURNEES;
			
			//And information specifically about their troops:
			//We'll interpolate our read with the real value using returneesFactor as weight:

			int str = (int)LA::readsOnNeighbor_t::fields::STRENGHT;
			decision_ptr->reads[neighborIDonAgent].readOf[str] *= (1 - returneesFactor);
			decision_ptr->reads[neighborIDonAgent].readOf[str] += 
					   returneesFactor * neighborState_ptr->parameters.strenght.current;

			int grd = (int)LA::readsOnNeighbor_t::fields::GUARD;
			decision_ptr->reads[neighborIDonAgent].readOf[grd] *= (1 - returneesFactor);
			decision_ptr->reads[neighborIDonAgent].readOf[grd] += 
					   returneesFactor * neighborState_ptr->parameters.strenght.externalGuard;
			
			//Finally we accept out troops back in the GUARD (to recover):
			param_ptr->strenght.externalGuard += returnees;

			//and set a recover time (proportional to the troops, and atenuated, use helper):
			action_ptr->phaseTiming.total = AS::ATT_I_L_prepTime(returnees);
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

			strenght_ptr->current += action_ptr->details.intensity;
			strenght_ptr->externalGuard -= action_ptr->details.intensity;
		}
		else { //maybe they died?
			strenght_ptr->current += strenght_ptr->externalGuard;
			strenght_ptr->externalGuard = 0;
		}

		defaultConclusionEnd(action_ptr);
	}
}