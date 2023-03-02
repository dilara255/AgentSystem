#include "miscStdHeaders.h"
#include "miscDefines.hpp"
#include "core.hpp"

#include "AS_API.hpp"
#include "CL_ExternalAPI.hpp"

#include "AS_testsAPI.hpp"

#include "timeHelpers.hpp"
#include "tests.hpp"

void testSayHello(void);
bool testMockData(void);
bool testFromTAifCLhasInitialized(void);
bool testReadingCLdataFromTA(void);
bool testChangingCLdataFromTAandRetrievingFromAS(void);
bool testReadingTickDataWhileASmainLoopRuns_start(void);
bool testReadingTickDataWhileASmainLoopRuns_end(void);
bool testSendingClientDataAndSaving(void);
bool testClientDataHAndlerInitialization(void);
bool testPause(bool printLog = false, int pauseUnpauseCycles = 5);
bool testMainLoopErrors(std::string filename);
bool testAgentsUpdating(bool printLog);

#define MINIMUM_PROPORTION_SLEEP_PASSES (0.95)

//TODO: the names of the test categories do not make sense any more
#define HELPER_FUNC_TESTS 7
#define BASIC_INIT_COMM_TESTS 4
#define SPECIFIC_DATA_FUNCTIONALITY_TESTS 9
#define SPECIFIC_THREADED_LOOP_TESTS 10
#define TOTAL_TESTS (HELPER_FUNC_TESTS+BASIC_INIT_COMM_TESTS+SPECIFIC_DATA_FUNCTIONALITY_TESTS+SPECIFIC_THREADED_LOOP_TESTS)

std::thread reader;//to test realtime reading of data trough CL as AS runs
uint64_t g_ticksRead[TST_TIMES_TO_QUERRY_TICK]; 

