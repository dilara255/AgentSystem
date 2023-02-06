#include "miscStdHeaders.h"

#include "logAPI.hpp"

#include "AS_API.hpp"
#include "CL_internalAPI.hpp"

#include "network/parameters.hpp" //exposes "currentNetworkParams"

#include "systems/AScoordinator.hpp"
#include "systems/prnsServer.hpp"

#include "timeHelpers.hpp"

#define MICROS_TO_BUSY_WAIT (10)

#define MAX_ERRORS_TO_ACCUMULATE_ON_MAIN 150

namespace AS{
	bool* g_shouldMainLoopBeRunning_ptr;
	std::thread::id* g_mainLoopId_ptr;
	std::thread* g_mainLoopThread_ptr;
	AS::PRNserver* g_prnServer_ptr;

	ActionSystem* g_actionSystem_ptr; 
	dataControllerPointers_t* g_agentDataControllerPtrs_ptr;
	networkParameters_t* g_currentNetworkParams_ptr;

	//TODO: CHANGE NAME (not really just about micro seconds any more)
	struct timingMicros_st {
		uint64_t ticks = 0;
		float timeMultiplier;
		double accumulatedMultiplier = 0;
		int64_t totalSnoozedMicros = 0; //time spent sleeping more then expected (or less)
		int64_t largestSnoozeMicros = 0;
		int64_t totalHotMicros = 0; //time spent actually doing work (except busy-waiting)
		int64_t largestHotMicros = 0;
		int64_t totalSleepMicros = 0;
		int64_t largestSleepMicros = 0;
		int64_t totalMicrosPreparation = 0;
		int64_t totalMicrosStep = 0;
		int64_t totalMicrosDataTransfer = 0;
		int64_t totalMicrosTimmingAndSleep = 0;
		std::chrono::steady_clock::time_point startFirstStep;
		std::chrono::steady_clock::time_point startLastStep;
		std::chrono::steady_clock::time_point startThisStep;
		std::chrono::steady_clock::time_point endPreparation;
		std::chrono::steady_clock::time_point endStep;
		std::chrono::steady_clock::time_point endDataTransfer;
		std::chrono::steady_clock::time_point endTimmingAndSleep;
		std::chrono::microseconds targetStepTime;
	};
}

std::chrono::microseconds zeroMicro = std::chrono::microseconds(0);

void prepareStep(AS::chopControl_st* chopControl_ptr);
void step(AS::chopControl_st chopControl, float timeMultiplier);
void receiveAndSendData(bool* hasThrownErrorsRecently_ptr, int* errorsAccumulated_ptr);
void timeAndSleep(AS::timingMicros_st* timing_ptr);

int getTotalPRNsToDraw(int numberLAs, int numberGAs);
inline bool isLastChop(int chopIndex);
int howManyPRNsThisCHop(int chopIndex, int numberLAs, int numberGAs);
int howManyDecisionsThisChop(int chopIndex, int numberAgents);

void timeOperation(std::chrono::steady_clock::time_point lastReferenceTime,
	               std::chrono::steady_clock::time_point* newReferenceTime,
	                                           int64_t* counterToIncrement);
void calculateAndprintMainTimingInfo(AS::timingMicros_st timingMicros);

void accumulateOrTrhowError(bool* hasThrownErrorsRecently_ptr, int* errorsAccumulated_ptr,
							                                                char* message);

bool AS::initMainLoopControl(bool* shouldMainLoopBeRunning_ptr,
							 std::thread::id* mainLoopId_ptr,
	                         std::thread* mainLoopThread_ptr,
						     ActionSystem* actionSystem_ptr, 
							 dataControllerPointers_t* agentDataControllerPtrs_ptr,
							 networkParameters_t* currentNetworkParams_ptr,
	                         AS::PRNserver* prnServer_ptr);

