#include "systems/AScoordinator.hpp"
#include "AS_testsAPI.hpp"
#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/diplomacy.hpp"
#include "data/dataMisc.hpp"

#include "systems/actionSystem.hpp"

namespace {
	static AS::WarningsAndErrorsCounter* g_errorsCounter_ptr;

	void updateLA(LA::stateData_t* state_ptr, int agentId, 
				  AS::dataControllerPointers_t* agentDataPtrs_ptr, float timeMultiplier,
		                                AS::WarningsAndErrorsCounter* errorsCounter_ptr);
	void updateGA(GA::stateData_t* state_ptr, int agentId, 
				  AS::dataControllerPointers_t* agentDataPtrs_ptr, float timeMultiplier,
		                                AS::WarningsAndErrorsCounter* errorsCounter_ptr);

	void makeDecisionLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
													  AS::PRNserver* prnServer_ptr);
	void makeDecisionGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
													  AS::PRNserver* prnServer_ptr);

	LA::readsOnNeighbor_t calculateLAreferences(int agentId, AS::dataControllerPointers_t* dp);
	GA::readsOnNeighbor_t calculateGAreferences(int agentId, AS::dataControllerPointers_t* dp);
	void updateRead(float* read, float real, float reference, float infiltration, 
							             float prnFrom0to1, float timeMultiplier);

	static float g_secondsSinceLastDecisionStep = 0;
}

namespace AS_TST {
	void updateReadTest(float* read, float real, float reference, float infiltration,
		float prnFrom0to1, float timeMultiplier) {

		updateRead(read, real, reference, infiltration, prnFrom0to1, timeMultiplier);
	}
}

void AS::stepAgents(int LAdecisionsToTakeThisChop, int GAdecisionsToTakeThisChop, 
	                          dataControllerPointers_t* dp, float timeMultiplier,
	                                        int numberLAs, int numberEffectiveGAs,
		                             WarningsAndErrorsCounter* errorsCounter_ptr,
	                                                AS::PRNserver* prnServer_ptr,
	                                          float secondsSinceLastDecisionStep) {

	g_errorsCounter_ptr = errorsCounter_ptr;
	g_secondsSinceLastDecisionStep = secondsSinceLastDecisionStep;

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

	for (int i = 0; i < numberEffectiveGAs; i++) {
		updateGA(&GAstateData_ptr->at(i), i, dp, timeMultiplier, errorsCounter_ptr);
	}
	
	//Make decisions:
	
	//For LAs:
	static int nextDecisionLAindex = 0;	
	int finalDecisionLAindex = nextDecisionLAindex + LAdecisionsToTakeThisChop - 1;

	//Note that this may be larger than the index of the last LA...
	while (nextDecisionLAindex <= finalDecisionLAindex) {
		//so we modulo it in the call:
		makeDecisionLA(nextDecisionLAindex % numberLAs, dp, prnServer_ptr);
		nextDecisionLAindex++;
	}
	nextDecisionLAindex %= numberLAs; //and then once outside the loop we modulo the static value
		
	//For GAs:
	static int nextDecisionGAindex = 0;	
	int finalDecisionGAindex = nextDecisionGAindex + GAdecisionsToTakeThisChop - 1;

	while (nextDecisionGAindex <= finalDecisionGAindex) {	
		//for the same reason we modulo the index here as well:
		makeDecisionGA(nextDecisionGAindex % numberEffectiveGAs, dp, prnServer_ptr);
		nextDecisionGAindex++;
	}
	nextDecisionGAindex %= numberEffectiveGAs;
}

namespace {

//Taxes are proportional to current resources, and can be "negative"
float taxPayedPerSecond(AS::resources_t resources) {
	return ((float)GA_TAX_RATE_PER_SECOND * resources.current);
}

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

	//now we can update the current resources:
	res_ptr->current += (res_ptr->updateRate - str_ptr->currentUpkeep) * timeMultiplier;
	
	//But resources, infiltration and relations can also change due to diplomacy:
	//TODO: EXTRACT and leave definition on diplomacy.cpp
	auto decision_ptr = &(dp->LAdecision_ptr->getDirectDataPtr()->at(agentId));

	LA::applyAttritionTradeInfiltrationAndDispostionChanges(agentId, timeMultiplier, 
	                                 state_ptr, decision_ptr, dp, errorsCounter_ptr);

