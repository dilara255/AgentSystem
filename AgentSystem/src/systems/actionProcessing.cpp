/***********************************************************************
* This file is renponsible for the ACTUAL PROCESSING OF ACTIONS
* For the spawining and initial detail setting of actions:
*		-> See actionPreProcessing.cpp
* 
* Each phase of each action variaton has two related functions:
* - tick, which updates the remaining time and applies any other continuous change;
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

#include "systems/actionSystem.hpp"
#include "data/agentDataStructures.hpp"

#include "data/agentDataControllers.hpp"

#include "systems/warningsAndErrorsCounter.hpp"

//These are forward declarations of the actual processing functions
//(so they can be used on initializeProcessingFunctions, bellow)
//If this is your first time on this file, collapse this and start at the next section : )
namespace AS {
	
	void initializeProcessingFunctions();

	void defaultOnSpawn(actionData_t* action_ptr);
	uint32_t defaultTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr);
	void defaultPhaseEnd(uint32_t tickTenthsOfMs, actionData_t* action_ptr);

	void defaultPrepEnd(uint32_t tickTenthsOfMs, actionData_t* action_ptr);
	void defaultTravelEnd(uint32_t tickTenthsOfMs, actionData_t* action_ptr);
	void defaultEffectEnd(uint32_t tickTenthsOfMs, actionData_t* action_ptr);
	void defaultReturnEnd(uint32_t tickTenthsOfMs, actionData_t* action_ptr);
	void defaultConclusionEnd(uint32_t tickTenthsOfMs, actionData_t* action_ptr);
}

//Here's the meat of this file: entry-point, initialization and processing actions
namespace AS {

	static ActionVariations::variationProcessingFunctions_t g_processingFunctions;
	static bool g_processingFunctionsInitialized = false;

	static dataControllerPointers_t* agentDataControllers_ptr = NULL;
	static ActionSystem* g_actionSystem_ptr = NULL; 
	static WarningsAndErrorsCounter* g_errors_ptr = NULL;

	//This is the ONLY entrypoint for this file.
	//Here we set the pointers to the system's data (globals on this file and scope).
	//After that, we forward the action to the appropriate function.
	//In case the processing actions structure is not initialized, this also initializes it.
	void dispatchActionProcessing(actionData_t* action_ptr, uint32_t timeElapsed,
								  dataControllerPointers_t* agentDataControllers_ptr,
								  ActionSystem* actionSystem_ptr,
								  WarningsAndErrorsCounter* errors_ptr) {

		//First we check and set the system pointers:
		if ((action_ptr == NULL) || (actionSystem_ptr == NULL)
			|| (agentDataControllers_ptr == NULL) || (errors_ptr == NULL)) {
			
			//taca erro
			return;
		}
		agentDataControllers_ptr = agentDataControllers_ptr;
		g_actionSystem_ptr = actionSystem_ptr;
		g_errors_ptr = errors_ptr;

		//Then we make sure the processing functions are initialized:
		if (!g_processingFunctionsInitialized) {

			initializeProcessingFunctions();
			if (!g_processingFunctionsInitialized) {
				return;
			}
		}

		//Finally, we dispatch the action processing 
		//(loop pq pode ir várias fases de uma vez)
	}

	//This is where the functions used for each phase of each action variation
	//are selected. Changing this will change their behavior.
	void initializeProcessingFunctions() {

		//initialize
	}


	void defaultOnSpawn(actionData_t* action_ptr) {

	}

	uint32_t defaultTick(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

		return 0;
	}

	void defaultPhaseEnd(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

	}

	void defaultPrepEnd(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

		defaultPhaseEnd(tickTenthsOfMs, action_ptr);
	}

	void defaultTravelEnd(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

		defaultPhaseEnd(tickTenthsOfMs, action_ptr);
	}

	void defaultEffectEnd(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

		defaultPhaseEnd(tickTenthsOfMs, action_ptr);
	}

	void defaultReturnEnd(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

		defaultPhaseEnd(tickTenthsOfMs, action_ptr);
	}

	void defaultConclusionEnd(uint32_t tickTenthsOfMs, actionData_t* action_ptr) {

		defaultPhaseEnd(tickTenthsOfMs, action_ptr);
	}
}