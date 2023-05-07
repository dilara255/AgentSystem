/******************************************************************************
* This file represents a thin control layer for action stepping
* The actual processing of the action steps is defined on actionProcessing.cpp
* 
* TODO: some pointers / globals are set here and again in actionProcessing.cpp
* --> consolidade both files (leaving AS::stepActions as the only entry-point)
******************************************************************************/

#include "systems/AScoordinator.hpp"
#include "data/actionData.hpp"
#include "systems/warningsAndErrorsCounter.hpp"

//TODO: the way I ended up organizing this makes these global variables redundant
static AS::WarningsAndErrorsCounter* g_errorsCounter_ptr = NULL;
static AS::dataControllerPointers_t* g_agentDataControllers_ptr = NULL;
static AS::PRNserver* g_prnServer_ptr = NULL;

void AS::stepActions(ActionSystem* ap, float timeMultiplier, 
					 WarningsAndErrorsCounter* errorsCounter_ptr, 
	                 dataControllerPointers_t* agentDataControllers_ptr,
	                 PRNserver* prnServer_ptr) {
	
	//First we check the system pointers:
	if ((ap == NULL) || (agentDataControllers_ptr == NULL) || (errorsCounter_ptr == NULL)
		|| (prnServer_ptr == NULL) ) {
			
		if(errorsCounter_ptr != NULL){
			errorsCounter_ptr->incrementError(AS::errors::AC_RECEIVED_BAD_SYSTEM_POINTERS);
			return;
		}
		else {
			//screw it, we'll have to log directly
			LOG_CRITICAL("Action Step found an error AND received bad error counter pointer!");
			return;
		}
	}
	//then we set them:
	g_errorsCounter_ptr = errorsCounter_ptr;
	g_agentDataControllers_ptr = agentDataControllers_ptr;
	g_prnServer_ptr = prnServer_ptr;

	//and finally we actually step the actions:
	ap->stepActions(ap, timeMultiplier);
}

namespace AS{

	//just a forward declaration for use in stepActions bellow
	void dispatchActionProcessing(actionData_t* action_ptr, float timeMultiplier,
								  dataControllerPointers_t* agentDataControllers_ptr,
								  ActionSystem* actionSystem_ptr, PRNserver* prnServer_ptr,
								  WarningsAndErrorsCounter* errors_ptr);
}

void AS::ActionSystem::stepActions(ActionSystem* ap, float timeMultiplier){

	std::vector<AS::actionData_t>* laActs = data.getDirectLAdataPtr();
	std::vector<AS::actionData_t>* gaActs = data.getDirectGAdataPtr();

	int LAactions = (int)laActs->size();
	int GAactions = (int)gaActs->size();

	for (int laActsIndex = 0; laActsIndex < LAactions; laActsIndex++) {

		AS::dispatchActionProcessing(&(laActs->at(laActsIndex)), timeMultiplier,
					g_agentDataControllers_ptr, ap, g_prnServer_ptr, g_errorsCounter_ptr);
	}

	for (int gaActsIndex = 0; gaActsIndex < GAactions; gaActsIndex++) {

		AS::dispatchActionProcessing(&(gaActs->at(gaActsIndex)), timeMultiplier,
			        g_agentDataControllers_ptr, ap, g_prnServer_ptr, g_errorsCounter_ptr);
	}
}