int main(void) {

	//TODO: review wich tests printo to console, and pass this (macro?)
	bool printSteps = false;
	#if ( (defined AS_DEBUG) || VERBOSE_RELEASE )
		printSteps = true;
	#endif

	LOG_INFO("Will first test some helper functionality:"); GETCHAR_PAUSE;

	LOG_TRACE("Drawing many PRNs, one at a time:");
	int resultsBattery0 = (int)AZ::testDraw1spcg32(); GETCHAR_PAUSE;

	LOG_TRACE("Drawing many PRNs, four at a time:");
	resultsBattery0 += (int)AZ::testDraw4spcg32s(); GETCHAR_PAUSE;

	LOG_TRACE("Will test sleeping and waking a few times...");
	double result = AZ::testHybridBusySleeping(); GETCHAR_PAUSE;
	bool passed = result > MINIMUM_PROPORTION_SLEEP_PASSES;
	if(!passed) { LOG_ERROR("Snoozed more than the maximum margin set"); }
	resultsBattery0 += passed;

	LOG_TRACE("Will test Flag Field functionality...");
	resultsBattery0 += (int)AZ::testFlagFields(printSteps); GETCHAR_PAUSE;

	resultsBattery0 += (int)AS::testActionVariationsInfo(printSteps); GETCHAR_PAUSE;

	resultsBattery0 += (int)AS::testMultipleAgentChopCalculations(printSteps); GETCHAR_PAUSE;

	resultsBattery0 += (int)AS::testWarningAndErrorCountingAndDisplaying(printSteps); GETCHAR_PAUSE;

	if (resultsBattery0 != HELPER_FUNC_TESTS) {
		LOG_CRITICAL("Not all of these tests passed:");
		printf("%d out of %d failed", HELPER_FUNC_TESTS - resultsBattery0, HELPER_FUNC_TESTS);
		GETCHAR_PAUSE;
	}
	else {
		LOG_INFO("All of these tests passed!"); GETCHAR_PAUSE;
	}

	testSayHello();

	LOG_INFO("AS, CL-internal and CL-external should have said hello above : )");
	LOG_TRACE("This will run a few batteries of tests..."); GETCHAR_PAUSE;

	LOG_INFO("Basic App, AS and CL communicaton and data storage tests:\n",1); GETCHAR_PAUSE;
	int resultsBattery1 = (int)testMockData(); GETCHAR_PAUSE;
	
	LOG_TRACE("Actual initialization tests...");

	resultsBattery1 += (int)AS::initializeASandCL(); GETCHAR_PAUSE;

	resultsBattery1 += (int)AS::testContainersAndAgentObjectCreation(); GETCHAR_PAUSE;

	resultsBattery1 += (int)testFromTAifCLhasInitialized(); GETCHAR_PAUSE;

	if (resultsBattery1 != BASIC_INIT_COMM_TESTS) {
		LOG_CRITICAL("Not all of these tests passed:");
		printf("%d out of %d failed", BASIC_INIT_COMM_TESTS - resultsBattery1, BASIC_INIT_COMM_TESTS);
		GETCHAR_PAUSE;
	}
	else {
		LOG_INFO("All of these tests passed!"); GETCHAR_PAUSE;
	}

	LOG_INFO("Specific functionality tests (DATA manipulation):\n",1); GETCHAR_PAUSE;
	
	int resultsBattery2 = (int)AS::testFileCreation(fileNameNoDefaults, fileNameWithDefaults); 
	GETCHAR_PAUSE;

	LOG_WARN("Will test loading network from a File and then saving it back with network name");
	resultsBattery2 += (int)AS::loadNetworkFromFile(fileNameWithDefaults); GETCHAR_PAUSE;

	resultsBattery2 += (int)AS::saveNetworkToFile(); GETCHAR_PAUSE;

	resultsBattery2 += (int)AS::testDataTransferFromAStoCL(); GETCHAR_PAUSE;
	
	resultsBattery2 += (int)testReadingCLdataFromTA(); GETCHAR_PAUSE;

	resultsBattery2 += (int)testChangingCLdataFromTAandRetrievingFromAS(); GETCHAR_PAUSE;

	//TODO: why does this take a while to start? Other saves are fast
	//Seems to only be the case in debug. Idk, really weird
	resultsBattery2 += (int)AS::saveNetworkToFile(customFilename, true); GETCHAR_PAUSE;

	resultsBattery2 += (int)AS::testNeighbourIDsetting(); GETCHAR_PAUSE;

	resultsBattery2 += (int)AS::testChoppedPRNdrawing(printSteps, true); GETCHAR_PAUSE;

	if (resultsBattery2 != SPECIFIC_DATA_FUNCTIONALITY_TESTS) {
		LOG_CRITICAL("Not all of these tests passed:");
		printf("%d out of %d failed", SPECIFIC_DATA_FUNCTIONALITY_TESTS - resultsBattery2, 
			                                             SPECIFIC_DATA_FUNCTIONALITY_TESTS);
		GETCHAR_PAUSE;
	}
	else {
		LOG_INFO("All of these tests passed!"); GETCHAR_PAUSE;
	}
	
	LOG_INFO("Specific functionality tests (with threaded LOOPs):\n",1); GETCHAR_PAUSE;

	int resultsBattery3 = (int)testMainLoopErrors(customFilename); GETCHAR_PAUSE;

	LOG_WARN("Will re-load the previously modified network for further testing");
	resultsBattery3 += (int)AS::loadNetworkFromFile(customFilename, true); GETCHAR_PAUSE;

	resultsBattery3 += (int)testReadingTickDataWhileASmainLoopRuns_start(); GETCHAR_PAUSE;
	
	resultsBattery3 += (int)testReadingTickDataWhileASmainLoopRuns_end(); GETCHAR_PAUSE;

	resultsBattery3 += (int)AS::saveNetworkToFile(customFilename, true); GETCHAR_PAUSE;

	//Also saves another network, with steps, and resumes it:
	resultsBattery3 += (int)testClientDataHAndlerInitialization(); GETCHAR_PAUSE;

	resultsBattery3 += (int)testSendingClientDataAndSaving(); GETCHAR_PAUSE;

	resultsBattery3 += (int)testPause(printSteps); GETCHAR_PAUSE;

	resultsBattery3 += (int)AS::quit(); GETCHAR_PAUSE;

	resultsBattery3 += (int)testAgentsUpdating(printSteps); GETCHAR_PAUSE;

	if (resultsBattery3 != SPECIFIC_THREADED_LOOP_TESTS) {
		LOG_CRITICAL("Not all of these tests passed:");
		printf("%d out of %d failed", SPECIFIC_THREADED_LOOP_TESTS - resultsBattery3,
			SPECIFIC_THREADED_LOOP_TESTS);
		GETCHAR_PAUSE;
	}
	else {
		LOG_INFO("All of these tests passed!"); GETCHAR_PAUSE;
	}

	LOG_TRACE("Tests ended...\n",1); GETCHAR_PAUSE;

	int totalPassed = resultsBattery0 + resultsBattery1 + resultsBattery2 + resultsBattery3;
	if (totalPassed == TOTAL_TESTS) {
		LOG_INFO("All automatically checked tests passed!"); GETCHAR_PAUSE;
	}
	else {
		LOG_CRITICAL("Not all tests were passed (as far as we checked)!");
		printf("%d out of %d failed\n", TOTAL_TESTS - totalPassed, TOTAL_TESTS);
		GETCHAR_PAUSE;
	}

	//TODO: Update expected changes after I'm done updating it
	LOG_WARN("Check that you have (at least): one network file with format specifiers,\none with default values and one with modified values:");
	printf("\t-The one with specifiers is %s\n\t-%s should have the defaults\n\t-%s received modifications from TA\n",
		                              fileNameNoDefaults, fileNameWithDefaults, customFilename);
	printf("\nThe modified file has different data:\n\t-The comment's first letter should be a %c;\n\t-Ticks should be the number of times mainLoopTrhead looped before last save;\n",TST_COMMENT_LETTER_CHANGE);
	printf("\t-Last GA`s id = %d and connected GAs = %d ;\n", TST_GA_ID, TST_GA_CONNECTIONS);
	printf("\t-Last LA`s reinforcement = %f, offset[%d][%d][1] = %f and last actions aux = %f.\n",
		TST_LA_REINFORCEMENT, TST_CHANGED_CATEGORY, TST_CHANGED_MODE, TST_LA_OFFSET, TST_LAST_ACTION_AUX);
	
	GETCHAR_FORCE_PAUSE;

	LOG_INFO("Done! Enter to exit"); GETCHAR_FORCE_PAUSE;

	return (1 + (totalPassed - TOTAL_TESTS));
}

