#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "timeHelpers.hpp"
#include "AS_warningAndErrorDefinitions.hpp"

#include "logAPI.hpp"
#include "AS_API.hpp"
#include "CL_externalAPI.hpp"

#include "textViz.hpp"

#include <Windows.h>

const char* initialNetworkFilename = "textModeVizBase.txt";
const char* networkFilenameSaveName = "textModeViz_run0.txt";

const std::chrono::seconds testTime = std::chrono::seconds(600);
const std::chrono::milliseconds loopSleepTime = std::chrono::milliseconds(60);
const float testResources = 0.60f * DEFAULT_LA_RESOURCES;
const float testPace = 40.0f;

#define PRINT_VIZ true
#define SHOULD_PAUSE_ON_NEW true

namespace TV{
	
	bool initializeAS() {

		LOG_DEBUG("Initializing the AS...\n",1);

		if (!AS::initializeASandCL()) {
			LOG_CRITICAL("Couldn't initialize the AS");
			GETCHAR_FORCE_PAUSE;
			return false;
		}

		LOG_INFO("AS_INITIALIZED");
		return true;
	}

	bool loadBaseNetworkAndTestAccessors() {
		LOG_DEBUG("Loading...\n",1);
		
		if (!AS::loadNetworkFromFile(initialNetworkFilename)) {
			LOG_CRITICAL("Couldn't load the test network");
			GETCHAR_FORCE_PAUSE;
			return false;
		}

		auto cldh_ptr = CL::getClientDataHandlerPtr();
		if (cldh_ptr == NULL) {
			LOG_ERROR("Couldn't get Client Data Handler pointer");
			return false;
		}

		if (CL::ASmirrorData_cptr == NULL) {
			LOG_ERROR("AS mirror's pointer is NULL");
			return false;
		}

		return true;
	}

	bool setInitialResources(float resources) {

		auto cldh_ptr = CL::getClientDataHandlerPtr();

		int numberLAs = CL::ASmirrorData_cptr->networkParams.numberLAs;

		bool aux = true;

		for (int agent = 0; agent < numberLAs; agent++) {
			aux &= cldh_ptr->LAstate.parameters.resources.changeCurrentTo(agent, resources);
		}

		if (!aux) {
			LOG_ERROR("Failed to sendo data to the client data handler");
		}

		return aux;
	}

	bool startNetworkAtPace(float pace) {
		LOG_DEBUG("Running network...\n",1);
		
		AS::changePace(pace);

		if (!AS::run(true)) {
			LOG_CRITICAL("Failed to run network");
			GETCHAR_FORCE_PAUSE;
			return false;
		}	

		return true;
	}

	void wait(std::chrono::seconds* timePassed_ptr, std::chrono::milliseconds sleepTime,
			                                std::chrono::steady_clock::time_point start) {

		AZ::hybridBusySleepForMicros(std::chrono::microseconds(sleepTime));
			auto now = std::chrono::steady_clock::now();
			*timePassed_ptr = 
				std::chrono::duration_cast<std::chrono::seconds>(now - start);
	}

	void initializeActionsVec(std::vector<TV::actionChanges_t>* actionsVec_ptr, 
															     int numberLAs){
			
		int maxActionsPerAgent = CL::ASmirrorData_cptr->networkParams.maxActions;
		int totalActions = maxActionsPerAgent * numberLAs;
		actionsVec_ptr->reserve(totalActions);
		TV::actionChanges_t emptyAction;
		for (int i = 0; i < totalActions; i++) {
			actionsVec_ptr->push_back(emptyAction);
		}
	}	

