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

//If decision is to do "doNothing", returns an innactive action
AS::actionData_t chooseAction(AD::notions_t* np, AD::allScoresAnyScope_t* sp,
							     int agent, AS::dataControllerPointers_t* dp, 
								 	     AS::scope scope, int totalNeighbors,
	                         AS::WarningsAndErrorsCounter* errorsCounter_ptr);

void calculateNotionsLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
             AD::notions_t* notions_ptr, LA::readsOnNeighbor_t* referenceReads_ptr, 
               int totalNeighbors, AS::WarningsAndErrorsCounter* errorsCounter_ptr);

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
	calculateNotionsLA(agent, dp, &notions, referenceReads_ptr, neighbors, errorsCounter_ptr);

	AD::allScoresAnyScope_t scores;
	scores.actualTotalScores = getTotalScoresLA(state_ptr, neighbors);
	
	AS::actionData_t choice =
		chooseAction(&notions, &scores, agent, dp, AS::scope::LOCAL, neighbors,
			                                                 errorsCounter_ptr);

	//TODO: add more sanity checks
	if ( (choice.ids.target >= (uint32_t)neighbors) 
		  && (choice.ids.mode != (uint32_t)AS::actModes::SELF) 
		  && (choice.ids.active == 1) ) {
		
		errorsCounter_ptr->incrementError(AS::errors::DS_CHOSE_INVALID_LA_TARGET);
		choice.ids.active = 0; //invalidate choice so we don't blow stuff up
	}

	return choice;
}

void calculateNotionsGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
             AD::notions_t* notions_ptr, GA::readsOnNeighbor_t* referenceReads_ptr, 
               int totalNeighbors, AS::WarningsAndErrorsCounter* errorsCounter_ptr);

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
	calculateNotionsGA(agent, dp, &notions, referenceReads_ptr, neighbors, errorsCounter_ptr);

	AD::allScoresAnyScope_t scores;
	scores.actualTotalScores = getTotalScoresGA(state_ptr, neighbors);

	AS::actionData_t choice = 
		chooseAction(&notions, &scores, agent, dp, AS::scope::GLOBAL, neighbors, 
			                                                  errorsCounter_ptr);
	
	//TODO: add more sanity checks
	if ( (choice.ids.target >= (uint32_t)neighbors)
		  && (choice.ids.mode != (uint32_t)AS::actModes::SELF) ) {
		
		errorsCounter_ptr->incrementError(AS::errors::DS_CHOSE_INVALID_GA_TARGET);
		choice.ids.active = 0; //invalidate choice so we don't blow stuff up
	}

	return choice;
}

void setScore(AD::actionScore_t* actionScore_ptr, AD::notions_t* np,
	                 const AD::notionsWeightsArray_t* notionWeights) {
	
	int cat = actionScore_ptr->actCategory;
	int mode = actionScore_ptr->actMode;

	//Actions in SELF mode will use self and neighbor average notions
	//Actions in other modes will use self and neighbor notions, so:
	float* notionsNeighbor_ptr;
	if (mode == (int)AS::actModes::SELF) {
		notionsNeighbor_ptr = np->averages;
	}
	else {
		notionsNeighbor_ptr = np->neighbors[actionScore_ptr->neighbor];
	}	

	float score = 0;

	//notions related to self:
	int notionsSelf = (int)AD::notionsSelf::TOTAL;
	for (int notion = 0; notion < notionsSelf; notion++) {

		score += notionWeights->at(cat).at(mode).at(notion) * np->self[notion];
	}

	//notions related to neighbor (or average of notions related to neighbors):	
	for (int notion = 0; notion < (int)AD::notionsNeighbor::TOTAL; notion++) {

		int notionWeightID = notion + notionsSelf;

		score += 
			notionWeights->at(cat).at(mode).at(notionWeightID) * notionsNeighbor_ptr[notion];
	}	

	actionScore_ptr->score = score;
}

