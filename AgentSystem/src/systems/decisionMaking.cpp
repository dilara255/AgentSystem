#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "systems/AScoordinator.hpp"
#include "systems/actionSystem.hpp"

#include "network/parameters.hpp"

#include "systems/warningsAndErrorsCounter.hpp"
#include "data/dataMisc.hpp"

namespace AD = AS::Decisions;
namespace AV = AS::ActionVariations;

void calculateNotionsLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
             AD::notions_t* notions_ptr, LA::readsOnNeighbor_t* referenceReads_ptr);
void scoreActionsByDesirabilityLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
															      AD::notions_t* notions_ptr, 
													     AD::LA::decisionScores_t* scores_ptr);
void redistributeScoreDueToImpedimmentsLA(int agent, 
	AS::dataControllerPointers_t* agentDataPtrs_ptr, AD::notions_t* notions_ptr,
	                                         AD::LA::decisionScores_t* scores_ptr);
void chooseActionLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                        AD::LA::decisionScores_t* scores_ptr);

int getTotalScoresLA(LA::stateData_t* state_ptr);

//TODO: updating of last dispositions should probably not be done in here
void makeDecisionLA(int agent, AS::dataControllerPointers_t* dp, 
					LA::stateData_t* state_ptr, AS::PRNserver* prnServer_ptr, 
	                LA::readsOnNeighbor_t* referenceReads_ptr, 
		            AS::WarningsAndErrorsCounter* errorsCounter_ptr,
	                       const float secondsSinceLastDecisionStep) {

	if (!dp->LAdecision_ptr->getDataCptr()->at(agent).shouldMakeDecisions) {
		return;
	}
	
	AD::notions_t notions;
	calculateNotionsLA(agent, dp, &notions, referenceReads_ptr);

	AD::LA::decisionScores_t scores;	
	scores.totalScores = getTotalScoresLA(state_ptr);		

	scoreActionsByDesirabilityLA(agent, dp, &notions, &scores);

	for(int i = 0; i < CONSTRAINT_CHECK_ROUNDS; i++){
		redistributeScoreDueToImpedimmentsLA(agent, dp, &notions, &scores);
	}

	//TODO: if I take out the random factor on action choosing, what does this become?
	chooseActionLA(agent, dp, &scores);
}

void calculateNotionsGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
             AD::notions_t* notions_ptr, GA::readsOnNeighbor_t* referenceReads_ptr);
void scoreActionsByDesirabilityGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
															      AD::notions_t* notions_ptr, 
													      AD::GA::decisionScores_t* scores_ptr);
void redistributeScoreDueToImpedimmentsGA(int agent, 
	AS::dataControllerPointers_t* agentDataPtrs_ptr, AD::notions_t* notions_ptr,
	                                         AD::GA::decisionScores_t* scores_ptr);
void chooseActionGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                        AD::GA::decisionScores_t* scores_ptr);

int getTotalScoresGA(GA::stateData_t* state_ptr);

//TODO: updating of last dispositions should probably not be done in here
void makeDecisionGA(int agent, AS::dataControllerPointers_t* dp,
				    GA::stateData_t* state_ptr, AS::PRNserver* prnServer_ptr, 
					GA::readsOnNeighbor_t* referenceReads_ptr,
		            AS::WarningsAndErrorsCounter* errorsCounter_ptr,
	                       const float secondsSinceLastDecisionStep) {

	if (!dp->GAdecision_ptr->getDataCptr()->at(agent).shouldMakeDecisions) {
		return;
	}

	AD::notions_t notions;
	calculateNotionsGA(agent, dp, &notions, referenceReads_ptr);

	AD::GA::decisionScores_t scores;
	scores.totalScores = getTotalScoresGA(state_ptr);

	scoreActionsByDesirabilityGA(agent, dp, &notions, &scores);

	for(int i = 0; i < CONSTRAINT_CHECK_ROUNDS; i++){
		redistributeScoreDueToImpedimmentsGA(agent, dp, &notions, &scores);
	}
	//TODO: if I take out the random factor on action choosing, what does this become?
	chooseActionGA(agent, dp, &scores);
}


//LA:

int getTotalScoresLA(LA::stateData_t* state_ptr) {
	int neighbors = state_ptr->locationAndConnections.connectedNeighbors.howManyAreOn();

	return AV::howManyActionsOfKind(AS::actModes::SELF, AS::scope::LOCAL)
		+ (neighbors * (
				AV::howManyActionsOfKind(AS::actModes::IMMEDIATE, AS::scope::LOCAL)
			  + AV::howManyActionsOfKind(AS::actModes::REQUEST, AS::scope::LOCAL) ) );
}

void calculateNotionsLA(int agent, AS::dataControllerPointers_t* dp, AD::notions_t* np,
	                                                         LA::readsOnNeighbor_t* rp) {

}

void scoreActionsByDesirabilityLA(int agent, AS::dataControllerPointers_t* dp, 
	                            AD::notions_t* np, AD::LA::decisionScores_t* sp) {

}

void redistributeScoreDueToImpedimmentsLA(int agent, AS::dataControllerPointers_t* dp,
			                            AD::notions_t* np, AD::LA::decisionScores_t* sp) {

}

void chooseActionLA(int agent, AS::dataControllerPointers_t* dp, 
	                                 AD::LA::decisionScores_t* sp) {

}

//GA:
int getTotalScoresGA(GA::stateData_t* state_ptr) {
	int neighbors = state_ptr->connectedGAs.howManyAreOn();

	return AV::howManyActionsOfKind(AS::actModes::SELF, AS::scope::GLOBAL)
		+ (neighbors * (
				AV::howManyActionsOfKind(AS::actModes::IMMEDIATE, AS::scope::GLOBAL)
			  + AV::howManyActionsOfKind(AS::actModes::REQUEST, AS::scope::GLOBAL) ) );
}

void calculateNotionsGA(int agent, AS::dataControllerPointers_t* dp, AD::notions_t* np,
	                                                         GA::readsOnNeighbor_t* rp) {

}

void scoreActionsByDesirabilityGA(int agent, AS::dataControllerPointers_t* dp, 
	                            AD::notions_t* np, AD::GA::decisionScores_t* sp) {

}

void redistributeScoreDueToImpedimmentsGA(int agent, AS::dataControllerPointers_t* dp,
													                AD::notions_t* np, 
											               AD::GA::decisionScores_t* sp) {

}

void chooseActionGA(int agent, AS::dataControllerPointers_t* dp, 
						             AD::GA::decisionScores_t* sp) {

}