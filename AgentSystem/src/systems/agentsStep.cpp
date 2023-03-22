#include "systems/AScoordinator.hpp"
#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/diplomacy.hpp"

namespace {
	static AS::WarningsAndErrorsCounter* g_errorsCounter_ptr;

	void updateLA(LA::stateData_t* state_ptr, int agentId, 
				  AS::dataControllerPointers_t* agentDataPtrs_ptr, float timeMultiplier,
		                                AS::WarningsAndErrorsCounter* errorsCounter_ptr);
	void updateGA(GA::stateData_t* state_ptr, int agentId, 
				  AS::dataControllerPointers_t* agentDataPtrs_ptr, float timeMultiplier,
		                                AS::WarningsAndErrorsCounter* errorsCounter_ptr);

	void makeDecisionLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
	void makeDecisionGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);

	LA::readsOnNeighbor_t calculateLAreferences(int agentId, AS::dataControllerPointers_t* dp);
	GA::readsOnNeighbor_t calculateGAreferences(int agentId, AS::dataControllerPointers_t* dp);
	void updateRead(float* read, float real, float reference, float infiltration);
}

void AS::stepAgents(int LAdecisionsToTakeThisChop, int GAdecisionsToTakeThisChop, 
	                          dataControllerPointers_t* dp, float timeMultiplier,
	                                                int numberLAs, int numberGAs,
		                             WarningsAndErrorsCounter* errorsCounter_ptr) {

	g_errorsCounter_ptr = errorsCounter_ptr;

	//update LAs and GAs:
	auto LAstateData_ptr = dp->LAstate_ptr->getDirectDataPtr();
	if (LAstateData_ptr == NULL) {
		g_errorsCounter_ptr->incrementError(errors::AS_LA_STATE_PTR_NULL);
	}
	
	for (int i = 0; i < numberLAs; i++) {	
		updateLA(&LAstateData_ptr->at(i), i, dp, timeMultiplier, errorsCounter_ptr);
	}

	auto GAstateData_ptr = dp->GAstate_ptr->getDirectDataPtr();
	if (GAstateData_ptr == NULL) {
		g_errorsCounter_ptr->incrementError(errors::AS_GA_STATE_PTR_NULL);
	}

	for (int i = 0; i < numberGAs; i++) {
		updateGA(&GAstateData_ptr->at(i), i, dp, timeMultiplier, errorsCounter_ptr);
	}
	
	//Make decisions:
	static int nextDecisionLAindex = 0;	
	if(nextDecisionLAindex >= numberLAs) { nextDecisionLAindex = 0; }

	int finalDecisionLAindex = nextDecisionLAindex + LAdecisionsToTakeThisChop - 1;
	for (; nextDecisionLAindex <= finalDecisionLAindex; nextDecisionLAindex++) {	
		makeDecisionLA(nextDecisionLAindex, dp);
	}
		
	static int nextDecisionGAindex = 0;	
	if(nextDecisionGAindex >= numberGAs){ nextDecisionGAindex = 0; }

	int finalDecisionGAindex = nextDecisionGAindex + GAdecisionsToTakeThisChop - 1;
	for (; nextDecisionGAindex <= finalDecisionGAindex; nextDecisionGAindex++) {	
		makeDecisionGA(nextDecisionGAindex, dp);
	}
}

