#include "systems/AScoordinator.hpp"
#include "data/actionData.hpp"
#include "network/parameters.hpp"

void processAction(AS::actionData_t action);

void AS::stepActions(ActionSystem* ap, int numLAs, int numGAs, float timeMultiplier) {
	
	int totalProccessed = ap->stepActions(timeMultiplier);
	int totalExpected = numLAs + numGAs;
	bool result = (totalExpected == totalProccessed);
}

int AS::ActionSystem::stepActions(float timeMultiplier){

	std::vector<AS::actionData_t>* laActs = data.getDirectLAdataPtr();
	std::vector<AS::actionData_t>* gaActs = data.getDirectGAdataPtr();

	int LAactions = laActs->size();
	int GAactions = gaActs->size();

	int laActsIndex;
	for (laActsIndex = 0; laActsIndex < LAactions; laActsIndex++) {
		processAction(laActs->at(laActsIndex));
	}

	int gaActsIndex;
	for (gaActsIndex = 0; gaActsIndex < GAactions; gaActsIndex++) {
		processAction(gaActs->at(gaActsIndex));
	}

	return (laActsIndex + gaActsIndex);
}

void processAction(AS::actionData_t action) {
	//Create classes with commom base to deal with actions, dispatch from here to them
}