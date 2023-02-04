#include "miscStdHeaders.h"

#include "logAPI.hpp"

#include "AS_API.hpp"
#include "CL_internalAPI.hpp"

#include "network/parameters.hpp" //exposes "currentNetworkParams"

#include "systems/AScoordinator.hpp"

#define MAX_ERRORS_TO_ACCUMULATE_ON_MAIN 150

namespace AS{
	bool* g_shouldMainLoopBeRunning_ptr;
	std::thread::id* g_mainLoopId_ptr;
	std::thread* g_mainLoopThread_ptr;

	ActionSystem* g_actionSystem_ptr; 
	dataControllerPointers_t* g_agentDataControllerPtrs_ptr;
	networkParameters_t* g_currentNetworkParams_ptr;
}

bool AS::initMainLoopControl(bool* shouldMainLoopBeRunning_ptr,
							std::thread::id* mainLoopId_ptr,
	                        std::thread* mainLoopThread_ptr,
						    ActionSystem* actionSystem_ptr, 
							dataControllerPointers_t* agentDataControllerPtrs_ptr,
							networkParameters_t* currentNetworkParams_ptr){

	LOG_TRACE("Getting Main Loop Control Pointers...");

	bool hasNullPtr =    (shouldMainLoopBeRunning_ptr == NULL)
					  || (mainLoopId_ptr == NULL)
					  || (mainLoopThread_ptr == NULL)
					  || (actionSystem_ptr == NULL)
					  || (agentDataControllerPtrs_ptr == NULL)
					  || (currentNetworkParams_ptr == NULL);
		
	if (hasNullPtr) {
		LOG_ERROR("At least one of the passed pointers is NULL");
		return false;
	}

	g_shouldMainLoopBeRunning_ptr = shouldMainLoopBeRunning_ptr;
	g_mainLoopId_ptr = mainLoopId_ptr;
	g_mainLoopThread_ptr = mainLoopThread_ptr;
	g_actionSystem_ptr = actionSystem_ptr;
	g_agentDataControllerPtrs_ptr = agentDataControllerPtrs_ptr;
	g_currentNetworkParams_ptr = currentNetworkParams_ptr;

	LOG_INFO("Done");
	return true;
}

//Creates thread to run AS's main loop, if it doesn't exist already. Stores the thread::id.
//Checks some conditions before initializing, and returns false if any are not met.
bool AS::run() {
	LOG_TRACE("Starting Main Loop Thread...");
	
	if (*g_shouldMainLoopBeRunning_ptr) {
		if (g_mainLoopThread_ptr->joinable()) {
			LOG_CRITICAL("Main Loop Thread already be running!");
			return false;
		}
		LOG_WARN("Main Loop Thread state control variable was set wrong. Will try to fix and start thread");
	}

	if (!CL::isClientDataPointerInitialized()) {
		LOG_ERROR("Client Data Pointer not initialized. Main Loop can't run");
		return false;
	}

	if (!CL::isASdataPointerInitialized()) {
		LOG_ERROR("AS Data Mirror not initialized. Main Loop can't run");
		return false;
	}

	*g_shouldMainLoopBeRunning_ptr = true;
	g_currentNetworkParams_ptr->lastMainLoopStartingTick = 
												g_currentNetworkParams_ptr->mainLoopTicks;
	*g_mainLoopThread_ptr = std::thread(mainLoop);
	*g_mainLoopId_ptr = g_mainLoopThread_ptr->get_id();

	LOG_INFO("Started main loop on new thread");

	return true;
}