	//finally, LAs "pay tax" to GA (and can receive resources from the GA if in debt):
	res_ptr->current -= taxPayedPerSecond(*res_ptr) * timeMultiplier;

	//let's also make sure disposition and infiltration remain bounded to [-1,1]
	auto infiltrationArr_ptr = &(decision_ptr->infiltration[0]);
	auto dispositionArr_ptr = &(state_ptr->relations.dispositionToNeighbors[0]);

	int quantityNeighbours = state_ptr->locationAndConnections.numberConnectedNeighbors;
	for (int neighbor = 0; neighbor < quantityNeighbours; neighbor++) {
		infiltrationArr_ptr[neighbor] =
			std::clamp(infiltrationArr_ptr[neighbor], MIN_INFILTRATION, MAX_INFILTRATION);
		dispositionArr_ptr[neighbor] =
			std::clamp(dispositionArr_ptr[neighbor], MIN_DISPOSITION, MAX_DISPOSITION);
	}
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
	param_ptr->LAguardTotal = 0;
	auto LAstates_cptr = dp->LAstate_ptr->getDataCptr();

	for (int i = 0; i < connectedLAs; i++) {
		int id = state_ptr->laIDs[i];
		
		param_ptr->LAesourceTotals.current += 
			LAstates_cptr->at(id).parameters.resources.current;
		param_ptr->LAesourceTotals.updateRate += 
			LAstates_cptr->at(id).parameters.resources.updateRate;
		param_ptr->LAstrenghtTotal += 
			LAstates_cptr->at(id).parameters.strenght.current;
		param_ptr->LAguardTotal += 
			LAstates_cptr->at(id).parameters.strenght.externalGuard;
	}
	
	//Get resoures from tax...
	param_ptr->lastTaxIncome = taxPayedPerSecond(state_ptr->parameters.LAesourceTotals)
							   * timeMultiplier;
	param_ptr->GAresources += param_ptr->lastTaxIncome;

	//... and from trade:
	//TODO: EXTRACT and move to diplomacy.cpp (also comment as in the LAs version)
	int quantityNeighbours = state_ptr->connectedGAs.howManyAreOn();
	param_ptr->lastTradeIncome = 0;

	auto decision_ptr = &(dp->GAdecision_ptr->getDirectDataPtr()->at(agentId));

	for (int neighbor = 0; neighbor < quantityNeighbours; neighbor++) {

		int idOther = state_ptr->neighbourIDs[neighbor];
		AS::diploStance stance = state_ptr->relations.diplomaticStanceToNeighbors[idOther];
				
		//raise relations and infiltration because of alliance
		//change infiltration according to neighbors disposition (can be negative):

		if ((stance == AS::diploStance::TRADE) ||
		    (stance == AS::diploStance::ALLY_WITH_TRADE)) {
			
			float share = LA::calculateShareOfPartnersTrade(idOther, stance, dp, 
				                                                errorsCounter_ptr);

			param_ptr->lastTradeIncome +=
				GA::calculateTradeIncomePerSecond(share, idOther, dp) * timeMultiplier;

			//raise relations and infiltration in proportion to share:
			state_ptr->relations.dispositionToNeighbors[neighbor] +=
					share * MAX_DISPOSITION_RAISE_FROM_TRADE_PER_SECOND * timeMultiplier;
			decision_ptr->infiltration[neighbor] +=
				    share * MAX_INFILTRATION_RAISE_FROM_TRADE_PER_SECOND * timeMultiplier;
		}

		else if (stance == AS::diploStance::WAR) {
			//lower relations and infiltration because of war:
			state_ptr->relations.dispositionToNeighbors[neighbor] -=
					MAX_DISPOSITION_RAISE_FROM_TRADE_PER_SECOND * timeMultiplier;
			decision_ptr->infiltration[neighbor] -=
				    MAX_INFILTRATION_RAISE_FROM_TRADE_PER_SECOND * timeMultiplier;
		}

		if ((stance == AS::diploStance::ALLY) ||
		    (stance == AS::diploStance::ALLY_WITH_TRADE)) {

			//raise relations and infiltration because of alliance:
			state_ptr->relations.dispositionToNeighbors[neighbor] +=
					DISPOSITION_RAISE_FROM_ALLIANCE_PER_SECOND * timeMultiplier;
			decision_ptr->infiltration[neighbor] +=
				    INFILTRATION_RAISE_FROM_ALLIANCE_PER_SECOND * timeMultiplier;
		}

		//We also change infiltration according to neighbors disposition
		//If the neighbor likes this agent, this agent gains infiltration, and vice-versa
		
		//First, we need to find this agent's index on the neighbor's arrays:
		auto partnerState_ptr = &(dp->GAstate_ptr->getDataCptr()->at(idOther));
		int idOnNeighbor = 
			AS::getGAsIDonNeighbor(agentId, idOther, partnerState_ptr);
		bool found = (idOnNeighbor != NATURAL_RETURN_ERROR);

		if(found){
			float neighborsDisposition = 
				partnerState_ptr->relations.dispositionToNeighbors[idOnNeighbor];

			decision_ptr->infiltration[neighbor] += timeMultiplier * neighborsDisposition
								* INFILTRATION_CHANGE_FROM_NEIGHBOR_DISPOSITION_PER_SECOND;
		}
		else {
			errorsCounter_ptr->incrementError(AS::errors::AS_GA_NOT_NEIGHBOR_OF_NEIGHBOR);
		}	
	}