//TODO: throughly test this (note loop index calculation, also test callees):
float calculateScores(AD::notions_t* np, AD::allScoresAnyScope_t* allScores_ptr, 
							                 AS::scope scope, int totalNeighbors) {

	assert(allScores_ptr->actualTotalScores > 0);

	//We will first calculate the scores of actions in SELF mode, then the others
	//Actions in SELF mode will leave neighbor = -1, others will also set the neighbor
	
	//TODO-CRITICAL: WARNING: all of the following pressupposes SELF = 0!
	assert((int)AS::actModes::SELF == 0);

	float maxAmbition = -1;
	//For self, both neighbor (-1) and mode (SELF, 0) are fixed, so:
	int totalActionsSelf = AS::ActionVariations::TOTAL_CATEGORIES;
	for (int cat = 0; cat < totalActionsSelf; cat++) {		

		auto sp = &(allScores_ptr->allScores[cat]);

		sp->ambitions.neighbor = NEIGHBOR_ID_FOR_SELF;
		sp->ambitions.actCategory = cat;
		sp->ambitions.actMode = (int)AS::actModes::SELF;
		setScore(&(sp->ambitions), np, &AD::notionWeightsInFavor);
				
		maxAmbition = std::max(maxAmbition, sp->ambitions.score);
				
		sp->worries.neighbor = NEIGHBOR_ID_FOR_SELF;
		sp->worries.actCategory = cat;
		sp->worries.actMode = (int)AS::actModes::SELF;
		setScore(&(sp->worries), np, &AD::notionWeightsAgainst);

		sp->overallUtility.neighbor = NEIGHBOR_ID_FOR_SELF;
		sp->overallUtility.actCategory = cat;
		sp->overallUtility.actMode = (int)AS::actModes::SELF;
				
		sp->overallUtility.score = sp->ambitions.score - sp->worries.score;
	}

	//Now we'll deal with the scores regarding neighbors:
	int widthPerNeighbor = 
				AS::ActionVariations::TOTAL_CATEGORIES * AS::ActionVariations::TOTAL_MODES;
	int widthPerCategory = AS::ActionVariations::TOTAL_MODES - 1; //SELF excluded
	
	int modesRegardingNeighbors = AS::ActionVariations::TOTAL_MODES - 1;

	for(int neighbor = 0; neighbor < totalNeighbors; neighbor++){
		for (int cat = 0; cat < AS::ActionVariations::TOTAL_CATEGORIES; cat++) {
			//mode will start at 1 so we exclude SELF
			for (int mode = 0; mode < modesRegardingNeighbors; mode++) {
			
				int index = totalActionsSelf
							+ (neighbor * widthPerNeighbor) + (cat * widthPerCategory) + mode;
				auto sp = &(allScores_ptr->allScores[index]);

				sp->ambitions.actCategory = cat;
				sp->ambitions.actMode = mode + 1; //to account for SELF
				sp->ambitions.neighbor = neighbor;
				setScore(&(sp->ambitions), np, &AD::notionWeightsInFavor);
				
				maxAmbition = std::max(maxAmbition, sp->ambitions.score);
				
				sp->worries.actCategory = cat;
				sp->worries.actMode =  mode + 1; //to account for SELF
				sp->worries.neighbor = neighbor;
				setScore(&(sp->worries), np, &AD::notionWeightsAgainst);

				sp->overallUtility.actCategory = cat;
				sp->overallUtility.actMode =  mode + 1; //to account for SELF
				sp->overallUtility.neighbor = neighbor;
				
				sp->overallUtility.score =
								sp->ambitions.score - sp->worries.score;
			}
		}
	}

	return maxAmbition;
}