void AS::mainLoop() {
	//setup:
	bool hasThrownErrorsRecently = false;
	int errorsAccumulated = 0;

	timingMicros_st timingMicros;
	chopControl_st chopControl;
	chopControl.numberLAs = g_currentNetworkParams_ptr->numberLAs;
	chopControl.numberGAs = g_currentNetworkParams_ptr->numberGAs;
	chopControl.totalPRNsNeeded = 
		            getTotalPRNsToDraw(chopControl.numberLAs, chopControl.numberGAs);
	
	timingMicros.targetStepTime = 
		            std::chrono::milliseconds(AS_MILLISECONDS_PER_STEP);
	timingMicros.timeMultiplier = (float)AS_MILLISECONDS_PER_STEP/MILLIS_IN_A_SECOND;
	timingMicros.startThisStep = std::chrono::steady_clock::now();
	timingMicros.startLastStep = timingMicros.startThisStep;
	timingMicros.startFirstStep = timingMicros.startThisStep;
	timingMicros.endTimmingAndSleep =  timingMicros.startThisStep; //for first iteration

	//Actual loop:
	do {
		prepareStep(&chopControl);
		timeOperation(timingMicros.endTimmingAndSleep, &timingMicros.endPreparation,
		                                       &timingMicros.totalMicrosPreparation);
		
		step(chopControl, timingMicros.timeMultiplier);
		timeOperation(timingMicros.endPreparation, &timingMicros.endStep,
			                               &timingMicros.totalMicrosStep);

		receiveAndSendData(&hasThrownErrorsRecently, &errorsAccumulated);
		timeOperation(timingMicros.endStep, &timingMicros.endDataTransfer,
			                        &timingMicros.totalMicrosDataTransfer);

		timeAndSleep(&timingMicros);
		timeOperation(timingMicros.endDataTransfer, &timingMicros.endTimmingAndSleep,
			                                &timingMicros.totalMicrosTimmingAndSleep);

		/*TODO-CRITICAL: EXTRACT INTO TEST:
		auto startDump = std::chrono::steady_clock::now();
		if(!g_prnServer_ptr->dumpData()){
			LOG_CRITICAL("CONSTIPATED");
			GETCHAR_PAUSE;
		}
		auto endDump = std::chrono::steady_clock::now();
		
		auto dumpDuration = (endDump-startDump);;
		auto dumpDurationMicro = 
					std::chrono::duration_cast<std::chrono::microseconds>(dumpDuration);
		timingMicros.totalMicrosPreparation -= dumpDurationMicro.count();
		*/
	} while (*g_shouldMainLoopBeRunning_ptr);

	calculateAndprintMainTimingInfo(timingMicros);
}

void prepareStep(AS::chopControl_st* chopControl_ptr) {
	
	int numLAs = chopControl_ptr->numberLAs;
	int numGAs = chopControl_ptr->numberGAs;

	//How many PRNs should be generated this tick?
	chopControl_ptr->PRNsToDrawThisChop =
	                howManyPRNsThisCHop(chopControl_ptr->chopIndex, numLAs, numGAs);

	AS::g_prnServer_ptr->drawPRNs(chopControl_ptr->chopIndex, 
			chopControl_ptr->PRNsToDrawThisChop, chopControl_ptr->totalPRNsNeeded);

	for (int i = 0; i < DRAW_WIDTH; i++) {
		AS::g_currentNetworkParams_ptr->seeds[i] = AS::g_prnServer_ptr->getSeed(i);
	}

	//How many decisions should the Agents Step calculate this tick?
	chopControl_ptr->LAdecisionsToMake = 
		               howManyDecisionsThisChop(chopControl_ptr->chopIndex, numLAs);
	chopControl_ptr->GAdecisionsToMake = 
		               howManyDecisionsThisChop(chopControl_ptr->chopIndex, numGAs);
	
	
	chopControl_ptr->chopIndex++;
	chopControl_ptr->chopIndex %= AS_TOTAL_CHOPS;
}

