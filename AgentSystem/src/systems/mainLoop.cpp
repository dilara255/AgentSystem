#include "miscStdHeaders.h"

#include "logAPI.hpp"

#include "AS_API.hpp"
#include "CL_internalAPI.hpp"

#include "network/parameters.hpp" //exposes "currentNetworkParams"

#include "systems/AScoordinator.hpp"
#include "systems/PRNserver.hpp"
#include "systems/warningsAndErrorsCounter.hpp"

#include "timeHelpers.hpp"

#define MICROS_TO_BUSY_WAIT (10)

#define MAX_ERRORS_TO_ACCUMULATE_ON_MAIN 150

namespace AS{
	bool* g_shouldMainLoopBeRunning_ptr;
	bool g_shouldMainLoopBePaused = false;
	bool g_isMainLoopBePaused = false;
	std::thread::id* g_mainLoopId_ptr;
	std::thread* g_mainLoopThread_ptr;
	AS::PRNserver* g_prnServer_ptr;

	ActionSystem* g_actionSystem_ptr; 
	dataControllerPointers_t* g_agentDataControllerPtrs_ptr;
	networkParameters_t* g_currentNetworkParams_ptr;

	static WarningsAndErrorsCounter* g_errorsCounter_ptr;
	int g_warnings;
	int g_errors;

	std::chrono::microseconds zeroMicro = std::chrono::microseconds(0);

	struct timing_st {
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
		std::chrono::steady_clock::time_point endTimingAndSleep;
		std::chrono::microseconds targetStepTime;
		std::chrono::microseconds timeSpentPaused = zeroMicro;
	};
}

void prepareStep(AS::chopControl_st* chopControl_ptr);
void step(AS::chopControl_st chopControl, float timeMultiplier);
void receiveAndSendData();
void timeAndSleep(AS::timing_st* timing_ptr);

int getTotalPRNsToDraw(int numberLAs, int numberGAs);
int howManyDecisionsThisChop(int chopIndex, int* decisionsMade_ptr, int numberAgents);

void timeOperation(std::chrono::steady_clock::time_point lastReferenceTime,
	               std::chrono::steady_clock::time_point* newReferenceTime,
	                                           int64_t* counterToIncrement);
void calculateAndPrintMainTimingInfo(AS::timing_st timingMicros);

bool AS::initMainLoopControl(bool* shouldMainLoopBeRunning_ptr,
							 std::thread::id* mainLoopId_ptr,
	                         std::thread* mainLoopThread_ptr,
						     ActionSystem* actionSystem_ptr, 
							 dataControllerPointers_t* agentDataControllerPtrs_ptr,
							 networkParameters_t* currentNetworkParams_ptr,
	                         AS::PRNserver* prnServer_ptr);

void AS::mainLoop() {
	//setup:
	uint64_t minimumTicksPerErrorDisplay = 
		(SECONDS_PER_ERROR_DISPLAY*MILLIS_IN_A_SECOND)/AS_MILLISECONDS_PER_STEP;
	WarningsAndErrorsCounter counter(0, minimumTicksPerErrorDisplay);
	g_errorsCounter_ptr = &counter;
	g_warnings = 0;
	g_errors = 0;

	timing_st timingMicros;
	chopControl_st chopControl;
		
	timingMicros.targetStepTime = 
		            std::chrono::milliseconds(AS_MILLISECONDS_PER_STEP);
	timingMicros.timeMultiplier = (float)AS_MILLISECONDS_PER_STEP/MILLIS_IN_A_SECOND;
	timingMicros.startThisStep = std::chrono::steady_clock::now();
	timingMicros.startLastStep = timingMicros.startThisStep;
	timingMicros.startFirstStep = timingMicros.startThisStep;
	timingMicros.endTimingAndSleep =  timingMicros.startThisStep; //for first iteration

	//Actual loop:
	do {
		prepareStep(&chopControl);
		timeOperation(timingMicros.endTimingAndSleep, &timingMicros.endPreparation,
		                                       &timingMicros.totalMicrosPreparation);
		
		step(chopControl, timingMicros.timeMultiplier);
		timeOperation(timingMicros.endPreparation, &timingMicros.endStep,
			                               &timingMicros.totalMicrosStep);

		receiveAndSendData();
		timeOperation(timingMicros.endStep, &timingMicros.endDataTransfer,
			                        &timingMicros.totalMicrosDataTransfer);

		timeAndSleep(&timingMicros);
		timeOperation(timingMicros.endDataTransfer, &timingMicros.endTimingAndSleep,
			                                &timingMicros.totalMicrosTimmingAndSleep);

		counter.showPendingIfEnoughTicksPassedAndClear(timingMicros.ticks);
	} while (*g_shouldMainLoopBeRunning_ptr);

	calculateAndPrintMainTimingInfo(timingMicros);

	g_warnings = counter.totalWarningsAlreadyDisplayed() + counter.totalWarnings();
	g_errors = counter.totalErrorsAlreadyDisplayed() + counter.totalErrors();

	uint64_t mockTick = timingMicros.ticks + 2*minimumTicksPerErrorDisplay;
	counter.showPendingIfEnoughTicksPassedAndClear(mockTick);
}

