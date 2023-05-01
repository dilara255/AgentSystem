//Cat => (action variation) category. Not a feline.

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

int getTotalPossibleActualScores(int neighbors) {

	int possibleActionsSelf = (int)AS::actCategories::TOTAL;
	int possibleActionsNeighbors = neighbors 
						* ((int)AS::actModes::TOTAL - 1) * (int)AS::actCategories::TOTAL;

	return possibleActionsSelf + possibleActionsNeighbors;
}

void calculateNotionsLA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
             AD::notions_t* notions_ptr, LA::readsOnNeighbor_t* referenceReads_ptr, 
               int totalNeighbors, AS::WarningsAndErrorsCounter* errorsCounter_ptr);



AS::actionData_t makeDecisionLA(int agent, AS::dataControllerPointers_t* dp, 
					 LA::stateData_t* state_ptr, LA::readsOnNeighbor_t* referenceReads_ptr, 
	                 AS::WarningsAndErrorsCounter* errorsCounter_ptr,
					 const float secondsSinceLastDecisionStep, int currentActions) {
	
	AS::actionData_t nullAction;
	nullAction.ids.slotIsUsed = 0; //not an actual action
	nullAction.ids.active = 0; //just to be sure	

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
	scores.actualTotalScores = getTotalPossibleActualScores(neighbors);
	
	AS::actionData_t choice =
		chooseAction(&notions, &scores, agent, dp, AS::scope::LOCAL, neighbors,
			                                                 errorsCounter_ptr);

	//TODO: add more sanity checks
	bool isTargetValid = (choice.ids.target >= 0)
				&& (choice.ids.target < (uint32_t)dp->LAstate_ptr->getDataCptr()->size());
	if ( !isTargetValid && (choice.ids.slotIsUsed == 1) ) {
		
		printf("targ: %d (%d neighs), agent: %d, mode: %d (cat: %d)\n",
			choice.ids.target, neighbors, choice.ids.origin, choice.ids.mode, choice.ids.category);

		errorsCounter_ptr->incrementError(AS::errors::DS_CHOSE_INVALID_LA_TARGET);
		choice.ids.slotIsUsed = 0; //invalidate choice so we don't blow stuff up
	}

	return choice;
}

void calculateNotionsGA(int agent, AS::dataControllerPointers_t* agentDataPtrs_ptr,
             AD::notions_t* notions_ptr, GA::readsOnNeighbor_t* referenceReads_ptr, 
               int totalNeighbors, AS::WarningsAndErrorsCounter* errorsCounter_ptr);

