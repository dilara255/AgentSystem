#include "systems/AScoordinator.hpp"
#include "AS_testsAPI.hpp"
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

	void makeDecisionLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
													  AS::PRNserver* prnServer_ptr);
	void makeDecisionGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
													  AS::PRNserver* prnServer_ptr);

	inline float calculateMaxDisparityFactor();
	LA::readsOnNeighbor_t calculateLAreferences(int agentId, AS::dataControllerPointers_t* dp);
	GA::readsOnNeighbor_t calculateGAreferences(int agentId, AS::dataControllerPointers_t* dp);
	void updateRead(float* read, float real, float reference, float infiltration, 
		                  float maxDisparityFactor, float prnFrom0to1);

	static float g_secondsSinceLastDecisionStep = 0;
}

void AS::stepAgents(int LAdecisionsToTakeThisChop, int GAdecisionsToTakeThisChop, 
	                          dataControllerPointers_t* dp, float timeMultiplier,
	                                                int numberLAs, int numberGAs,
		                             WarningsAndErrorsCounter* errorsCounter_ptr,
	                                                AS::PRNserver* prnServer_ptr,
	                                          float secondsSinceLastDecisionStep) {

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
	g_secondsSinceLastDecisionStep = secondsSinceLastDecisionStep;

	static int nextDecisionLAindex = 0;	

	int finalDecisionLAindex = nextDecisionLAindex + LAdecisionsToTakeThisChop - 1;
	while (nextDecisionLAindex <= finalDecisionLAindex) {
		//Say we have to deal with 4 agents this chop, 
		//but the first one is the second to last agent.
		//In this case finalDecisionLAindex will be greater than the last LA's index,
		//Since we need to draw "high" and "low" indexes, we can't modulo them before the loop
		//(and if we do in the loop, then we loop forever),
		//so we modulo the nextDecisionLAindex just in the call:
		makeDecisionLA(nextDecisionLAindex % numberLAs, dp, prnServer_ptr);
		nextDecisionLAindex++;
	}
	nextDecisionLAindex %= numberLAs; //and then once outside the loop we modulo the static value
		
	static int nextDecisionGAindex = 0;	

	int finalDecisionGAindex = nextDecisionGAindex + GAdecisionsToTakeThisChop - 1;
	while (nextDecisionGAindex <= finalDecisionGAindex) {	
		//See comments above about the modulo
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
	                                float maxDisparityFactor, AS::PRNserver* prnServer_ptr);
void calculateNotionsLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void preScoreActionsLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void redistributeScoreDueToImpedimmentsLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void chooseActionLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);


void makeDecisionLA(int agent, AS::dataControllerPointers_t* dp, 
								    AS::PRNserver* prnServer_ptr) {

	LA::stateData_t* state_ptr = &(dp->LAstate_ptr->getDirectDataPtr()->at(agent));
	
	updateReadsLA(agent, dp, state_ptr, calculateMaxDisparityFactor(), prnServer_ptr);
	
	calculateNotionsLA(agent, dp);
	preScoreActionsLA(agent, dp);
	redistributeScoreDueToImpedimmentsLA(agent, dp);
	chooseActionLA(agent, dp);
}


void updateReadsGA(int agent, AS::dataControllerPointers_t* dp, GA::stateData_t* state_ptr, 
	                                float maxDisparityFactor, AS::PRNserver* prnServer_ptr);
void calculateNotionsGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void preScoreActionsGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void redistributeScoreDueToImpedimmentsGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);
void chooseActionGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr);


