#include "systems/AScoordinator.hpp"
#include "AS_testsAPI.hpp"
#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/diplomacy.hpp"

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
	                                                int numberLAs, int numberGAs,
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

	for (int i = 0; i < numberGAs; i++) {
		updateGA(&GAstateData_ptr->at(i), i, dp, timeMultiplier, errorsCounter_ptr);
	}
	
	//Make decisions:
	
	static int nextDecisionLAindex = 0;	
	int finalDecisionLAindex = nextDecisionLAindex + LAdecisionsToTakeThisChop - 1;

	//Note that this may be larger than the index of the last LA...
	while (nextDecisionLAindex <= finalDecisionLAindex) {
		//so we modulo it in the call:
		makeDecisionLA(nextDecisionLAindex % numberLAs, dp, prnServer_ptr);
		nextDecisionLAindex++;
	}
	nextDecisionLAindex %= numberLAs; //and then once outside the loop we modulo the static value
		
	static int nextDecisionGAindex = 0;	
	int finalDecisionGAindex = nextDecisionGAindex + GAdecisionsToTakeThisChop - 1;

	while (nextDecisionGAindex <= finalDecisionGAindex) {	
		//for the same reason we modulo the index here as well:
		makeDecisionGA(nextDecisionGAindex % numberGAs, dp, prnServer_ptr);
		nextDecisionGAindex++;
	}
	nextDecisionGAindex %= numberGAs;
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


void updateReadsLA(int agent, AS::dataControllerPointers_t* dp, LA::stateData_t* state_ptr, 
															  AS::PRNserver* prnServer_ptr);

namespace AD = AS::Decisions;
void calculateNotionsLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                                    AD::notions_t* notions_ptr);
void preScoreActionsLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
										               AD::notions_t* notions_ptr, 
	                                           AD::LA::actionScores_t* scores_ptr);
void redistributeScoreDueToImpedimmentsLA(int agent, 
	AS::dataControllerPointers_t* agentDataPtrs_ptr, AD::notions_t* notions_ptr,
	                                         AD::LA::actionScores_t* scores_ptr);
void chooseActionLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                        AD::LA::actionScores_t* scores_ptr);

void makeDecisionLA(int agent, AS::dataControllerPointers_t* dp, 
								    AS::PRNserver* prnServer_ptr) {

	if (!dp->LAdecision_ptr->getDataCptr()->at(agent).shouldMakeDecisions) {
		return;
	}

	LA::stateData_t* state_ptr = &(dp->LAstate_ptr->getDirectDataPtr()->at(agent));
	
	updateReadsLA(agent, dp, state_ptr, prnServer_ptr);
	
	AD::notions_t notions;
	calculateNotionsLA(agent, dp, &notions);

	AD::LA::actionScores_t scores;
	//TODO: rename to something like scoreActionsByDesirability
	preScoreActionsLA(agent, dp, &notions, &scores);

	for(int i = 0; i < CONSTRAINT_CHECK_ROUNDS; i++){
		redistributeScoreDueToImpedimmentsLA(agent, dp, &notions, &scores);
	}

	//TODO: if I take out the random factor on action choosing, what does this become?
	chooseActionLA(agent, dp, &scores);
}


void updateReadsGA(int agent, AS::dataControllerPointers_t* dp, GA::stateData_t* state_ptr, 
										                      AS::PRNserver* prnServer_ptr);

void calculateNotionsGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                                   AD::notions_t* notions_ptr);
void preScoreActionsGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
									                   AD::notions_t* notions_ptr, 
									           AD::GA::actionScores_t* scores_ptr);
void redistributeScoreDueToImpedimmentsGA(int agent, 
	AS::dataControllerPointers_t* agentDataPtrs_ptr, AD::notions_t* notions_ptr,
	                                         AD::GA::actionScores_t* scores_ptr);
void chooseActionGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                        AD::GA::actionScores_t* scores_ptr);

void makeDecisionGA(int agent, AS::dataControllerPointers_t* dp, 
	                                AS::PRNserver* prnServer_ptr) {

	if (!dp->GAdecision_ptr->getDataCptr()->at(agent).shouldMakeDecisions) {
		return;
	}

	GA::stateData_t* state_ptr = &(dp->GAstate_ptr->getDirectDataPtr()->at(agent));

	updateReadsGA(agent, dp, state_ptr, prnServer_ptr);

	AD::notions_t notions;
	calculateNotionsGA(agent, dp, &notions);

	AD::GA::actionScores_t scores;
	preScoreActionsGA(agent, dp, &notions, &scores);

	for(int i = 0; i < CONSTRAINT_CHECK_ROUNDS; i++){
		redistributeScoreDueToImpedimmentsGA(agent, dp, &notions, &scores);
	}

	chooseActionGA(agent, dp, &scores);
}


//LA:
void calculateNotionsLA(int agent, AS::dataControllerPointers_t* dp, AD::notions_t* np) {

}

void preScoreActionsLA(int agent, AS::dataControllerPointers_t* dp, AD::notions_t* np,
	                                                       AD::LA::actionScores_t* sp) {

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

void preScoreActionsGA(int agent, AS::dataControllerPointers_t* dp, AD::notions_t* np, 
														   AD::GA::actionScores_t* sp) {

}

void redistributeScoreDueToImpedimmentsGA(int agent, AS::dataControllerPointers_t* dp,
													                AD::notions_t* np, 
											               AD::GA::actionScores_t* sp) {

}

void chooseActionGA(int agent, AS::dataControllerPointers_t* dp, 
						             AD::GA::actionScores_t* sp) {

}

//READS AND EXPECTATIONS:
//TODO: EXTRACT STUFF AND ADD WARNING AND ERROR HANDLING (REFACTOR: PULL SOME INTO CLASSES)

LA::readsOnNeighbor_t getRealValuesLA(int agent, AS::dataControllerPointers_t* dp, int neighbor);
GA::readsOnNeighbor_t getRealValuesGA(int agent, AS::dataControllerPointers_t* dp, int neighbor);

void updateReadsLA(int agent, AS::dataControllerPointers_t* dp, LA::stateData_t* state_ptr, 
										                      AS::PRNserver* prnServer_ptr) {

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

		GA::readsOnNeighbor_t realValues = getRealValuesGA(agent, dp, neighbor);
		float infiltration = decisionData_ptr->infiltration[neighbor];

		for (int field = 0; field < (int)GA::readsOnNeighbor_t::fields::TOTAL; field++) {

			//this is the payload:
			updateRead(&reads_ptr->readOf[field], realValues.readOf[field], 
		                        referenceReads.readOf[field], infiltration, 
				  prnServer_ptr->getNext(), g_secondsSinceLastDecisionStep);
		}
	}	
}

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