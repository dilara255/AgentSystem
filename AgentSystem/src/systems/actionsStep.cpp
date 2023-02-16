#include "systems/AScoordinator.hpp"
#include "data/actionData.hpp"
#include "network/parameters.hpp"

void processAction(AS::actionData_t action);

void AS::stepActions(ActionSystem* ap, int numberLAs, int numberGAs, float timeMultiplier) {
	
	std::vector<AS::actionData_t>* laActs;
	std::vector<AS::actionData_t>* gaActs;
	laActs = ap->data.getDirectLAdataPtr();
	gaActs = ap->data.getDirectGAdataPtr();

	int LAactions = ap->data.getMaxActionsPerAgent()*numberLAs;
	int GAactions = ap->data.getMaxActionsPerAgent()*numberGAs;

	for (int i = 0; i < LAactions; i++) {
		processAction(laActs->at(i));
	}

	for (int i = 0; i < GAactions; i++) {
		processAction(gaActs->at(i));
	}

}

void processAction(AS::actionData_t action) {
	//Create classes with commom base to deal with actions, dispatch from here to them
}