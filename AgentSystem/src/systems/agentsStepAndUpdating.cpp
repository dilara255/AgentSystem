#include "systems/AScoordinator.hpp"
#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/diplomacy.hpp"
#include "data/dataMisc.hpp"

#include "systems/actionSystem.hpp"
#include "systems/actionHelpers.hpp"
#include "systems/actionCreation.hpp"

namespace {
	static AS::WarningsAndErrorsCounter* g_errorsCounter_ptr;
	static float g_secondsSinceLastDecisionStep = 0;
}

void updateLA(LA::stateData_t* state_ptr, int agentId, 
				  AS::dataControllerPointers_t* agentDataPtrs_ptr, float timeMultiplier,
		                                AS::WarningsAndErrorsCounter* errorsCounter_ptr);
void updateGA(GA::stateData_t* state_ptr, int agentId, 
				  AS::dataControllerPointers_t* agentDataPtrs_ptr, float timeMultiplier,
		                                AS::WarningsAndErrorsCounter* errorsCounter_ptr);

//Action returns as innactive in case no decision is made
//TODO-CRITICAL: BUG: scores + notions WILL overflow the stack for larger networks
AS::actionData_t makeDecisionLA(int agent, 
	             AS::dataControllerPointers_t* agentDataPtrs_ptr,
				 LA::stateData_t* state_ptr, LA::readsOnNeighbor_t* referenceReads_ptr,
	             AS::WarningsAndErrorsCounter* errorsCounter_ptr, 
				 const AS::ActionSystem* actionSystem_cptr,
	             AS::Decisions::networksDecisionsReflection_t* networksDecisionsReflection_ptr,
				 const float secondsSinceLastDecisionStep, int currentActions);

//Action returns as innactive in case no decision is made
//TODO-CRITICAL: BUG: scores + notions WILL overflow the stack for larger networks
AS::actionData_t makeDecisionGA(int agent, 
				 AS::dataControllerPointers_t* agentDataPtrs_ptr,
				 GA::stateData_t* state_ptr, GA::readsOnNeighbor_t* referenceReads_ptr,
	             AS::WarningsAndErrorsCounter* errorsCounter_ptr, 
				 const AS::ActionSystem* actionSystem_cptr,
	             AS::Decisions::networksDecisionsReflection_t* networksDecisionsReflection_ptr,
				 const float secondsSinceLastDecisionStep, int currentActions);

LA::readsOnNeighbor_t calculateLAreferences(int agentId, AS::dataControllerPointers_t* dp);
GA::readsOnNeighbor_t calculateGAreferences(int agentId, AS::dataControllerPointers_t* dp);
void updateRead(float* read, float real, float reference, float infiltration, 
							             float prnFrom0to1, float timeMultiplier);
void updateReadsLA(int agent, AS::dataControllerPointers_t* dp, LA::stateData_t* state_ptr, 
                             LA::readsOnNeighbor_t* refs_ptr, AS::PRNserver* prnServer_ptr);
void updateReadsGA(int agent, AS::dataControllerPointers_t* dp, GA::stateData_t* state_ptr, 
                             GA::readsOnNeighbor_t* refs_ptr, AS::PRNserver* prnServer_ptr);

void updatedLastDispositionsLA(LA::stateData_t* agentState_ptr);
void updatedLastDispositionsGA(GA::stateData_t* agentState_ptr);

void updateInfiltrationAndRelationsFromLAs(int agent, AS::dataControllerPointers_t* dp, 
           GA::stateData_t* state_ptr, AS::WarningsAndErrorsCounter* errorsCounter_ptr);