//If doNothing, returns innactive action. Else, score is recorded on action's intensity
//Also returns innactive action in case of error.
//TODO: full testing (including the sorting)
AS::actionData_t doLeastHarmful(AD::allScoresAnyScope_t* allScores_ptr, 
											int agent, AS::scope scope, 
					   AS::WarningsAndErrorsCounter* errorsCounter_ptr) {
	//We first sort scores from least to most worriesome:
	std::sort(&allScores_ptr->allScores[0], 
		      &allScores_ptr->allScores[allScores_ptr->actualTotalScores - 1], 
		      AD::ascendingWorriesCompare);

	//Then we try to choose the least worriesome action with overallUtility >= 0:
	int i = 0;
	bool chose = false;
	while ( (!chose) && (i < allScores_ptr->actualTotalScores) ) {

		if (allScores_ptr->allScores[i].overallUtility.score > 0.0f) {
			chose = true;
		}
		else {
			i++;
		}
	}
		
	AS::actionData_t choice;

	auto decision_ptr = &(allScores_ptr->allScores[i].overallUtility);
	if (chose) {
		choice.ids.active = 1;

		if ( (decision_ptr->neighbor == NEIGHBOR_ID_FOR_SELF)
			  && (decision_ptr->actMode != (int)AS::actModes::SELF) ) {

			errorsCounter_ptr->incrementError(AS::errors::DS_NEIGHBOR_MARKED_SELF_WRONG_MODE_ON_LEAST_HARM);
			choice.ids.active = 0; //invalidate
		}

		choice.ids.category = decision_ptr->actCategory;
		choice.ids.mode = decision_ptr->actMode;
		choice.ids.scope = (int)scope;
		choice.ids.origin = agent;
		choice.ids.target = decision_ptr->neighbor;
		choice.details.intensity = decision_ptr->score;
	}
	else {
		choice.ids.active = 0; //Marks as "doNothing"
	}

	return choice;
}

//Notions wich impact negatively actions with high ambition score get more weight
//NOTE: expects to receive allScores_ptr ordered by descending ambition
//TODO: test : )
void prepareForMitigationRound(AD::notionWeights_t wp, AD::notions_t* np,
						           AD::allScoresAnyScope_t* allScores_ptr) {

	//TODO-CRITICAL: These will be part of agent's personalities in the future: FIX then
	int goalShortlistSize = ACT_GOAL_SHORTLIST_SIZE;

	//First we set some initial values:
	float totalAmbitionScoreForGoals = 0;
	for (int action = 0; action < goalShortlistSize; action++) {
		
		totalAmbitionScoreForGoals += allScores_ptr->allScores[action].ambitions.score;
	}
	for (int notion = 0; notion < AD::TOTAL_NOTIONS; notion++) {
		wp[notion] = 0;
	}
	
	//scoreAfavorAção/scoreAfavorGoalActions * quantoAnocaoAtrapalhaEssaAção

	//Now for each action on the shortlist we will, for each notion:
	//- See how much it adds worry to this action;
	//- Add to it's weight the more it adds worry and the higher ambition score for this action
	for (int action = 0; action < goalShortlistSize; action++) {
	
		int cat = allScores_ptr->allScores[action].ambitions.actCategory;
		int mode = allScores_ptr->allScores[action].ambitions.actMode;

		//The more ambition for a given action, the more it'll influence the weights:
		float relativeAmbition =
				allScores_ptr->allScores[action].ambitions.score / totalAmbitionScoreForGoals;

		//First we check the effect of the notions about ourselves:
		int notionsSelf = (int)AD::notionsSelf::TOTAL;
		for (int notion = 0; notion < notionsSelf; notion++) {

			float howMuchThisNotionIsAworryForThisAction =
				np->self[notion] * AD::notionWeightsAgainst.at(cat).at(mode).at(notion);
			
			wp[notion] =+ relativeAmbition * howMuchThisNotionIsAworryForThisAction;
		}

		//Now we have to choose between using average or spefici neighbor notions:
		float* notionsNeighbor_ptr;
		if (mode == (int)AS::actModes::SELF) {
			notionsNeighbor_ptr = np->averages;
		}
		else {
			int neighbor = allScores_ptr->allScores[action].ambitions.neighbor;
			notionsNeighbor_ptr = np->neighbors[neighbor];
		}	

		//Then we can calculate the effect from notions about the neighbor or their average:
		for (int notion = 0; notion < (int)AD::notionsNeighbor::TOTAL; notion++) {

			int notionWeightID = notion + notionsSelf;

			float howMuchThisNotionIsAworryForThisAction = notionsNeighbor_ptr[notion]
							* AD::notionWeightsAgainst.at(cat).at(mode).at(notionWeightID);
			
			wp[notion] =+ relativeAmbition * howMuchThisNotionIsAworryForThisAction;
		}
	}
}