//TODO-CRITICAL: this test needs to somehow guarantee no actions change the incomes and etc
//UPDATE when actions are implemented
//TODO: make a more interesting test-case using clientDataHandler (include war/attrition)
bool testAgentsUpdating(bool print) {
	LOG_WARN("Will load a network, run for several ticks, stop it, and check agent updating");
	
	//Load, but don't start, so we can calculate and set test conditions:
	bool result = AS::loadNetworkFromFile(updateTestFilename, false);
	if (!result) {
		LOG_ERROR("Failed to load network. Aborting test");
		return false;
	}

	//Let's change some values for the test. First and Last LA and GA will be tested.
	int quantityGAs = CL::ASmirrorData_cptr->networkParams.numberGAs;
	quantityGAs--; //last doesn't count
	if (quantityGAs < 2) {
		LOG_ERROR("Too few GAs for this test. Will abort");
		return false;
	}

	int quantityLAs = CL::ASmirrorData_cptr->networkParams.numberLAs;
	if (quantityLAs < 2) {
		LOG_ERROR("Too few LAs for this test. Will abort");
		return false;
	}

	auto clientData_ptr = CL::getClientDataHandlerPtr();
	if (clientData_ptr == NULL) {
		LOG_ERROR("Couldn't get client data handler. Will abort test");
		return false;
	}

	AS::GAflagField_t neighboursFirstGA;
	uint32_t flagField = (uint32_t)pow(2,quantityGAs) - 1; //All on
	flagField--; //turn off first, since this will be used for the first GA
	neighboursFirstGA.loadField(flagField);

	AS::GAflagField_t neighboursLastGA; //empty

	clientData_ptr->GAstate.changeConnectedGAs(0, &neighboursFirstGA);
	clientData_ptr->GAstate.changeConnectedGAs(quantityGAs-1, &neighboursLastGA);

	float externalGuardFirstLA = 182;
	float externalGuardLastLA = 179;
	float startingResourcesFirstLA = 0;
	float startingResourcesLastLA = -10;

	clientData_ptr->LAstate.parameters.strenght.changeGuard(0, 182);
	clientData_ptr->LAstate.parameters.resources.changeCurrentTo(0, 0);

	clientData_ptr->LAstate.parameters.strenght.changeGuard(quantityLAs - 1, 179);
	clientData_ptr->LAstate.parameters.resources.changeCurrentTo(quantityLAs - 1, -10);

	auto laState_ptr = &CL::ASmirrorData_cptr->agentMirrorPtrs.LAstate_ptr->data;
	
	float incomeFirstLA = laState_ptr->at(0).parameters.resources.updateRate;
	float incomeLastLA = laState_ptr->at(quantityLAs - 1).parameters.resources.updateRate;
	float strenghtFirstLA = laState_ptr->at(0).parameters.strenght.current;
	float strenghtLastLA = laState_ptr->at(quantityLAs - 1).parameters.strenght.current;
	float thresholdFirstLA = laState_ptr->at(0).parameters.strenght.thresholdToCostUpkeep;
	float thresholdLastLA = laState_ptr->at(quantityLAs - 1).parameters.strenght.thresholdToCostUpkeep;

	auto gaState_ptr = &CL::ASmirrorData_cptr->agentMirrorPtrs.GAstate_ptr->data;
	
	float startingResourcesFirstGA = gaState_ptr->at(0).parameters.GAresources;
	float startingResourcesLastGA = gaState_ptr->at(quantityGAs - 1).parameters.GAresources;
	
	//How long to wait for test to run?
	int millisToRun = A_THOUSAND;
	int targetTicksToRun = millisToRun/AS_MILLISECONDS_PER_STEP;
	int microsToBusyWait = 50;
	//Should try to sleep "target - 1" ticks, as the pause only happens at the end of the tick
	int sleepMicros = (targetTicksToRun-1)*AS_MILLISECONDS_PER_STEP*MICROS_IN_A_MILLI;
	
	std::chrono::microseconds sleepTime(sleepMicros);
	std::chrono::microseconds threshold(A_THOUSAND);

	//Now we can run the test:
	result = AS::run();

	AZ::hybridBusySleepForMicros(sleepTime, threshold);

	AS::pauseMainLoop(); //we don't stop/quit, so the info is still available
	bool paused = false; //pause is NOT instantaneous
	while(!paused){
		paused = AS::checkIfMainLoopIsPaused();
	}

	if (!result) { //we didn't check earlier to try and keep things in synch
		LOG_ERROR("Failed to run test network. Aborting test");
		return false;
	}
	
	//How long did it actually run?
	uint64_t initialTick =  CL::ASmirrorData_cptr->networkParams.lastMainLoopStartingTick;
	uint64_t finalTick =  CL::ASmirrorData_cptr->networkParams.mainLoopTicks;							
	uint64_t ticksRan = finalTick - initialTick;

	//When you pause, timing info from the last step is not yet sent, although step is, so:
	ticksRan++;
	double totalMultiplier = 
				CL::ASmirrorData_cptr->networkParams.accumulatedMultiplier;
	//for the same reason:
	double adjustedTotalMultiplier = totalMultiplier*((double)ticksRan/(ticksRan - 1)); 

	float absoluteDifference = fabs(millisToRun - (ticksRan * AS_MILLISECONDS_PER_STEP));
	float propotionalAbsoluteDifference = absoluteDifference/millisToRun;
	float graceFactor = 0.015;
	if (propotionalAbsoluteDifference > graceFactor) {
		LOG_WARN("Didn't run the expected time/ticks. Will proceed, but may fail");
	}

	//Did anything change which shouldn't have changed?
	if (gaState_ptr->at(0).connectedGAs.getField() != neighboursFirstGA.getField()) {
		LOG_ERROR("Unexpected changes happened, maybe through actions or decisions. Will fail.");
		return false;
	}

	if (gaState_ptr->at(quantityGAs - 1).connectedGAs.getField() != neighboursLastGA.getField()) {
		LOG_ERROR("Unexpected changes happened, maybe through actions or decisions. Will fail.");
		return false;
	}

	bool changed = laState_ptr->at(0).parameters.strenght.externalGuard != externalGuardFirstLA;
	changed |= laState_ptr->at(quantityLAs - 1).parameters.strenght.externalGuard != externalGuardLastLA;
	changed |= incomeFirstLA !=  laState_ptr->at(0).parameters.resources.updateRate;
	changed |= incomeLastLA != laState_ptr->at(quantityLAs - 1).parameters.resources.updateRate;
	changed |= strenghtFirstLA !=  laState_ptr->at(0).parameters.strenght.current;
	changed |= strenghtLastLA != laState_ptr->at(quantityLAs - 1).parameters.strenght.current;
	changed |= thresholdFirstLA !=  laState_ptr->at(0).parameters.strenght.thresholdToCostUpkeep;
	changed |= thresholdLastLA != laState_ptr->at(quantityLAs - 1).parameters.strenght.thresholdToCostUpkeep;

	if (changed) {
		LOG_ERROR("Unexpected changes happened, maybe through actions or decisions. Will fail.");
		return false;
	}

	//TODO: Check that diplomatic relations haven't changed either
		
	//Finally, we check the results:	
	float incomeFirstGA = 0;
	float incomeLastGA = 0;
	float expectedTradeLastGA = 0;
	float expectedTradeFirstGA = 0;
	float expectedTotalResourcesFirstGA = 0;
	float expectedTotalResourcesLastGA = 0;
	
	float epsGA = 0;

	result &= (fabs(expectedTotalResourcesFirstGA -  gaState_ptr->at(0).parameters.GAresources) <= epsGA);
	result &= (fabs(expectedTotalResourcesLastGA - gaState_ptr->at(quantityGAs - 1).parameters.GAresources) <= epsGA);

	float expectedTradeFirstLA = 0;
	float expectedTradeLastLA = 0;
	float expectedTotalResourcesFirstLA = 0;
	float expectedTotalResourcesLastLA = 0;

	float epsLA = 0;
	
	result &= (fabs(expectedTotalResourcesFirstLA - laState_ptr->at(0).parameters.resources.current) <= epsLA);
	result &= (fabs(expectedTotalResourcesLastLA - laState_ptr->at(quantityLAs - 1).parameters.resources.current) <= epsLA);
	
	//And then save and show reults and wrap things up:
	bool couldSave = AS::saveNetworkToFile(updateTestOutputFilename, true, false, true);

	if (print) {
		printf("Ran for %llu ticks, total multiplier: %f (adj: %f)\n", 
			       ticksRan, totalMultiplier, adjustedTotalMultiplier);
		printf("LA %d: curr: %f (starting: %f, diff: %f, income: %f, expected trade: %f);\n\tstr: %f (guard: %f, thresh: %f)\n",
			0,  laState_ptr->at(0).parameters.resources.current, startingResourcesFirstLA,
			((double)laState_ptr->at(0).parameters.resources.current - startingResourcesFirstLA),
			laState_ptr->at(0).parameters.resources.updateRate, expectedTradeFirstLA,
			laState_ptr->at(0).parameters.strenght.current, 
			laState_ptr->at(0).parameters.strenght.externalGuard,
			laState_ptr->at(0).parameters.strenght.thresholdToCostUpkeep);
		printf("LA %d: curr: %f (starting: %f, diff: %f, income: %f, expected trade: %f);\n\tstr: %f (guard: %f, thresh: %f)\n",
			(quantityLAs - 1),  laState_ptr->at(quantityLAs - 1).parameters.resources.current, startingResourcesLastLA,
			((double)laState_ptr->at(quantityLAs - 1).parameters.resources.current - startingResourcesLastLA),
			laState_ptr->at(quantityLAs - 1).parameters.resources.updateRate, expectedTradeLastLA,
			laState_ptr->at(quantityLAs - 1).parameters.strenght.current, 
			laState_ptr->at(quantityLAs - 1).parameters.strenght.externalGuard,
			laState_ptr->at(quantityLAs - 1).parameters.strenght.thresholdToCostUpkeep);
		printf("\nGA %d: curr: %f (starting: %f, diff: %f, income: %f, expected trade: %f)\n",
			0, gaState_ptr->at(0).parameters.GAresources, startingResourcesFirstGA,
			((double)gaState_ptr->at(0).parameters.GAresources - startingResourcesFirstGA),
			incomeFirstGA, expectedTradeFirstGA);
		printf("GA %d: curr: %f (starting: %f, diff: %f, income: %f, expected trade: %f)\n",
			(quantityGAs - 1), gaState_ptr->at(quantityGAs - 1).parameters.GAresources, startingResourcesLastGA,
			((double)gaState_ptr->at(quantityGAs - 1).parameters.GAresources - startingResourcesLastGA),
			incomeLastGA, expectedTradeLastGA);
	}

	if (!result) {
		LOG_ERROR("Final results don't match expected. Test failed");
	}

	bool aux = AS::quit();
	if (!aux) {
		LOG_ERROR("Failed to quit test network or main loop found errors. Aborting test");
		return false;
	}

	if (!couldSave) {
		LOG_ERROR("Saving of updated network failed. Test will fail.");
		return false;
	}

	if(result) { LOG_TRACE("Test passed!"); }
	return result;
}