void makeDecisionGA(int agent, AS::dataControllerPointers_t* dp, 
	                                AS::PRNserver* prnServer_ptr) {

	GA::stateData_t* state_ptr = &(dp->GAstate_ptr->getDirectDataPtr()->at(agent));

	updateReadsGA(agent, dp, state_ptr, calculateMaxDisparityFactor(), prnServer_ptr);

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
//TODO: EXTRACT STUFF AND ADD WARNING AND ERROR HANDLING (REFACTOR: PULL SOME INTO CLASSES)

LA::readsOnNeighbor_t getRealValuesLA(int agent, AS::dataControllerPointers_t* dp, int neighbor);
GA::readsOnNeighbor_t getRealValuesGA(int agent, AS::dataControllerPointers_t* dp, int neighbor);

void updateReadsLA(int agent, AS::dataControllerPointers_t* dp, LA::stateData_t* state_ptr, 
	                                float maxDisparityFactor, AS::PRNserver* prnServer_ptr) {

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
								maxDisparityFactor, prnServer_ptr->getNext());
		}
	}	
}

void updateReadsGA(int agent, AS::dataControllerPointers_t* dp, GA::stateData_t* state_ptr, 
	                                float maxDisparityFactor, AS::PRNserver* prnServer_ptr) {

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
				                         maxDisparityFactor, prnServer_ptr->getNext());
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
	
	/*ref[i] = D * (SUM_viz(expec[viz][i] * abs(inf[viz]) / SUM_viz(abs(inf[viz])
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

#define FAC_A EXPC_MAX_PROPORTIONAL_CHANGE_PER_SECOND //A + B
#define FAC_B EXPC_INFILTRATION_EQUIVALENT_TO_MAXIMUM_NOISE //sqrt(B/A)
#define A (FAC_A / ((FAC_B*FAC_B) + 1))
#define B (EXPC_MAX_PROPORTIONAL_CHANGE_PER_SECOND - A)
#define FAC_F (EXPC_ERROR_CORRECTION_WEIGHT_RELATIVE_TO_INFO * A)
#define C (EXPC_PROPORTIONAL_ERROR_OVER_MINIMUM_FOR_MAX_CORRECTION * FAC_F)
#define BIAS EXPC_MIN_PROPORTIONAL_ERROR_TO_CORRECT
#define S EXPC_ERROR_CORRECTION_SHARPNESS

inline float calculateMaxDisparityFactor() {
	return (powf((1.0f/EXPC_PROPORTIONAL_ERROR_FOR_MAX_CORRECTION),(1.0f/S)) + BIAS);
}

void updateRead(float* read, float real, float reference, float infiltration, 
	                  float maxDisparityFactor, float prnFrom0to1) {
	
	assert( (prnFrom0to1 >= 0.f) && (prnFrom0to1 <= 1.f) );

	float difference = real - (*read);
	float effectiveReference = std::max(abs(reference), abs(real));
	if (effectiveReference == 0) {
		effectiveReference = 1; //"neutral" to multiplication and division
	}
	if(difference == 0) {
		difference = EXPC_EFFECTIVE_MIN_PROPORTIONAL_DIFF * effectiveReference;
	}

	float correctionFactor = std::min(maxDisparityFactor, abs(difference)/effectiveReference);
	float multiplier = 
		std::min(2.0f, ( 
						A*infiltration*fabs(infiltration) 
						+ B*( (2*prnFrom0to1) - 1) 
						+ C*(powf(correctionFactor, S)) 
					) );
	*read += difference * multiplier * g_secondsSinceLastDecisionStep;
}
}

namespace AS_TST {
	
	typedef struct {
		float real, read;
	} readTestOutputPoint_t;

	typedef struct scen_st {
		float initialReal = DEFAULT_LA_RESOURCES;
		float real = initialReal;
		float reference = real;
		float read, infiltration, changeSize;
		uint64_t seed = DEFAULT_PRNG_SEED0;
		uint64_t* seed_ptr = &seed;
		double avgDiff = 0;
		double diffStdDev = 0;

		int changeSteps, interpolationSteps;
		std::vector<readTestOutputPoint_t> out;
	} scenario_t;

	typedef;

	//Runs a test scenario of the updateRead method. 
	//Stores read results on the scenarios out vector. 
	//Stores on the scenario the avg and std dev of the difference of real and read values.
	scenario_t runReadTestScenario(scenario_t scn, bool saveSteps, bool zeroReadPrn) {
		
		float maxDisparityFactor = calculateMaxDisparityFactor();
		float invUint32max = 1.0f/UINT32_MAX;

		int changeSteps = scn.changeSteps;
		int interpolationSteps = scn.interpolationSteps;

		for (int i = 0; i < changeSteps; i++) {
			float drawn = 2*(AZ::draw1spcg32(scn.seed_ptr)*invUint32max) - 1;
			assert((drawn <= 1.f) && (drawn >= -1.f));

			float changePerInterpolationStep = drawn*scn.changeSize/interpolationSteps;

			for (int j = 0; j < interpolationSteps; j++) {
				scn.real += changePerInterpolationStep;

				float readPrn = 0;
				if(!zeroReadPrn) {
					readPrn = AZ::draw1spcg32(scn.seed_ptr)*invUint32max;
				}
				updateRead(&scn.read, scn.real, scn.reference, scn.infiltration,
					                                maxDisparityFactor, readPrn);

				if(saveSteps){
					readTestOutputPoint_t out;
					out.read = scn.read;
					out.real = scn.real;
					scn.out.push_back(out);
				}
				
				float invTotalSteps = 1.f/(changeSteps * interpolationSteps);
				float diff = scn.read - scn.real;
				scn.avgDiff += ((double)scn.read - scn.real) * invTotalSteps;
				scn.diffStdDev += (double)diff * diff * invTotalSteps;
			}
		}

		//actually calculate the std deviation from the avg and the avg of the squares:
		scn.diffStdDev -= scn.avgDiff * scn.avgDiff;
		scn.diffStdDev = sqrt(scn.diffStdDev);

		return scn;
	}
}

//Tests ballpark functionality and outputs data for different scenarios, which can be graphed
//True if positive info keeps read close to expected and negative far, but within bounds
//TODO: better automatic tests (and more robust to parameter changes)
//TODO: GRAPH the files. Look at them plus results. Adjust CHANGES if needed.
bool AS::testUpdateRead(bool printResults, bool dumpInfo, bool overwriteDump) {

	//some definitions:
	enum testSteps {CHANGES = 20, INTERPOLATIONS = AS_TOTAL_CHOPS,
							 TOTAL_STEPS = CHANGES*INTERPOLATIONS};

	enum testScenarios {INFO_VARS = 5, FIRST_GUESS_VARS = 4, CHANGE_SPEED_VARS = 4,
		                TOTAL_SCENARIOS = 
							(INFO_VARS*FIRST_GUESS_VARS*CHANGE_SPEED_VARS) };

	//setup test parameters:
	g_secondsSinceLastDecisionStep = 
		(float)AS_TOTAL_CHOPS * AS_MILLISECONDS_PER_STEP / MILLIS_IN_A_SECOND;

	float infoLevels[INFO_VARS];
	int index = 0;
	infoLevels[index++] = -1;
	infoLevels[index++] = -0.5;
	infoLevels[index++] = 0;
	infoLevels[index++] = 0.5;
	infoLevels[index++] = 1;
	assert (index == INFO_VARS);

	float initialReads[FIRST_GUESS_VARS];
	float real = DEFAULT_LA_RESOURCES;
	float goodPropotion = 0.9f;
	float badPropotion = EXPC_MIN_PROPORTIONAL_ERROR_TO_CORRECT*goodPropotion;
	float awfulFirstGuess = real*EXPC_PROPORTIONAL_ERROR_FOR_MAX_CORRECTION;
	index = 0;
	initialReads[index++] = awfulFirstGuess;
	initialReads[index++] = real*badPropotion;
	initialReads[index++] = real*goodPropotion;
	initialReads[index++] = real;
	assert (index == FIRST_GUESS_VARS);

	float realChangeVariations[CHANGE_SPEED_VARS];
	float absurdProportion = 10;
	index = 0;
	realChangeVariations[index++] = 0;
	realChangeVariations[index++] = real/absurdProportion;
	realChangeVariations[index++] = real;
	realChangeVariations[index++] = real*absurdProportion;
	assert (index == CHANGE_SPEED_VARS);

	//these will hold the actual test data:
	AS_TST::scenario_t scenarios[INFO_VARS][FIRST_GUESS_VARS][CHANGE_SPEED_VARS];

	typedef struct {
		double avgDiff, diffStdDev;
		float infiltration;
	} statisticalResults_t;

	//the results of the automatically checked tests will be here:
	std::vector<statisticalResults_t> autoTestResults;

	//Now, for the tests. For each scenario:
	for(int guessVar = 0; guessVar < FIRST_GUESS_VARS; guessVar++) {
		for (int infoVar = 0; infoVar < INFO_VARS; infoVar++) {
			for (int changeVar = 0; changeVar < CHANGE_SPEED_VARS; changeVar++) {
				
				//we set the scenario-specific values:
				AS_TST::scenario_t* scn_ptr = &scenarios[guessVar][infoVar][changeVar];
				scn_ptr->read = initialReads[guessVar];
				scn_ptr->infiltration = infoLevels[infoVar];
				scn_ptr->changeSize = realChangeVariations[changeVar];
				scn_ptr->changeSteps = CHANGES;
				scn_ptr->interpolationSteps = INTERPOLATIONS;
				
				//and, if needed, we run the manually checked tests:
				if (dumpInfo || printResults) {

					runReadTestScenario(scenarios[guessVar][infoVar][changeVar], dumpInfo, false);
					//(false -> we don't want to zero the updateRead prn on these tests)
				
					//possibly logging the aggregate results:
					if (printResults) {
						printf("\nSCN: %d-%d-%d (%d steps): avg: %f, dev: %f",
							guessVar, infoVar, changeVar, scn_ptr->changeSteps * scn_ptr->interpolationSteps,
							scn_ptr->avgDiff, scn_ptr->diffStdDev);
						printf("\n\t(initial diff: %f, info: %f, changeSize: %f)",
							initialReads[guessVar] - DEFAULT_LA_RESOURCES, scn_ptr->infiltration,
							scn_ptr->changeSize);
					}
				}

				//we then check if we're in an automatically checked case:
				bool shouldCouple = (scn_ptr->infiltration == 1) 
									&& (initialReads[guessVar] == scn_ptr->initialReal);
				bool shouldOrbit = (scn_ptr->infiltration == 0) 
									&& (initialReads[guessVar] == scn_ptr->initialReal);
				bool shouldExplode = (scn_ptr->infiltration == -1) 
									&& (initialReads[guessVar] == awfulFirstGuess);

				//and if so we run the test:
				if (shouldCouple || shouldOrbit || shouldExplode) {
					
					statisticalResults_t results;

					AS_TST::scenario_t returnedScn = 
						runReadTestScenario(scenarios[guessVar][infoVar][changeVar], false, true);
					//(true -> killing the random element of updateRead to assure determinism)

					//and gather the results:
					results.avgDiff = returnedScn.avgDiff;
					results.diffStdDev = returnedScn.diffStdDev;
					results.infiltration = returnedScn.infiltration;

					autoTestResults.push_back(results);
				}
			}
		}
	}

	//We dump the info, if calculated:

	//if(dumpInfo){
	//	create file;
	//	for test scenarios scenarios[guessVar][infoVar][changeVar]:
	//		{check outs capacity and add their info to the file};
	//	closeFile

	//And finally check the statistical results:
	bool result = true;
	
	//for autoTestResults:
	// 	if (inf = 1 && initialReads[guessVar] == real)
	//		result &= avg very close to zero, stdev too
	//	if (inf = 0 && initialReads[guessVar] == real)
	//		result &= avg close to zero, stdev a little higher
	// 	if (inf == -1 && initialReads[guessVar] == awfulFirstGuess)
	//		result &= avg close to initial, not too sure about stdev

	return result;
}