//Mitigation uses the weights on wp to award extra score to actions which might
//mitigate the worries which are blocking us from executing the highest ambition actions
void mitigate(AD::notionWeights_t mitigationWeights_arr, AD::notions_t* np,
	                                AD::allScoresAnyScope_t* allScores_ptr) {

	//TODO-CRITICAL: This will be part of agent's personalities in the future: FIX then
	int choiceShortlistSize = ACT_CHOICE_SHORTLIST_SIZE;
	float extraScoreMultiplier = ACT_EXTRA_SCORE_MULTIPLIER;

	for (int action = 0; action < choiceShortlistSize; action++) {
		
		int cat = allScores_ptr->allScores[action].ambitions.actCategory;
		int mode = allScores_ptr->allScores[action].ambitions.actMode;
		
		float extraScore = 0;

		//First we calculate the extra score from notions related to self:
		int notionsSelf = (int)AD::notionsSelf::TOTAL;
		for (int notion = 0; notion < notionsSelf; notion++) {

			float weightInFavorThisAction =
				AD::notionWeightsInFavor.at(cat).at(mode).at(notion);

			extraScore += 
				np->self[notion] * weightInFavorThisAction * mitigationWeights_arr[notion];
		}

		//For notions related to neighbors, we choose wheter to use averages or not:
		float* notionsNeighbor_ptr;
		if (mode == (int)AS::actModes::SELF) {
			notionsNeighbor_ptr = np->averages;
		}
		else {
			int neighbor = allScores_ptr->allScores[action].ambitions.neighbor;
			notionsNeighbor_ptr = np->neighbors[neighbor];
		}	

		//And then calculate the extra score from those:	
		for (int notion = 0; notion < (int)AD::notionsNeighbor::TOTAL; notion++) {

			int notionWeightID = notion + notionsSelf;
			float weightInFavorThisAction =
				AD::notionWeightsInFavor.at(cat).at(mode).at(notionWeightID);
			
			extraScore += 
				notionsNeighbor_ptr[notion] * weightInFavorThisAction 
										    * mitigationWeights_arr[notionWeightID];
		}	

		//Finally we add the extra score:
		allScores_ptr->allScores[action].overallUtility.score +=
										extraScore * extraScoreMultiplier;
	}
}

