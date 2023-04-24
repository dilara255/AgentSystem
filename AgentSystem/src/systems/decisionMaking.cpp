#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "systems/AScoordinator.hpp"
#include "systems/actionSystem.hpp"
#include "systems/actionHelpers.hpp"
#include "systems/notions.hpp"

#include "network/parameters.hpp"

#include "systems/warningsAndErrorsCounter.hpp"
#include "data/dataMisc.hpp"

namespace AD = AS::Decisions;
namespace AV = AS::ActionVariations;

void calculateNotionsLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
             AD::notions_t* notions_ptr, LA::readsOnNeighbor_t* referenceReads_ptr, 
	                                                            int totalNeighbors);

//If decision is to do "doNothing", returns an innactive action
AS::actionData_t chooseActionLA(AD::notions_t* np, AD::LA::decisionScores_t* sp,
	                                int agent, AS::dataControllerPointers_t* dp);

int getTotalScoresLA(LA::stateData_t* state_ptr, int neighbors);

AS::actionData_t makeDecisionLA(int agent, AS::dataControllerPointers_t* dp, 
					 LA::stateData_t* state_ptr, LA::readsOnNeighbor_t* referenceReads_ptr, 
					 AS::WarningsAndErrorsCounter* errorsCounter_ptr,
					 const float secondsSinceLastDecisionStep, int currentActions) {
	
	AS::actionData_t nullAction;
	nullAction.ids.active = 0;

	if (!dp->LAdecision_ptr->getDataCptr()->at(agent).shouldMakeDecisions) {
		return nullAction;
	}
	if (currentActions >= MAX_ACTIONS_PER_AGENT) {
		return nullAction; //won't be able to spawn any action anyway;
	}
	if (currentActions == NATURAL_RETURN_ERROR) {
		errorsCounter_ptr->incrementWarning(AS::warnings::DS_LA_GOT_BAD_ACT_COUNT);
		return nullAction; //won't be able to charge for the action;
	}
	float cost = AS::nextActionsCost(currentActions);
	if ((cost > 0) && (cost > state_ptr->parameters.resources.current)) {
		return nullAction; //won't be able to pay for any action anyway
	}
	
	int neighbors = state_ptr->locationAndConnections.connectedNeighbors.howManyAreOn();

	//TODO: PERF: notions and scores can be static, if their allocation's measured to matter

	AD::notions_t notions;
	calculateNotionsLA(agent, dp, &notions, referenceReads_ptr, neighbors);

	AD::LA::decisionScores_t scores;
	scores.totalScores = getTotalScoresLA(state_ptr, neighbors);

	return chooseActionLA(&notions, &scores, agent, dp);
}

void calculateNotionsGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
             AD::notions_t* notions_ptr, GA::readsOnNeighbor_t* referenceReads_ptr, 
	                                                            int totalNeighbors);

//If decision is to do "doNothing", returns an innactive action
AS::actionData_t chooseActionGA(AD::notions_t* np, AD::GA::decisionScores_t* sp,
	                                int agent, AS::dataControllerPointers_t* dp);

int getTotalScoresGA(GA::stateData_t* state_ptr, int neighbors);

AS::actionData_t makeDecisionGA(int agent, AS::dataControllerPointers_t* dp,
				 GA::stateData_t* state_ptr, GA::readsOnNeighbor_t* referenceReads_ptr,
				 AS::WarningsAndErrorsCounter* errorsCounter_ptr,
				 const float secondsSinceLastDecisionStep, int currentActions) {

	AS::actionData_t nullAction;
	nullAction.ids.active = 0;

	if (!dp->GAdecision_ptr->getDataCptr()->at(agent).shouldMakeDecisions) {
		return nullAction;
	}
	if (currentActions >= MAX_ACTIONS_PER_AGENT) {
		return nullAction; //won't be able to spawn any action anyway;
	}
	if (currentActions == NATURAL_RETURN_ERROR) {
		errorsCounter_ptr->incrementWarning(AS::warnings::DS_GA_GOT_BAD_ACT_COUNT);
		return nullAction; //won't be able to charge for the action;
	}
	float cost = AS::nextActionsCost(currentActions);
	if ((cost > 0) && (cost > state_ptr->parameters.GAresources)) {
		return nullAction; //won't be able to pay for any action anyway
	}

	int neighbors = state_ptr->connectedGAs.howManyAreOn();

	//TODO: PERF: notions and scores can be static, if their allocation's measured to matter

	AD::notions_t notions;
	calculateNotionsGA(agent, dp, &notions, referenceReads_ptr, neighbors);

	AD::GA::decisionScores_t scores;
	scores.totalScores = getTotalScoresGA(state_ptr, neighbors);

	return chooseActionGA(&notions, &scores, agent, dp);
}