//WARNING: This is the only entry point to this file, and should remain so!
void AS::stepAgents(int LAdecisionsToTakeThisChop, int GAdecisionsToTakeThisChop, 
                    dataControllerPointers_t* dp, ActionSystem* actionSystem_ptr,
	                 float timeMultiplier, int numberLAs, int numberEffectiveGAs,
		                             WarningsAndErrorsCounter* errorsCounter_ptr,
									   const AS::ActionSystem* actionSystem_cptr,
   AS::Decisions::networksDecisionsReflection_t* networksDecisionsReflection_ptr,
	                            bool makeDecisions, AS::PRNserver* prnServer_ptr,
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

	while ( nextDecisionLAindex <= finalDecisionLAindex ) {
		//Note that finalDecisionLAindex may be larger than the index of the last LA...
		//so we modulo it in here:
		int agent = nextDecisionLAindex % numberLAs;
		LA::stateData_t* state_ptr = &(dp->LAstate_ptr->getDirectDataPtr()->at(agent));

		if (state_ptr->onOff) {

			LA::readsOnNeighbor_t referenceReads =  calculateLAreferences(agent, dp);
			updateReadsLA(agent, dp, state_ptr, &referenceReads, prnServer_ptr);

			if(makeDecisions) {
				int currentActions = AS::getQuantityOfCurrentActions(AS::scope::LOCAL, agent,
														 actionSystem_ptr, errorsCounter_ptr);

				actionData_t chosenAction = 
						makeDecisionLA(agent, dp, state_ptr, &referenceReads, 
							           errorsCounter_ptr, actionSystem_cptr,
							           networksDecisionsReflection_ptr,
							           g_secondsSinceLastDecisionStep, currentActions);
				
				//In case no decision is made, makeDecisionLA returns an innactive action, so:
				if( AS::isActionValid(&chosenAction) ) {
					chargeForAndSpawnAction(chosenAction, dp, actionSystem_ptr, 
														     errorsCounter_ptr);
				}
			}

			updatedLastDispositionsLA(state_ptr);
		}

		nextDecisionLAindex++;
	}
	nextDecisionLAindex %= numberLAs; //and then once outside the loop we modulo the static value
		
	//For GAs:
	static int nextDecisionGAindex = 0;	
	int finalDecisionGAindex = nextDecisionGAindex + GAdecisionsToTakeThisChop - 1;

	while ( nextDecisionGAindex <= finalDecisionGAindex ) {	
		//for the same reason we modulo the index here as well:
		int agent = nextDecisionGAindex % numberEffectiveGAs;
		GA::stateData_t* state_ptr = &(dp->GAstate_ptr->getDirectDataPtr()->at(agent));

		if (state_ptr->onOff) {

			updateInfiltrationAndRelationsFromLAs(agent, dp, state_ptr, errorsCounter_ptr);

			GA::readsOnNeighbor_t referenceReads =  calculateGAreferences(agent, dp);
			updateReadsGA(agent, dp, state_ptr, &referenceReads, prnServer_ptr);

			if(makeDecisions) {
				int currentActions = AS::getQuantityOfCurrentActions(AS::scope::GLOBAL, agent,
														  actionSystem_ptr, errorsCounter_ptr);

				actionData_t chosenAction = 
						makeDecisionGA(agent , dp, state_ptr, &referenceReads, 
									   errorsCounter_ptr, actionSystem_cptr,
							           networksDecisionsReflection_ptr,
							           g_secondsSinceLastDecisionStep, currentActions);

				//In case no decision is made, makeDecisionGA returns an innactive action, so:
				if( AS::isActionValid(&chosenAction) ){
					chargeForAndSpawnAction(chosenAction, dp, actionSystem_ptr,
														     errorsCounter_ptr);
				}
			}

			updatedLastDispositionsGA(state_ptr);			
		}

		nextDecisionGAindex++;
	}
	nextDecisionGAindex %= numberEffectiveGAs;
}

//TODO-CRITICAL: this is time-step dependent (naive numerical integration)
void defectFromDebt(LA::stateData_t* state_ptr, 
	                AS::dataControllerPointers_t* dp, float timeMultiplier) {

	auto str_ptr = &(state_ptr->parameters.strenght.current);

	float strToDefect = (*str_ptr) * STR_PROPORTION_DEFECTED_PER_SECOND * timeMultiplier;
	*str_ptr -= strToDefect;

	float strToShareNeighbors = strToDefect * STR_DEFECTION_PROPORTION_TO_GO_TO_NEIGHBORS;

	int totalNeighbors = state_ptr->locationAndConnections.connectedNeighbors.howManyAreOn();

	float neighborShare = 0;
	if(totalNeighbors > 0) {
		neighborShare = strToShareNeighbors / totalNeighbors;
	}
	auto LAstates_ptr = dp->LAstate_ptr->getDirectDataPtr();

	for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {
		int neighborID = state_ptr->locationAndConnections.neighbourIDs[neighbor];
		LAstates_ptr->at(neighborID).parameters.strenght.current += neighborShare;
	}
}