namespace {
void updateLA(LA::stateData_t* state_ptr, int agentId, 
	          AS::dataControllerPointers_t* dp, float timeMultiplier,
	                 AS::WarningsAndErrorsCounter* errorsCounter_ptr) {
	
	if (state_ptr->onOff == false) {
		return;
	}

	auto res_ptr = &state_ptr->parameters.resources;
	auto str_ptr = &state_ptr->parameters.strenght;

	//External guard costs a portion of upkeep for each agent:
	float externalUpkeep = (float)(str_ptr->externalGuard*EXTERNAL_GUARD_UPKEEP_RATIO_BY_DEFENDED);
	//upkeep is linear with strenght above trheshold:
	str_ptr->currentUpkeep = str_ptr->current + externalUpkeep - str_ptr->thresholdToCostUpkeep;
	//and proportional to a factor:
	str_ptr->currentUpkeep *= LA_UPKEEP_PER_EXCESS_STRENGHT;
	//and never smaller then zero : )
	str_ptr->currentUpkeep = std::max(0.0f, str_ptr->currentUpkeep);

	res_ptr->current += (res_ptr->updateRate - str_ptr->currentUpkeep)*timeMultiplier;
	
	int quantityNeighbours = state_ptr->locationAndConnections.numberConnectedNeighbors;
	for (int i = 0; i < quantityNeighbours; i++) {
		AS::diploStance stance = state_ptr->relations.diplomaticStanceToNeighbors[i];

		if ((stance == AS::diploStance::TRADE) ||
		    (stance == AS::diploStance::ALLY_WITH_TRADE)) {
			int partnerID = state_ptr->locationAndConnections.neighbourIDs[i];
			res_ptr->current += 
				LA::calculateTradeIncomePerSecond(partnerID, stance, dp, errorsCounter_ptr)*timeMultiplier;
		}

		if ((stance == AS::diploStance::WAR)) {
			int partnerID = state_ptr->locationAndConnections.neighbourIDs[i];
			str_ptr->current -= 
				LA::calculateAttritionLossesPerSecond(agentId, partnerID, dp)*timeMultiplier;
		}
	}

	//finally, LAs pay tax to GA (and can cost the GA if in debt):
	res_ptr->current -= (float)GA_TAX_RATE_PER_SECOND*res_ptr->current*timeMultiplier;
}

void updateGA(GA::stateData_t* state_ptr, int agentId, 
	          AS::dataControllerPointers_t* dp, float timeMultiplier,
	                 AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

	if (state_ptr->onOff == false) {
		return;
	}

	//Update LA totals
	int connectedLAs = state_ptr->localAgentsBelongingToThis.howManyAreOn();
	auto param_ptr = &state_ptr->parameters;
	param_ptr->LAesourceTotals.current = 0;
	param_ptr->LAesourceTotals.updateRate = 0;
	param_ptr->LAstrenghtTotal = 0;
	auto LAstates_cptr = dp->LAstate_ptr->getDataCptr();

	for (int i = 0; i < connectedLAs; i++) {
		int id = state_ptr->laIDs[i];
		
		param_ptr->LAesourceTotals.current += 
			LAstates_cptr->at(id).parameters.resources.current;
		param_ptr->LAesourceTotals.updateRate += 
			LAstates_cptr->at(id).parameters.resources.updateRate;
		param_ptr->LAstrenghtTotal += 
			LAstates_cptr->at(id).parameters.strenght.current;
	}
	
	//Get resoures from tax...
	param_ptr->lastTaxIncome = (float)GA_TAX_RATE_PER_SECOND*state_ptr->parameters.LAesourceTotals.current;
	param_ptr->GAresources += param_ptr->lastTaxIncome*timeMultiplier;

	//... and from trade:
	int quantityNeighbours = state_ptr->connectedGAs.howManyAreOn();
	param_ptr->lastTradeIncome = 0;

	for (int i = 0; i < quantityNeighbours; i++) {
		int idOther = state_ptr->neighbourIDs[i];
		AS::diploStance stance = state_ptr->relations.diplomaticStanceToNeighbors[idOther];

		if ((stance == AS::diploStance::TRADE) ||
		    (stance == AS::diploStance::ALLY_WITH_TRADE)) {
			param_ptr->lastTradeIncome +=
				GA::calculateTradeIncomePerSecond(idOther, stance, dp, errorsCounter_ptr)*timeMultiplier;
		}
	}

	param_ptr->GAresources += param_ptr->lastTradeIncome;
}

void updateReadsLA(int agent, AS::dataControllerPointers_t* dp, LA::stateData_t* state_ptr);
void calculateNotionsLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void preScoreActionsLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void redistributeScoreDueToImpedimmentsLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void chooseActionLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);