//If doNothing, returns innactive action. Else, score is recorded on action's intensity
//Also returns innactive action in case of error.
//TODO: test A LOT (and make sure sorting is working as expected)
AS::actionData_t chooseBestOptionOrThinkHarder(AD::allScoresAnyScope_t* allScores_ptr, 
	                                    AD::notions_t* np, int agent, AS::scope scope,
	                                  AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

	AS::actionData_t chosenAction;
	chosenAction.ids.active = 0;

	//We'll try to make a choice. If the most desired action passes a treshold, we do that
	//On the first round, we try our top desire. In later rounds, the highest overallUtility.
	//Otherwise, if we still can, we try to mitigate worries associated to our top desires,
	//and then we sort from the highest overallUtility and try again, 
	//until either an action is chosen or we give up : )

	//Before starting, we sort the scores from the most desirable to the least, by ambition:
	std::sort(&allScores_ptr->allScores[0], 
		      &allScores_ptr->allScores[allScores_ptr->actualTotalScores - 1], 
		      AD::descendingAmbitionsCompare);

	//TODO-CRITICAL: These will be part of agent's personalities in the future: FIX then
	float justDoIt = ACT_JUST_DO_IT_THRESOLD;
	int maxMitigationAttempts = ACT_MITIGATION_ROUNDS;
	int choiceShortlistSize = ACT_CHOICE_SHORTLIST_SIZE;

	int mitigationAttempts = 0;
	AD::notionWeights_t inconvennienceWeightsForExtraScoring; //for mitigation rounds

	//Now for the loop:
	while( (allScores_ptr->allScores[0].overallUtility.score < justDoIt)
				       && (mitigationAttempts < maxMitigationAttempts) ) { 
	
		if(mitigationAttempts == 0){
			prepareForMitigationRound(&inconvennienceWeightsForExtraScoring[0], np, allScores_ptr);
		}

		mitigate(&inconvennienceWeightsForExtraScoring[0], np, allScores_ptr);

		//sort from highest overallUtility to smallest. The maximum will be at allScores[0]
		//Note: since mitigation only affects the choices shortlist, and only positivelly,
		//after the first round we only need to sort the top actions. So:
		int scoresToSort = allScores_ptr->actualTotalScores;
		if(mitigationAttempts > 0){
			scoresToSort = choiceShortlistSize;
		}
		std::sort(&allScores_ptr->allScores[0], &allScores_ptr->allScores[scoresToSort - 1], 
		                                                AD::descendingOverallUtilityCompare);

		mitigationAttempts++;
	}

	//We've either found something nice to do or gave up trying. Is anything good enough?

	//TODO-CRITICAL: This will be part of agent's personalities in the future: FIX then
	float whyBother = ACT_WHY_BOTHER_THRESOLD;

	if(allScores_ptr->allScores[0].overallUtility.score >= whyBother) { 
		
		//the best score is good enough, so let's build and return that action!
		AS::actionData_t choice;
		auto decision_ptr = &(allScores_ptr->allScores[0].overallUtility);
		                    
		choice.ids.active = 1;

		if ( (decision_ptr->neighbor == NEIGHBOR_ID_FOR_SELF) 
			  && (decision_ptr->actMode != (int)AS::actModes::SELF) ) {
				
			errorsCounter_ptr->incrementError(AS::errors::DS_NEIGHBOR_MARKED_SELF_WRONG_MODE_ON_TRY_BEST);
			choice.ids.active = 0; //invalidate
		}

		choice.ids.category = decision_ptr->actCategory;
		choice.ids.mode = decision_ptr->actMode;
		choice.ids.scope = (int)scope;
		choice.ids.origin = agent;
		choice.ids.target = decision_ptr->neighbor;
		choice.details.intensity = decision_ptr->score;

		return choice;
	}
	else {
		
		//We didn't find anything interesting enough, so let's play safe:
		return doLeastHarmful(allScores_ptr, agent, scope, errorsCounter_ptr);
   }
}

AS::actionData_t chooseAction(AD::notions_t* np, AD::allScoresAnyScope_t* sp,
							     int agent, AS::dataControllerPointers_t* dp, 
							             AS::scope scope, int totalNeighbors,
 	                         AS::WarningsAndErrorsCounter* errorsCounter_ptr) {
	
	//TODO-CRITICAL: use agent's values after that's implemented
	float whyBother = ACT_WHY_BOTHER_THRESOLD;
	float justDoIt = ACT_JUST_DO_IT_THRESOLD;

	//choose action:
	float maxAmbition = calculateScores(np, sp, scope, totalNeighbors);
	
	AS::actionData_t chosenAction;
	if(maxAmbition < whyBother){ 

		chosenAction = doLeastHarmful(sp, agent, scope, errorsCounter_ptr);
	}
	else {

		chosenAction = chooseBestOptionOrThinkHarder(sp, np, agent, scope, errorsCounter_ptr);
	}
	
	//If we choose to doNothing, chosenAction.ids.active will be 0, else, 1, so:
	if(chosenAction.ids.active) {

		setActionDetails(chosenAction.details.intensity, whyBother, justDoIt, 
			                            &chosenAction, dp, errorsCounter_ptr);
	}
	
	return chosenAction;
}

//LA:
int getTotalScoresLA(LA::stateData_t* state_ptr, int neighbors) {

	return AV::howManyActionsOfKind(AS::actModes::SELF, AS::scope::LOCAL)
		+ (neighbors * (
				AV::howManyActionsOfKind(AS::actModes::IMMEDIATE, AS::scope::LOCAL)
			  + AV::howManyActionsOfKind(AS::actModes::REQUEST, AS::scope::LOCAL) ) );
}

