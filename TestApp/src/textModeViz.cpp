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
const float testResources = DEFAULT_LA_RESOURCES;
const float testPace = 1.0f;

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
		
		if (!AS::run()) {
			LOG_CRITICAL("Failed to run network");
			GETCHAR_FORCE_PAUSE;
			return false;
		}

		AS::changePace(pace);

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

	bool updateActionsAndChangeOnPause(std::vector<TV::actionChanges_t>* actionsVec_ptr, 
											                              int numberLAs) {

		int maxActionsPerAgent = CL::ASmirrorData_cptr->networkParams.maxActions;
		assert( actionsVec_ptr->size() == (maxActionsPerAgent * numberLAs) ); //from init

		auto LAactions_ptr = &(CL::ASmirrorData_cptr->actionMirror.dataLAs);

		AS::actionData_t newActionData;
		int local = (int)AS::scope::LOCAL; //we're only dealing with locals for now

		bool foundNewAction = false;
		for(int agent = 0; agent < numberLAs; agent++){
			for (int action = 0; action < maxActionsPerAgent; action++) {

				int actionIndex = AS::getAgentsActionIndex(agent, action, maxActionsPerAgent);
				newActionData = LAactions_ptr->at(actionIndex);

				AS::actionData_t* oldAction_ptr = &(actionsVec_ptr->at(actionIndex).data);

				if (newActionData.ids.slotIsUsed && newActionData.ids.active) {
					
					//Change in IDs mean something important happened
					bool isNew = (oldAction_ptr->ids != newActionData.ids);
					foundNewAction |= isNew;
					actionsVec_ptr->at(actionIndex).hasChanged = isNew;

					*oldAction_ptr = newActionData; //we always update if used and active
				}
			}
		}

		return foundNewAction;
	}

	void printAction(AS::actionData_t actionData) {

		std::string_view cat = AS::catToString((AS::actCategories)actionData.ids.category);
		char mode = AS::modeToChar((AS::actModes)actionData.ids.mode);
		int phase = actionData.ids.phase;
		char target = 'x';
		if (actionData.ids.target != actionData.ids.origin) {
			target = '0' + actionData.ids.target; //expects target <= 9
		}
		
		double secondsElapsed = 
			(double)actionData.phaseTiming.elapsed/(double)TENTHS_OF_MS_IN_A_SECOND;
		double phaseTotal = 
			(double)actionData.phaseTiming.total/(double)TENTHS_OF_MS_IN_A_SECOND;

		float intensity = actionData.details.intensity;
		float aux = actionData.details.processingAux;

		printf("\t-> %6.2f/%6.2f s | %s_%c_%u -> %c | intens: %7.2f, aux: %+7.2f\n",
						     secondsElapsed, phaseTotal, cat.data(), mode, phase, 
												   target, intensity, aux);
	}

	const char* placeholderActionFormatLine = 
		       "\t-> --------------- | ------------ | ------------------------------";
	const char* separatorFormatLine = 
		       "**********************************************************************\n";
	const char* newArrow = ">>--NEW-->";


	void printLAactionData(int agent, std::vector<TV::actionChanges_t>*actionsVec_ptr) {
		
		int maxActions = CL::ASmirrorData_cptr->networkParams.maxActions;

		AS::actionData_t* actionData_ptr;

		for (int action = 0; action < maxActions; action++) {

			int actionIndex = AS::getAgentsActionIndex(agent, action, maxActions);
			actionData_ptr = &(actionsVec_ptr->at(actionIndex).data);
			bool* isNew_ptr = &(actionsVec_ptr->at(actionIndex).hasChanged);

			if (actionData_ptr->ids.slotIsUsed && actionData_ptr->ids.active) {

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
		
	void printLAheaderAndstateData(int agent) {
		
		std::string agentName = 
			CL::ASmirrorData_cptr->agentMirrorPtrs.LAcoldData_ptr->data.at(agent).name;

		auto agentState_ptr =
			&(CL::ASmirrorData_cptr->agentMirrorPtrs.LAstate_ptr->data.at(agent));

		AS::pos_t position = agentState_ptr->locationAndConnections.position;
		auto resources_ptr = &(agentState_ptr->parameters.resources);
		auto strenght_ptr = &(agentState_ptr->parameters.strenght);


		strenght_ptr->current; strenght_ptr->currentUpkeep; strenght_ptr->externalGuard;
		strenght_ptr->thresholdToCostUpkeep;

		printf("LA%d (GA %d) | X: %+4.2f, Y: %+4.2f | name: %s\n",
			    agent, agentState_ptr->GAid, position.x, position.y, agentName.c_str());

		printf("\tSTATE | %+10.2f $ (%+6.2f $/sec) | %7.2f S , %7.2f D (%+5.2f $/sec)\n",
			                         resources_ptr->current, resources_ptr->updateRate,
			                        strenght_ptr->current, strenght_ptr->externalGuard,
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
			
			float resources = decisionData_ptr->reads[neighbor].readOf[resorucesReadField];
			float income = decisionData_ptr->reads[neighbor].readOf[incomeReadField];
			float strenght = decisionData_ptr->reads[neighbor].readOf[strenghtReadField];
			float guard = decisionData_ptr->reads[neighbor].readOf[guardReadField];
			
			printf("\tNEIGHBOR: %d | %c, disp: %+3.2f | %+3.2f ? | %+10.2f $ (%+6.2f $/sec) | %7.2f S , %7.2f D\n",
									neighborID, stanceChar, disposition, 
									infiltration, resources, income, strenght, guard);		
		}
	}

	void printLAdecisionData(int agent) {
		printf("\tDECISION: CCC_M (NTN+, NTN-) | CCC_M? CCC_M (NTN+, NTN-) | CCC_M -> x, YY\n");
	}

	void printSeparation() {
		puts(separatorFormatLine);
	}

	void resetScreen() {
		system("cls");
	}

	void printStandardAgent(int agent, std::vector<TV::actionChanges_t>* actionsVec_ptr) {

		printLAheaderAndstateData(agent);
		printLAneighborData(agent);
		printLAdecisionData(agent);
		printLAactionData(agent, actionsVec_ptr);
	}

	void screenStepStandard(int numberLAs, std::vector<TV::actionChanges_t>* actionsVec_ptr,
		                     std::chrono::seconds timePassed, std::chrono::seconds loopTime) {
		
		resetScreen(); puts("");

		for(int agent = 0; agent < numberLAs; agent++){
				
			printStandardAgent(agent, actionsVec_ptr);
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
	
		LOG_DEBUG("Will starting visualization Main Loop...",20);
		GETCHAR_FORCE_PAUSE;

		auto start = std::chrono::steady_clock::now();
		auto timePassed = std::chrono::seconds(0);
		printf("\n\n\n\nWill run test for %llu seconds...\n", loopTime.count());

		int numberLAs = CL::ASmirrorData_cptr->networkParams.numberLAs;

		std::vector<TV::actionChanges_t> actionsVec;
		initializeActionsVec(&actionsVec, numberLAs);

		bool newAction = false;
		while (timePassed < loopTime) {
			
			newAction = updateActionsAndChangeOnPause(&actionsVec, numberLAs);

			screenStepStandard(numberLAs, &actionsVec, timePassed, loopTime);

			if(newAction) { pauseLoop(&loopTime); } //after displaying it			

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