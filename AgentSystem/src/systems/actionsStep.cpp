#include "systems/AScoordinator.hpp"
#include "data/actionData.hpp"
#include "systems/warningsAndErrorsCounter.hpp"
#include "network/parameters.hpp"

static AS::WarningsAndErrorsCounter* g_errorsCounter_ptr;

bool processAction(AS::actionData_t action);

void AS::stepActions(ActionSystem* ap, int numLAs, int numGAs, float timeMultiplier,
		             WarningsAndErrorsCounter* errorsCounter_ptr) {
	
	g_errorsCounter_ptr = errorsCounter_ptr;

	int totalProccessed = ap->stepActions(timeMultiplier);
	int totalExpected = numLAs + numGAs;
	bool result = (totalExpected == totalProccessed);
}

int AS::ActionSystem::stepActions(float timeMultiplier){

	std::vector<AS::actionData_t>* laActs = data.getDirectLAdataPtr();
	std::vector<AS::actionData_t>* gaActs = data.getDirectGAdataPtr();

	int LAactions = (int)laActs->size();
	int GAactions = (int)gaActs->size();

	int laActsIndex;
	for (laActsIndex = 0; laActsIndex < LAactions; laActsIndex++) {
		bool result = processAction(laActs->at(laActsIndex));
		if(result == false) {
			g_errorsCounter_ptr->incrementError(AS::errors::AC_FAILED_PROCESS_LA_ACT);
		}
	}

	int gaActsIndex;
	for (gaActsIndex = 0; gaActsIndex < GAactions; gaActsIndex++) {
		bool result = processAction(gaActs->at(gaActsIndex));
		if(result == false) {
			g_errorsCounter_ptr->incrementError(AS::errors::AC_FAILED_PROCESS_GA_ACT);
		}
	}

	return (laActsIndex + gaActsIndex);
}

bool processAction(AS::actionData_t action) {
	//Create classes with commom base to deal with actions, dispatch from here to them

	return true;
}