void updateLA(LA::stateData_t* state_ptr, int agentId, 
	          AS::dataControllerPointers_t* dp, float timeMultiplier,
	                 AS::WarningsAndErrorsCounter* errorsCounter_ptr) {
	
	if (state_ptr->onOff == false) {
		return;
	}
	
	auto res_ptr = &state_ptr->parameters.resources;
	auto str_ptr = &state_ptr->parameters.strenght;

	assert(isfinite(res_ptr->current));

	str_ptr->currentUpkeep = AS::calculateUpkeep(str_ptr->current, str_ptr->externalGuard, 
													       str_ptr->thresholdToCostUpkeep);
	
	assert(isfinite(str_ptr->currentUpkeep));
	assert(isfinite(res_ptr->updateRate));

	float maxDebt = AS::getMaxDebt(res_ptr->updateRate);
	float liquidIncome = res_ptr->updateRate - str_ptr->currentUpkeep;

	//now we can update the current resources:
	if( (liquidIncome >= 0) || (res_ptr->current > maxDebt) ) {
		//Note that if the agent is in too much debt, no more will be charged
		res_ptr->current += (res_ptr->updateRate - str_ptr->currentUpkeep) * timeMultiplier;
	}
	else if ((liquidIncome < 0) && (res_ptr->current <= maxDebt)) {
		//we're deep in debt and still bleeding money: our troops are defecting
		defectFromDebt(state_ptr, dp, timeMultiplier);
	}
	
	assert(isfinite(res_ptr->current));
		
	//But resources, infiltration and relations can also change due to diplomacy:
	//TODO: EXTRACT and leave definition on diplomacy.cpp
	auto decision_ptr = &(dp->LAdecision_ptr->getDirectDataPtr()->at(agentId));

	LA::applyAttritionTradeInfiltrationAndDispostionChanges(agentId, timeMultiplier, 
	                                 state_ptr, decision_ptr, dp, errorsCounter_ptr);

	//finally, LAs "pay tax" to GA (and can receive resources from the GA if in debt):
	res_ptr->taxRate = taxPayedPerSecond(*res_ptr);

	res_ptr->current -= res_ptr->taxRate * timeMultiplier;

	//let's also make sure disposition and infiltration remain bounded to [-1,1]
	auto infiltrationArr_ptr = &(decision_ptr->infiltration[0]);
	auto dispositionArr_ptr = &(state_ptr->relations.dispositionToNeighbors[0]);

	int quantityNeighbours = state_ptr->locationAndConnections.numberConnectedNeighbors;
	for (int neighbor = 0; neighbor < quantityNeighbours; neighbor++) {
		//first we want to clamp the infiltration:
		if (!std::isfinite(infiltrationArr_ptr[neighbor])){
			//but NaN and the like always compare false, so they don't clamp
			infiltrationArr_ptr[neighbor] = 0;
			errorsCounter_ptr->incrementError(AS::errors::AS_LA_INFILTRATION_NOT_FINITE);
		}
		else {
			//If the number is reasonable, then we clamp:
			infiltrationArr_ptr[neighbor] =
			std::clamp(infiltrationArr_ptr[neighbor], MIN_INFILTRATION, MAX_INFILTRATION);
		}
		//same for disposition:
		if (!std::isfinite(dispositionArr_ptr[neighbor])){
			dispositionArr_ptr[neighbor] = 0;
			errorsCounter_ptr->incrementError(AS::errors::AS_LA_DISPOSITION_NOT_FINITE);
		}
		else {
			dispositionArr_ptr[neighbor] =
			std::clamp(dispositionArr_ptr[neighbor], MIN_DISPOSITION, MAX_DISPOSITION);
		}	
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

		assert(isfinite(param_ptr->LAesourceTotals.current));
	}
	
	//Get resoures from tax...
	param_ptr->lastTaxIncome = taxOwed(state_ptr->parameters.LAesourceTotals, timeMultiplier);
	param_ptr->GAresources += param_ptr->lastTaxIncome;

	//Get resources from trade and update infiltration and disposition to each neighbor:
	auto decision_ptr = &(dp->GAdecision_ptr->getDirectDataPtr()->at(agentId));
	GA::applyTradeInfiltrationAndDispostionChanges(state_ptr, decision_ptr, agentId, dp, 
													  timeMultiplier, errorsCounter_ptr);

	//and make sure disposition and infiltration remain bounded to [-1,1]
	auto infiltrationArr_ptr = &(decision_ptr->infiltration[0]);
	auto dispositionArr_ptr = &(state_ptr->relations.dispositionToNeighbors[0]);

	int quantityNeighbours = state_ptr->connectedGAs.howManyAreOn();
	for (int neighbor = 0; neighbor < quantityNeighbours; neighbor++) {
		//first we want to clamp the infiltration:
		if (!std::isfinite(infiltrationArr_ptr[neighbor])){
			//but NaN and the like always compare false, so they don't clamp
			infiltrationArr_ptr[neighbor] = 0;
			errorsCounter_ptr->incrementError(AS::errors::AS_GA_INFILTRATION_NOT_FINITE);
		}
		else {
			//If the number is reasonable, then we clamp:
			infiltrationArr_ptr[neighbor] =
			std::clamp(infiltrationArr_ptr[neighbor], MIN_INFILTRATION, MAX_INFILTRATION);
		}
		//same for disposition:
		if (!std::isfinite(dispositionArr_ptr[neighbor])){
			dispositionArr_ptr[neighbor] = 0;
			errorsCounter_ptr->incrementError(AS::errors::AS_GA_DISPOSITION_NOT_FINITE);
		}
		else {
			dispositionArr_ptr[neighbor] =
			std::clamp(dispositionArr_ptr[neighbor], MIN_DISPOSITION, MAX_DISPOSITION);
		}		
	}
}