void step(AS::chopControl_st chopControl, float timeMultiplier) {
	
	int numberLAs = chopControl.numberLAs;
	int numberGAs = chopControl.numberGAs;

	/*
	//TODO-CRITICAL: MOVE TO TEST
	if (shouldMakeDecisions) {
		AS::g_prnServer_ptr->printDataDebug();
		GETCHAR_PAUSE;
	}
	*/

	AS::stepActions(AS::g_actionSystem_ptr, numberLAs, numberGAs, timeMultiplier);

	AS::stepAgents(chopControl.LAdecisionsToMake, chopControl.GAdecisionsToMake, 
		                                      AS::g_agentDataControllerPtrs_ptr, 
		                                   timeMultiplier, numberLAs, numberGAs);
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

void timeAndSleep(AS::timingMicros_st* timing_ptr) {
	
	timing_ptr->startLastStep = timing_ptr->startThisStep;

	auto targetWakeTime = timing_ptr->startThisStep + timing_ptr->targetStepTime;
	std::chrono::microseconds threshold(MICROS_TO_BUSY_WAIT);

	auto bedTime = std::chrono::steady_clock::now();
	
	AZ::hybridBusySleep(targetWakeTime, threshold);

	//this is when next step timing starts
	timing_ptr->startThisStep = std::chrono::steady_clock::now();
	timing_ptr->ticks++;

	auto actualSleepTime = timing_ptr->startThisStep - bedTime;
	int64_t actualSleepTimeMicros =
		    std::chrono::duration_cast<std::chrono::microseconds>(actualSleepTime).count();
	timing_ptr->totalSleepMicros += actualSleepTimeMicros;
	if (actualSleepTimeMicros > timing_ptr->largestSleepMicros) {
		timing_ptr->largestSleepMicros = actualSleepTimeMicros;
	}
	
	auto hotTime = bedTime - timing_ptr->startLastStep;
	int64_t hotTimeMicros = 
		        std::chrono::duration_cast<std::chrono::microseconds>(hotTime).count();
	timing_ptr->totalHotMicros += hotTimeMicros;
	if (hotTimeMicros > timing_ptr->largestHotMicros) {
		timing_ptr->largestHotMicros = hotTimeMicros;
	}

	auto snoozed = timing_ptr->startThisStep - targetWakeTime;
	if(hotTime > timing_ptr->targetStepTime){snoozed = zeroMicro;}
	int64_t snoozedMicros = 
		 std::chrono::duration_cast<std::chrono::microseconds>(snoozed).count();
	timing_ptr->totalSnoozedMicros += snoozedMicros;
	if (snoozedMicros > timing_ptr->largestSnoozeMicros) {
		timing_ptr->largestSnoozeMicros = snoozedMicros;
	}
	
	//Calculate timeMultiplier, which will be used to keep logic frquency-independent (to an extent)
	auto lastStepDuration = timing_ptr->startThisStep - timing_ptr->startLastStep;
	double lastStepDurationMicros =
			  (double)(std::chrono::duration_cast<std::chrono::microseconds>(lastStepDuration).count());
	
	double targetStepTimeMicros = (double)timing_ptr->targetStepTime.count();
	double proportionalDurationError = lastStepDurationMicros/targetStepTimeMicros;

	if (proportionalDurationError > MAX_PROPORTIONAL_STEP_DURATION_ERROR) {
		lastStepDurationMicros = targetStepTimeMicros*MAX_PROPORTIONAL_STEP_DURATION_ERROR;
	}

	timing_ptr->timeMultiplier = (float)lastStepDurationMicros/MICROS_IN_A_SECOND;
	timing_ptr->accumulatedMultiplier += timing_ptr->timeMultiplier;

	//update externally available step counting and timing information:
	AS::g_currentNetworkParams_ptr->lastStepTimeMicros = 
		       std::chrono::duration_cast<std::chrono::microseconds>(timing_ptr->startThisStep - timing_ptr->startLastStep);

	AS::g_currentNetworkParams_ptr->mainLoopTicks++;
}

void timeOperation(std::chrono::steady_clock::time_point lastReferenceTime,
	               std::chrono::steady_clock::time_point* newReferenceTime,
	                                           int64_t* counterToIncrement) {
	*newReferenceTime = std::chrono::steady_clock::now();
	auto delta = *newReferenceTime - lastReferenceTime;
	*counterToIncrement += std::chrono::duration_cast<std::chrono::microseconds>(delta).count();
}

void calculateAndprintMainTimingInfo(AS::timingMicros_st timingMicros) {
	
	auto duration = std::chrono::steady_clock::now() - timingMicros.startFirstStep;

	double inverseOfTicks = 1.0/timingMicros.ticks;

	int64_t durationMicro = 
		std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
	double durationSeconds = (double)durationMicro/MICROS_IN_A_SECOND;
	
	double averagaTimeMultiplier = timingMicros.accumulatedMultiplier*inverseOfTicks;
	double msPerTick = (durationSeconds*inverseOfTicks)*MILLIS_IN_A_SECOND;
	int64_t targetMicrosPerTick = timingMicros.targetStepTime.count();
	double averageSnoozeMicros = timingMicros.totalSnoozedMicros*inverseOfTicks;
	double hotMicrosPerTick = timingMicros.totalHotMicros*inverseOfTicks;
	double percentHot = (hotMicrosPerTick/targetMicrosPerTick)*100;
	double averageSleepMicros = timingMicros.totalSleepMicros*inverseOfTicks;
	int64_t totalMicrosTimming = timingMicros.totalMicrosTimmingAndSleep -
		           (timingMicros.totalSleepMicros - timingMicros.totalSnoozedMicros);
	double avgMicrosPreparation = timingMicros.totalMicrosPreparation*inverseOfTicks;
	double avgMicrosStep = timingMicros.totalMicrosStep*inverseOfTicks;
	double avgMicrosDataTransfer = timingMicros.totalMicrosDataTransfer*inverseOfTicks;
	double avglMicrosTimming = totalMicrosTimming*inverseOfTicks;

	LOG_INFO("MainLoop Timings:");

	printf("Main Loop Ended: %llu ticks in %f seconds (%f ms/tick, target: %lld micros)\n", 
		   timingMicros.ticks, durationSeconds, msPerTick, targetMicrosPerTick);
	printf("Averages (micro): sleep: %f (soozed: %f), time multiplier: %f, hot micros/tick: %f\n(%f %% target)\n",
		   averageSleepMicros, averageSnoozeMicros, averagaTimeMultiplier, 
		   hotMicrosPerTick, percentHot);

#if (defined AS_DEBUG) || VERBOSE_RELEASE
	printf("Largest (micro): sleep: %lld, snooze: %lld, hot: %lld\n",
		   timingMicros.largestSleepMicros, timingMicros.largestSnoozeMicros, 
		                                     timingMicros.largestHotMicros);
	printf("Totals (micro): sleep: %lld, snooze: %lld, hot: %lld\n",
		   timingMicros.totalSleepMicros, timingMicros.totalSnoozedMicros, 
		   timingMicros.totalHotMicros);
	printf("Substep Averages (micro): preparation: %f, step: %f, dataTransfer: %f, timming: %f\n",
		   avgMicrosPreparation, avgMicrosStep, avgMicrosDataTransfer, avglMicrosTimming);
	printf("Substep Percents of Budget: preparation: %f%%, step: %f%%, dataTransfer: %f%%, timming: %f%%\n",
		   (avgMicrosPreparation/targetMicrosPerTick)*100, 
		   (avgMicrosStep/targetMicrosPerTick)*100,
		   (avgMicrosDataTransfer/targetMicrosPerTick)*100, 
		   (avglMicrosTimming/targetMicrosPerTick)*100);

	GETCHAR_PAUSE;
#endif //AS_DEBUG
}

int getTotalPRNsToDraw(int numberLAs, int numberGAs) {
	return PRNS_PER_LA*numberLAs + PRNS_PER_GA*numberGAs + MAX_ACT_PRNS;
}

inline bool isLastChop(int chopIndex) {
	return (chopIndex == (AS_TOTAL_CHOPS-1));
}

int howManyPRNsThisCHop(int chopIndex, int numberLAs, int numberGAs) {

	int totalPrnsToDraw = getTotalPRNsToDraw(numberLAs, numberGAs);
	int prnsToDrawPerRegularChop = totalPrnsToDraw / AS_TOTAL_CHOPS;
	int remainderPRNs = totalPrnsToDraw % AS_TOTAL_CHOPS;

	int toDrawThisChop = prnsToDrawPerRegularChop;
	if (isLastChop(chopIndex)) {
		toDrawThisChop += remainderPRNs;
	}

	return toDrawThisChop;
}

int howManyDecisionsThisChop(int chopIndex, int numberAgents) {

	int decisionsToTakePerRegularChop = numberAgents / AS_TOTAL_CHOPS;
	int remainderDecisions = numberAgents % AS_TOTAL_CHOPS;

	int toDecideThisChop = decisionsToTakePerRegularChop;
	if (isLastChop(chopIndex)) {
		toDecideThisChop += remainderDecisions;
	}

	return toDecideThisChop;
}

bool AS::initMainLoopControl(bool* shouldMainLoopBeRunning_ptr,
							std::thread::id* mainLoopId_ptr,
	                        std::thread* mainLoopThread_ptr,
						    ActionSystem* actionSystem_ptr, 
							dataControllerPointers_t* agentDataControllerPtrs_ptr,
							networkParameters_t* currentNetworkParams_ptr,
	                        AS::PRNserver* prnServer_ptr){

	LOG_TRACE("Getting Main Loop Control Pointers...");

	bool hasNullPtr =    (shouldMainLoopBeRunning_ptr == NULL)
					  || (mainLoopId_ptr == NULL)
					  || (mainLoopThread_ptr == NULL)
					  || (actionSystem_ptr == NULL)
					  || (agentDataControllerPtrs_ptr == NULL)
					  || (currentNetworkParams_ptr == NULL)
		              || (prnServer_ptr == NULL);
		
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
	g_prnServer_ptr = prnServer_ptr;

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

	LOG_TRACE("Loading seeds...");

	for (int i = 0; i < DRAW_WIDTH; i++) {
		AS::g_prnServer_ptr->setSeed(i, AS::g_currentNetworkParams_ptr->seeds[i]);
	}

	LOG_TRACE("Creating Main Loop Thread and marking as started...");

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
			printf("\nRan for %llu ticks\n", g_currentNetworkParams_ptr->mainLoopTicks - 
								   g_currentNetworkParams_ptr->lastMainLoopStartingTick);
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