//LA:
int getTotalScoresLA(LA::stateData_t* state_ptr, int neighbors) {

	return AV::howManyActionsOfKind(AS::actModes::SELF, AS::scope::LOCAL)
		+ (neighbors * (
				AV::howManyActionsOfKind(AS::actModes::IMMEDIATE, AS::scope::LOCAL)
			  + AV::howManyActionsOfKind(AS::actModes::REQUEST, AS::scope::LOCAL) ) );
}

void calculateNotionsLA(int agent, AS::dataControllerPointers_t* dp, AD::notions_t* np,
	                                     LA::readsOnNeighbor_t* rp, int totalNeighbors) {

	int totalNotionsSelf = (int)AS::Decisions::notionsSelf::TOTAL;

	for (int notion = 0; notion < totalNotionsSelf; notion++) {

		np->self[notion] = 
			AS::Decisions::calculateNotionSelf((AS::Decisions::notionsSelf)notion, 
											          AS::scope::LOCAL, agent, dp);
	}

	int totalNotionsNeighbor = (int)AS::Decisions::notionsNeighbor::TOTAL;
	
	for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {
		for(int notion = 0; notion < totalNotionsNeighbor; notion++){
			
			np->neighbors[neighbor][notion] =
				AS::Decisions::calculateNotionNeighbor((AS::Decisions::notionsNeighbor)notion, 
					                                    AS::scope::LOCAL, neighbor, agent, dp);
		}
	}
}

AS::actionData_t chooseActionLA(AD::notions_t* np, AD::LA::decisionScores_t* sp,
	                                int agent, AS::dataControllerPointers_t* dp) {

	AS::actionData_t chosenAction;
	float chosenActionsScore = 0;

	//choose action
	//(if doNothing, chosenAction.ids.active == 0, else, == 1)
	
	setActionDetails(chosenActionsScore, ACT_WHY_BOTHER_THRESOLD, ACT_JUST_DO_IT_THRESOLD, 
		                                                                &chosenAction, dp);

	return chosenAction;
}

//GA:
int getTotalScoresGA(GA::stateData_t* state_ptr, int neighbors) {

	return AV::howManyActionsOfKind(AS::actModes::SELF, AS::scope::GLOBAL)
		+ (neighbors * (
				AV::howManyActionsOfKind(AS::actModes::IMMEDIATE, AS::scope::GLOBAL)
			  + AV::howManyActionsOfKind(AS::actModes::REQUEST, AS::scope::GLOBAL) ) );
}

void calculateNotionsGA(int agent, AS::dataControllerPointers_t* dp, AD::notions_t* np,
                                         GA::readsOnNeighbor_t* rp, int totalNeighbors) {

	int totalNotionsSelf = (int)AS::Decisions::notionsSelf::TOTAL;

	for (int notion = 0; notion < totalNotionsSelf; notion++) {

		np->self[notion] = 
			AS::Decisions::calculateNotionSelf((AS::Decisions::notionsSelf)notion, 
											         AS::scope::GLOBAL, agent, dp);
	}

	int totalNotionsNeighbor = (int)AS::Decisions::notionsNeighbor::TOTAL;
	
	for (int notion = 0; notion < totalNotionsNeighbor; notion++) {
		for(int neighbor = 0; neighbor < totalNeighbors; neighbor++){
			
			np->neighbors[neighbor][notion] =
				AS::Decisions::calculateNotionNeighbor((AS::Decisions::notionsNeighbor)notion, 
					                                   AS::scope::GLOBAL, neighbor, agent, dp);
		}
	}
}

AS::actionData_t chooseActionGA(AD::notions_t* np, AD::GA::decisionScores_t* sp,
	                                int agent, AS::dataControllerPointers_t* dp) {
	
	AS::actionData_t chosenAction;
	float chosenActionsScore = 0;

	//choose action
	//(if doNothing, chosenAction.ids.active == 0, else, == 1)
	
	setActionDetails(chosenActionsScore, ACT_WHY_BOTHER_THRESOLD, ACT_JUST_DO_IT_THRESOLD, 
		                                                                &chosenAction, dp);

	return chosenAction;
}