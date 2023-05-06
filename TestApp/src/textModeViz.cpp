#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "logAPI.hpp"
#include "timeHelpers.hpp"

#include "AS_API.hpp"
#include "CL_externalAPI.hpp"

#include "AS_warningAndErrorDefinitions.hpp"

#include "textViz.hpp"

const char* initialNetworkFilename = "textModeVizBase.txt";
const char* networkFilenameSaveName = "textModeViz_run0.txt";

const std::chrono::seconds testTime = std::chrono::seconds(30);
const std::chrono::milliseconds loopSleepTime = std::chrono::milliseconds(200);
const float testResources = 999999.99f;
const float testPace = 6.0f;

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

	void printAction(AS::actionData_t actionData) {

		int cat = actionData.ids.category;
		int mode = actionData.ids.mode;
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

		printf("\t-> %6.2f/%6.2f s | %u_%u_%u -> %c | intens: %7.2f, aux: %+7.2f\n",
						     secondsElapsed, phaseTotal, cat, mode, phase, 
												   target, intensity, aux);
	}

	const char* placeholderActionFormatLine = 
		       "\t-> --------------- | -------------- | ------------------------------";
	const char* separatorFormatLine = 
		       "**********************************************************************\n";

	void printLAactionData(int agent) {
		
		int maxActions = CL::ASmirrorData_cptr->networkParams.maxActions;
		auto LAactions_ptr = &(CL::ASmirrorData_cptr->actionMirror.dataLAs);

		AS::actionData_t actionData;
		int local = (int)AS::scope::LOCAL;

		for (int action = 0; action < maxActions; action++) {

			int actionIndex = AS::getAgentsActionIndex(agent, action, maxActions);
			actionData = LAactions_ptr->at(actionIndex);

			if (actionData.ids.slotIsUsed && actionData.ids.active) {

				printAction(actionData);
			}
			else {
				puts(placeholderActionFormatLine);
			}
		}
	}
		
	void printLAstateData(int agent) {
		printf("LA STATE: ");
		puts(placeholderActionFormatLine);
	}

	void printLAneighborData(int agent) {

		auto agentState_ptr = 
			&(CL::ASmirrorData_cptr->agentMirrorPtrs.LAstate_ptr->data.at(agent));
		int totalNeighbors = 
			agentState_ptr->locationAndConnections.connectedNeighbors.howManyAreOn();

		for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {
			printf("NEIGHBOR DATA: ");
			puts(placeholderActionFormatLine);
		}

	}

	void printLAdecisionData(int agent) {
		printf("DECISION DATA: ");
		puts(placeholderActionFormatLine);
	}

	void printSeparation() {
		puts(separatorFormatLine);
	}

	void textModeVisualizationLoop(std::chrono::seconds loopTime) {
	
		LOG_DEBUG("Will starting visualization Main Loop...",20);
		GETCHAR_FORCE_PAUSE;

		auto start = std::chrono::steady_clock::now();
		auto timePassed = std::chrono::seconds(0);
		printf("\n\n\n\nWill run test for %llu seconds...\n", loopTime.count());


		int numberLAs = CL::ASmirrorData_cptr->networkParams.numberLAs;
		while (timePassed < loopTime) {
			
			for(int agent = 0; agent < numberLAs; agent++){
			
				printLAstateData(agent);
				printLAneighborData(agent);
				printLAdecisionData(agent);
				printLAactionData(agent);
				printSeparation();
			}

			wait(&timePassed, loopSleepTime, start);
			printf("\t\tSeconds remaining: %llu...\n", (loopTime - timePassed).count());			
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