AS::actionData_t makeDecisionGA(int agent, AS::dataControllerPointers_t* dp,
				 GA::stateData_t* state_ptr, GA::readsOnNeighbor_t* referenceReads_ptr,
	             AS::WarningsAndErrorsCounter* errorsCounter_ptr,
				 const float secondsSinceLastDecisionStep, int currentActions) {

	AS::actionData_t nullAction;
	nullAction.ids.slotIsUsed = 0;

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
	scores.actualTotalScores = getTotalPossibleActualScores(neighbors);

	AS::actionData_t choice = 
		chooseAction(&notions, &scores, agent, dp, AS::scope::GLOBAL, neighbors, 
			                                                  errorsCounter_ptr);
	
	//TODO: add more sanity checks
	bool isTargetValid = (choice.ids.target >= 0)
				&& (choice.ids.target < (uint32_t)dp->GAstate_ptr->getDataCptr()->size());
	if ( !isTargetValid && (choice.ids.slotIsUsed == 1) ) {
		
		errorsCounter_ptr->incrementError(AS::errors::DS_CHOSE_INVALID_GA_TARGET);
		choice.ids.slotIsUsed = 0; //invalidate choice so we don't blow stuff up
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
							                 AS::scope scope, int totalNeighbors,
								 AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

	assert(allScores_ptr->actualTotalScores != UNINITIALIZED_ACTUAL_TOTAL_SCORES);
	assert(allScores_ptr->actualTotalScores > 0);

	//We will first calculate the scores of actions in SELF mode, then the others.
	//Actions in SELF mode will leave neighbor = -1 (the others will also set the neighbor).
	//Invalid variations will have score = -1
	
	//TODO-CRITICAL: WARNING: all of the following pressupposes SELF == 0!
	assert((int)AS::actModes::SELF == 0);

	float maxAmbition = -1;
	//For self, both neighbor (-1) and mode (SELF, 0) are fixed, so:
	int totalActionsSelf = AS::ActionVariations::TOTAL_CATEGORIES;

	for (int cat = 0; cat < totalActionsSelf; cat++) {		

		bool valid = AD::isValid(cat, (int)AS::actModes::SELF, (int)scope);

		auto sp = &(allScores_ptr->allScores[cat]);

		sp->ambitions.neighbor = NEIGHBOR_ID_FOR_SELF;
		sp->ambitions.actCategory = cat;
		sp->ambitions.actMode = (int)AS::actModes::SELF;
		if(valid) {
			setScore(&(sp->ambitions), np, &AD::notionWeightsInFavor);
		}
		else {
			sp->ambitions.score = BAD_AMBITION; //variation is invalid
		}
				
		maxAmbition = std::max(maxAmbition, sp->ambitions.score);
				
		sp->worries.neighbor = NEIGHBOR_ID_FOR_SELF;
		sp->worries.actCategory = cat;
		sp->worries.actMode = (int)AS::actModes::SELF;
		if(valid) {
			setScore(&(sp->worries), np, &AD::notionWeightsAgainst);
		}
		else {
			sp->worries.score = BAD_WORRY;
		}

		sp->overallUtility.neighbor = NEIGHBOR_ID_FOR_SELF;
		sp->overallUtility.actCategory = cat;
		sp->overallUtility.actMode = (int)AS::actModes::SELF;
				
		sp->overallUtility.score = sp->ambitions.score - sp->worries.score;
	}
	
	//Now we'll deal with the scores regarding neighbors:
	int modesRegardingNeighbors = AS::ActionVariations::TOTAL_MODES - 1; //SELF excluded
	int widthPerCategory = modesRegardingNeighbors; 
	int widthPerNeighbor = AS::ActionVariations::TOTAL_CATEGORIES * widthPerCategory;	

	for(int neighbor = 0; neighbor < totalNeighbors; neighbor++){
		for (int cat = 0; cat < AS::ActionVariations::TOTAL_CATEGORIES; cat++) {
			for (int mode = 0; mode < modesRegardingNeighbors; mode++) {
				//mode should start at 1 to exclude SELF, so:
				int actualMode = mode + 1;

				bool valid = AD::isValid(cat, actualMode, (int)scope);

				//also, we should start from where the actions about self left off, so:
				int index = totalActionsSelf
							+ (neighbor * widthPerNeighbor) 
					        + (cat * widthPerCategory) + mode;
				auto sp = &(allScores_ptr->allScores[index]);

				sp->ambitions.actCategory = cat;
				sp->ambitions.actMode = actualMode;
				sp->ambitions.neighbor = neighbor;
				if(valid) {
					setScore(&(sp->ambitions), np, &AD::notionWeightsInFavor);
				}
				else {
					sp->ambitions.score = BAD_AMBITION;
				}
				
				maxAmbition = std::max(maxAmbition, sp->ambitions.score);
				
				sp->worries.actCategory = cat;
				sp->worries.actMode = actualMode; 
				sp->worries.neighbor = neighbor;
				if(valid){
					setScore(&(sp->worries), np, &AD::notionWeightsAgainst);
				}
				else {
					sp->worries.score = BAD_WORRY;
				}

				sp->overallUtility.actCategory = cat;
				sp->overallUtility.actMode =  actualMode;
				sp->overallUtility.neighbor = neighbor;
				
				sp->overallUtility.score =
								sp->ambitions.score - sp->worries.score;
			}
		}
	}

	//Nota that in general allScores_ptr's actualTotalScores < sizeOfArrays;
	//So let's initialize whatever is left. But first, sanity check:
	int lastIndexWhichShouldBeInitialized =  allScores_ptr->actualTotalScores - 1;
	int firstUninitializedIndex = allScores_ptr->actualTotalScores;

	int lastGoodCat = 
		allScores_ptr->allScores[lastIndexWhichShouldBeInitialized].overallUtility.actCategory;
	int lastGoodMode = 
		allScores_ptr->allScores[lastIndexWhichShouldBeInitialized].overallUtility.actMode;
	int lastGoodScope = (int)scope;
	bool isValid = AD::isValid(lastGoodCat, lastGoodMode, lastGoodScope);

	if (!isValid) {
		errorsCounter_ptr->incrementError(AS::errors::DS_LAST_ACTION_SCORED_IS_INVALID);
	}

	int firstBadCat = 
		allScores_ptr->allScores[firstUninitializedIndex].overallUtility.actCategory;
	int firstBadMode = 
		allScores_ptr->allScores[firstUninitializedIndex].overallUtility.actMode;

	bool badCatModeAreAsExpected = (firstBadCat == firstBadMode) 
								&& (firstBadCat == SCORE_CAT_AND_MODE_UNINITIALIZED_DEFAULT);

	if (!badCatModeAreAsExpected) {
		errorsCounter_ptr->incrementError(AS::errors::DS_FIRST_UNSCORED_ACTION_NOT_AS_EXPECTED);
	}

	//now we can safely initialize what's left:
	for (int i = firstUninitializedIndex; i < allScores_ptr->sizeOfArrays; i++ ) {
		
		//We'll actually leave their bad categories and modes untouched,
		//to make sure there's no mix up. Instead, we will bomb their scores:
		allScores_ptr->allScores[i].ambitions.score = BAD_AMBITION;
		allScores_ptr->allScores[i].worries.score = BAD_WORRY;
		allScores_ptr->allScores[i].overallUtility.score = BAD_AMBITION - BAD_WORRY;
	}
	
	//Finally, we return the hightest ambition value:
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
		choice.ids.slotIsUsed = 1;

		if ( (decision_ptr->neighbor == NEIGHBOR_ID_FOR_SELF)
			  && (decision_ptr->actMode != (int)AS::actModes::SELF) ) {

			errorsCounter_ptr->incrementError(AS::errors::DS_NEIGHBOR_MARKED_SELF_WRONG_MODE_ON_LEAST_HARM);
			choice.ids.slotIsUsed = 0; //invalidate
		}

		choice.ids.category = decision_ptr->actCategory;
		choice.ids.mode = decision_ptr->actMode;
		choice.ids.scope = (int)scope;
		choice.ids.origin = agent;
		choice.ids.target = decision_ptr->neighbor;
		choice.details.intensity = decision_ptr->score;
	}
	else {
		choice.ids.slotIsUsed = 0; //Marks as "doNothing"
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
	
	//Now for each action on the shortlist we will, for each notion:
	//- See how much it adds worry to this action;
	//- Add to it's weight, based on that and on the action's ambition score.
	for (int action = 0; action < goalShortlistSize; action++) {
	
		int cat = allScores_ptr->allScores[action].ambitions.actCategory;
		int mode = allScores_ptr->allScores[action].ambitions.actMode;

		//The more ambition for a given action, the more it'll influence the weights:
		float relativeAmbition =
				allScores_ptr->allScores[action].ambitions.score / totalAmbitionScoreForGoals;

		//Let's check the effect of the notions about ourselves:
		int notionsSelf = (int)AD::notionsSelf::TOTAL;
		for (int notion = 0; notion < notionsSelf; notion++) {

			float howMuchThisNotionIsAworryForThisAction =
				np->self[notion] * AD::notionWeightsAgainst.at(cat).at(mode).at(notion);
			
			wp[notion] =+ relativeAmbition * howMuchThisNotionIsAworryForThisAction;
		}

		//Now we have to choose between using average or speficic neighbor notions:
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

void updateWeightsForMitigation(AD::notionWeights_t wp, AD::notions_t* np,
			                       AD::allScoresAnyScope_t* allScores_ptr,
		       				     AD::extraScore_t* extraScoreReceived_ptr,
	                                              int choiceShortlistSize) {
	
	//First we sort the extra scores (we only scored the top choiceShortlistSize ones):
	std::sort(&extraScoreReceived_ptr[0], &extraScoreReceived_ptr[choiceShortlistSize],
													   AD::descendingExtraScoreCompare);

	//TODO-CRITICAL: These will be part of agent's personalities in the future: FIX then
	int goalShortlistSize = ACT_GOAL_SHORTLIST_SIZE;

	float totalExtraScore = 0;
	for (int action = 0; action < goalShortlistSize; action++) {
		totalExtraScore += extraScoreReceived_ptr[action].score;
	}

	float tooSmall = 0.1;
	if (totalExtraScore < tooSmall) {
		return; //nothing to do here : )
	}

	//in case total score < 1, this will be used to rebalance the contribution
	//while keeping the effect of a possibly isolated high extra score:
	float lowTotalContributionMultiplier = std::min(1.0f, totalExtraScore);
	
	assert(ACT_EXTRA_SCORE_CONTRIBUTION_MITIGATION_WEIGHTS <= 1);
	float proportionOfExtrasOnNewWeight =
		ACT_EXTRA_SCORE_CONTRIBUTION_MITIGATION_WEIGHTS * lowTotalContributionMultiplier;

	//The top goalShortlistSize extra scoring actions will contribute to the new weights
	//TODO: EXTRACT: this repeats prepareForMitigationRound a lot
	for (int action = 0; action < goalShortlistSize; action++) {
		
		//The more extra score this got, the more it contributes to our ambitions:
		float contribution = extraScoreReceived_ptr[action].score / totalExtraScore;

		//If this contribution is zero, all next ones will be too:
		if(contribution == 0) { break; } //so no need to keep going

		//But what was this extra score even given for?
		int actionID = extraScoreReceived_ptr[action].actionIdOnChoiceShortlist;

		int cat = allScores_ptr->allScores[actionID].ambitions.actCategory;
		int mode = allScores_ptr->allScores[actionID].ambitions.actMode;

		//Let's check the effect of the notions about ourselves:
		int notionsSelf = (int)AD::notionsSelf::TOTAL;
		for (int notion = 0; notion < notionsSelf; notion++) {

			float howMuchThisNotionIsAworryForThisAction =
				np->self[notion] * AD::notionWeightsAgainst.at(cat).at(mode).at(notion);
			
			wp[notion] *= (1 - proportionOfExtrasOnNewWeight);
			wp[notion] += howMuchThisNotionIsAworryForThisAction * contribution
											    * proportionOfExtrasOnNewWeight;
		}

		//Now we have to choose between using average or speficic neighbor notions:
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
			
			wp[notion] *= (1 - proportionOfExtrasOnNewWeight);
			wp[notion] += howMuchThisNotionIsAworryForThisAction * contribution
											    * proportionOfExtrasOnNewWeight;
		}
	}
}

//Mitigation uses the mitigation weights to award extra score to actions which might
//mitigate the worries which are blocking us from executing the highest ambition actions
void mitigate(AD::notionWeights_t mitigationWeights_arr, AD::notions_t* np,
								    AD::allScoresAnyScope_t* allScores_ptr,
								     AD::extraScore_t* extraScoresReceived, 
	                                              int mitigationRoundsDone) {

	//TODO-CRITICAL: This will be part of agent's personalities in the future: FIX then
	int choiceShortlistSize = ACT_CHOICE_SHORTLIST_SIZE;
	float extraScoreMultiplier = ACT_EXTRA_SCORE_MULTIPLIER;

	for (int action = 0; action < choiceShortlistSize; action++) {
		
		int cat = allScores_ptr->allScores[action].overallUtility.actCategory;
		int mode = allScores_ptr->allScores[action].overallUtility.actMode;
		
		float extraScore = 0;

		//First we calculate the extra score from notions related to self:
		int notionsSelf = (int)AD::notionsSelf::TOTAL;
		for (int notion = 0; notion < notionsSelf; notion++) {

			float overallNotionWeightForThisAction =
					AD::notionWeights.at(cat).at(mode).at(notion);

			extraScore += 
				np->self[notion] * overallNotionWeightForThisAction 
								 * mitigationWeights_arr[notion];
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
			float overallNotionWeightForThisAction =
				AD::notionWeights.at(cat).at(mode).at(notionWeightID);
			
			extraScore += 
				notionsNeighbor_ptr[notion] * overallNotionWeightForThisAction 
										    * mitigationWeights_arr[notionWeightID];
		}	

		//sucessive mitigation rounds have their effects dampened:
		//TODO-CRITICAL: This will be part of agent's personalities in the future: FIX then
		float dampeningBase = ACT_SUCESSIVE_MITIGATION_DAMPENNING_MULTIPLIER;

		float dampeningFactor = powf(dampeningBase, mitigationRoundsDone);
		
		extraScore *= dampeningFactor;

		//Finally we add the extra score:
		allScores_ptr->allScores[action].overallUtility.score +=
										extraScore * extraScoreMultiplier;
		//and keep track of it, but only if the action seems helpful:
		extraScoresReceived[action].score = std::max(0.0f, extraScore); 
		extraScoresReceived[action].actionIdOnChoiceShortlist = action;
	}
}

//If doNothing, returns innactive action. Else, score is recorded on action's intensity
//Also returns innactive action in case of error.
//TODO: test A LOT (and make sure sorting is working as expected)
AS::actionData_t chooseBestOptionOrThinkHarder(AD::allScoresAnyScope_t* allScores_ptr, 
	                                    AD::notions_t* np, int agent, AS::scope scope,
	                                  AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

	AS::actionData_t chosenAction;
	chosenAction.ids.slotIsUsed = 0;
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
	//We'll also keep the extra score given each round. The more an action helps alleviate
	//the inconvenniences, the more it will contribute to the weights next step.
	AD::extraScore_t extraScoresReceived[MAX_ACT_CHOICE_SHORTLIST_SIZE]; 

	//Now for the loop:
	//Note that on first pass scores are sorted by ambition;
	//On subsequent passes, they're sorted by overallUtility;
	//This affects the loop condition as well as wich actions get extra score via mitigation.
	while( (allScores_ptr->allScores[0].overallUtility.score < justDoIt)
				       && (mitigationAttempts < maxMitigationAttempts) ) { 

		//we need to define the mitigation weights before mitigating.
		//In the first round, we do it here:
		if(mitigationAttempts == 0){
			
			prepareForMitigationRound(&inconvennienceWeightsForExtraScoring[0], 
															 np, allScores_ptr);
		}

		//The choice shortlist for first pass is arranged by ambition; 
		//For later passes, by overall utility (sort done later in the loop).
		mitigate(&inconvennienceWeightsForExtraScoring[0], np, allScores_ptr, 
					             &extraScoresReceived[0], mitigationAttempts);

		mitigationAttempts++;

		//In later rounds, if we expect to do more mitigation, we update the weights here:
		if( mitigationAttempts < maxMitigationAttempts) {
			
			updateWeightsForMitigation(&inconvennienceWeightsForExtraScoring[0], np, 
						 allScores_ptr, &extraScoresReceived[0], choiceShortlistSize);
		}

		//sort from highest overallUtility to smallest. The maximum will be at allScores[0]
		//Note: since mitigation only affects the choices shortlist,
		//from the 2nd round onwards we only need to sort 2 times as many as the top actions;
		int scoresToSort = allScores_ptr->actualTotalScores; //for the first round
		if(mitigationAttempts > 1){
			scoresToSort = std::min(allScores_ptr->actualTotalScores, 2 * choiceShortlistSize);
		}
		int lastToSort = scoresToSort - 1;
		assert(lastToSort >= 0);

		std::sort(&allScores_ptr->allScores[0], &allScores_ptr->allScores[lastToSort], 
		                                                AD::descendingOverallUtilityCompare);

		//While: Did we find something good enough? If not, should we keep trying?
	}

	//We've either found something nice to do or gave up trying. Is anything good enough?

	int choiceIndex = 0; //index zero is the highest overall score

	//But if the best score is really high, let's try to prioritize by ambition!
	if (allScores_ptr->allScores[0].overallUtility.score >= justDoIt) {
		float maxElegibleAmbition = 0;

		//between the top picks...
		for (int i = 0; i < choiceShortlistSize; i++) {
			//and so long as the overall score is still above justDoIt:
			if (allScores_ptr->allScores[i].overallUtility.score >= justDoIt) {
				if (allScores_ptr->allScores[0].ambitions.score > maxElegibleAmbition) {

					maxElegibleAmbition = allScores_ptr->allScores[0].ambitions.score;
					choiceIndex = i;
				}
			}
		}		
	}

	//TODO-CRITICAL: This will be part of agent's personalities in the future: FIX then
	float whyBother = ACT_WHY_BOTHER_THRESOLD;

	//Either way, our preferred choice is at choiceIndex now, so if it's interesting:
	if(allScores_ptr->allScores[choiceIndex].overallUtility.score >= whyBother) { 
		
		//the best score is good enough, so let's build and return that action!
		AS::actionData_t choice;
		auto decision_ptr = &(allScores_ptr->allScores[0].overallUtility);
		                  
		choice.ids.slotIsUsed = 1; //we do expect to choose something
		choice.ids.active = 1;

		if ( (decision_ptr->neighbor == NEIGHBOR_ID_FOR_SELF) 
			  && (decision_ptr->actMode != (int)AS::actModes::SELF) ) {
				
			errorsCounter_ptr->incrementError(AS::errors::DS_NEIGHBOR_MARKED_SELF_WRONG_MODE_ON_TRY_BEST);
			choice.ids.slotIsUsed = 0; //invalidate
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
		
		//We didn't find anything interesting enough, so let's play it safe:
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
	float maxAmbition = calculateScores(np, sp, scope, totalNeighbors, errorsCounter_ptr);
	
	AS::actionData_t chosenAction;
	if(maxAmbition < whyBother){ 

		chosenAction = doLeastHarmful(sp, agent, scope, errorsCounter_ptr);
	}
	else {

		chosenAction = chooseBestOptionOrThinkHarder(sp, np, agent, scope, errorsCounter_ptr);
	}
	
	//If we choose to doNothing, chosenAction.ids.slotIsUsed will be 0, else, 1, so:
	if(chosenAction.ids.slotIsUsed) {

		bool isValid = AD::isValid(chosenAction.ids.category, 
								   chosenAction.ids.mode, chosenAction.ids.scope);

		if(isValid) {
			setActionDetails(chosenAction.details.intensity, whyBother, justDoIt, 
											&chosenAction, dp, errorsCounter_ptr);
		}
		else {
			errorsCounter_ptr->incrementError(AS::errors::DS_CHOSE_INVALID_VARIATION);
			chosenAction.ids.slotIsUsed = false; //invalidate bad choice
		}
	}

	return chosenAction;
}

//LA:
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