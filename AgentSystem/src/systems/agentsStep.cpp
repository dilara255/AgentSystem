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

	std::vector<LA::stateData_t>* LAstateData_ptr = dp->LAstate_ptr->getDirectDataPtr();
	
	for (int i = 0; i < numberLAs; i++) {	
		updateLA(&LAstateData_ptr->at(i), i, dp, timeMultiplier);
	}
	
	static int nextDecisionLAindex = 0;	
	if(nextDecisionLAindex >= numberLAs) nextDecisionLAindex = 0;

	for (int i = nextDecisionLAindex; i < LAdecisionsToTakeThisChop; i++) {	
		makeDecisionLA(i, dp);
	}
	
	std::vector<GA::stateData_t>* GAstateData_ptr = dp->GAstate_ptr->getDirectDataPtr();

	//Remember last GA doesn't count 
	//TODO-CRITICAL: FIX: should store "numberEffectiveGAs" or something, and fix everywhere
	for (int i = 0; i < (numberGAs - 1); i++) {	
		updateGA(&GAstateData_ptr->at(i), i, dp, timeMultiplier);
	}
	
	static int nextDecisionGAindex = 0;	
	if(nextDecisionGAindex >= (numberGAs-1)) nextDecisionGAindex = 0;

	for (int i = nextDecisionGAindex; i < GAdecisionsToTakeThisChop; i++) {	
		makeDecisionGA(i, dp);
	}
}

void updateLA(LA::stateData_t* state_ptr, int agentId, 
	          AS::dataControllerPointers_t* dp, float timeMultiplier) {
	
	if (state_ptr->onOff == false) {
		return;
	}

	AS::resources_t* res_ptr = &state_ptr->parameters.resources;
	AS::strenght_t* str_ptr = &state_ptr->parameters.strenght;

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
				LA::calculateTradeIncome(partnerID, stance, dp)*timeMultiplier;
		}

		if ((stance == AS::diploStance::WAR)) {
			int partnerID = state_ptr->locationAndConnections.neighbourIDs[i];
			str_ptr->current -= 
				LA::calculateAttritionLosses(agentId, partnerID, dp)*timeMultiplier;
		}
	}
}

void updateGA(GA::stateData_t* state_ptr, int agentId, 
	          AS::dataControllerPointers_t* dp, float timeMultiplier) {

	//placeholder
	state_ptr->parameters.GAresources += 1*timeMultiplier;
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