void calculateNotionsLA(int agent, AS::dataControllerPointers_t* dp, AD::notions_t* np,
	                           LA::readsOnNeighbor_t* refReads_ptr, int totalNeighbors,
							           AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

	int totalNotionsSelf = (int)AS::Decisions::notionsSelf::TOTAL;

	for (int notion = 0; notion < totalNotionsSelf; notion++) {

		np->self[notion] = 
			AS::Decisions::calculateNotionSelfLA((AS::Decisions::notionsSelf)notion, 
									                        agent, dp, refReads_ptr);
	}

	int totalNotionsNeighbor = (int)AS::Decisions::notionsNeighbor::TOTAL;
	
	for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {
		for(int notion = 0; notion < totalNotionsNeighbor; notion++){
			
			np->neighbors[neighbor][notion] =
				AS::Decisions::calculateNotionNeighborLA((AS::Decisions::notionsNeighbor)notion, 
					                                      neighbor, agent, dp, refReads_ptr);
		}
	}
	
	//We also have the np->averages to populate.
	for(int notion = 0; notion < totalNotionsNeighbor; notion++){	
		
		np->averages[notion] = 0;

		//We'll actually compute the RMS of each neighbors contribution:
		for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {
			np->averages[notion] += 
				(np->neighbors[neighbor][notion] * np->neighbors[neighbor][notion]);
		}
		if (!std::isfinite(np->averages[notion])) {
			
			errorsCounter_ptr->incrementWarning(AS::warnings::DS_LA_NOTIONS_RMS_BLEW_UP);
			
			//Anyway, if the squaring blew things up, we go back to a simple average:
			np->averages[notion] = 0;

			for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {
				np->averages[notion] += np->neighbors[neighbor][notion];
			}
			np->averages[notion] /= totalNeighbors;
		}
		else {
			//Otherwise, we keep going with the original RMS plan:
			np->averages[notion] /= totalNeighbors;
			np->averages[notion] = sqrt(np->averages[notion]); //notions are bounded to [0,1]
		}
	}
}

//GA:
int getTotalScoresGA(GA::stateData_t* state_ptr, int neighbors) {

	return AV::howManyActionsOfKind(AS::actModes::SELF, AS::scope::GLOBAL)
		+ (neighbors * (
				AV::howManyActionsOfKind(AS::actModes::IMMEDIATE, AS::scope::GLOBAL)
			  + AV::howManyActionsOfKind(AS::actModes::REQUEST, AS::scope::GLOBAL) ) );
}

void calculateNotionsGA(int agent, AS::dataControllerPointers_t* dp, AD::notions_t* np,
                               GA::readsOnNeighbor_t* refReads_ptr, int totalNeighbors,
							           AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

	int totalNotionsSelf = (int)AS::Decisions::notionsSelf::TOTAL;

	for (int notion = 0; notion < totalNotionsSelf; notion++) {

		np->self[notion] = 
			AS::Decisions::calculateNotionSelfGA((AS::Decisions::notionsSelf)notion, 
									                        agent, dp, refReads_ptr);
	}

	int totalNotionsNeighbor = (int)AS::Decisions::notionsNeighbor::TOTAL;
	
	for (int notion = 0; notion < totalNotionsNeighbor; notion++) {
		for(int neighbor = 0; neighbor < totalNeighbors; neighbor++){
			
			np->neighbors[neighbor][notion] =
				AS::Decisions::calculateNotionNeighborGA((AS::Decisions::notionsNeighbor)notion, 
					                                      neighbor, agent, dp, refReads_ptr);
		}
	}
	
	//We also have the np->averages to populate.
	for(int notion = 0; notion < totalNotionsNeighbor; notion++){	
		
		np->averages[notion] = 0;

		//We'll actually compute the RMS of each neighbors contribution:
		for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {
			np->averages[notion] += 
				(np->neighbors[neighbor][notion] * np->neighbors[neighbor][notion]);
		}
		if (!std::isfinite(np->averages[notion])) {
			
			errorsCounter_ptr->incrementWarning(AS::warnings::DS_GA_NOTIONS_RMS_BLEW_UP);
			
			//Anyway, if the squaring blew things up, we go back to a simple average:
			np->averages[notion] = 0;

			for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {
				np->averages[notion] += np->neighbors[neighbor][notion];
			}
			np->averages[notion] /= totalNeighbors;
		}
		else {
			//Otherwise, we keep going with the original RMS plan:
			np->averages[notion] /= totalNeighbors;
			np->averages[notion] = sqrt(np->averages[notion]); //notions are bounded to [0,1]
		}
	}
}