void prepareStep(AS::chopControl_st* chopControl_ptr) {
	
	int numLAs = AS::g_currentNetworkParams_ptr->numberLAs;
	int numGAs = AS::g_currentNetworkParams_ptr->numberGAs - 1; //last doesn't count

	chopControl_ptr->quantityLAs = numLAs;
	chopControl_ptr->quantityEffectiveGAs = numGAs;
	chopControl_ptr->totalPRNsNeeded = getTotalPRNsToDraw(numLAs, numGAs);
	
	bool error = AS::g_prnServer_ptr->drawPRNs(chopControl_ptr->chopIndex, 
			chopControl_ptr->totalChops, chopControl_ptr->totalPRNsNeeded).error;
	if (error) {
		AS::g_errorsCounter_ptr->incrementError(AS::errors::PS_FAILED_TO_DRAW_PRNS);
	}

	for (int i = 0; i < DRAW_WIDTH; i++) {
		AS::g_currentNetworkParams_ptr->seeds[i] = AS::g_prnServer_ptr->getSeed(i);
	}

	//How many decisions should the Agents Step calculate this tick?
	chopControl_ptr->LAdecisionsToMake = 
		               howManyDecisionsThisChop(chopControl_ptr->chopIndex, 
						          &chopControl_ptr->LAdecisionsMade, numLAs);
	chopControl_ptr->GAdecisionsToMake = 
		               howManyDecisionsThisChop(chopControl_ptr->chopIndex, 
						          &chopControl_ptr->GAdecisionsMade, numGAs);
	
	chopControl_ptr->chopIndex++;
	chopControl_ptr->chopIndex %= AS_TOTAL_CHOPS;
}

void step(AS::chopControl_st chopControl, float timeMultiplier) {
	
	int numLAs = chopControl.quantityLAs;
	int numGAs = chopControl.quantityEffectiveGAs;


	AS::stepActions(AS::g_actionSystem_ptr, numLAs, numGAs, timeMultiplier, 
		                                           AS::g_errorsCounter_ptr);

	AS::stepAgents(chopControl.LAdecisionsToMake, chopControl.GAdecisionsToMake, 
		                                      AS::g_agentDataControllerPtrs_ptr, 
											  timeMultiplier, numLAs, numGAs, 
											  AS::g_errorsCounter_ptr);
}

void receiveAndSendData() {
	if (!*AS::g_shouldMainLoopBeRunning_ptr) 
			{ LOG_TRACE("Main loop will get Client Data and send AS Data to CL"); }

		bool result = CL::getNewClientData(AS::g_currentNetworkParams_ptr, 
			                               AS::g_agentDataControllerPtrs_ptr,
						   			       AS::g_actionSystem_ptr->getDataDirectPointer(), 
			                               *AS::g_shouldMainLoopBeRunning_ptr);
		if (!result) { 
			AS::g_errorsCounter_ptr->incrementError(AS::errors::RS_FAILED_RECEIVING);
		}

		result = AS::sendReplacementDataToCL(true);
		if (!result) { 
			AS::g_errorsCounter_ptr->incrementError(AS::errors::RS_FAILED_SENDING);
		}		
}