	//actually add the resources from all the trade:
	param_ptr->GAresources += param_ptr->lastTradeIncome;

	//and make sure disposition and infiltration remain bounded to [-1,1]
	auto infiltrationArr_ptr = &(decision_ptr->infiltration[0]);
	auto dispositionArr_ptr = &(state_ptr->relations.dispositionToNeighbors[0]);

	for (int neighbor = 0; neighbor < quantityNeighbours; neighbor++) {

		bool print = false &&
			((infiltrationArr_ptr[neighbor] > 1) || (infiltrationArr_ptr[neighbor] < -1))
			|| ((dispositionArr_ptr[neighbor] > 1) || (dispositionArr_ptr[neighbor] < -1));

		infiltrationArr_ptr[neighbor] =
			std::clamp(infiltrationArr_ptr[neighbor], MIN_INFILTRATION, MAX_INFILTRATION);
		dispositionArr_ptr[neighbor] =
			std::clamp(dispositionArr_ptr[neighbor], MIN_DISPOSITION, MAX_DISPOSITION);
	}

	
}


void updateReadsLA(int agent, AS::dataControllerPointers_t* dp, LA::stateData_t* state_ptr, 
															  AS::PRNserver* prnServer_ptr);

namespace AD = AS::Decisions;
void calculateNotionsLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                                    AD::notions_t* notions_ptr);
void scoreActionsByDesirabilityLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
															      AD::notions_t* notions_ptr, 
													     AD::LA::actionScores_t* scores_ptr);
void redistributeScoreDueToImpedimmentsLA(int agent, 
	AS::dataControllerPointers_t* agentDataPtrs_ptr, AD::notions_t* notions_ptr,
	                                         AD::LA::actionScores_t* scores_ptr);
void chooseActionLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                        AD::LA::actionScores_t* scores_ptr);

void makeDecisionLA(int agent, AS::dataControllerPointers_t* dp, 
								    AS::PRNserver* prnServer_ptr) {

	LA::stateData_t* state_ptr = &(dp->LAstate_ptr->getDirectDataPtr()->at(agent));

	if (state_ptr->onOff == false) {
		return;
	}
	
	updateReadsLA(agent, dp, state_ptr, prnServer_ptr);

	if (!dp->LAdecision_ptr->getDataCptr()->at(agent).shouldMakeDecisions) {
		return;
	}
	
	AD::notions_t notions;
	calculateNotionsLA(agent, dp, &notions);

	AD::LA::actionScores_t scores;
	scoreActionsByDesirabilityLA(agent, dp, &notions, &scores);

	for(int i = 0; i < CONSTRAINT_CHECK_ROUNDS; i++){
		redistributeScoreDueToImpedimmentsLA(agent, dp, &notions, &scores);
	}

	//TODO: if I take out the random factor on action choosing, what does this become?
	chooseActionLA(agent, dp, &scores);
}

void updateInfiltrationAndRelationsFromLAs(int agent, AS::dataControllerPointers_t* dp, 
															GA::stateData_t* state_ptr);

void updateReadsGA(int agent, AS::dataControllerPointers_t* dp, GA::stateData_t* state_ptr, 
										                      AS::PRNserver* prnServer_ptr);

void calculateNotionsGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                                   AD::notions_t* notions_ptr);
void scoreActionsByDesirabilityGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
															      AD::notions_t* notions_ptr, 
													      AD::GA::actionScores_t* scores_ptr);