bool testMainLoopErrors(std::string filename) {
	LOG_WARN("Will load a network, run for several ticks, stop it, and check for errors");
	
	bool result = AS::loadNetworkFromFile(filename, true);
	if (!result) {
		LOG_ERROR("Failed to Load test network. Aborting test");
		return false;
	}

	int ticks = 25;
	std::chrono::microseconds microsToSleep(ticks*AS_MILLISECONDS_PER_STEP*MICROS_IN_A_MILLI);
	AZ::hybridBusySleepForMicros(microsToSleep);

	return AS::quit();
}

bool testPause(bool printLog, int pauseUnpauseCycles) {
	LOG_WARN("Will test pausing and resuming the main loop");

	for(int i = 0; i < pauseUnpauseCycles; i++){
		const CL::mirror_t* mp = CL::ASmirrorData_cptr;
		if (mp == NULL) {
			LOG_ERROR("Failed to acquire const pointer to mirror Data");
			return false;
		}

		uint64_t ticksBeforePause = mp->networkParams.mainLoopTicks;

		AS::pauseMainLoop();
		auto pauseIssued = std::chrono::steady_clock::now();
		if (!AS::chekIfMainLoopShouldBePaused()) {
			LOG_ERROR("Pause Command not issued to AS");
			return false;
		}

		int pauseGraceFactor = 2; //give ample time to pause
		bool timedOut = false;

		while (!AS::checkIfMainLoopIsPaused() && !timedOut) {

			std::chrono::nanoseconds waitedFor = std::chrono::steady_clock::now() - pauseIssued;
			int millisWaited = (int)(((double)waitedFor.count()/NANOS_IN_A_SECOND)
				                                              *MILLIS_IN_A_SECOND);

			bool timedOut = (millisWaited > (pauseGraceFactor*AS_MILLISECONDS_PER_STEP));
		}
		if (timedOut) {
			LOG_ERROR("Timed out waiting for AS to actually pause");
			return false;
		}

		auto actualPause = std::chrono::steady_clock::now();
	
		uint64_t ticksAfterPause = mp->networkParams.mainLoopTicks;
		if ((ticksAfterPause - ticksBeforePause) > 1) {
			LOG_ERROR("Stepped more than one tick before pausing");
			return false;
		}

		std::chrono::microseconds ASstepTime = 
				std::chrono::duration_cast<std::chrono::microseconds>(
						std::chrono::milliseconds(AS_MILLISECONDS_PER_STEP));

		int periodsToWaitOnPause = 10;
		AZ::hybridBusySleepForMicros(periodsToWaitOnPause*ASstepTime); 
	
		uint64_t ticksAfterWait =  mp->networkParams.mainLoopTicks;
		if (ticksAfterWait != ticksAfterPause) {
			LOG_ERROR("AS kept running while should be paused");
			return false;
		}

		AS::unpauseMainLoop();
		auto unpauseIssued = std::chrono::steady_clock::now();
		if (AS::chekIfMainLoopShouldBePaused()) {
			LOG_ERROR("Unpause Command not issued to AS");
			return false;
		}

		while (AS::checkIfMainLoopIsPaused() && !timedOut) {

			std::chrono::nanoseconds waitedFor = std::chrono::steady_clock::now() - unpauseIssued;
			int millisWaited = (int)(((double)waitedFor.count()/NANOS_IN_A_SECOND)
				                                              *MILLIS_IN_A_SECOND);

			bool timedOut = (millisWaited > (pauseGraceFactor*AS_MILLISECONDS_PER_STEP));
		}
		if (timedOut) {
			LOG_ERROR("Timed out waiting for AS to actually unpause");
			return false;
		}

		auto actualUnpause = std::chrono::steady_clock::now();

		int periodsToWaitAfterUnpause = 10;
		AZ::hybridBusySleepForMicros(periodsToWaitAfterUnpause*ASstepTime); 

		uint64_t ticksNow = mp->networkParams.mainLoopTicks;
		if (ticksNow == ticksAfterPause) {
			LOG_ERROR("AS isn't ticking after unpausing");
			return false;
		}
		else if ((ticksNow - ticksAfterPause) < (periodsToWaitAfterUnpause/pauseGraceFactor)) {
			LOG_ERROR("AS is ticking too slow after unpausing");
			return false;
		}

		if (printLog) {
			auto timeToPause = actualPause - pauseIssued;
			auto timeToUnpause = actualUnpause - unpauseIssued;
			float millisToPause = ((float)timeToPause.count()/NANOS_IN_A_SECOND)
				                                                 *MILLIS_IN_A_SECOND;
			float millisToUnpause = ((float)timeToUnpause.count()/NANOS_IN_A_SECOND)
				                                                     *MILLIS_IN_A_SECOND;

			float AScyclesToPause = millisToPause/AS_MILLISECONDS_PER_STEP;
			float AScyclesToUnpause = millisToUnpause/AS_MILLISECONDS_PER_STEP;

			float nanosPerMicro = NANOS_IN_A_SECOND/MICROS_IN_A_SECOND;
			printf("AS cycles to pause: %f (%f micros), to unpause: %f (%f micros)\n", 
					AScyclesToPause, timeToPause.count()/nanosPerMicro, AScyclesToUnpause,
					timeToUnpause.count()/nanosPerMicro);
		}
	}

	LOG_INFO("Pause test passed");

	return true;
}

