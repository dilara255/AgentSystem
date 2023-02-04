#include "systems/AScoordinator.hpp"

void updateLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void updateGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void makeDecisionLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void makeDecisionGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);

void AS::stepAgents(bool shouldMakeDecisions, dataControllerPointers_t* dp,
	                                          int numberLAs, int numberGAs) {
	
	for (int i = 0; i < numberLAs; i++) {
		updateLA(i, dp);
		if(shouldMakeDecisions) makeDecisionLA(i, dp);
	}
	
	for (int i = 0; i < numberGAs; i++) {
		updateGA(i, dp);
		if(shouldMakeDecisions) makeDecisionGA(i, dp);
	}
}

void updateLA(int agent, AS::dataControllerPointers_t* dp) {

}

void updateGA(int agent, AS::dataControllerPointers_t* dp) {

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