void timeAndSleep(AS::timing_st* timing_ptr) {
	
	timing_ptr->startLastStep = timing_ptr->startThisStep;

	auto targetWakeTime = timing_ptr->startThisStep + timing_ptr->targetStepTime;
	std::chrono::microseconds threshold(MICROS_TO_BUSY_WAIT);

	auto bedTime = std::chrono::steady_clock::now();
	
	AZ::hybridBusySleep(targetWakeTime, threshold);

	//this is when next step timing starts
	timing_ptr->startThisStep = std::chrono::steady_clock::now();
	timing_ptr->ticks++;

	//update info on time slept:
	auto actualSleepTime = timing_ptr->startThisStep - bedTime;
	int64_t actualSleepTimeMicros =
		    std::chrono::duration_cast<std::chrono::microseconds>(actualSleepTime).count();
	timing_ptr->totalSleepMicros += actualSleepTimeMicros;
	if (actualSleepTimeMicros > timing_ptr->largestSleepMicros) {
		timing_ptr->largestSleepMicros = actualSleepTimeMicros;
	}
	
	//update info on time running:
	auto hotTime = bedTime - timing_ptr->startLastStep;
	int64_t hotTimeMicros = 
		        std::chrono::duration_cast<std::chrono::microseconds>(hotTime).count();
	timing_ptr->totalHotMicros += hotTimeMicros;
	if (hotTimeMicros > timing_ptr->largestHotMicros) {
		timing_ptr->largestHotMicros = hotTimeMicros;
	}

	//update info on time "snoozed":
	auto snoozed = timing_ptr->startThisStep - targetWakeTime;
	if(hotTime > timing_ptr->targetStepTime){snoozed = AS::zeroMicro;}
	int64_t snoozedMicros = 
		 std::chrono::duration_cast<std::chrono::microseconds>(snoozed).count();
	timing_ptr->totalSnoozedMicros += snoozedMicros;
	if (snoozedMicros > timing_ptr->largestSnoozeMicros) {
		timing_ptr->largestSnoozeMicros = snoozedMicros;
	}
	
	//Calculate timeMultiplier, which will be used to keep logic frequency-independent (to an extent)
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

	//Deals with pause (sleeps in cycles of half targetStepTime until unpaused)
	if(AS::g_shouldMainLoopBePaused){
		AS::g_isMainLoopBePaused = true;

		auto pauseStartTime = std::chrono::steady_clock::now();
		while (AS::g_shouldMainLoopBePaused) {
			auto pauseStepStartTime = std::chrono::steady_clock::now();
			auto targetWakeTimePause = pauseStepStartTime + (timing_ptr->targetStepTime/2);
			AZ::hybridBusySleep(targetWakeTimePause, threshold);
		}
		auto pauseEndTime = std::chrono::steady_clock::now();

		auto timePaused = 
			std::chrono::duration_cast<std::chrono::microseconds>(pauseEndTime - pauseStartTime);
		timing_ptr->timeSpentPaused += timePaused;

		timing_ptr->startThisStep = std::chrono::steady_clock::now();
	}
	AS::g_isMainLoopBePaused = false;
}

void timeOperation(std::chrono::steady_clock::time_point lastReferenceTime,
	               std::chrono::steady_clock::time_point* newReferenceTime,
	                                           int64_t* counterToIncrement) {
	
	*newReferenceTime = std::chrono::steady_clock::now();
	auto delta = *newReferenceTime - lastReferenceTime;
	*counterToIncrement += std::chrono::duration_cast<std::chrono::microseconds>(delta).count();
}