//Main loop STARTS sleeping, does its thing*, consumes Client Changes and sends data to the CL.
//After this it just updates timmings. Ticks are updated right before AS sends its data.
//*includes progressing all actions and updating the states and the decisions of each agent;
//TODO: implement what is described : )
void AS::mainLoop() {
	uint64_t initialTick = g_currentNetworkParams_ptr->mainLoopTicks;

	std::chrono::microseconds zeroMicro = std::chrono::microseconds(0);
	//std::chrono::microseconds stepStartTime;
	std::chrono::microseconds durationThisStepMicro = zeroMicro;
	std::chrono::microseconds tagetStepTimeMicro = std::chrono::microseconds(TST_MAINLOOP_FREQUENCY_MS*1000);
	std::chrono::microseconds timeToSleepMicro = zeroMicro;
	
	bool result = false;
	bool hasThrownErrorsRecently = false;
	int errorsAccumulated = 0;

	do {
		if (!*g_shouldMainLoopBeRunning_ptr)
			{ LOG_TRACE("Main loop will sleep first"); }

		//fflush(stdout);
		std::this_thread::sleep_for(timeToSleepMicro);

		//stepStartTime = blah (get the current time in micro)
		
		//do stuff

		if (!*g_shouldMainLoopBeRunning_ptr) 
			{ LOG_TRACE("Main loop will get Client Data"); }

		result = CL::getNewClientData(g_currentNetworkParams_ptr, 
			                          g_agentDataControllerPtrs_ptr,
									  &(g_actionSystem_ptr->data), 
			                          *g_shouldMainLoopBeRunning_ptr);
		
		//TODO: Extract
		if (!result) { 
			if (!hasThrownErrorsRecently) {
				LOG_ERROR("Failed to get Client Data!"); 
				printf("(had accumulated %d errors before this)",errorsAccumulated);
				hasThrownErrorsRecently = true;
				errorsAccumulated = 0;
			}
			else {
				errorsAccumulated++;
				if (errorsAccumulated > MAX_ERRORS_TO_ACCUMULATE_ON_MAIN) {
					hasThrownErrorsRecently = false;
				}
			}
		}
		
		if (!*g_shouldMainLoopBeRunning_ptr) 
			{ LOG_TRACE("Main loop will update ticks"); }

		g_currentNetworkParams_ptr->mainLoopTicks++;

		if (!*g_shouldMainLoopBeRunning_ptr) 
			{ LOG_TRACE("Main loop will send AS Data to CL"); }

		result = sendReplacementDataToCL(true);
		if (!result) { 
			if (!hasThrownErrorsRecently) {
				LOG_ERROR("Failed to send AS Data to the CL!");
				printf("(had accumulated %d errors before this)",errorsAccumulated);
				hasThrownErrorsRecently = true;
				errorsAccumulated = 0;
			}
			else {
				errorsAccumulated++;
				if (errorsAccumulated > MAX_ERRORS_TO_ACCUMULATE_ON_MAIN) {
					hasThrownErrorsRecently = false;
				}
			}
		}

		if (!*g_shouldMainLoopBeRunning_ptr) 
			{ LOG_TRACE("Main loop will calculate time to sleep"); }

		//durationThisStepMicro = "current time in micro" - stepStartTime
		timeToSleepMicro = tagetStepTimeMicro - durationThisStepMicro;
		if(timeToSleepMicro < zeroMicro) timeToSleepMicro = zeroMicro;

		if (!*g_shouldMainLoopBeRunning_ptr)
			{ LOG_TRACE("Main loop should stop now!"); }

	} while (*g_shouldMainLoopBeRunning_ptr);
}

//Stops AS execution thread, marks it as stopped and clears the stored thread::id;
bool AS::stop() {
	LOG_TRACE("Stopping Main Loop Thread...");
		
	if (!*g_shouldMainLoopBeRunning_ptr) {
		LOG_ERROR("Main Loop Thread already supposed to be stopped...");
		return false;
	}

	*g_shouldMainLoopBeRunning_ptr = false;

	if (isMainLoopRunning()) {
		LOG_TRACE("Waiting for main loop to finish execution...");
		g_mainLoopThread_ptr->join();
		*g_mainLoopId_ptr = std::thread::id(); //default-constructs to "no thread" value
		LOG_INFO("Done.");

		#if (defined AS_DEBUG) || VERBOSE_RELEASE
			printf("\nRan for %llu ticks\n", currentNetworkParams.mainLoopTicks - 
											currentNetworkParams.lastMainLoopStartingTick);
		#endif // AS_DEBUG		
		return true;
	}
	else {
		LOG_ERROR("Main Loop Thread was supposed to be active, but was not!");
		return false;
	}	
}

bool AS::chekIfMainLoopShouldBeRunning() {
	return *g_shouldMainLoopBeRunning_ptr;
}

bool AS::isMainLoopRunning() {
	return g_mainLoopThread_ptr->joinable();
}