bool testSendingClientDataAndSaving(void) {
	LOG_WARN("Will try to send data through CL's Client Data Handler");

	uint32_t id = CL::ASmirrorData_cptr->networkParams.numberLAs - 1;
	
	LOG_TRACE("Calculated last LA's ID from AS mirror data");

	auto cdHandler_ptr = CL::getClientDataHandlerPtr();

	if (cdHandler_ptr == NULL) {
		LOG_ERROR("A pointer to the Client Data Handler has to be acquired before this test");
		return false;
	}

	bool result = cdHandler_ptr->LAstate.parameters.resources.changeCurrentTo(id,
		(float)TST_RES_CHANGE);

	if (!result) {
		LOG_ERROR("Test failed issuing change!");
		return false;
	}

	LOG_INFO("Change issued. Will save. Check that the last LA's current resources are changed");

	std::string fileName = std::string("ClientDataHandler_") + customFilename;
	if (!AS::saveNetworkToFile(fileName, false)) {
		LOG_ERROR("Failed to save network. Test will pass, but fix this to actually check changes");
	}
	else {
		LOG_INFO("Network saved to ClientDataHandler_*customFilename*");
	}

	return true;
}

bool testClientDataHAndlerInitialization(void) {
	LOG_WARN("Will test ClientDataHandler acquisition");

	auto cdHandler_ptr = CL::getClientDataHandlerPtr();

	if (cdHandler_ptr == NULL) {
		LOG_ERROR("Test failed getting client data handler!");
		return false;
	}
	LOG_TRACE("Got Client Data Handler");

	if (!cdHandler_ptr->hasInitialized()) {
		LOG_ERROR("Client Data Handler is not initialized!");
		return false;
	}

	LOG_INFO("Acquired a Client Data Handler which is marked as initialized");

	return true;
}

void TAreadLoop(int numberTicks) {

	g_ticksRead[0] = CL::ASmirrorData_cptr->networkParams.mainLoopTicks;
	
	auto sleepTime = std::chrono::milliseconds(TST_TA_QUERY_FREQUENCY_MS);
	for (int i = 1; i < numberTicks; i++) {
		
		AZ::hybridBusySleepForMicros(sleepTime);

		g_ticksRead[i] = CL::ASmirrorData_cptr->networkParams.mainLoopTicks;
	}
}