void makeDecisionLA(int agent, AS::dataControllerPointers_t* dp) {

	LA::stateData_t* state_ptr = &(dp->LAstate_ptr->getDirectDataPtr()->at(agent));
	
	updateReadsLA(agent, dp, state_ptr);
	
	calculateNotionsLA(agent, dp);
	preScoreActionsLA(agent, dp);
	redistributeScoreDueToImpedimmentsLA(agent, dp);
	chooseActionLA(agent, dp);
}


void updateReadsGA(int agent, AS::dataControllerPointers_t* dp, GA::stateData_t* state_ptr);
void calculateNotionsGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void preScoreActionsGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void redistributeScoreDueToImpedimmentsGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void chooseActionGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);


void makeDecisionGA(int agent, AS::dataControllerPointers_t* dp) {

	GA::stateData_t* state_ptr = &(dp->GAstate_ptr->getDirectDataPtr()->at(agent));

	updateReadsGA(agent, dp, state_ptr);

	calculateNotionsGA(agent, dp);
	preScoreActionsGA(agent, dp);
	redistributeScoreDueToImpedimmentsGA(agent, dp);
	chooseActionGA(agent, dp);
}


//LA:
void calculateNotionsLA(int agent, AS::dataControllerPointers_t* dp) {

}

void preScoreActionsLA(int agent, AS::dataControllerPointers_t* dp) {

}

void redistributeScoreDueToImpedimmentsLA(int agent, AS::dataControllerPointers_t* dp) {

}

void chooseActionLA(int agent, AS::dataControllerPointers_t* dp) {

}

//GA:
void calculateNotionsGA(int agent, AS::dataControllerPointers_t* dp) {

}

void preScoreActionsGA(int agent, AS::dataControllerPointers_t* dp) {

}

void redistributeScoreDueToImpedimmentsGA(int agent, AS::dataControllerPointers_t* dp) {

}

void chooseActionGA(int agent, AS::dataControllerPointers_t* dp) {

}



//READS AND EXPECTATIONS:

LA::readsOnNeighbor_t getRealValuesLA(int agent, AS::dataControllerPointers_t* dp, int neighbor);
GA::readsOnNeighbor_t getRealValuesGA(int agent, AS::dataControllerPointers_t* dp, int neighbor);

void updateReadsLA(int agent, AS::dataControllerPointers_t* dp, LA::stateData_t* state_ptr) {

	LA::readsOnNeighbor_t referenceReads =  calculateLAreferences(agent, dp);

	auto decisionData_ptr = &(dp->LAdecision_ptr->getDirectDataPtr()->at(agent));
	LA::readsOnNeighbor_t* reads_ptr = &(decisionData_ptr->reads[0]);

	int totalNeighbors = state_ptr->locationAndConnections.numberConnectedNeighbors;

	for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {

		LA::readsOnNeighbor_t realValues = getRealValuesLA(agent, dp, neighbor);
		float infiltration = decisionData_ptr->infiltration[neighbor];

		for (int field = 0; field < (int)LA::readsOnNeighbor_t::fields::TOTAL; field++) {

			//this is the payload:
			updateRead(&reads_ptr->readOf[field], realValues.readOf[field], 
		                        referenceReads.readOf[field], infiltration);
		}
	}	
}