void calculateAndPrintMainTimingInfo(AS::timing_st timingMicros) {
	
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

int howManyDecisionsThisChop(int chopIndex, int* decisionsMade_ptr, int numberAgents) {
	
	if(chopIndex < 0){
		AS::g_errorsCounter_ptr->incrementError(AS::errors::DS_RECEIVED_BAD_CHOP_INDEX);
		chopIndex = 0;
	}

	int chopsRemaining = AS_TOTAL_CHOPS - chopIndex;
	if(chopsRemaining < 1){ 
		AS::g_errorsCounter_ptr->incrementWarning(AS::warnings::DS_FINISH_IN_LESS_THAN_ONE_CHOP);
		chopsRemaining = 1;
	}

	if(*decisionsMade_ptr < 0){ 
		AS::g_errorsCounter_ptr->incrementError(AS::errors::DS_NEGATIVE_DECISIONS_MADE);
		*decisionsMade_ptr = 0;
	}

	if(numberAgents <= 0){ 
		AS::g_errorsCounter_ptr->incrementError(AS::errors::DS_NEGATIVE_NUMBER_OF_AGENTS);
		numberAgents = 0;
		*decisionsMade_ptr = 0; //there are no agents, so no decisions should remain either
	}
	else { 
		*decisionsMade_ptr %= numberAgents; //if we made more decisions, this is a new cycle
	}

	int decisionsRemaining = (numberAgents - (*decisionsMade_ptr));

	int decisionsToMake = decisionsRemaining / chopsRemaining;
	if (chopsRemaining == 1) {
		decisionsToMake = decisionsRemaining;
	}

	*decisionsMade_ptr += decisionsToMake;

	return decisionsToMake;
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
			LOG_CRITICAL("Main Loop Thread is already running!");
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
	g_shouldMainLoopBePaused = false;
	g_currentNetworkParams_ptr->lastMainLoopStartingTick = 
												g_currentNetworkParams_ptr->mainLoopTicks;
	*g_mainLoopThread_ptr = std::thread(mainLoop);
	*g_mainLoopId_ptr = g_mainLoopThread_ptr->get_id();

	LOG_INFO("Started main loop on new thread");

	return true;
}

//Stops AS execution thread, marks it as stopped and clears the stored thread::id;
//Returns false if this fails or if the Main Loop found errors while running.
bool AS::stop() {
	LOG_TRACE("Stopping Main Loop Thread...");
		
	if (!*g_shouldMainLoopBeRunning_ptr) {
		LOG_ERROR("Main Loop Thread already supposed to be stopped...");
		return false;
	}

	*g_shouldMainLoopBeRunning_ptr = false;

	if (!isMainLoopRunning()) {
		LOG_ERROR("Main Loop Thread was supposed to be active, but was not!");
		return false;
	}

	if (chekIfMainLoopShouldBePaused()) {
		//Needs to unpause to end step
		unpauseMainLoop();
	}

	LOG_TRACE("Waiting for main loop to finish execution...");
	g_mainLoopThread_ptr->join();
	*g_mainLoopId_ptr = std::thread::id();
	LOG_INFO("Done.");

	#if (defined AS_DEBUG) || VERBOSE_RELEASE
		printf("\nRan for %llu ticks\n", g_currentNetworkParams_ptr->mainLoopTicks - 
								g_currentNetworkParams_ptr->lastMainLoopStartingTick);
	#endif
		
	if (g_warnings) {
		LOG_WARN("Main Loop emitte warnings...");
		#if (defined AS_DEBUG) || VERBOSE_RELEASE
			printf("\t%d warnings\n", g_warnings);
		#endif
	}

	if (g_errors) {
		LOG_ERROR("Main Loop emitted errors");
		#if (defined AS_DEBUG) || VERBOSE_RELEASE
			printf("\t%d errors\n", g_errors);
		#endif

		return false;
	}

	return true;
}

bool AS::chekIfMainLoopShouldBeRunning() {
	return *g_shouldMainLoopBeRunning_ptr;
}

bool AS::chekIfMainLoopShouldBePaused() {
	return g_shouldMainLoopBePaused;
}

bool AS::checkIfMainLoopIsPaused() {
	return g_isMainLoopBePaused;
}

void AS::pauseMainLoop() {
	g_shouldMainLoopBePaused = true;
}

void AS::unpauseMainLoop() {
	g_shouldMainLoopBePaused = false;
}

bool AS::isMainLoopRunning() {
	return g_mainLoopThread_ptr->joinable();
}

bool testAgentChopCalculation(int chopIndex, int decisionsMade, int numberAgents, bool log) {	
	int firstTestChop = chopIndex;
	int chopIndexSmallerThen = AS_TOTAL_CHOPS;

	if(chopIndex >= AS_TOTAL_CHOPS) {
		chopIndexSmallerThen = (chopIndex + 1);//run once
	} 

	int newDecisionsMade = 0;
	int decisionsSoFar = decisionsMade;
	for (int i = firstTestChop; i < chopIndexSmallerThen; i++) {
		newDecisionsMade += 
			howManyDecisionsThisChop(i, &decisionsSoFar, numberAgents);
	}

	int correctedDecisionsMade = std::max(decisionsMade, 0);
	int correctedNumberAgents = std::max(numberAgents, 0);
	if(correctedNumberAgents != 0) { correctedDecisionsMade %= correctedNumberAgents; }
	int expectedNewDecisions = std::max((correctedNumberAgents - correctedDecisionsMade), 0);

	bool result = (newDecisionsMade == expectedNewDecisions);
	result &= (decisionsSoFar == correctedNumberAgents);
	if (log && !result) {
		printf("\nError: Chop: %d, Made: %d, Agents: %d, Total: %d (expected: %d), New: %d (expected: %d)",
			chopIndex, decisionsMade, numberAgents, decisionsSoFar, correctedNumberAgents, 
			                                       newDecisionsMade, expectedNewDecisions);
	}

	return result;
}

#define TST_CHOP_CALC_TEST_CASES 5
bool AS::testMultipleAgentChopCalculations(bool log) {
	LOG_WARN("Will test calculation of how many decisions to make per chop");

	//Needs to initialize its own error counter for testing
	WarningsAndErrorsCounter counter(0, 0);
	g_errorsCounter_ptr = &counter;

	int chopIndex[TST_CHOP_CALC_TEST_CASES] = {-1, 0, 1, 5, 99999};
	int numberAgents[TST_CHOP_CALC_TEST_CASES] = {-1, 0, 1, 101, 99999};
	int decisionsMade[TST_CHOP_CALC_TEST_CASES] = {-1, 0, 1, 101, 99999};
	
	//TODO: Veery brittle and magic-numbery: generalize : )
	int expectedWarnings[(int)warnings::TOTAL] = {};
	int expectedErrors[(int)errors::TOTAL] = {};
	expectedWarnings[(int)warnings::DS_FINISH_IN_LESS_THAN_ONE_CHOP] = 25;
	expectedErrors[(int)errors::DS_RECEIVED_BAD_CHOP_INDEX] = 25;
	expectedErrors[(int)errors::DS_NEGATIVE_DECISIONS_MADE] = 25;
	expectedErrors[(int)errors::DS_NEGATIVE_NUMBER_OF_AGENTS] = 360;	

	bool result = true;
	for (int i = 0; i < TST_CHOP_CALC_TEST_CASES; i++) {
		for (int j = 0; j < TST_CHOP_CALC_TEST_CASES; j++) {
			for (int k = 0; k < TST_CHOP_CALC_TEST_CASES; k++) {
				result &= testAgentChopCalculation(chopIndex[i],decisionsMade[j],
					                                        numberAgents[k], log);
			}
		}
	}

	if (!result) {
		LOG_ERROR("Expected a different number of decisions in some conditions");
	}

	result = true;
	for (int i = 0; i < (int)warnings::TOTAL; i++) {
		result &= counter.getWarnings((warnings)i) == expectedWarnings[i];
	}
	for (int i = 0; i < (int)errors::TOTAL; i++) {
		result &= counter.getErrors((errors)i) == expectedErrors[i];
	}

	if (!result) {
		LOG_ERROR("Warnings and errors from howManyDecisionsThisChop not as expected");
		if (log) {
			counter.showPendingIfEnoughTicksPassedAndClear(10);
		}
	}

	return result;
}