//TODO: fairly brittle - is there a (reasonable) way to make this more robust?
//TODO: This test makes more sense if we store tick time as well : )
//WARNING: test breaks for large values (about an order of magnitude bellow uin64_t maximum)
//WARNING: this test ONLY makes sense if the loop actually takes around the expected time
//(meaning very low step times make this mor brittle, especially when repetions are higher)
//WARNING: this is fairly brittle, check actual results on failing
bool testReadingTickDataWhileASmainLoopRuns_end(void) {

	LOG_WARN("Will check if reader thread's results are as expected. May need to wait for execution to finish");
	
	bool result = false;
	if (reader.joinable()) {
		reader.join();
		result = AS::testMainLoopStopAndRestart();
	}
	else {
		LOG_WARN("Reader thread seem innactive. If it wasn't created, this test will make no sense!");
	}

	if(!result) {
		return false;
	}

	LOG_TRACE("Execution finished. Checking...");

	//Whole part of the multiplier gives a base for the pace of the count
	uint64_t baseKickUpdateRate = (uint64_t)abs(TST_TA_QUERY_MULTIPLIER);
	//Remainder will help calculate how often we should diverge from that (jump)
	double remainder = TST_TA_QUERY_MULTIPLIER - fabs(TST_TA_QUERY_MULTIPLIER);
	
	double finalRemainder;
	if(remainder == 0) { finalRemainder = 0; }
	else{
		double inverseRemainder = 1.0 / remainder;

		//If the inverse of the remainder has a remainder, then you may skip some jumps;
		finalRemainder = inverseRemainder - fabs(inverseRemainder);
	}

	//Which makes testing harder, so I'll try to control for that:
	double refRemainder = (double)baseKickUpdateRate / (double)TST_TIMES_TO_QUERRY_TICK;
	//this would make skipJumpPerior =~ TST_TIMES_TO_QUERRY_TICK, and let's me define:
	double small = refRemainder / TST_TICK_COUNT_SAFETY_FACTOR;

	bool exact;
	if (finalRemainder <= small) exact = true;
	else exact = false;
	if (!exact) {
		LOG_WARN("Fractional relationship between read and write may make the test's auto-checking fail!");
		#if (defined AS_DEBUG) || VERBOSE_RELEASE
			printf("Final Remainder: %f, small: %f\n", finalRemainder, small);
		#endif // AS_DEBUG

	}

	//Now we "simulate" the ticking, giving a first error for free, for adjustment

	uint64_t expectedTicks[TST_TIMES_TO_QUERRY_TICK];
	double expectedTicksFrac[TST_TIMES_TO_QUERRY_TICK];
	expectedTicks[0] = g_ticksRead[0];
	expectedTicksFrac[0] = (double)g_ticksRead[0];

	//We will give two "chances": first, an initial jump, and then a fixed time grace
	bool firstJumpFound = false;
	bool differenceCalculated = false;
	int fails = 0;
	bool mayFail = false;
	int64_t difference = 0;
	int jumpIndex = -1;
	for (int i = 1; i < TST_TIMES_TO_QUERRY_TICK; i++) {
		
		//TODO: this may fail if starting tick is huge (because of float stuff)
		expectedTicksFrac[i] = expectedTicksFrac[i - 1] + TST_TA_QUERY_MULTIPLIER;
		expectedTicks[i] = (uint64_t)abs(expectedTicksFrac[i]);

		if ((!firstJumpFound) && (g_ticksRead[i] != expectedTicks[i])) {
			expectedTicksFrac[i] = (double)g_ticksRead[i];

			printf("tick: %d: First jump and different! read: %llu, expected: %llu, newFrac: %f\n",
							i, g_ticksRead[i], expectedTicks[i], expectedTicksFrac[i]);

			firstJumpFound = true;
			jumpIndex = i;
		}
		else if (g_ticksRead[i] != expectedTicks[i]){
			
			int64_t absDifference = (int64_t)g_ticksRead[i] - (int64_t)expectedTicks[i];
			absDifference = abs(absDifference);

			if (!differenceCalculated) {
				difference =  g_ticksRead[i] - expectedTicks[i];
				differenceCalculated = true;
			}                           //not actually keeping pace
			else if (absDifference > abs(difference)) {
				if(!mayFail){ 
					//makes the second chance meaningful:
					difference =  g_ticksRead[i] - expectedTicks[i];
					mayFail = true; 
				}
				else { fails++; }
			}
			else { mayFail = false; }
		}
		else { mayFail = false; }

		//so the second chance is actually meaningful

		printf("tk: %d: Difference: %lld, fails: %d, may: %d, expected: %llu, exp+dif: %llu, read: %llu\n", 
				            i, difference, fails, (int)mayFail, 
				               expectedTicks[i], (expectedTicks[i] + difference),  g_ticksRead[i]);
	}

	//This gives some margin of error since the fractional part may still byte us in the ass
	//(in practice this may well be effectively the same as fails == zero)
	result = (fails < TST_TIMES_TO_QUERRY_TICK / TST_TICK_COUNT_SAFETY_FACTOR);

	if (result) {
		LOG_INFO("TA read ticks correctly from CL while AS was running");
	}
	else {
		LOG_ERROR("TA didn't read ticks correctly from CL while AS was running");
	}

	printf("1 + %d ticks read at interval %d ms (main loop freq: %d ms, jumpIndex: %d):\n",
			                       TST_TIMES_TO_QUERRY_TICK - 1, TST_TA_QUERY_FREQUENCY_MS, 
		                                             TST_MAINLOOP_FREQUENCY_MS, jumpIndex);
	
#if (defined AS_DEBUG) || VERBOSE_RELEASE
	int maxPrint = 10;
	if (TST_TIMES_TO_QUERRY_TICK < maxPrint) {
		maxPrint = maxPrint;
	}
	for (int i = 0; i < maxPrint; i++) {
		printf("%d: %llu (%llu) - ", i, g_ticksRead[i], expectedTicks[i]);
	}
#endif

	return result;
}

bool testReadingTickDataWhileASmainLoopRuns_start(void) {

	LOG_WARN("Will spawn a new thread which will try to read a few times from CL the number of ticks while AS's main loop runs");

	if (!AS::isMainLoopRunning() || !AS::chekIfMainLoopShouldBeRunning()) {
		LOG_CRITICAL("Main loop has to be running for this test to work");
		return false;
	}

	if (!CL::isASdataPointerInitialized()) {
		LOG_CRITICAL("This test needs the AS mirror in CL to be initialized");
		return false;
	}

	for (int i = 0; i < TST_TIMES_TO_QUERRY_TICK; i++) {
		g_ticksRead[i] = 0;
	}

	reader = std::thread(TAreadLoop, TST_TIMES_TO_QUERRY_TICK);

	LOG_INFO("Thread created, another test will check back on the results in a while...");
	return true;
}

