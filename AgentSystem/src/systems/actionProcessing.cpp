/***********************************************************************
* This file is renponsible for the ACTUAL PROCESSING OF ACTIONS
* For the spawining and initial detail setting of actions:
*		-> See actionPreProcessing.cpp
* 
* Each phase of each action variaton has two related functions:
* - tick, which updates the remaining phase time and applies any other continuous change;
* - endPhase, which applies any end-of-phase effects and sets the action to the next phase.
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
#include "systems/PRNserver.hpp"
#include "systems/warningsAndErrorsCounter.hpp"

#include "data/agentDataStructures.hpp"
#include "data/agentDataControllers.hpp"

//These are forward declarations of the actual processing functions
//(so they can be used on initializeProcessingFunctions, bellow)
//If this is your first time on this file, collapse this and start at the next section : )
namespace AS {
	
	void initializeProcessingFunctions();

	void defaultOnSpawn(actionData_t* action_ptr);
	uint32_t defaultTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr);
	void defaultPhaseEnd(actionData_t* action_ptr);

	void defaultPrepEnd(actionData_t* action_ptr);
	void defaultTravelEnd(actionData_t* action_ptr);
	void defaultEffectEnd(actionData_t* action_ptr);
	void defaultReturnEnd(actionData_t* action_ptr);
	void defaultConclusionEnd(actionData_t* action_ptr);
}

//Here's the meat of this file: entry-point, initialization and processing actions
namespace AS {
	
	static ActionVariations::actionProcessingFunctions_t g_processingFunctions[(int)AS::scope::TOTAL];
	static bool g_processingFunctionsInitialized = false;

	static dataControllerPointers_t* agentDataControllers_ptr = NULL;
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
		agentDataControllers_ptr = agentDataControllers_ptr;
		g_actionSystem_ptr = actionSystem_ptr;
		g_prnServer_ptr = prnServer_ptr;
		g_errors_ptr = errors_ptr;		

		//we do need to make sure the processing functions are initialized:
		if (!g_processingFunctionsInitialized) {

			initializeProcessingFunctions();
			if (!g_processingFunctionsInitialized) {
				return;
			}
		}

		//timeMultiplier is in seconds, and has to be changed into a uint32_tenthsOfMilli_t:
		double tenthsOfMillisThisStep = (double)timeMultiplier * (double)TENTHS_OF_MS_IN_A_SECOND;
		uint32_tenthsOfMilli_t timeElapsedThisStep = (uint32_t)tenthsOfMillisThisStep;	
		
		//Finally, we dispatch the action processing:
		int cat = action_ptr->ids.category;
		int mode = action_ptr->ids.mode;
		int scope = action_ptr->ids.scope;
		uint32_tenthsOfMilli_t timeRemainingToProcess = timeElapsedThisStep;

		//first of all, let's check if the action just spawned and, if so, call it's onSpawn:
		if (action_ptr->ids.phase == (int)actPhases::SPAWN) {

			g_processingFunctions[scope][cat][mode].onSpawn(action_ptr);
			action_ptr->ids.phase = 0;
		}

		int phase = action_ptr->ids.phase;

		//Since a phase may end and spawn another phase, this is done in a loop:
		while ( (timeRemainingToProcess > 0)
			     && (action_ptr->ids.active == 1) && (action_ptr->ids.slotIsUsed == 1) ) {

			assert(phase < (int)actPhases::TOTAL);

			//first we tick the action and receive any time still to be processed:
			timeRemainingToProcess = 
				g_processingFunctions[scope][cat][mode].onTick[phase](timeRemainingToProcess,
					                                                              action_ptr);
			//if that is larger then zero, we've reached a phase end and must process that
			if (timeRemainingToProcess > 0) {
				//which will do any end-of-phase effect and advance phase or deactivate action
				g_processingFunctions[scope][cat][mode].onEnd[phase](action_ptr);
				//since there's timeRemainingToProcess, we'll loop after this
			}
		}	
		
		//Done, the action was processed until it consumed all the step time or was deactivated
	}

	void setProcessingFunctionsToDefaults();

	//This is where the functions used for each phase of each action variation
	//are selected. Changing this will change their behavior.
	void initializeProcessingFunctions() {

		setProcessingFunctionsToDefaults();		

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

	void defaultOnSpawn(actionData_t* action_ptr) {

	}

	uint32_t defaultTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

		return 0;
	}

	void defaultPhaseEnd(actionData_t* action_ptr) {

		action_ptr->ids.phase++;
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
}