	//Returns true if there are significant changes on actions or decisions made
	bool updateActionsAndCheckForChanges(std::vector<TV::actionChanges_t>* actionsVec_ptr, 
		                    std::vector<TV::decisionHasChanges_t>* decisionHasChanges_ptr,
											                                int numberLAs) {
		int maxActionsPerAgent = CL::ASmirrorData_cptr->networkParams.maxActions;
		assert( actionsVec_ptr->size() == (maxActionsPerAgent * numberLAs) ); //from init

		auto decisionReflection_ptr = 
					&(CL::ASmirrorData_cptr->decisionReflectionMirror.LAdecisionReflection);
		auto LAactions_ptr = &(CL::ASmirrorData_cptr->actionMirror.dataLAs);

		AS::actionData_t newActionData;
		int local = (int)AS::scope::LOCAL; //we're only dealing with locals for now

		bool foundChanges = false;
		for(int agent = 0; agent < numberLAs; agent++){

			//First we check for changes in decisions:
			decisionHasChanges_ptr->at(agent).hasChanged = 
				decisionReflection_ptr->at(agent).decisionAttemptCounter
					> decisionHasChanges_ptr->at(agent).lastAmountOfDecisions;
			
			if (decisionHasChanges_ptr->at(agent).hasChanged) {
				foundChanges = true;
				decisionHasChanges_ptr->at(agent).lastAmountOfDecisions =
						decisionReflection_ptr->at(agent).decisionAttemptCounter;
			}			

			//And then the actions for (reasonanly important) changes:
			for (int action = 0; action < maxActionsPerAgent; action++) {

				int actionIndex = AS::getAgentsActionIndex(agent, action, maxActionsPerAgent);
				newActionData = LAactions_ptr->at(actionIndex);

				AS::actionData_t* oldAction_ptr = &(actionsVec_ptr->at(actionIndex).data);

				if (newActionData.ids.slotIsUsed && newActionData.ids.active) {
					
					//Change in IDs mean something important happened
					bool isNew = ((oldAction_ptr->ids != newActionData.ids)
						           && (newActionData.ids.phase != (int)AS::actPhases::SPAWN));
					foundChanges |= isNew;
					actionsVec_ptr->at(actionIndex).hasChanged = isNew;
				}
				
				//Either way we always update, so old actions drop out
				*oldAction_ptr = newActionData;
			}
		}

		return foundChanges;
	}

	float setStyleAndcalculateDisplayIntensity(AS::actionData_t actionData, bool* isRate) {
		
		float intensity = actionData.details.intensity;
		float rateMultiplier = TENTHS_OF_MS_IN_A_SECOND;

		switch (actionData.ids.mode)
		{
		case((int)AS::actModes::SELF):
			switch (actionData.ids.category)
			{
			case((int)AS::actCategories::STRENGHT):
				if(actionData.ids.phase == (int)AS::actPhases::EFFECT){
					*isRate = true;
					return intensity * rateMultiplier;
				}
				else {
					*isRate = false;
					return intensity;
				}
			case((int)AS::actCategories::RESOURCES):
				if(actionData.ids.phase == (int)AS::actPhases::EFFECT){
					*isRate = true;
					return intensity * rateMultiplier;
				}
				else {
					*isRate = false;
					return intensity;
				}
			default:
				*isRate = false;
				return intensity;
			}
		default:
			*isRate = false;
			return intensity;
		}
	}

	void printAction(AS::actionData_t actionData) {

		std::string_view cat = AS::catToString((AS::actCategories)actionData.ids.category);
		char mode = AS::modeToChar((AS::actModes)actionData.ids.mode);
		char phase = AS::phaseToChar((AS::actPhases)actionData.ids.phase);
		char target = 'x';
		if (actionData.ids.target != actionData.ids.origin) {
			target = '0' + actionData.ids.target; //expects target <= 9
		}
		
		double secondsElapsed = 
			(double)actionData.phaseTiming.elapsed/(double)TENTHS_OF_MS_IN_A_SECOND;
		double phaseTotal = 
			(double)actionData.phaseTiming.total/(double)TENTHS_OF_MS_IN_A_SECOND;

		bool intensityIsRate = false;
		float intensity = setStyleAndcalculateDisplayIntensity(actionData, &intensityIsRate);
		
		float aux = actionData.details.processingAux;

		if(!intensityIsRate){
			printf("\t-> %6.2f/%6.2f s | %s_%c_%c -> %c | intens: %7.2f, aux: %+7.2f\n",
								 secondsElapsed, phaseTotal, cat.data(), mode, phase, 
													   target, intensity, aux);
		}
		else {
			printf("\t-> %6.2f/%6.2f s | %s_%c_%c -> %c | rate: %7.4f/s, aux: %+7.2f\n",
								 secondsElapsed, phaseTotal, cat.data(), mode, phase, 
													   target, intensity, aux);
		}
	}

	const char* placeholderActionFormatLine = 
		       "\t-> --------------- | ------------ | ------------------------------";
	const char* separatorFormatLine = 
		       "**********************************************************************\n";
	const char* newArrow = ">>--NEW-->";

	void printLAactionData(int agent, std::vector<TV::actionChanges_t>* actionsVec_ptr) {
		
		int maxActions = CL::ASmirrorData_cptr->networkParams.maxActions;

		AS::actionData_t* actionData_ptr;

		for (int action = 0; action < maxActions; action++) {

			int actionIndex = AS::getAgentsActionIndex(agent, action, maxActions);
			actionData_ptr = &(actionsVec_ptr->at(actionIndex).data);
			bool* isNew_ptr = &(actionsVec_ptr->at(actionIndex).hasChanged);

			if (actionData_ptr->ids.slotIsUsed && actionData_ptr->ids.active ) {

				if(*isNew_ptr) {
					printf(newArrow);
					*isNew_ptr = false;
				}
				printAction(*actionData_ptr);
			}
			else {
				puts(placeholderActionFormatLine);
			}
		}
	}
		