void updatedLastDispositionsLA(LA::stateData_t* agentState_ptr) {

	int neighbors = agentState_ptr->locationAndConnections.connectedNeighbors.howManyAreOn();

	for (int i = 0; i < neighbors; i++) {
		agentState_ptr->relations.dispositionToNeighborsLastStep[i] =
						agentState_ptr->relations.dispositionToNeighbors[i];
	}
}

void updatedLastDispositionsGA(GA::stateData_t* agentState_ptr) {

	int neighbors = agentState_ptr->connectedGAs.howManyAreOn();

	for (int i = 0; i < neighbors; i++) {
		agentState_ptr->relations.dispositionToNeighborsLastStep[i] =
						agentState_ptr->relations.dispositionToNeighbors[i];
	}
}

//Reads disposition and infiltration of connected LAs towards other LAs from each GA.
//Interpolates gaID's disposition and infiltration towards each connected GA with that.
//TODO-CRITICAL: testing
void updateInfiltrationAndRelationsFromLAs(int gaID, AS::dataControllerPointers_t* dp, 
		   GA::stateData_t* state_ptr, AS::WarningsAndErrorsCounter* errorsCounter_ptr) {
	
	//This will be collected from all connected LAs and then interpolated with the GA data:
	struct neighborData_st {
		int timesNeighborAppeared = 0;
		float infiltration = 0;
		float relation = 0;
	};

	//The data will be stored here:
	neighborData_st dataFromLAs[MAX_GA_QUANTITY];

	//Now we loop the agents. For each neighbor, check it's GA 
	//and increment the data regarding it (even if it's not a neighbor of this GA)
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

		int timesAppeared = dataFromLAs[neighborID].timesNeighborAppeared;

		//calculate weights:
		float proportion = (float)timesAppeared / totalInfluences;

		float weightLAs = proportion * TOTAL_LA_INFO_RELATION_WEIGHT_FOR_GA_PER_SECOND
						  * g_secondsSinceLastDecisionStep;
		float weightGA = 1 - weightLAs;

		//We only care abut neighbors which appeared:
		if (timesAppeared > 0) {
			//take the average:
			dataFromLAs[neighborID].infiltration /= dataFromLAs[neighborID].timesNeighborAppeared;
			dataFromLAs[neighborID].relation /= dataFromLAs[neighborID].timesNeighborAppeared;

			//interpolate values:
			state_ptr->relations.dispositionToNeighbors[neighbor] *= weightGA;
			state_ptr->relations.dispositionToNeighbors[neighbor] += 
											weightLAs * dataFromLAs[neighborID].relation;
			infiltration_ptr[neighbor] *= weightGA;
			infiltration_ptr[neighbor] += weightLAs * dataFromLAs[neighborID].infiltration;
		}

		//some sanity checks (division by zero can be traumatizing):
		if (!std::isfinite(infiltration_ptr[neighbor])){
			errorsCounter_ptr->incrementError(AS::errors::AS_GA_INFILTRATION_FROM_LAS_NOT_FINITE);
		}
		if (!std::isfinite(state_ptr->relations.dispositionToNeighbors[neighbor])){
			errorsCounter_ptr->incrementError(AS::errors::AS_GA_DISPOSITION_FROM_LAS_NOT_FINITE);
		}
	}
}

