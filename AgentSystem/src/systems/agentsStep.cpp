#include "systems/AScoordinator.hpp"
#include "systems/diplomacy.hpp"

void updateLA(LA::stateData_t* state_ptr, int agentId, 
	          AS::dataControllerPointers_t* agentDataPtrs_ptr, float timeMultiplier);
void updateGA(GA::stateData_t* state_ptr, int agentId, 
	          AS::dataControllerPointers_t* agentDataPtrs_ptr, float timeMultiplier);

void makeDecisionLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void makeDecisionGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);


void AS::stepAgents(int LAdecisionsToTakeThisChop, int GAdecisionsToTakeThisChop, 
	                          dataControllerPointers_t* dp, float timeMultiplier,
	                                                int numberLAs, int numberGAs) {

	auto LAstateData_ptr = dp->LAstate_ptr->getDirectDataPtr();
	
	for (int i = 0; i < numberLAs; i++) {	
		updateLA(&LAstateData_ptr->at(i), i, dp, timeMultiplier);
	}
	
	static int nextDecisionLAindex = 0;	
	if(nextDecisionLAindex >= numberLAs) nextDecisionLAindex = 0;
	int finalDecisionLAindex = nextDecisionLAindex + LAdecisionsToTakeThisChop - 1;

	for (int i = nextDecisionLAindex; i <= finalDecisionLAindex; i++) {	
		makeDecisionLA(i, dp);
	}
	
	auto GAstateData_ptr = dp->GAstate_ptr->getDirectDataPtr();

	for (int i = 0; i < numberGAs; i++) {	
		updateGA(&GAstateData_ptr->at(i), i, dp, timeMultiplier);
	}
	
	static int nextDecisionGAindex = 0;	
	if(nextDecisionGAindex >= numberGAs) nextDecisionGAindex = 0;
	int finalDecisionGAindex = nextDecisionGAindex + GAdecisionsToTakeThisChop - 1;

	for (int i = nextDecisionGAindex; i <= finalDecisionGAindex; i++) {	
		makeDecisionGA(i, dp);
	}
}

void updateLA(LA::stateData_t* state_ptr, int agentId, 
	          AS::dataControllerPointers_t* dp, float timeMultiplier) {
	
	if (state_ptr->onOff == false) {
		return;
	}

	auto res_ptr = &state_ptr->parameters.resources;
	auto str_ptr = &state_ptr->parameters.strenght;

	//External guard costs a portion of upkeep for each agent:
	float externalUpkeep = str_ptr->externalGuard*EXTERNAL_GUARD_UPKEEP_RATIO_BY_DEFENDED;
	//upkeep is linear with strenght above trheshold:
	str_ptr->currentUpkeep = str_ptr->current + externalUpkeep - str_ptr->thresholdToCostUpkeep;
	//and proportional to a factor:
	str_ptr->currentUpkeep *= LA_UPKEEP_PER_EXCESS_STRENGHT;
	//and never smaller then zero : )
	if(str_ptr->currentUpkeep < 0) {str_ptr->currentUpkeep = 0;}

	res_ptr->current += (res_ptr->updateRate - str_ptr->currentUpkeep)*timeMultiplier;
	
	int quantityNeighbours = state_ptr->locationAndConnections.numberConnectedNeighbors;
	for (int i = 0; i < quantityNeighbours; i++) {
		AS::diploStance stance = state_ptr->relations.diplomaticStanceToNeighbors[i];

		if ((stance == AS::diploStance::TRADE) ||
		    (stance == AS::diploStance::ALLY_WITH_TRADE)) {
			int partnerID = state_ptr->locationAndConnections.neighbourIDs[i];
			res_ptr->current += 
				LA::calculateTradeIncomePerSecond(partnerID, stance, dp)*timeMultiplier;
		}

		if ((stance == AS::diploStance::WAR)) {
			int partnerID = state_ptr->locationAndConnections.neighbourIDs[i];
			str_ptr->current -= 
				LA::calculateAttritionLossesPerSecond(agentId, partnerID, dp)*timeMultiplier;
		}
	}

	//finally, LAs pay tax to GA (and can cost the GA if in debt):
	res_ptr->current -= GA_TAX_RATE_PER_SECOND*res_ptr->current;
}

void updateGA(GA::stateData_t* state_ptr, int agentId, 
	          AS::dataControllerPointers_t* dp, float timeMultiplier) {

	if (state_ptr->onOff == false) {
		return;
	}
	int quantityLAs = state_ptr->localAgentsBelongingToThis.howManyAreOn();
	auto param_ptr = &state_ptr->parameters;
	param_ptr->LAesourceTotals.current = 0;
	param_ptr->LAesourceTotals.updateRate = 0;
	param_ptr->LAstrenghtTotal = 0;

	//TODO-CRITICAL: GAs need to KEEP IDs of their LAs (and initialize that on load)
	for (int i = 0; i < quantityLAs; i++) {
		param_ptr->LAesourceTotals.current += 0;
		param_ptr->LAesourceTotals.updateRate += 0;
		param_ptr->LAstrenghtTotal += 0;
	}
	
	float taxIncome = GA_TAX_RATE_PER_SECOND*state_ptr->parameters.LAesourceTotals.current;
	
	param_ptr->GAresources += taxIncome*timeMultiplier;
	int quantityNeighbours = state_ptr->connectedGAs.howManyAreOn();
	for (int i = 0; i < quantityNeighbours; i++) {
		//TODO-CRITICAL: wrong (should use neighbourIDs[i]
		AS::diploStance stance = state_ptr->relations.diplomaticStanceToNeighbors[i];

		if ((stance == AS::diploStance::TRADE) ||
		    (stance == AS::diploStance::ALLY_WITH_TRADE)) {
			//TODO-CRITICAL: GA neighbour IDs!
			int partnerID = 0;
			param_ptr->GAresources += 
				GA::calculateTradeIncomePerSecond(partnerID, stance, dp)*timeMultiplier;
		}
	}
}


void calculateNotionsLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void preScoreActionsLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void redistributeScoreDueToImpedimmentsLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void chooseActionLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);

void makeDecisionLA(int agent, AS::dataControllerPointers_t* dp) {
	calculateNotionsLA(agent, dp);
	preScoreActionsLA(agent, dp);
	redistributeScoreDueToImpedimmentsLA(agent, dp);
	chooseActionLA(agent, dp);
}

void calculateNotionsGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void preScoreActionsGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void redistributeScoreDueToImpedimmentsGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void chooseActionGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);

void makeDecisionGA(int agent, AS::dataControllerPointers_t* dp) {
	calculateNotionsGA(agent, dp);
	preScoreActionsGA(agent, dp);
	redistributeScoreDueToImpedimmentsGA(agent, dp);
	chooseActionGA(agent, dp);
}

void calculateNotionsLA(int agent, AS::dataControllerPointers_t* dp) {

}

void preScoreActionsLA(int agent, AS::dataControllerPointers_t* dp) {

}

void redistributeScoreDueToImpedimmentsLA(int agent, AS::dataControllerPointers_t* dp) {

}

void chooseActionLA(int agent, AS::dataControllerPointers_t* dp) {

}

void calculateNotionsGA(int agent, AS::dataControllerPointers_t* dp) {

}

void preScoreActionsGA(int agent, AS::dataControllerPointers_t* dp) {

}

void redistributeScoreDueToImpedimmentsGA(int agent, AS::dataControllerPointers_t* dp) {

}

void chooseActionGA(int agent, AS::dataControllerPointers_t* dp) {

}