	float costNextAgentsAction(int agent, std::vector<TV::actionChanges_t>* actionsVec_ptr) {

		int maxActionsPerAgent = CL::ASmirrorData_cptr->networkParams.maxActions;
		int startingIndex = maxActionsPerAgent * agent;
		int boundingIndex = startingIndex + maxActionsPerAgent;

		int currentValidActions = 0;
		for (int action = startingIndex; action < boundingIndex; action++) {
			if (actionsVec_ptr->at(action).data.ids.slotIsUsed
				&& actionsVec_ptr->at(action).data.ids.active) {
				currentValidActions++;
			}
		}

		return AS::nextActionsCost(currentValidActions);
	}

	void printLAheaderAndstateData(int agent,  
		                           std::vector<TV::actionChanges_t>* actionsVec_ptr) {
		
		std::string agentName = 
			CL::ASmirrorData_cptr->agentMirrorPtrs.LAcoldData_ptr->data.at(agent).name;

		auto agentState_ptr =
			&(CL::ASmirrorData_cptr->agentMirrorPtrs.LAstate_ptr->data.at(agent));

		AS::pos_t position = agentState_ptr->locationAndConnections.position;
		auto resources_ptr = &(agentState_ptr->parameters.resources);
		auto strenght_ptr = &(agentState_ptr->parameters.strenght);

		strenght_ptr->current; strenght_ptr->currentUpkeep; strenght_ptr->externalGuard;
		strenght_ptr->thresholdToCostUpkeep;

		float tax = -resources_ptr->taxRate;
		float trade = resources_ptr->tradeRate;
		float onAttacks = strenght_ptr->onAttacks;
		float attrition = -strenght_ptr->attritionLossRate;

		int attacksUnder = agentState_ptr->underAttack;
		float costNextAction = costNextAgentsAction(agent, actionsVec_ptr);

		if(attacksUnder != 0){ 
			printf("LA%d (GA %d) | X: %+4.2f, Y: %+4.2f | Name: %s | $ Next Action: %6.2f | Under %d attacks!\n",
			       agent, agentState_ptr->GAid, position.x, position.y, agentName.c_str(), 
			       costNextAction, attacksUnder);
		}
		else {
			printf("LA%d (GA %d) | X: %+4.2f, Y: %+4.2f | Name: %s | $ Next Action: %6.2f\n",
			          agent, agentState_ptr->GAid, position.x, position.y, agentName.c_str(), 
				                                                              costNextAction);
		}

		printf("\tSTATE | %+8.1f $ (%5.3f %+5.3f %+5.2f $/sec) | %5.0f S, %5.0f D (%+4.3f /s), %5.2f A (%+5.2f $/sec)\n",
								resources_ptr->current, trade, tax, resources_ptr->updateRate,
			         strenght_ptr->current, strenght_ptr->externalGuard, attrition, onAttacks,
			                                                     -strenght_ptr->currentUpkeep);
	}

	void printLAneighborData(int agent) {

		auto agentState_ptr = 
			&(CL::ASmirrorData_cptr->agentMirrorPtrs.LAstate_ptr->data.at(agent));
		int totalNeighbors = 
			agentState_ptr->locationAndConnections.connectedNeighbors.howManyAreOn();
		
		auto decisionData_ptr =
			&(CL::ASmirrorData_cptr->agentMirrorPtrs.LAdecision_ptr->data.at(agent));

		int resorucesReadField = (int)LA::readsOnNeighbor_t::fields::RESOURCES;
		int incomeReadField = (int)LA::readsOnNeighbor_t::fields::INCOME;
		int strenghtReadField = (int)LA::readsOnNeighbor_t::fields::STRENGHT;
		int guardReadField = (int)LA::readsOnNeighbor_t::fields::GUARD;

		for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {
			
			int neighborID = agentState_ptr->locationAndConnections.neighbourIDs[neighbor];

			int stance = (int)agentState_ptr->relations.diplomaticStanceToNeighbors[neighbor];
			char stanceChar = AS::diploStanceToChar((AS::diploStance)stance);

			float disposition = agentState_ptr->relations.dispositionToNeighbors[neighbor];
			float infiltration = decisionData_ptr->infiltration[neighbor];
			
			float resources = decisionData_ptr->reads[neighbor].of[resorucesReadField].read;
			float income = decisionData_ptr->reads[neighbor].of[incomeReadField].read;
			float strenght = decisionData_ptr->reads[neighbor].of[strenghtReadField].read;
			float guard = decisionData_ptr->reads[neighbor].of[guardReadField].read;
			
			printf("\tNEIGHBOR: %d | %c, disp: %+3.2f | %+3.2f ? | %+10.2f $ (%+6.2f $/sec) | %7.2f S , %7.2f D\n",
									neighborID, stanceChar, disposition, 
									infiltration, resources, income, strenght, guard);		
		}
	}