//READS AND EXPECTATIONS:
//TODO: EXTRACT STUFF AND ADD WARNING AND ERROR HANDLING (REFACTOR: PULL SOME INTO CLASSES)

LA::readsOnNeighbor_t getRealValuesLA(AS::dataControllerPointers_t* dp, int neighborID);
GA::readsOnNeighbor_t getRealValuesGA(AS::dataControllerPointers_t* dp, int neighborID);

void updateReadsLA(int agent, AS::dataControllerPointers_t* dp, LA::stateData_t* state_ptr, 
				             LA::readsOnNeighbor_t* refs_ptr, AS::PRNserver* prnServer_ptr) {

	auto decisionData_ptr = &(dp->LAdecision_ptr->getDirectDataPtr()->at(agent));
	LA::readsOnNeighbor_t* reads_ptr = &(decisionData_ptr->reads[0]);

	int totalNeighbors = state_ptr->locationAndConnections.numberConnectedNeighbors;

	for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {

		int neighborID = state_ptr->locationAndConnections.neighbourIDs[neighbor];
		auto readsOnNeighbor_ptr = &reads_ptr[neighbor];

		LA::readsOnNeighbor_t realValues = getRealValuesLA(dp, neighborID);
		float infiltration = decisionData_ptr->infiltration[neighbor];

		for (int field = 0; field < (int)LA::readsOnNeighbor_t::fields::TOTAL; field++) {

			assert(isfinite(refs_ptr->readOf[field]));
			assert(isfinite(realValues.readOf[field]));

			//this is the payload:
			updateRead(&readsOnNeighbor_ptr->readOf[field], realValues.readOf[field], 
		             refs_ptr->readOf[field], infiltration, prnServer_ptr->getNext(), 
													  g_secondsSinceLastDecisionStep);
		}
	}	
}

