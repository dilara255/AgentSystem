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

std::chrono::microseconds nowMicros() {
	auto now = std::chrono::steady_clock::now().time_since_epoch();
	return std::chrono::duration_cast<std::chrono::microseconds>(now);
}

bool AS::initMainLoopControl(bool* shouldMainLoopBeRunning_ptr,
							std::thread::id* mainLoopId_ptr,
	                        std::thread* mainLoopThread_ptr,
						    ActionSystem* actionSystem_ptr, 
							dataControllerPointers_t* agentDataControllerPtrs_ptr,
							networkParameters_t* currentNetworkParams_ptr);

std::chrono::microseconds zeroMicro = std::chrono::microseconds(0);
std::chrono::microseconds sysWakeUpDelayMicros = 
										std::chrono::microseconds(SYS_WAKEUP_DELAY_MICROS);

void prepareStep(uint64_t* stepsWithoutDecisions_ptr, bool* shouldMakeDecisions_ptr);
void step(bool shouldMakeDecisions);
void receiveAndSendData(bool* hasThrownErrorsRecently_ptr, int* errorsAccumulated_ptr);
void timeAndSleep(std::chrono::microseconds tagetStepTimeMicro,
	              std::chrono::microseconds* stepStartTimeMicro_ptr);

void accumulateOrTrhowError(bool* hasThrownErrorsRecently_ptr, int* errorsAccumulated_ptr,
							                                                char* message);

void AS::mainLoop() {
	uint64_t initialTick = g_currentNetworkParams_ptr->mainLoopTicks;

	std::chrono::microseconds tagetStepTimeMicro = 
		                       std::chrono::microseconds(AS_MILLISECONDS_PER_STEP*1000);
	
	bool hasThrownErrorsRecently = false;
	int errorsAccumulated = 0;
	uint64_t stepsWithoutDecisions = 0;
	bool shouldMakeDecisions = false;

	std::chrono::microseconds stepStartTimeMicro = nowMicros();

	do {
		prepareStep(&stepsWithoutDecisions, &shouldMakeDecisions);
		step(shouldMakeDecisions);
		receiveAndSendData(&hasThrownErrorsRecently, &errorsAccumulated);
		timeAndSleep(tagetStepTimeMicro, &stepStartTimeMicro);
	} while (*g_shouldMainLoopBeRunning_ptr);
}

void prepareStep(uint64_t* stepsWithoutDecisions_ptr, bool* shouldMakeDecisions_ptr) {
	
	//Should the Agents Step calculate decisions?
	(*stepsWithoutDecisions_ptr)++;
	*shouldMakeDecisions_ptr = ((*stepsWithoutDecisions_ptr) == AS_STEPS_PER_DECISION_STEP);
	if(*shouldMakeDecisions_ptr) {*stepsWithoutDecisions_ptr = 0;}

	AS::drawPRNs(*shouldMakeDecisions_ptr);
}

void step(bool shouldMakeDecisions) {
	AS::stepActions();
	AS::stepAgents(shouldMakeDecisions);
}

void receiveAndSendData(bool* hasThrownErrorsRecently_ptr, int* errorsAccumulated_ptr) {
	if (!*AS::g_shouldMainLoopBeRunning_ptr) 
			{ LOG_TRACE("Main loop will get Client Data and send AS Data to CL"); }

		bool result = CL::getNewClientData(AS::g_currentNetworkParams_ptr, 
			                               AS::g_agentDataControllerPtrs_ptr,
						   			       &(AS::g_actionSystem_ptr->data), 
			                               *AS::g_shouldMainLoopBeRunning_ptr);
		if (!result) { 
			accumulateOrTrhowError(hasThrownErrorsRecently_ptr, errorsAccumulated_ptr,
									                     "Failed to get Client Data!");
		}

		result = AS::sendReplacementDataToCL(true);
		if (!result) { 
				accumulateOrTrhowError(hasThrownErrorsRecently_ptr, errorsAccumulated_ptr,
									                  "Failed to send AS Data to the CL!");
		}		
}

void timeAndSleep(std::chrono::microseconds tagetStepTimeMicro,
	              std::chrono::microseconds* stepStartTimeMicro_ptr) {
	
	if (!*AS::g_shouldMainLoopBeRunning_ptr) { 
		LOG_TRACE("Main loop will sleep first"); 
	}

	std::chrono::microseconds elapsedMicro = nowMicros() - *stepStartTimeMicro_ptr;
	std::chrono::microseconds microsToSleep = tagetStepTimeMicro - elapsedMicro - 
		                                                      sysWakeUpDelayMicros;
	
	if(microsToSleep > zeroMicro) {
		std::this_thread::sleep_for(microsToSleep);
	}

	*stepStartTimeMicro_ptr = nowMicros();

	AS::g_currentNetworkParams_ptr->lastStepTimeMicros = elapsedMicro;

	AS::g_currentNetworkParams_ptr->mainLoopTicks++;	

	if (!*AS::g_shouldMainLoopBeRunning_ptr) { 
		LOG_TRACE("Main loop should stop now!"); 
	}
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
		*g_mainLoopId_ptr = std::thread::id();
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

void accumulateOrTrhowError(bool* hasThrownErrorsRecently_ptr, int* errorsAccumulated_ptr,
							                                                char* message) {
	if (!(*hasThrownErrorsRecently_ptr)) {
				LOG_ERROR(message); 
				printf("(had accumulated %d errors before this)",*errorsAccumulated_ptr);
				*hasThrownErrorsRecently_ptr = true;
				*errorsAccumulated_ptr = 0;
	}
	else {
		(*errorsAccumulated_ptr)++;
		if (*errorsAccumulated_ptr > MAX_ERRORS_TO_ACCUMULATE_ON_MAIN) {
			*hasThrownErrorsRecently_ptr = false;
		}
	}
}