	void printInitialThoughtsOnDecision(const AS::Decisions::notionsRecord_t*, 
										const AS::Decisions::scoresRecord_t*,
		                                bool isNewDecision) {

		if(isNewDecision) { printf(newArrow); }
		printf("\tI feel like %s (%3.2f) and %s (%3.2f). I'd like to %s%c (%3.2f)\n",
			"AVG_WELL_DEFENDED", 0.68f, "IAM_LOW_INCOME", 0.53f, "RES_S_L -> ", 'x', 0.63f);
	}

	void printMitigationRound(const AS::Decisions::mitigationRecord_t* mitigation_ptr,
		                      bool isNewDecision, bool printActualDecisionInfo) {

		if(isNewDecision) { printf(newArrow); }
		if(printActualDecisionInfo) {
			printf("I worry that %s (%3.2f). %s might help. I'm, leaning to %s (%3.2f)\n",
				"LA2_IS_STRONG", 0.44, "STR_S_L", "RES_S_L", (0.6f));
		}
		else {
			printf("\t---- -- - -  ---------- - - - - ---------- - - - -- --- \n");
		}
	}

	void printFinalDecision(const bool choseLeastHarmful, const bool  triedToMitigate, 
		                    const bool choiceWasAboveJustDoIt, const bool decidedToDoNothing, 
		                    const bool initialAmbitionTooLow, const AS::actionData_t* choice_ptr,
		                      bool isNewDecision) {

		//First, we use the data to determine what it is we want to print:
		bool notSureButNoTime = false;
		bool thisSoundsGood = true;
		bool nothingIsWorthTHeTrouble = true;
		bool illJustAvoidProblems = false;
		
		if(isNewDecision) { printf(newArrow); }
		printf("\t----- - -- - -%d, %d, %d, %d ----- - -- - ------------- \n",
			nothingIsWorthTHeTrouble, illJustAvoidProblems, notSureButNoTime, thisSoundsGood);
	}

	void printLAdecisionData(int agent, std::vector<TV::actionChanges_t>* actionsVec_ptr,
		                   std::vector<TV::decisionHasChanges_t>* decisionHasChanges_ptr) {

		auto reflection_ptr = 
			&(CL::ASmirrorData_cptr->decisionReflectionMirror.LAdecisionReflection.at(agent));

		//This is the info we have about the agent's decision-making:
		auto ambitions_ptr = &(reflection_ptr->initialAmbitions);
		auto initialNotions_ptr = &(reflection_ptr->initialTopNotions);

		int mitigationRounds = reflection_ptr->totalMitigationRounds;
		auto firstMitigation_ptr = &(reflection_ptr->mitigationAttempts[0]);
		int lastMitigationRound = std::max(0, (mitigationRounds - 1) );
		auto lastMitigation_ptr = &(reflection_ptr->mitigationAttempts[lastMitigationRound]);

		auto choice_ptr = &(reflection_ptr->finalChoice);

		//These will help us determine how the decision process went:
		bool isNewDecision = decisionHasChanges_ptr->at(agent).hasChanged;
		bool choseLeastHarmful = reflection_ptr->decidedToDoLeastHarmful;
		
		bool triedToMitigate = mitigationRounds > 0;
		bool choiceWasAboveJustDoIt = choice_ptr->details.intensity >= ACT_JUST_DO_IT_THRESOLD;
		bool decidedToDoNothing = choice_ptr->ids.slotIsUsed && choice_ptr->ids.active;
		bool initialAmbitionTooLow = ambitions_ptr->record[0].score < ACT_WHY_BOTHER_THRESOLD;
		
		//We'll have four lines to show: initial, mitigation1, mitigation2, final:
		printInitialThoughtsOnDecision(initialNotions_ptr, ambitions_ptr, isNewDecision);
		printMitigationRound(firstMitigation_ptr, isNewDecision, mitigationRounds > 0);
		printMitigationRound(lastMitigation_ptr, isNewDecision, mitigationRounds > 1);
		printFinalDecision(choseLeastHarmful, triedToMitigate, choiceWasAboveJustDoIt, 
						        decidedToDoNothing, initialAmbitionTooLow, choice_ptr, 
																		isNewDecision);
	}