void updateReadsGA(int agent, AS::dataControllerPointers_t* dp, GA::stateData_t* state_ptr, 
                             GA::readsOnNeighbor_t* refs_ptr, AS::PRNserver* prnServer_ptr) {

	auto decisionData_ptr = &(dp->GAdecision_ptr->getDirectDataPtr()->at(agent));
	GA::readsOnNeighbor_t* reads_ptr = &(decisionData_ptr->reads[0]);

	int totalNeighbors = state_ptr->connectedGAs.howManyAreOn();

	for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {

		int neighborID = state_ptr->neighbourIDs[neighbor];
		auto readsOnNeighbor_ptr = &reads_ptr[neighbor];
		
		GA::readsOnNeighbor_t realValues = getRealValuesGA(dp, neighborID);
		float infiltration = decisionData_ptr->infiltration[neighbor];

		for (int field = 0; field < (int)GA::readsOnNeighbor_t::fields::TOTAL; field++) {
			
			assert(isfinite(refs_ptr->readOf[field]));
			assert(isfinite(realValues.readOf[field]));

			//this is the payload:
			updateRead(&(readsOnNeighbor_ptr->readOf[field]), realValues.readOf[field], 
						refs_ptr->readOf[field], infiltration, prnServer_ptr->getNext(), 
														g_secondsSinceLastDecisionStep);
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
	
	assert(isfinite(neighborState_ptr->parameters.resources.current));

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
	
	/*ref[i] = D * (SUM_viz(expec[viz][i] * abs(inf[viz]) / SUM_viz(abs(inf[viz]))
	         + E * GA[i]/#LAs
	*/
	
	auto state_ptr = &(dp->LAstate_ptr->getDataCptr()->at(agentId));
	auto decision_ptr = &(dp->LAdecision_ptr->getDataCptr()->at(agentId));

	LA::readsOnNeighbor_t GAreferences;
	auto GAstate_ptr = &(dp->GAstate_ptr->getDataCptr()->at(state_ptr->GAid));
	auto GAparams_ptr = &(GAstate_ptr->parameters);

	GAreferences.readOf[(int)LA::readsOnNeighbor_t::fields::GUARD] =
										GAparams_ptr->LAguardTotal;
	assert(isfinite(GAparams_ptr->LAguardTotal));

	GAreferences.readOf[(int)LA::readsOnNeighbor_t::fields::INCOME] =
										GAparams_ptr->LAesourceTotals.updateRate;
	assert(isfinite(GAparams_ptr->LAesourceTotals.updateRate));

	GAreferences.readOf[(int)LA::readsOnNeighbor_t::fields::RESOURCES] =
										GAparams_ptr->LAesourceTotals.current;
	assert(isfinite(GAparams_ptr->LAesourceTotals.current));

	GAreferences.readOf[(int)LA::readsOnNeighbor_t::fields::STRENGHT] =
										GAparams_ptr->LAstrenghtTotal;
	assert(isfinite(GAparams_ptr->LAstrenghtTotal));

	LA::readsOnNeighbor_t references; 
	for (int field = 0; field < (int)LA::readsOnNeighbor_t::fields::TOTAL; field++) {
			references.readOf[field] = 0;
	}

	int totalNeighbors = state_ptr->locationAndConnections.numberConnectedNeighbors;
	float totalAbsoluteInfiltration = 0;

	for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {

		float absIniltration =  std::abs(decision_ptr->infiltration[neighbor]);
		totalAbsoluteInfiltration += absIniltration;
		for (int field = 0; field < (int)LA::readsOnNeighbor_t::fields::TOTAL; field++) {

			references.readOf[field] += 
				(decision_ptr->reads[neighbor].readOf[field] * absIniltration);
		}
	}

	int totalLAs = GAstate_ptr->localAgentsBelongingToThis.howManyAreOn();
	assert(totalLAs > 0); //at least the agent, right?
	for (int field = 0; field < (int)LA::readsOnNeighbor_t::fields::TOTAL; field++) {

		float effectiveReference = 0;
		if (totalAbsoluteInfiltration == 0) {
			effectiveReference = references.readOf[field] / totalAbsoluteInfiltration;
		}

		references.readOf[field] = (D * effectiveReference) 
									+ (E * GAreferences.readOf[field] / totalLAs);
		
		assert(isfinite(references.readOf[field]));
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
	assert(isfinite(GAparams_ptr->GAresources));

	GAreferences.readOf[(int)GA::readsOnNeighbor_t::fields::GUARD_LAS] =
										GAparams_ptr->LAguardTotal;
	assert(isfinite(GAparams_ptr->LAguardTotal));

	GAreferences.readOf[(int)GA::readsOnNeighbor_t::fields::STRENGHT_LAS] =
										GAparams_ptr->LAstrenghtTotal;
	assert(isfinite(GAparams_ptr->LAstrenghtTotal));

	GAreferences.readOf[(int)GA::readsOnNeighbor_t::fields::TAX_INCOME] =
										GAparams_ptr->lastTaxIncome;
	assert(isfinite(GAparams_ptr->lastTaxIncome));

	GAreferences.readOf[(int)GA::readsOnNeighbor_t::fields::TRADE_INCOME] =
										GAparams_ptr->lastTradeIncome;
	assert(isfinite(GAparams_ptr->lastTradeIncome));
	
	GA::readsOnNeighbor_t references; 
	for (int field = 0; field < (int)GA::readsOnNeighbor_t::fields::TOTAL; field++) {
			references.readOf[field] = 0;
	}

	int totalNeighbors = state_ptr->connectedGAs.howManyAreOn();
	float totalAbsoluteInfiltration = 0;

	for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {

		float absoluteIniltration =  std::abs(decision_ptr->infiltration[neighbor]);
		totalAbsoluteInfiltration += absoluteIniltration;
		for (int field = 0; field < (int)GA::readsOnNeighbor_t::fields::TOTAL; field++) {

			references.readOf[field] += 
				(decision_ptr->reads[neighbor].readOf[field] * absoluteIniltration);

			assert(isfinite(references.readOf[field]));
		}
	}

	for (int field = 0; field < (int)GA::readsOnNeighbor_t::fields::TOTAL; field++) {
		
		float effectiveReference = 0;
		if( totalAbsoluteInfiltration != 0 ){
			effectiveReference = references.readOf[field] / totalAbsoluteInfiltration;
		}
				
		references.readOf[field] = (D * effectiveReference) + (E * GAreferences.readOf[field]);

		assert(isfinite(references.readOf[field]));
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

//TODO: document the math
//TODO: use the same order for the difference between real and read for this and its tets
//TODO: this sorta supposes real values are almost always positive. Either make that so, or fix this
void updateRead(float* read_ptr, float real, float reference, float infiltration, 
										 float prnFrom0to1, float timeMultiplier) {
	
	assert( (prnFrom0to1 >= 0.f) && (prnFrom0to1 <= 1.f) );
	assert(isfinite(real));
	assert(isfinite(reference));

	//Reads can get dangerously unstable if the time-multiplier is too high
	//TODO: timeMultiplier ceiling

	static const float MAX_CORRECTION_WEIGHT_RELATIVE_TO_INFO = powf(FAC_CW , S);
	static const float C = A*MAX_CORRECTION_WEIGHT_RELATIVE_TO_INFO/powf(EXPC_PROPORTIONAL_ERROR_OVER_MINIMUM_FOR_MAX_CORRECTION,S);
	
	float effectiveReference = std::max(abs(reference), abs(real));
	if (effectiveReference < 1) {
		effectiveReference = 1; //"neutral" to multiplication and division
	}

	assert(isfinite(effectiveReference));

	float difference = real - (*read_ptr);

	assert(isfinite(difference));

	float minimumDifference = EXPC_EFFECTIVE_MIN_PROPORTIONAL_DIFF * effectiveReference;
	int sign = difference >= 0 ? 1 : -1;
	if (abs(difference) < abs(minimumDifference)) {
		difference = minimumDifference * sign;
	}

	assert(isfinite(difference) && true);

	float correctionFactor = std::min(EXPC_PROPORTIONAL_ERROR_FOR_MAX_CORRECTION, 
		                                      abs(difference)/effectiveReference);
	correctionFactor = std::max(0.f, (correctionFactor - BIAS));
	float multiplier = 
		std::min(2.0f, ( 
						A*infiltration*fabs(infiltration) 
						+ B*( (2*prnFrom0to1) - 1) 
						+ C*(powf(correctionFactor, S))
					) );

	assert(isfinite(multiplier));
	
	float change = difference * multiplier * timeMultiplier;
	float absChange = std::abs(change);
	if (absChange > EXPC_MINIMUM_CHANGE_TO_WORRY) {
		change = sign * std::min(absChange, std::abs(difference));
	}
	
	assert(isfinite(change));

	*read_ptr += change;
	*read_ptr = std::max(-*read_ptr, *read_ptr);
}

namespace AS_TST {
	void updateReadTest(float* read, float real, float reference, float infiltration,
		float prnFrom0to1, float timeMultiplier) {

		updateRead(read, real, reference, infiltration, prnFrom0to1, timeMultiplier);
	}
}