void redistributeScoreDueToImpedimmentsGA(int agent, 
	AS::dataControllerPointers_t* agentDataPtrs_ptr, AD::notions_t* notions_ptr,
	                                         AD::GA::actionScores_t* scores_ptr);
void chooseActionGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                        AD::GA::actionScores_t* scores_ptr);

void makeDecisionGA(int agent, AS::dataControllerPointers_t* dp, 
	                                AS::PRNserver* prnServer_ptr) {

	GA::stateData_t* state_ptr = &(dp->GAstate_ptr->getDirectDataPtr()->at(agent));

	if (state_ptr->onOff == false) {
		return;
	}

	updateInfiltrationAndRelationsFromLAs(agent, dp, state_ptr);

	updateReadsGA(agent, dp, state_ptr, prnServer_ptr);

	if (!dp->GAdecision_ptr->getDataCptr()->at(agent).shouldMakeDecisions) {
		return;
	}

	AD::notions_t notions;
	calculateNotionsGA(agent, dp, &notions);

	AD::GA::actionScores_t scores;
	scoreActionsByDesirabilityGA(agent, dp, &notions, &scores);

	for(int i = 0; i < CONSTRAINT_CHECK_ROUNDS; i++){
		redistributeScoreDueToImpedimmentsGA(agent, dp, &notions, &scores);
	}
	//TODO: if I take out the random factor on action choosing, what does this become?
	chooseActionGA(agent, dp, &scores);
}


//LA:
void calculateNotionsLA(int agent, AS::dataControllerPointers_t* dp, AD::notions_t* np) {

}

void scoreActionsByDesirabilityLA(int agent, AS::dataControllerPointers_t* dp, 
	                            AD::notions_t* np, AD::LA::actionScores_t* sp) {

}

void redistributeScoreDueToImpedimmentsLA(int agent, AS::dataControllerPointers_t* dp,
			                            AD::notions_t* np, AD::LA::actionScores_t* sp) {

}

void chooseActionLA(int agent, AS::dataControllerPointers_t* dp, 
	                                 AD::LA::actionScores_t* sp) {

}

//GA:
void calculateNotionsGA(int agent, AS::dataControllerPointers_t* dp, AD::notions_t* np) {

}

void scoreActionsByDesirabilityGA(int agent, AS::dataControllerPointers_t* dp, 
	                            AD::notions_t* np, AD::GA::actionScores_t* sp) {

}

void redistributeScoreDueToImpedimmentsGA(int agent, AS::dataControllerPointers_t* dp,
													                AD::notions_t* np, 
											               AD::GA::actionScores_t* sp) {

}

void chooseActionGA(int agent, AS::dataControllerPointers_t* dp, 
						             AD::GA::actionScores_t* sp) {

}