void updateReadsGA(int agent, AS::dataControllerPointers_t* dp, GA::stateData_t* state_ptr) {

	GA::readsOnNeighbor_t referenceReads =  calculateGAreferences(agent, dp);

	auto decisionData_ptr = &(dp->GAdecision_ptr->getDirectDataPtr()->at(agent));
	GA::readsOnNeighbor_t* reads_ptr = &(decisionData_ptr->reads[0]);

	int totalNeighbors = state_ptr->connectedGAs.howManyAreOn();

	for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {

		GA::readsOnNeighbor_t realValues = getRealValuesGA(agent, dp, neighbor);
		float infiltration = decisionData_ptr->infiltration[neighbor];

		for (int field = 0; field < (int)GA::readsOnNeighbor_t::fields::TOTAL; field++) {

			//this is the payload:
			updateRead(&reads_ptr->readOf[field], realValues.readOf[field], 
		                        referenceReads.readOf[field], infiltration);
		}
	}	
}

//TODO: REFACTOR: this should be inside some class? Or at least in another more sensible file
LA::readsOnNeighbor_t getRealValuesLA(int agent, AS::dataControllerPointers_t* dp, int neighbor) {
	LA::readsOnNeighbor_t real;

	auto agentState_ptr = &(dp->LAstate_ptr->getDirectDataPtr()->at(agent));
	int neighborID = agentState_ptr->locationAndConnections.neighbourIDs[neighbor];
	auto neighborState_ptr = &(dp->LAstate_ptr->getDirectDataPtr()->at(neighborID));

	real.readOf[(int)LA::readsOnNeighbor_t::fields::GUARD] = 
						neighborState_ptr->parameters.strenght.externalGuard;
	real.readOf[(int)LA::readsOnNeighbor_t::fields::INCOME] = 
						neighborState_ptr->parameters.resources.updateRate;
	real.readOf[(int)LA::readsOnNeighbor_t::fields::RESOURCES] = 
						neighborState_ptr->parameters.resources.current;
	real.readOf[(int)LA::readsOnNeighbor_t::fields::STRENGHT] = 
						neighborState_ptr->parameters.strenght.current;
	
	return real;
}

//TODO: REFACTOR: this should be inside some class? Or at least in another more sensible file
GA::readsOnNeighbor_t getRealValuesGA(int agent, AS::dataControllerPointers_t* dp, int neighbor) {
	GA::readsOnNeighbor_t real;

	auto agentState_ptr = &(dp->GAstate_ptr->getDirectDataPtr()->at(agent));
	int neighborID = agentState_ptr->neighbourIDs[neighbor];
	auto neighborState_ptr = &(dp->GAstate_ptr->getDirectDataPtr()->at(neighborID));

	real.readOf[(int)GA::readsOnNeighbor_t::fields::GA_RESOURCES] = 
						neighborState_ptr->parameters.GAresources;
	real.readOf[(int)GA::readsOnNeighbor_t::fields::GUARD_LAS] = 
						neighborState_ptr->parameters.LAguardTotal;
	real.readOf[(int)GA::readsOnNeighbor_t::fields::STRENGHT_LAS] = 
						neighborState_ptr->parameters.LAstrenghtTotal;
	real.readOf[(int)GA::readsOnNeighbor_t::fields::TAX_INCOME] = 
						neighborState_ptr->parameters.lastTaxIncome;
	real.readOf[(int)GA::readsOnNeighbor_t::fields::TRADE_INCOME] = 
						neighborState_ptr->parameters.lastTradeIncome;
	
	return real;
}


//TODO: REFACTOR: this should be inside some class? Or at least in another more sensible file
LA::readsOnNeighbor_t calculateLAreferences(int agentId, AS::dataControllerPointers_t* dp) {
	LA::readsOnNeighbor_t references = getRealValuesLA(agentId, dp, 0);

	return references;
}

//TODO: REFACTOR: this should be inside some class? Or at least in another more sensible file
GA::readsOnNeighbor_t calculateGAreferences(int agentId, AS::dataControllerPointers_t* dp) {
	GA::readsOnNeighbor_t references = getRealValuesGA(agentId, dp, 0);

	return references;
}

void updateRead(float* read, float real, float reference, float infiltration) {
	
}
}