//Changes: Comment, GAid, GAconn, Guard, LA GA offsets, aux
bool testChangingCLdataFromTAandRetrievingFromAS(void) {
	
	LOG_WARN("TA will try to change some data in CL so AS can check it...");
	
	LOG_TRACE("Will acquire pointers to the DATA on AS and CL");

	const CL::mirror_t* mp = CL::ASmirrorData_cptr;
	if (mp == NULL) {
		LOG_ERROR("const AS Data pointer returned null. Aborting test.");
		return false;
	}

	CL::ClientData::Handler* cp = CL::getClientDataHandlerPtr();
	if (cp == NULL) {
		LOG_ERROR("Client Data Handler pointer returned null. Aborting test.");
		return false;
	}

	LOG_TRACE("Pointers acquired and not NULL");
	
	std::string newComment;
	newComment = std::string(mp->networkParams.comment);
	newComment[0] = TST_COMMENT_LETTER_CHANGE;

	LOG_TRACE("Will issue new network comment");

	bool result = cp->networkParameters.changeCommentTo(0, newComment.c_str());
	if (!result) {
		LOG_ERROR("Error issuing comment change. Aborting test.");
		return false;
	}
	
	//only at most maxGA - 1 have data (last is reserved for "no GA", so:
	uint32_t lastGA = mp->networkParams.numberGAs - 1 - 1; 

	LOG_TRACE("Will issue new ID for last GA");

	result = cp->GAcold.changeID(lastGA, TST_GA_ID);
	if (!result) {
		LOG_ERROR("Error issuing GA id. change Aborting test.");
		return false;
	}

	AS::GAflagField_t newGAconnectionField;
	newGAconnectionField.loadField(TST_GA_CONNECTIONS);

	LOG_TRACE("Will issue new GA Connections for last GA");

	result = cp->GAstate.changeConnectedGAs(lastGA, &newGAconnectionField);
	if (!result) {
		LOG_ERROR("Error issuing Connected GAs change. Aborting test.");
		return false;
	}

	uint32_t lastLA = mp->networkParams.numberLAs - 1;

	LOG_TRACE("Will issue new Guard for last LA");

	result = cp->LAstate.parameters.strenght.changeGuard(lastLA ,TST_LA_REINFORCEMENT);
	if (!result) {
		LOG_ERROR("Error issuing Guard change. Aborting test.");
		return false;
	}

	LA::decisionData_t currentData = mp->agentMirrorPtrs.LAdecision_ptr->data.at(lastLA);
	AS::LAdecisionOffsets_t newOffsets;
	
	LOG_TRACE("Will copy AS's current LA decision offset data...");

	for (int i = 0; i < (int)AS::actCategories::TOTAL; i++) {
		for (int j = 0; j < (int)AS::actModes::TOTAL; j++) {

			newOffsets[i][j] = currentData.offsets.incentivesAndConstraintsFromGA[i][j];
		}
	}
	newOffsets[TST_CHANGED_CATEGORY][TST_CHANGED_MODE]  = (float)TST_LA_OFFSET;

	LOG_TRACE("And issue a change");

	result = cp->LAdecision.personality.changeGAoffsets(lastLA, &newOffsets);
	if (!result) {
		LOG_ERROR("Error issuing LA's GA offsets change. Aborting test.");
		return false;
	}

	int maxActionsPerAgent = mp->networkParams.maxActions;

	LOG_TRACE("Will issue new Aux to last LA's last action");

	cp->LAaction.details.changeAuxTo(false, lastLA, maxActionsPerAgent - 1, 
		                                               TST_LAST_ACTION_AUX);

	LOG_INFO("All test changes issued...");

	return AS::testGotNewValuesFromASthroughCL();
}

bool testReadingCLdataFromTA(void) {

	LOG_WARN("TA will try to read data from CL...");

	bool result = CL::ASmirrorData_cptr->agentMirrorPtrs.haveData;
	result &= CL::ASmirrorData_cptr->networkParams.isNetworkInitialized;
	result &= CL::ASmirrorData_cptr->actionMirror.hasData();
	if (!result) {
		LOG_ERROR("CL Mirror data controllers say they have no data!");
		return false;
	}
	LOG_TRACE("Data Flags ok. Will try to read data...");

	result = (CL::ASmirrorData_cptr->networkParams.numberGAs == TST_NUMBER_GAS);
	result &= (CL::ASmirrorData_cptr->networkParams.numberLAs == TST_NUMBER_LAS);
	if (!result) {
		LOG_ERROR("Agent amounts were not as expected according to CL. Aborting test.");
		return false;
	}

	//strcmp returns 0 on perfect match
	if (strcmp(CL::ASmirrorData_cptr->networkParams.name, fileNameWithDefaults) != 0) {
		LOG_ERROR("Network name doesn't match expected.");
		result = false;
	}
	
	AS::actionData_t firstGAaction =
		CL::ASmirrorData_cptr->actionMirror.dataGAs[0];
	AS::actionData_t lastLAaction =
		CL::ASmirrorData_cptr->actionMirror.dataLAs[(TST_NUMBER_LAS * MAX_ACTIONS_PER_AGENT) - 1];

	bool resultAux = (firstGAaction.ticks.initial == DEFAULT_FIRST_TICK);
	resultAux &= (lastLAaction.details.processingAux == DEFAULT_ACTION_AUX);
	if (!resultAux) {
		LOG_ERROR("Test action data doesn't match expected.");
		result = false;
	}

	GA::coldData_t firstGAcold = CL::ASmirrorData_cptr->agentMirrorPtrs.GAcoldData_ptr->data[0];
	LA::decisionData_t lastLAdecision = CL::ASmirrorData_cptr->agentMirrorPtrs.LAdecision_ptr->data[TST_NUMBER_LAS-1];

	resultAux = (firstGAcold.id == 0);
	resultAux &= (lastLAdecision.offsets.personality[0][0] == (float)DEFAULT_LA_OFFSET);
	if (!resultAux) {
		LOG_ERROR("Test agent data doesn't match expected.");
		result = false;
	}

	if (!result) {
		LOG_ERROR("Test failed");
		return false;
	}

	LOG_INFO("TA read data from CL as expected!");
	return true;
}