//Reads disposition and infiltration of connected LAs towards other LAs from each GA.
//Interpolates gaID's disposition and infiltration towards each connected GA with that.
//TODO-CRITICAL: testing
void updateInfiltrationAndRelationsFromLAs(int gaID, AS::dataControllerPointers_t* dp, 
															GA::stateData_t* state_ptr) {

	//This will be collected from all connected LAs and then interpolated with the GA data:
	struct neighborData_st {
		int timesNeighborAppeared = 0;
		float infiltration = 0;
		float relation = 0;
	};

	//The data will be stored here:
	neighborData_st dataFromLAs[MAX_GA_QUANTITY];

	//Now we loop the agents. For each neighbor, check it's GA 
	//And increment the data regarding it (even if it's not a neighbor of this GA)
	int connectedLAs = state_ptr->localAgentsBelongingToThis.howManyAreOn();
	int totalInfluences = 0;
	for (int thisLA = 0; thisLA < connectedLAs; thisLA++) {

		int laID = state_ptr->laIDs[thisLA];
		auto LAstateData_ptr = dp->LAstate_ptr->getDataCptr();
		auto thisLAstateData_ptr = &(LAstateData_ptr->at(laID));
		auto thisLAdecisionData_ptr = &(dp->LAdecision_ptr->getDataCptr()->at(laID));
		auto LAconnections_ptr = &(thisLAstateData_ptr->locationAndConnections);

		int totalLAneighbors = LAconnections_ptr->connectedNeighbors.howManyAreOn();
		
		for (int LAneighbor = 0; LAneighbor < totalLAneighbors; LAneighbor++) {
			
			int neighborID = LAconnections_ptr->neighbourIDs[LAneighbor];
			int GAid = LAstateData_ptr->at(neighborID).GAid;

			dataFromLAs[GAid].timesNeighborAppeared++;
			dataFromLAs[GAid].relation += 
				thisLAstateData_ptr->relations.dispositionToNeighbors[LAneighbor];
			dataFromLAs[GAid].infiltration += thisLAdecisionData_ptr->infiltration[LAneighbor];

			totalInfluences++;
		}
	}

	//For the GAs which are actually neighbors of this GA, we interpolate the new data:
	auto infiltration_ptr = 
		&(dp->GAdecision_ptr->getDirectDataPtr()->at(gaID).infiltration[0]);

	int connectedGAs = state_ptr->connectedGAs.howManyAreOn();
	for (int neighbor = 0; neighbor < connectedGAs; neighbor++) {
		int neighborID = state_ptr->neighbourIDs[neighbor];

		//take the average:
		dataFromLAs[neighborID].infiltration /= dataFromLAs[neighborID].timesNeighborAppeared;
		dataFromLAs[neighborID].relation /= dataFromLAs[neighborID].timesNeighborAppeared;

		//calculate weights:
		float proportion = 
			(float)dataFromLAs[neighborID].timesNeighborAppeared / totalInfluences;

		float weightLAs = proportion * TOTAL_LA_INFO_RELATION_WEIGHT_FOR_GA_PER_SECOND
						  * g_secondsSinceLastDecisionStep;
		float weightGA = 1 - weightLAs;

		//interpolate values:
		state_ptr->relations.dispositionToNeighbors[neighbor] *= weightGA;
		state_ptr->relations.dispositionToNeighbors[neighbor] += 
											weightLAs * dataFromLAs[neighborID].relation;
		infiltration_ptr[neighbor] *= weightGA;
		infiltration_ptr[neighbor] += weightLAs * dataFromLAs[neighborID].infiltration;
	}
}

//READS AND EXPECTATIONS:
//TODO: EXTRACT STUFF AND ADD WARNING AND ERROR HANDLING (REFACTOR: PULL SOME INTO CLASSES)

LA::readsOnNeighbor_t getRealValuesLA(AS::dataControllerPointers_t* dp, int neighborID);
GA::readsOnNeighbor_t getRealValuesGA(AS::dataControllerPointers_t* dp, int neighborID);

void updateReadsLA(int agent, AS::dataControllerPointers_t* dp, LA::stateData_t* state_ptr, 
										                      AS::PRNserver* prnServer_ptr) {

	LA::readsOnNeighbor_t referenceReads =  calculateLAreferences(agent, dp);

	auto decisionData_ptr = &(dp->LAdecision_ptr->getDirectDataPtr()->at(agent));
	LA::readsOnNeighbor_t* reads_ptr = &(decisionData_ptr->reads[0]);

	int totalNeighbors = state_ptr->locationAndConnections.numberConnectedNeighbors;

	for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {

		int neighborID = state_ptr->locationAndConnections.neighbourIDs[neighbor];
		auto readsOnNeighbor_ptr = &reads_ptr[neighbor];

		LA::readsOnNeighbor_t realValues = getRealValuesLA(dp, neighborID);
		float infiltration = decisionData_ptr->infiltration[neighbor];

		for (int field = 0; field < (int)LA::readsOnNeighbor_t::fields::TOTAL; field++) {

			//this is the payload:
			updateRead(&readsOnNeighbor_ptr->readOf[field], realValues.readOf[field], 
		                        referenceReads.readOf[field], infiltration, 
		          prnServer_ptr->getNext(), g_secondsSinceLastDecisionStep);
		}
	}	
}