	void printSeparation() {
		puts(separatorFormatLine);
	}

	void resetScreen() {
		system("cls");
	}

	void printStandardAgent(int agent, std::vector<TV::actionChanges_t>* actionsVec_ptr,
		                  std::vector<TV::decisionHasChanges_t>* decisionHasChanges_ptr) {

		printLAheaderAndstateData(agent, actionsVec_ptr);
		printLAneighborData(agent);
		printLAdecisionData(agent, actionsVec_ptr, decisionHasChanges_ptr);
		printLAactionData(agent, actionsVec_ptr);
	}

	void screenStepStandard(int numberLAs, std::vector<TV::actionChanges_t>* actionsVec_ptr,
		                      std::vector<TV::decisionHasChanges_t>* decisionHasChanges_ptr,
		                     std::chrono::seconds timePassed, std::chrono::seconds loopTime) {
		
		resetScreen(); puts("");

		for(int agent = 0; agent < numberLAs; agent++){
				
			printStandardAgent(agent, actionsVec_ptr, decisionHasChanges_ptr);
			printSeparation();
		}

		printf("\t\tSeconds remaining: %llu...\n", (loopTime - timePassed).count());
	}
			
	void pauseLoop(std::chrono::seconds* loopTime_ptr) {

		auto pauseStart = std::chrono::steady_clock::now();

		AS::pauseMainLoop();

		GETCHAR_PAUSE_SILENT;

		AS::unpauseMainLoop();
		while (AS::checkIfMainLoopIsPaused()) {
			AZ::hybridBusySleepForMicros(std::chrono::microseconds(1*MICROS_IN_A_MILLI));
		}

		auto pauseEnd = std::chrono::steady_clock::now();
				

		*loopTime_ptr +=
			std::chrono::duration_cast<std::chrono::seconds>(pauseEnd - pauseStart);
	}

	void textModeVisualizationLoop(std::chrono::seconds loopTime) {
	
		LOG_DEBUG("Will start visualization Main Loop...", 20);
		GETCHAR_FORCE_PAUSE;

		int numberLAs = CL::ASmirrorData_cptr->networkParams.numberLAs;

		std::vector<TV::actionChanges_t> actionsVec;
		initializeActionsVec(&actionsVec, numberLAs);
		
		std::vector<TV::decisionHasChanges_t> decisionHasChangesVec;
		decisionHasChangesVec.resize(numberLAs);

		bool vizActive = PRINT_VIZ;
		bool changesDetected = false;
		auto timePassed = std::chrono::seconds(0);
		auto start = std::chrono::steady_clock::now();
		printf("\n\n\n\nWill run test for %llu seconds...\n", loopTime.count());
		while (timePassed < loopTime) {
			
			changesDetected = updateActionsAndCheckForChanges(&actionsVec, 
				                        &decisionHasChangesVec, numberLAs);

			if(vizActive){
				screenStepStandard(numberLAs, &actionsVec, &decisionHasChangesVec,
					                                         timePassed, loopTime);

				if(changesDetected && SHOULD_PAUSE_ON_NEW) { pauseLoop(&loopTime); }		
			}

			wait(&timePassed, loopSleepTime, start);					
		}
		printf("\nDone! Leaving Main Loop...\n\n\n");

		return;
	}

	bool stopNetworkAndCheckForErrors() {

		LOG_DEBUG("Stopping AS...\n",1);
		return AS::stop();
	}

}

int TV::textModeVisualizationEntry() {

	if (!initializeAS()) { return 1; }

	if (!loadBaseNetworkAndTestAccessors()) { return 1; }

	if (!setInitialResources(testResources)) { return 1; }

	if (!startNetworkAtPace(testPace)) { return 1; }

	textModeVisualizationLoop(testTime);
	
	bool result = stopNetworkAndCheckForErrors();	

	LOG_DEBUG("Saving results...\n",1);
	result &= AS::saveNetworkToFile(networkFilenameSaveName, false, false, true);

	LOG_DEBUG("Quitting AS...\n\n",1);
	AS::quit();

	LOG_DEBUG("Done. Press enter to exit",1);
	GETCHAR_FORCE_PAUSE;

	int returnCode = !result;
	return returnCode;
}