bool testFromTAifCLhasInitialized(void) {

	LOG_WARN("TA will querry wether CL's Data Controller are properly initialized...");

	bool result = CL::ASmirrorData_cptr->actionMirror.isInitialized();
	result &= CL::ASmirrorData_cptr->agentMirrorPtrs.haveBeenCreated;
	if (!result) {
		LOG_ERROR("CL Mirror data controllers not initialized or bad pointer");
		return false;
	}
	LOG_TRACE("Initialization flags ok. Checking capacities...");
	
	//Dividing by sizeof to get back to quantity of items, which is easier to check expected
	size_t capLAact = CL::ASmirrorData_cptr->actionMirror.capacityForDataInBytesLAs();
	size_t sizeLAact = sizeof(CL::ASmirrorData_cptr->actionMirror.dataLAs[0]);

	size_t capGAact = CL::ASmirrorData_cptr->actionMirror.capacityForDataInBytesGAs();
	size_t sizeGAact = sizeof(CL::ASmirrorData_cptr->actionMirror.dataGAs[0]);

	size_t capLAcold = CL::ASmirrorData_cptr->agentMirrorPtrs.LAcoldData_ptr->capacityForDataInBytes();
	size_t sizeLAcold = sizeof(CL::ASmirrorData_cptr->agentMirrorPtrs.LAcoldData_ptr->data[0]);

	size_t capGAcold = CL::ASmirrorData_cptr->agentMirrorPtrs.GAcoldData_ptr->capacityForDataInBytes();
	size_t sizeGAcold = sizeof(CL::ASmirrorData_cptr->agentMirrorPtrs.GAcoldData_ptr->data[0]);

	size_t capLAstate = CL::ASmirrorData_cptr->agentMirrorPtrs.LAstate_ptr->capacityForDataInBytes();
	size_t sizeLAstate = sizeof(CL::ASmirrorData_cptr->agentMirrorPtrs.LAstate_ptr->data[0]);

	size_t capGAstate = CL::ASmirrorData_cptr->agentMirrorPtrs.GAstate_ptr->capacityForDataInBytes();
	size_t sizeGAstate = sizeof(CL::ASmirrorData_cptr->agentMirrorPtrs.GAstate_ptr->data[0]);

	size_t capLAdecs = CL::ASmirrorData_cptr->agentMirrorPtrs.LAdecision_ptr->capacityForDataInBytes();
	size_t sizeLAdecs = sizeof(CL::ASmirrorData_cptr->agentMirrorPtrs.LAdecision_ptr->data[0]);

	size_t capGAdecs = CL::ASmirrorData_cptr->agentMirrorPtrs.GAdecision_ptr->capacityForDataInBytes();
	size_t sizeGAdecs = sizeof(CL::ASmirrorData_cptr->agentMirrorPtrs.GAdecision_ptr->data[0]);

	int failed = 0;

	int GAquantity = CL::ASmirrorData_cptr->networkParams.numberGAs; //already corrected
	int LAquantity = CL::ASmirrorData_cptr->networkParams.numberLAs;
	int maxActions = CL::ASmirrorData_cptr->networkParams.maxActions;

	failed += capLAact != (LAquantity * maxActions * sizeLAact);
	failed += capGAact != (GAquantity * maxActions * sizeGAact);

	//TODO-CRITICAL: GA's are counting the "phantom" last one
	failed += capLAcold != (MAX_LA_QUANTITY* sizeLAcold);
	failed += capGAcold != (MAX_GA_QUANTITY* sizeGAcold);
	failed += capLAstate != (MAX_LA_QUANTITY* sizeLAstate);
	failed += capGAstate != (MAX_GA_QUANTITY*sizeGAstate);
	failed += capLAdecs != (MAX_LA_QUANTITY* sizeLAdecs);
	failed += capGAdecs != (MAX_GA_QUANTITY* sizeGAdecs);

	if (failed) {
		LOG_ERROR("Some CL data controller capacities don't match the expected!");

		#if (defined AS_DEBUG) || VERBOSE_RELEASE
			printf("Failed on %d out of 8 checks\nsizeLAact; %zi, sizeGAact: %zi\n", 
															failed, sizeLAact, sizeGAact);
			printf("capLAact: %zi, expected: %zi - capGAact: %zi, expected: %zi\n",
				capLAact, (MAX_LA_QUANTITY * MAX_ACTIONS_PER_AGENT * sizeLAact), capGAact,
				(MAX_GA_QUANTITY * MAX_ACTIONS_PER_AGENT* sizeLAact));
		#endif // AS_DEBUG

		GETCHAR_PAUSE;
		return false;
	}
	
	LOG_INFO("CL data controller capacities are as expected. CL Initialization seems ok : )");
	return true;
}

bool testMockData(void) {

	LOG_WARN("Sanity checks for the communication between APP and DLLs (with mock initialization");

	AS::CLsanityTest();

	int numberAS = CL::getASnumber();
	int numberCL = CL::getCLnumber();

	bool result = numberAS == AS_TST_INIT_EXPECTED_NUMBER;
	result &= numberCL == CL_TST_INIT_EXPECTED_NUMBER;

	if (result) {
		LOG_INFO("AS and CL initialization and basic communication test numbers are Right");
	}
	else {
		LOG_ERROR("AS and/or CL initialization and basic communication test numbers are WRONG");
	}

	int* tstArray_ptr = CL::getTestArrayPtr();
	if (tstArray_ptr[0] == AS_TST_EXPECTED_ARR0 && tstArray_ptr[1] == AS_TST_EXPECTED_ARR1) {
		LOG_INFO("AS test array numbers relayed correctly");
	}
	else {
		LOG_ERROR("AS test array numbers relayed INCORRECTLY");
		if (tstArray_ptr[0] == MS_DEBUG_MALLOC_INIT_VALUE) {
			LOG_TRACE("(it seems like no value was copied to CL's test array)");
		}
		result = false;
	}

	#if (defined AS_DEBUG) || VERBOSE_RELEASE
		printf("TstArray[0]: %d \nTstArray[1]: %d \n",
			   tstArray_ptr[0], tstArray_ptr[1]);
	#endif

	return result;
}

void testSayHello(void) {
	
	AS::sayHello();

	CL::sayHelloExternal();
	
	GETCHAR_PAUSE;
}