void updateReadsGA(int agent, AS::dataControllerPointers_t* dp, GA::stateData_t* state_ptr, 
										                      AS::PRNserver* prnServer_ptr) {

	GA::readsOnNeighbor_t referenceReads =  calculateGAreferences(agent, dp);

	auto decisionData_ptr = &(dp->GAdecision_ptr->getDirectDataPtr()->at(agent));
	GA::readsOnNeighbor_t* reads_ptr = &(decisionData_ptr->reads[0]);

	int totalNeighbors = state_ptr->connectedGAs.howManyAreOn();

	for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {

		int neighborID = state_ptr->neighbourIDs[neighbor];
		auto readsOnNeighbor_ptr = &reads_ptr[neighbor];
		
		//int neighborID = agentState_ptr->locationAndConnections.neighbourIDs[neighbor];
		
		GA::readsOnNeighbor_t realValues = getRealValuesGA(dp, neighborID);
		float infiltration = decisionData_ptr->infiltration[neighbor];

		for (int field = 0; field < (int)GA::readsOnNeighbor_t::fields::TOTAL; field++) {

			//this is the payload:
			updateRead(&(readsOnNeighbor_ptr->readOf[field]), realValues.readOf[field], 
		                        referenceReads.readOf[field], infiltration, 
				  prnServer_ptr->getNext(), g_secondsSinceLastDecisionStep);
		}
	}	
}

LA::readsOnNeighbor_t getRealValuesLA(AS::dataControllerPointers_t* dp, int neighborID) {
	
	LA::readsOnNeighbor_t real;
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

GA::readsOnNeighbor_t getRealValuesGA(AS::dataControllerPointers_t* dp, int neighborID) {
	
	GA::readsOnNeighbor_t real;
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

//Definitions for calculateLAreferences
#define D (1/TOTAL_WEIGHT_FOR_REF_EXPECTATIONS)
#define E (1 - D)

LA::readsOnNeighbor_t calculateLAreferences(int agentId, AS::dataControllerPointers_t* dp) {
	
	/*ref[i] = D * (SUM_viz(expec[viz][i] * abs(inf[viz]) / SUM_viz(abs(inf[viz])
	         + E * GA[i]/#LAs
	*/
	
	auto state_ptr = &(dp->LAstate_ptr->getDataCptr()->at(agentId));
	auto decision_ptr = &(dp->LAdecision_ptr->getDataCptr()->at(agentId));

	LA::readsOnNeighbor_t GAreferences;
	auto GAstate_ptr = &(dp->GAstate_ptr->getDataCptr()->at(state_ptr->GAid));
	auto GAparams_ptr = &(GAstate_ptr->parameters);

	//TODO: extract this
	GAreferences.readOf[(int)LA::readsOnNeighbor_t::fields::GUARD] =
										GAparams_ptr->LAguardTotal;
	GAreferences.readOf[(int)LA::readsOnNeighbor_t::fields::INCOME] =
										GAparams_ptr->LAesourceTotals.updateRate;
	GAreferences.readOf[(int)LA::readsOnNeighbor_t::fields::RESOURCES] =
										GAparams_ptr->LAesourceTotals.current;
	GAreferences.readOf[(int)LA::readsOnNeighbor_t::fields::STRENGHT] =
										GAparams_ptr->LAstrenghtTotal;

	int totalNeighbors = state_ptr->locationAndConnections.numberConnectedNeighbors;
	float totalInfiltration = 0;

	LA::readsOnNeighbor_t references; 
	for (int field = 0; field < (int)LA::readsOnNeighbor_t::fields::TOTAL; field++) {
			references.readOf[field] = 0;
	}
	for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {

		float iniltration =  decision_ptr->infiltration[neighbor];
		totalInfiltration += iniltration;
		for (int field = 0; field < (int)LA::readsOnNeighbor_t::fields::TOTAL; field++) {

			references.readOf[field] += 
				(decision_ptr->reads[neighbor].readOf[field] * abs(iniltration));
		}
	}
	int totalLAs = GAstate_ptr->localAgentsBelongingToThis.howManyAreOn();
	for (int field = 0; field < (int)LA::readsOnNeighbor_t::fields::TOTAL; field++) {
			references.readOf[field] = 
								(D * references.readOf[field] / totalInfiltration)
								+ (E * GAreferences.readOf[field] / totalLAs);
	}

	return references;
}

GA::readsOnNeighbor_t calculateGAreferences(int agentId, AS::dataControllerPointers_t* dp) {
	
	/*We're doing:
	ref[i] = D * (SUM_viz(expec[viz][i] * abs(inf[viz]) / SUM_viz(abs(inf[viz])
	       + E * GA[i]
	*/

	auto state_ptr = &(dp->GAstate_ptr->getDataCptr()->at(agentId));
	auto decision_ptr = &(dp->GAdecision_ptr->getDataCptr()->at(agentId));
	
	GA::readsOnNeighbor_t GAreferences;
	auto GAparams_ptr = &(state_ptr->parameters);

	//TODO: extract this
	GAreferences.readOf[(int)GA::readsOnNeighbor_t::fields::GA_RESOURCES] =
										GAparams_ptr->GAresources;
	GAreferences.readOf[(int)GA::readsOnNeighbor_t::fields::GUARD_LAS] =
										GAparams_ptr->LAguardTotal;
	GAreferences.readOf[(int)GA::readsOnNeighbor_t::fields::STRENGHT_LAS] =
										GAparams_ptr->LAstrenghtTotal;
	GAreferences.readOf[(int)GA::readsOnNeighbor_t::fields::TAX_INCOME] =
										GAparams_ptr->lastTaxIncome;
	GAreferences.readOf[(int)GA::readsOnNeighbor_t::fields::TRADE_INCOME] =
										GAparams_ptr->lastTradeIncome;

	int totalNeighbors = state_ptr->connectedGAs.howManyAreOn();
	float totalInfiltration = 0;

	GA::readsOnNeighbor_t references; 
	for (int field = 0; field < (int)GA::readsOnNeighbor_t::fields::TOTAL; field++) {
			references.readOf[field] = 0;
	}
	for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {

		float iniltration =  decision_ptr->infiltration[neighbor];
		totalInfiltration += iniltration;
		for (int field = 0; field < (int)GA::readsOnNeighbor_t::fields::TOTAL; field++) {

			references.readOf[field] += 
				(decision_ptr->reads[neighbor].readOf[field] * abs(iniltration));
		}
	}
	for (int field = 0; field < (int)GA::readsOnNeighbor_t::fields::TOTAL; field++) {
			references.readOf[field] = 
								(D * references.readOf[field] / totalInfiltration)
								+ (E * GAreferences.readOf[field]);
	}

	return references;
}

//Definitions for updateRead
#define FAC_A EXPC_MAX_PROPORTIONAL_CHANGE_PER_SECOND //A + B
#define FAC_B EXPC_INFILTRATION_EQUIVALENT_TO_MAXIMUM_NOISE //sqrt(B/A)
#define A (FAC_A / ((FAC_B*FAC_B) + 1))
#define B (FAC_A - A)
#define FAC_CW (EXPC_PROPORTIONAL_ERROR_OVER_MINIMUM_FOR_MAX_CORRECTION / EXPC_PROPORTIONAL_ERROR_OVER_MINIMUM_FOR_EQUILIBRIUM)
#define BIAS EXPC_MIN_PROPORTIONAL_ERROR_TO_CORRECT
#define S EXPC_ERROR_CORRECTION_SHARPNESS

//TODO: document the math and for this and it's tets use the same order for the difference
//TODO: this sorta supposes real values are almost always positive. Either make that so, or fix this
void updateRead(float* read_ptr, float real, float reference, float infiltration, 
										 float prnFrom0to1, float timeMultiplier) {
	
	assert( (prnFrom0to1 >= 0.f) && (prnFrom0to1 <= 1.f) );

	static const float MAX_CORRECTION_WEIGHT_RELATIVE_TO_INFO = powf(FAC_CW , S);
	static const float C = A*MAX_CORRECTION_WEIGHT_RELATIVE_TO_INFO/powf(EXPC_PROPORTIONAL_ERROR_OVER_MINIMUM_FOR_MAX_CORRECTION,S);
	
	float effectiveReference = std::max(abs(reference), abs(real));
	if (effectiveReference < 1) {
		effectiveReference = 1; //"neutral" to multiplication and division
	}

	float difference = real - (*read_ptr);
	float minimumDifference = EXPC_EFFECTIVE_MIN_PROPORTIONAL_DIFF * effectiveReference;
	if (abs(difference) < abs(minimumDifference)) {
		int sign = difference >= 0 ? 1 : -1;
		difference = minimumDifference * sign;
	}

	float correctionFactor = std::min(EXPC_PROPORTIONAL_ERROR_FOR_MAX_CORRECTION, 
		                                      abs(difference)/effectiveReference);
	correctionFactor = std::max(0.f, (correctionFactor - BIAS));
	float multiplier = 
		std::min(2.0f, ( 
						A*infiltration*fabs(infiltration) 
						+ B*( (2*prnFrom0to1) - 1) 
						+ C*(powf(correctionFactor, S))
					) );
	*read_ptr += difference * multiplier * timeMultiplier;
	*read_ptr = std::max(-*read_ptr, *read_ptr);
}
}