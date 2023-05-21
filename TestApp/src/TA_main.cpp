//TODO: This file is pretty ugly : )

#include "miscStdHeaders.h"
#include "miscDefines.hpp"
#include "fileHelpers.h"
#include "core.hpp"

#include "AS_API.hpp"
#include "CL_ExternalAPI.hpp"

#include "AS_testsAPI.hpp"

#include "timeHelpers.hpp"
#include "tests.hpp"

#include "textViz.hpp"

int testsBattery(void);

void testSayHello(void);
bool testMockData(void);
bool testSnooze(bool printLog = true);
bool testFromTAifCLhasInitialized(void);
bool testReadingCLdataFromTA(void);
bool testChangingCLdataFromTAandRetrievingFromAS(void);
bool testReadingTickDataWhileASmainLoopRuns_start(void);
bool testReadingTickDataWhileASmainLoopRuns_end(void);
bool testSendingClientDataAndSaving(void);
bool testClientDataHAndlerInitialization(void);
bool testPause(bool printLog = false, int pauseUnpauseCycles = 5);
bool testMainLoopErrors(std::string filename);
bool testAgentsUpdating(bool printLog, bool fixedAndStepped = false);
bool testReads(bool printLog, float secondsToRun = 1);
bool testDecisionsAndActionsForThrownErrorsAndCalculateTiming(bool printLog, bool dump);
void quickTestInit(bool run, bool disableDecisions, bool blockActions) {
	AS::initializeASandCL();
	AS::testFileCreation(fileNameNoDefaults, fileNameWithDefaults);
	AS::loadNetworkFromFile(fileNameWithDefaults, run, disableDecisions, blockActions);
}

#define TESTS_BATTERY 0
#define TEXT_VIZ 1

#define MINIMUM_PROPORTION_SLEEP_PASSES (0.95)

#define TEST_BATTERIES 4
//TODO: the names of the test categories do not make complete sense any more
enum batterySizes { HELPER_FUNC_TESTS = 7, BASIC_INIT_COMM_TESTS = 4,
				    SPECIFIC_DATA_FUNCTIONALITY_TESTS = 13,
					SPECIFIC_THREADED_LOOP_TESTS = 13 };

int batterySizes[TEST_BATTERIES] = {HELPER_FUNC_TESTS, BASIC_INIT_COMM_TESTS, 
									SPECIFIC_DATA_FUNCTIONALITY_TESTS, 
									SPECIFIC_THREADED_LOOP_TESTS };

//these are mostly to avoid changing a lot of old code : p
#define HELPER_FUNC_TESTS batterySizes::HELPER_FUNC_TESTS
#define BASIC_INIT_COMM_TESTS batterySizes::BASIC_INIT_COMM_TESTS
#define SPECIFIC_DATA_FUNCTIONALITY_TESTS batterySizes::SPECIFIC_DATA_FUNCTIONALITY_TESTS
#define SPECIFIC_THREADED_LOOP_TESTS batterySizes::SPECIFIC_THREADED_LOOP_TESTS
#define TOTAL_TESTS (HELPER_FUNC_TESTS+BASIC_INIT_COMM_TESTS+SPECIFIC_DATA_FUNCTIONALITY_TESTS+SPECIFIC_THREADED_LOOP_TESTS)

//These will hold the test results:
bool battery0[HELPER_FUNC_TESTS];
bool battery1[BASIC_INIT_COMM_TESTS];
bool battery2[SPECIFIC_DATA_FUNCTIONALITY_TESTS];
bool battery3[SPECIFIC_THREADED_LOOP_TESTS];

bool* battery_ptrs[TEST_BATTERIES] = { &battery0[0], &battery1[0], 
								       &battery2[0], &battery3[0] };

void printFailedTests() {
	for (int i = 0; i < TEST_BATTERIES; i++) {
		
		auto results_arr = battery_ptrs[i];
		int failed = 0;

		printf("Battery %d Failed tests: ", i);
		for (int j = 0; j < batterySizes[i]; j++) {
			if (results_arr[j] == false) {
				printf("%d; ",j);
				failed++;
			}
		}
		if (!failed) {
			printf("none");
		}
		printf("\n");
	}
}

std::thread reader;//to test realtime reading of data trough CL as AS runs
uint64_t g_ticksRead[TST_TIMES_TO_QUERRY_TICK]; 

void printOptions(){

	printf("Program expects zero arguments (default test) or a single argument, to define the test to run\n\n");
	printf("Available tests are:\n%d - Tests Battery;\n%d - Text Mode Visualization;\n",
																TESTS_BATTERY, TEXT_VIZ);
	return;
}

int main(int argc, char **argv) {

	if (argc == 1) {
		LOG_INFO("No arguments entered, will run the tests battery\n\n");
		return testsBattery();
	}
	else if (argc != 2) {
		LOG_ERROR("Bad number of arguments");
		printOptions();
		GETCHAR_FORCE_PAUSE;
		return 1;
	}

	switch (std::stoi(argv[1]))
	{
	case TESTS_BATTERY:
		return testsBattery();
	case TEXT_VIZ:
		return TV::textModeVisualizationEntry();
	default:
		LOG_ERROR("Received bad argument");
		printOptions();
		GETCHAR_FORCE_PAUSE;
		return 1;
	}
}


int testsBattery(void) {
	//TODO: review wich tests print to console, and pass this (macro?)
	bool printSteps = false;
	#if ( (defined AS_DEBUG) || VERBOSE_RELEASE )
		printSteps = true;
	#endif

	LOG_DEBUG("This is a battery of tests for the Multi Agent System\n"); GETCHAR_PAUSE;

	LOG_INFO("\tBATTERY 0 - Helper Functionality\n", 1); GETCHAR_PAUSE;

	battery0[0] = AZ::testDraw1spcg32();

	battery0[1] = AZ::testDraw4spcg32s();

	battery0[2] = testSnooze(printSteps);

	battery0[3] = AZ::testFlagFields(printSteps);

	battery0[4] = AS::testActionVariationsInfo(printSteps);

	battery0[5] = AS::testMultipleAgentChopCalculations(printSteps);

	battery0[6] = AS::testWarningAndErrorCountingAndDisplaying(printSteps);

	int resultsBattery0 = 0;
	for(int i = 0; i < HELPER_FUNC_TESTS; i++) { 
		resultsBattery0 += (int)battery0[i]; 
	}

	if (resultsBattery0 != HELPER_FUNC_TESTS) {
		LOG_CRITICAL("Not all of these tests passed:", 1);
		printf("%d out of %d failed", HELPER_FUNC_TESTS - resultsBattery0, HELPER_FUNC_TESTS);
		GETCHAR_PAUSE;
	}
	else {
		LOG_INFO("All of these tests passed!", 1); 
		GETCHAR_PAUSE;
	}

	LOG_DEBUG("\tBATTERY 1 - Communicaton and Data Storage\n",1); GETCHAR_PAUSE;

	testSayHello(); //nothing to test automatically. TODO: make it do so
		
	battery1[0] = testMockData();
	
	LOG_DEBUG("Actual initialization tests...", 1);

	LOG_DEBUG("Will test the API`s AS::initializeASandCL()\n", 1); GETCHAR_PAUSE;
	battery1[1] = AS::initializeASandCL(); 

	battery1[2] = AS::testContainersAndAgentObjectCreation();

	battery1[3] = testFromTAifCLhasInitialized();

	int resultsBattery1 = 0;
	for(int i = 0; i < BASIC_INIT_COMM_TESTS; i++) { 
		resultsBattery1 += (int)battery1[i]; 
	}

	if (resultsBattery1 != BASIC_INIT_COMM_TESTS) {
		LOG_CRITICAL("Not all of these tests passed:", 1);
		printf("%d out of %d failed", BASIC_INIT_COMM_TESTS - resultsBattery1, BASIC_INIT_COMM_TESTS);
		GETCHAR_PAUSE;
	}
	else {
		LOG_INFO("All of these tests passed!", 1); GETCHAR_PAUSE;
	}

	LOG_DEBUG("\tBATTERY 2 - Basic Data Manipulation\n",1); GETCHAR_PAUSE;
	
	battery2[0] = AS::testFileCreation(fileNameNoDefaults, fileNameWithDefaults); 

	LOG_DEBUG("Will test loading network from a File\n", 1); GETCHAR_PAUSE;
	battery2[1] = AS::loadNetworkFromFile(fileNameWithDefaults, false, false, false, true); 
	
	LOG_DEBUG("Will test saving back the network\n", 1); GETCHAR_PAUSE;	
	battery2[2] = AS::saveNetworkToFile();

	battery2[3] = AS::testDataTransferFromAStoCL();
	
	battery2[4] = testReadingCLdataFromTA(); 

	battery2[5] = testChangingCLdataFromTAandRetrievingFromAS();

	LOG_DEBUG("Will test saving back the modified network with another name\n", 1); 
	GETCHAR_PAUSE;	
	//TODO: why does this take a while to start? Other saves are fast
	//Seems to only be the case in debug. Idk, really weird
	battery2[6] = AS::saveNetworkToFile(customFilename, true); 

	battery2[7] = AS::testNeighbourIDsetting(); 

	battery2[8] = AS::testChoppedPRNdrawing(printSteps, true); 

	battery2[9] = AS::testNormalDrawing(printSteps, true);

	battery2[10] = AS::testRedDrawing(printSteps, true);

	battery2[11] = AS::testDecisionStepTiming(printSteps); 

	battery2[12] = AS::testUpdateRead(true, false, readUpdatingTestFilename);

	int resultsBattery2 = 0;
	for(int i = 0; i < SPECIFIC_DATA_FUNCTIONALITY_TESTS; i++) { 
		resultsBattery2 += (int)battery2[i]; 
	}

	if (resultsBattery2 != SPECIFIC_DATA_FUNCTIONALITY_TESTS) {
		LOG_CRITICAL("Not all of these tests passed:", 1);
		printf("%d out of %d failed", SPECIFIC_DATA_FUNCTIONALITY_TESTS - resultsBattery2, 
			                                             SPECIFIC_DATA_FUNCTIONALITY_TESTS);
		GETCHAR_PAUSE;
	}
	else {
		LOG_INFO("All of these tests passed!", 1); GETCHAR_PAUSE;
	}
	
	LOG_INFO("BATTERY 3 - Specific Functionality and Main Loop\n",1); GETCHAR_PAUSE;

	battery3[0] = testMainLoopErrors(customFilename); 

	LOG_DEBUG("Will re-load the previously modified network for further testing\n", 1); GETCHAR_PAUSE;
	battery3[1] = AS::loadNetworkFromFile(customFilename, true); 

	battery3[2] = testReadingTickDataWhileASmainLoopRuns_start(); 
	
	battery3[3] = testReadingTickDataWhileASmainLoopRuns_end(); 

	LOG_DEBUG("Will save again\n", 1); GETCHAR_PAUSE;
	battery3[4] = AS::saveNetworkToFile(customFilename, true);

	//Also saves another network, with steps, and resumes it:
	battery3[5] = testClientDataHAndlerInitialization(); 

	battery3[6] = testSendingClientDataAndSaving(); 

	battery3[7] = testPause(printSteps); 

	LOG_DEBUG("Will test quitting the AS\n", 1); GETCHAR_PAUSE;
	battery3[8] = AS::quit();

	battery3[9] = testAgentsUpdating(printSteps); 
	
	battery3[10] = testAgentsUpdating(printSteps, true); 

	battery3[11] = testReads(printSteps);

	battery3[12] = 
			testDecisionsAndActionsForThrownErrorsAndCalculateTiming(printSteps, true);
	
	
	int resultsBattery3 = 0;
	for(int i = 0; i < SPECIFIC_THREADED_LOOP_TESTS; i++) { 
		resultsBattery3 += (int)battery3[i]; 
	}

	if (resultsBattery3 != SPECIFIC_THREADED_LOOP_TESTS) {
		LOG_CRITICAL("Not all of these tests passed:", 1);
		printf("%d out of %d failed", SPECIFIC_THREADED_LOOP_TESTS - resultsBattery3,
			SPECIFIC_THREADED_LOOP_TESTS);
		GETCHAR_PAUSE;
	}
	else {
		LOG_INFO("All of these tests passed!", 1); GETCHAR_PAUSE;
	}

	LOG_DEBUG("Tests ended...\n", 1); GETCHAR_PAUSE;

	int totalPassed = resultsBattery0 + resultsBattery1 + resultsBattery2 + resultsBattery3;
	if (totalPassed == TOTAL_TESTS) {
		LOG_INFO("All automatically checked tests passed!", 1); GETCHAR_PAUSE;
	}
	else {
		LOG_CRITICAL("Not all tests were passed!", 1);
		printf("%d out of %d failed\n", TOTAL_TESTS - totalPassed, TOTAL_TESTS);

		printFailedTests();

		GETCHAR_PAUSE;
	}

	//TODO: Update expected changes after I'm done updating it
	LOG_DEBUG("Check that you have (at least): one network file with format specifiers,\none with default values and one with modified values:", 1);
	printf("\t-The one with specifiers is %s\n\t-%s should have the defaults\n\t-%s received modifications from TA\n",
		                              fileNameNoDefaults, fileNameWithDefaults, customFilename);
	printf("\nThe modified file has different data:\n\t-The comment's first letter should be a %c;\n\t-Ticks should be the number of times mainLoopTrhead looped before last save;\n",TST_COMMENT_LETTER_CHANGE);
	printf("\t-Last GA`s id = %d and connected GAs = %d ;\n", TST_GA_ID, TST_GA_CONNECTIONS);
	printf("\t-Last LA`s reinforcement = %f, offset[%d][%d][1] = %f and last actions aux = %f.\n",
		TST_LA_REINFORCEMENT, TST_CHANGED_CATEGORY, TST_CHANGED_MODE, TST_LA_OFFSET, TST_LAST_ACTION_AUX);
	
	GETCHAR_FORCE_PAUSE;

	AZ::hybridBusySleepForMicros(std::chrono::microseconds(MICROS_IN_A_SECOND));

	LOG_INFO("Done! Enter to exit", 1); GETCHAR_FORCE_PAUSE;
	return (TOTAL_TESTS - totalPassed);
}

#define MINIMUM_PROPORTION_SLEEP_PASSES (0.95)
bool testSnooze(bool printLog) {
	
	LOG_DEBUG("Will test sleeping and waking a few times...\n", 1); 
	GETCHAR_PAUSE; 

	double result = AZ::testHybridBusySleeping(printLog); 

	bool passed = result > MINIMUM_PROPORTION_SLEEP_PASSES;
	if(!passed) { LOG_ERROR("Snoozed more than the maximum margin set"); }
	else if(printLog) { LOG_INFO("Passed"); }

	return passed;
}

//If print, will print a digest of the results. If dump, will save to a file all the
//timing data, including amount of active actions for each tick.
//Fails in case mainLoop finds errors or updating of the timing data doesn't work.
bool testDecisionsAndActionsForThrownErrorsAndCalculateTiming(bool print, bool dump) {
	
	LOG_DEBUG("Will load a network, run it with decisions and actions and check for errors, while logging performance\n", 1);
	GETCHAR_PAUSE;
	
	bool result = AS::loadNetworkFromFile(fileNameWithDefaults, false);
	if (!result) {
		LOG_ERROR("Failed to load network. Aborting test");
		return false;
	}

	auto params_ptr = &(CL::ASmirrorData_cptr->networkParams);

	if (params_ptr->mainLoopTicks != 0) {
		LOG_ERROR("This tests expects a network with mainLoopTicks == 0. Aborting");
		return false;
	}

	int numberLAs = CL::ASmirrorData_cptr->networkParams.numberLAs;
	int numberGAs = CL::ASmirrorData_cptr->networkParams.numberGAs - 1; //effective GAs
	int maxActionsPerAgent = CL::ASmirrorData_cptr->networkParams.maxActions;
	
	//Set resources on all agents as infinite, to facilitate decisions:
	constexpr float hugeResources = ACT_REFERENCE_RESOURCES * NANOS_IN_A_SECOND;
	assert(std::isfinite(hugeResources));

	auto cldh_ptr = CL::getClientDataHandlerPtr();
	
	for (int agent = 0; agent < numberLAs; agent++) {
		cldh_ptr->LAstate.parameters.resources.changeCurrentTo(agent, hugeResources);
	}
	for (int agent = 0; agent < numberGAs; agent++) {
		cldh_ptr->GAstate.parameters.changeGAresourcesTo(agent, hugeResources);
	}
	
	//We want to run enough steps so that all action slots could be exhausted
	int stepsToRun = ((maxActionsPerAgent + 1)) * AS_TOTAL_CHOPS * 2;
	assert(stepsToRun > 0);

	//And we'll need to check often to get the data we want:
	int stepTimeMicros = AS_MILLISECONDS_PER_STEP * MICROS_IN_A_MILLI;
	std::chrono::microseconds testStepTime = std::chrono::microseconds(stepTimeMicros / 3);

	//As the network runs, we'll want to log some data:
	struct tickData_st {
		int totalActions = 0;
		std::chrono::microseconds hotMicros = std::chrono::microseconds(0);
	};

	std::vector<tickData_st> ticks;

	tickData_st tickData;
	ticks.reserve(stepsToRun);
	for(int i = 0; i < stepsToRun; i++) { ticks.push_back(tickData); }

	//We're now ready to run the network
	result = AS::run(true, stepsToRun);

	if (!result) {
		LOG_ERROR("Failed to run network. Aborting test");
		return false;
	}

	//And log the data we want:
	bool hasPaused = false;
	uint32_t nextTickToRecord = 0;
	auto LAactions_ptr = &(CL::ASmirrorData_cptr->actionMirror.dataLAs);
	auto GAactions_ptr = &(CL::ASmirrorData_cptr->actionMirror.dataGAs);
	while (!hasPaused) {

		if (params_ptr->mainLoopTicks > (std::numeric_limits<uint32_t>::max())) {
			LOG_ERROR("Too many ticks - would overflow buffer: aborting test");
			return false;
		}
		uint32_t tickOnMirror = (uint32_t)params_ptr->mainLoopTicks;
		if ( (tickOnMirror >= nextTickToRecord) && (tickOnMirror < (uint32_t)stepsToRun) ) {

			nextTickToRecord = tickOnMirror + 1;

			//record the data:
			tickData.hotMicros = params_ptr->lastStepHotMicros;

			int activeActions = 0;
			for (int i = 0; i < maxActionsPerAgent; i++) {

				for (int j = 0; j < numberLAs; j++) {

					int index = (i * numberLAs) + j;
					activeActions += (LAactions_ptr->at(index).ids.active == 1);
				}
				for (int j = 0; j < numberGAs; j++) {

					int index = (i * numberGAs) + j;
					activeActions += (GAactions_ptr->at(index).ids.active == 1);
				}
			}
			tickData.totalActions = activeActions;

			ticks.at(tickOnMirror) = tickData;
		}
		
		AZ::hybridBusySleepForMicros(testStepTime);
		hasPaused = AS::checkIfMainLoopIsPaused();
	}

	//We'll now check how many ticks, if any, we missed:
	int missedTicks = 0;

	for (int i = 0; i < stepsToRun; i++) {
		missedTicks += (ticks.at(i).hotMicros.count() == 0);
	}

	//If we lost too many ticks, this is an error:
	float maxMissProportion = 0.05f;
	int maxMisses = (int)(stepsToRun * (float)maxMissProportion);

	result = (missedTicks < maxMisses);

	if (!result) {
		LOG_ERROR("Test logging missed too many ticks. Will fail, but return from main will still be checked");
	}
	else {
		//let's interpolate any missing data (we hope there are no sequences of missing data):
		for (int i = 0; i < stepsToRun; i++) {
			if (ticks.at(i).hotMicros.count() == 0) {
				if(i == 0) { ticks.at(0) = ticks.at(1); }
				else if(i == (stepsToRun-1)) { ticks.at(stepsToRun-1) = ticks.at(stepsToRun-2); }
				else {
					ticks.at(i).hotMicros = 
						(ticks.at(i - 1).hotMicros + ticks.at(i + 1).hotMicros) / 2;
					ticks.at(i).totalActions = 
						(ticks.at(i - 1).totalActions + ticks.at(i + 1).totalActions) / 2;
				}
			}
		}

	}

	//The mainLoop has stepped and is now paused and we have our data.
	//Let's stop the main loop and check for errors:
	bool aux = AS::stop();
	
	result &= aux;
	if (!aux) {
		LOG_ERROR("Main loop found errors during the test... will fail");
	}
	else {
		LOG_INFO("Main loop found no errors during the test");
	}
	
	//Now we calculate some indicators from our data  and display/dump as requested:

	int leastActions = maxActionsPerAgent * (numberGAs + numberLAs) * 2; //so, too many
	int mostActions = -1;
	int64_t shortestHotMicros = AS_MILLISECONDS_PER_STEP * MICROS_IN_A_MILLI * 2; //too long
	int64_t longestHotMicros = 0;

	int idLeastActions = -1;
	int idMostActions = -1;
	int idShortest = -1;
	int idLongest = -1;

	for (int i = 0; i < stepsToRun; i++) {
		int actions = ticks.at(i).totalActions;
		int64_t micros = ticks.at(i).hotMicros.count();

		if(actions < leastActions) { leastActions = actions; idLeastActions = i;}
		if(actions > mostActions) { mostActions = actions; idMostActions = i; }
		if(micros < shortestHotMicros) { shortestHotMicros = micros; idShortest = i;}
		if(micros > longestHotMicros) { longestHotMicros = micros; idLongest = i;}
	}
	if ((idLeastActions == -1) || (idMostActions == -1)
		|| (idShortest == -1) || (idLongest == -1)) {
		LOG_WARN("Something went wrong calculating the data extremes. Results may be bogus");
	}

	if (print) {
		printf("Steps: %d (missed %d) MaxTotalActions: %d (maxActs: %d, LAs: %d, GAs: %d)\n\n",
						stepsToRun, missedTicks, maxActionsPerAgent * (numberLAs + numberGAs), 
						maxActionsPerAgent, numberLAs, numberGAs );
		printf("Least Actions: %d at step %d (%lld hot micros)\n",
			leastActions, idLeastActions, ticks.at(idLeastActions).hotMicros.count());
		printf("Most Actions: %d at step %d (%lld hot micros)\n",
			mostActions, idMostActions, ticks.at(idMostActions).hotMicros.count());
		printf("Shortest step: %lld micros at step %d (%d actions)\n",
			shortestHotMicros, idShortest, ticks.at(idShortest).totalActions);
		printf("Longest step: %lld micros at step %d (%d actions)\n",
			longestHotMicros, idLongest, ticks.at(idLongest).totalActions);
	}

	if (dump) {

		aux = AS::saveNetworkToFile(decisionsAndActionsTestNetworkFilename, 
												        false, false, true);
		if (!aux) {
			LOG_ERROR("Failed to save network. Will try to dump timing data, but test will fail");
		}
		result &= aux;

		FILE* fp = AZ::acquireFilePointerToSave(decisionsAndActionsTimingFilename, false, 
																		 defaultFilePath);

		if (fp == NULL) {
			LOG_WARN("Couldn't acquire file pointer to save test data, so won't");
		}
		else {
			fprintf(fp, "Steps: %d (%d interpolated) MaxTotalActions: %d (maxActs: %d, LAs: %d, GAs: %d)\n\n",
					stepsToRun, missedTicks, maxActionsPerAgent * (numberLAs + numberGAs), 
					maxActionsPerAgent, numberLAs, numberGAs );

			fprintf(fp, "Least Actions: %d at step %d (%lld hot micros)\n",
				leastActions, idLeastActions, ticks.at(idLeastActions).hotMicros.count());
			fprintf(fp, "Most Actions: %d at step %d (%lld hot micros)\n",
				mostActions, idMostActions, ticks.at(idMostActions).hotMicros.count());
			fprintf(fp, "Shortest step: %lld micros at step %d (%d actions)\n",
				shortestHotMicros, idShortest, ticks.at(idShortest).totalActions);
			fprintf(fp, "Longest step: %lld micros at step %d (%d actions)\n\n",
				longestHotMicros, idLongest, ticks.at(idLongest).totalActions);

			fprintf(fp, "Step\tActions\tMicros\n\n");

			for (int i = 0; i < stepsToRun; i++) {
				fprintf(fp, "%d\t%d\t%lld\n", i, ticks.at(i).totalActions, 
					                       ticks.at(i).hotMicros.count() );
			}
		}
	}

	//Now we show the overall result:
	if (!result) {
		LOG_ERROR("Test failed");
	}
	else {
		LOG_INFO("Test passed");
	}

	//and walk away : )
	return result;
}

//Tests that reads are happening, have reasonable values, and save and load as expected.
//Loads a network, sets some values, runs it, checks new values, saves, loads, checks again
//TODO: check all fields, better sanity checking
bool testReads(bool print, float secondsToRun) {
	
	LOG_DEBUG("Will load a network, run for several ticks, stop it, and check reads\n", 1);
	GETCHAR_PAUSE;
	
	//Load, but don't start, so we can calculate and set test conditions:
	bool result = AS::loadNetworkFromFile(fileNameWithDefaults, false);
	if (!result) {
		LOG_ERROR("Failed to load network. Aborting test");
		return false;
	}

	//We want to focus on the reading, so we'll disable decision-making and action processing:
	auto clientData_ptr = CL::getClientDataHandlerPtr();
	clientData_ptr->networkParameters.changeMakeDecisionsTo(false);
	clientData_ptr->networkParameters.changeProcessActionsTo(false);

	//Set some test values: we want one LA and one GA to have -0,5 , 0 and 0,5 infiltration
	auto cldh_ptr = CL::getClientDataHandlerPtr();
	if (cldh_ptr == NULL) {
		LOG_ERROR("Failed to get client data handler. Aborting test");
		return false;
	}

	int requiredTotalGAs = 5; //4 effective, so 0 is left alone and we have 3 more
	int quantityGAs = CL::ASmirrorData_cptr->networkParams.numberGAs;
	if (quantityGAs < requiredTotalGAs) { 
		LOG_ERROR("Not enough GAs to perform this test. Aborting");
		return false;
	}

	//Will leave GA0 alone and set the values in the others:
	cldh_ptr->GAdecision.changeInfiltrationOnAll(1, -0.5f);
	cldh_ptr->GAdecision.changeInfiltrationOnAll(2, 0.0f);
	cldh_ptr->GAdecision.changeInfiltrationOnAll(3, 0.5f);

	int quantityLAs = CL::ASmirrorData_cptr->networkParams.numberLAs;

	//For the LAs, we will leave LA0 alone and for the others intercalate the test values:
	for (int i = 1; i < quantityLAs; i++) {
		int ref = i - 1; //so we start at -0,5 for LA1
		float newInfiltration = (ref%3)*(0.5f) - 0.5f; //-0,5 , 0, 0,5
		cldh_ptr->LAdecision.changeInfiltrationOnAll(i, newInfiltration);
	}
		
	//How long to wait for test to run?
	int millisToRun = int(secondsToRun * A_THOUSAND);
	int targetTicksToRun = millisToRun/AS_MILLISECONDS_PER_STEP;
	
	//Now we can run the test:
	result = AS::run(true, targetTicksToRun);
	if (!result) {
		LOG_ERROR("Failed to run network. Aborting test");
		return false;
	}

	//wait for it to be done and pause:
	bool paused = false; //pause is NOT instantaneous
	while(!paused){
		paused = AS::checkIfMainLoopIsPaused();
	}

	//check that the reads are reasonable and save some of them
	
	std::vector<std::vector<float>> valuesToBeSavedPerNeighborPerGA;
	int weirdValuesFound = 0;

	auto GAdecisionsVector = &(CL::ASmirrorData_cptr->agentMirrorPtrs.GAdecision_ptr->data);
	auto GAstateVector = &(CL::ASmirrorData_cptr->agentMirrorPtrs.GAstate_ptr->data);

	for (int thisGA = 0; thisGA < quantityGAs - 1; thisGA++) {

		auto thisState_ptr = &(GAstateVector->at(thisGA));
		auto reads_ptr = &(GAdecisionsVector->at(thisGA).reads[0]);
		
		std::vector<float> valuesToBeSavedPerNeighborThisGA;

		int totalConnections = thisState_ptr->connectedGAs.howManyAreOn();
		for (int neighbor = 0; neighbor < totalConnections; neighbor++) {
			
			int neighborID = thisState_ptr->neighbourIDs[neighbor];
			
			int field = (int)GA::readsOnNeighbor_t::fields::GA_RESOURCES;
			float read = reads_ptr[neighbor].of[field].read;
			float real = GAstateVector->at(neighborID).parameters.GAresources;

			float small = 1;
			if (abs(real) < small) {
				if(real >= 0) { 
					real = small; 
				}
				else {
					real = -small;
				}
			}

			float proportionalDifference = abs((real - read))/abs(real);

			if (proportionalDifference > 2 * EXPC_PROPORTIONAL_ERROR_FOR_MAX_CORRECTION) {
				//absurd!
				weirdValuesFound++;
			}

			valuesToBeSavedPerNeighborThisGA.push_back(read);
		}

		valuesToBeSavedPerNeighborPerGA.push_back(valuesToBeSavedPerNeighborThisGA);
	}

	std::vector<std::vector<float>> valuesToBeSavedPerNeighborPerLA;

	auto LAdecisionsVector = &(CL::ASmirrorData_cptr->agentMirrorPtrs.LAdecision_ptr->data);
	auto LAstateVector = &(CL::ASmirrorData_cptr->agentMirrorPtrs.LAstate_ptr->data);

	for (int thisLA = 0; thisLA < quantityLAs; thisLA++) {

		auto thisState_ptr = &(LAstateVector->at(thisLA));
		auto reads_ptr = &(LAdecisionsVector->at(thisLA).reads[0]);
		
		std::vector<float> valuesToBeSavedPerNeighborThisLA;

		thisState_ptr->locationAndConnections.connectedNeighbors.updateHowManyAreOn();
		int totalConnections = 
			thisState_ptr->locationAndConnections.connectedNeighbors.howManyAreOn();
		for (int neighbor = 0; neighbor < totalConnections; neighbor++) {
			
			int neighborID = thisState_ptr->locationAndConnections.neighbourIDs[neighbor];
			
			int field = (int)LA::readsOnNeighbor_t::fields::RESOURCES;
			float read = reads_ptr[neighbor].of[field].read;
			float real = LAstateVector->at(neighborID).parameters.resources.current;

			float small = 1;
			if (abs(real) < small) {
				if(real >= 0) { 
					real = small; 
				}
				else {
					real = -small;
				}
			}

			float proportionalDifference = abs((real - read))/abs(real);

			if (proportionalDifference > 2 * EXPC_PROPORTIONAL_ERROR_FOR_MAX_CORRECTION) {
				//absurd!
				weirdValuesFound++;
			}

			valuesToBeSavedPerNeighborThisLA.push_back(read);
		}

		valuesToBeSavedPerNeighborPerLA.push_back(valuesToBeSavedPerNeighborThisLA);
	}

	//save the network
	result = AS::saveNetworkToFile(inNetworkReadsTest, true, false, true);
	if (!result) {
		LOG_ERROR("Failed to save network. Aborting test");
		return false;
	}
	
	//reload
	result = AS::loadNetworkFromFile(inNetworkReadsTest, false);
	if (!result) {
		LOG_ERROR("Failed to reload network. Aborting test");
		return false;
	}

	//test that the saved values were loaded right:

	int loadedWrong = 0;
	for (int thisGA = 0; thisGA < quantityGAs - 1; thisGA++) {

		auto thisState_ptr = &(GAstateVector->at(thisGA));
		auto reads_ptr = &(GAdecisionsVector->at(thisGA).reads[0]);
		
		thisState_ptr->connectedGAs.updateHowManyAreOn();
		int totalConnections = thisState_ptr->connectedGAs.howManyAreOn();
		for (int neighbor = 0; neighbor < totalConnections; neighbor++) {
					
			int field = (int)GA::readsOnNeighbor_t::fields::GA_RESOURCES;
			float read = reads_ptr[neighbor].of[field].read;

			float expected = valuesToBeSavedPerNeighborPerGA.at(thisGA).at(neighbor);
			float difference = read - expected;
			float tolerance = 2.0f/A_MILLION;
			if(abs(difference) > tolerance){
				loadedWrong++;
				if (print) {
					printf("GA%d (n%d) - read: %f, expected: %f | ", thisGA, neighbor, 
						                                               read, expected);
				}
			}
		}

	}

	for (int thisLA = 0; thisLA < quantityLAs; thisLA++) {

		auto thisState_ptr = &(LAstateVector->at(thisLA));
		auto reads_ptr = &(LAdecisionsVector->at(thisLA).reads[0]);
		
		thisState_ptr->locationAndConnections.connectedNeighbors.updateHowManyAreOn();
		int totalConnections = 
			thisState_ptr->locationAndConnections.connectedNeighbors.howManyAreOn();
		for (int neighbor = 0; neighbor < totalConnections; neighbor++) {
					
			int field = (int)LA::readsOnNeighbor_t::fields::RESOURCES;
			float read = reads_ptr[neighbor].of[field].read;

			float expected = valuesToBeSavedPerNeighborPerLA.at(thisLA).at(neighbor);
			float difference = read - expected;
			float tolerance = 2.0f/A_MILLION;
			if(abs(difference) > tolerance){
				loadedWrong++;
				if (print) {
					printf("LA%d (n%d) - read: %f, expected: %f | ", thisLA, neighbor, 
						                                               read, expected);
				}
			}
		}
	}

	if (loadedWrong > 0) {
		LOG_ERROR("Values were not read back as expected after loading");
		result = false;
		if (print) {
			printf("%d values read wrong", loadedWrong);
		}
	}

	if (weirdValuesFound > 0) {
		LOG_ERROR("Some weird values were found, updating is likely not working");
		result = false;
		if (print) {
			printf("%d weird values found", weirdValuesFound);
		}
	}

	if (result) {
		LOG_INFO("Reads seem ok");
	}

	return result;
}

//TODO: make a more interesting test-case using clientDataHandler (include war/attrition)
//TODO: this is really brittle : /
//NOTE: ASSUMES EVERYONE IS TRADING WITH EVERYONE ELSE
bool testAgentsUpdating(bool print, bool fixedAndStepped) {

	if(fixedAndStepped){
		LOG_DEBUG("Will load a network, run for several ticks with FIXED timestep, stop it, and check agent updating\n", 1);
	}
	else {
		LOG_DEBUG("Will load a network, run for several ticks with VARIABLE timestep, stop it, and check agent updating\n", 1);
	}
	GETCHAR_PAUSE;

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
		AS::quit();
		return false;
	}

	int quantityLAs = CL::ASmirrorData_cptr->networkParams.numberLAs;
	if (quantityLAs < 2) {
		LOG_ERROR("Too few LAs for this test. Will abort");
		AS::quit();
		return false;
	}

	auto clientData_ptr = CL::getClientDataHandlerPtr();
	if (clientData_ptr == NULL) {
		LOG_ERROR("Couldn't get client data handler. Will abort test");
		AS::quit();
		return false;
	}

	//We don't really want to test actions now, so all decisions and actions will be frozen:
	clientData_ptr->networkParameters.changeMakeDecisionsTo(false);
	clientData_ptr->networkParameters.changeProcessActionsTo(false);

	AS::GAflagField_t neighboursOfLastGA; //none will be on: no trade
	AS::GAflagField_t neighboursOfFirstGA;
	uint32_t flagField = (uint32_t)pow(2,(quantityGAs - 1)) - 1; //All on, but last
	flagField--; //turn off first, since this will be used for the first GA
	neighboursOfFirstGA.loadField(flagField);

	clientData_ptr->GAstate.changeConnectedGAs(0, &neighboursOfFirstGA);
	clientData_ptr->GAstate.changeConnectedGAs(quantityGAs-1, &neighboursOfLastGA);

	//Other GAs can't be neighbour of last either (no need to change IDs since it's the last),
	//but must be of first (nieghbor IDs are set when data is sent):
	//We first set them up on the mirror:
	for (int i = 1; i < (quantityGAs - 1); i++) {
		auto data_ptr = &CL::ASmirrorData_cptr->agentMirrorPtrs.GAstate_ptr->data.at(i);
		data_ptr->connectedGAs.setBitOff(quantityGAs - 1);
		data_ptr->connectedGAs.setBitOn(0);
	}
	//Then use the connections field in the mirror to issue the actual change:
	for (int i = 1; i < (quantityGAs - 1); i++) {
		auto data_ptr = &CL::ASmirrorData_cptr->agentMirrorPtrs.GAstate_ptr->data.at(i);
		clientData_ptr->GAstate.changeConnectedGAs(i, &(data_ptr->connectedGAs));
	}

	float externalGuardFirstLA = 184;
	float startingResourcesFirstLA = 0.1f;
	
	clientData_ptr->LAstate.parameters.strenght.changeGuard(0, externalGuardFirstLA);
	clientData_ptr->LAstate.parameters.resources.changeCurrentTo(0, startingResourcesFirstLA);

	auto laState_ptr = &CL::ASmirrorData_cptr->agentMirrorPtrs.LAstate_ptr->data;
	
	//These are initial values, to make sure they change (or don't change) as expected:
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
	
	//Should try to sleep "target - 1" ticks, as the pause only happens at the end of the tick
	int sleepMicros = (targetTicksToRun-1)*AS_MILLISECONDS_PER_STEP*MICROS_IN_A_MILLI;
	
	std::chrono::microseconds sleepTime(sleepMicros);
	std::chrono::microseconds threshold(MICROS_IN_A_MILLI);

	//Now we can run the test:
	if (fixedAndStepped) {
		result = AS::run(true, targetTicksToRun);
	}
	else {
		result = AS::run();

		//stepped pauses when done. Since we're not stepping, we have to control that:
		AZ::hybridBusySleepForMicros(sleepTime, threshold);

		AS::pauseMainLoop(); //we don't stop/quit, so the info is still available
	}

	//Either way, pause is NOT instantaneous, so wait for confirmation on pause:
	bool paused = false; 
	while(!paused){
		paused = AS::checkIfMainLoopIsPaused();
	}

	if (!result) { //we didn't check earlier to try and keep things in synch
		LOG_ERROR("Failed to run test network. Aborting test");
		AS::quit();
		return false;
	}

	//How long did it actually run?
	uint64_t initialTick =  CL::ASmirrorData_cptr->networkParams.lastMainLoopStartingTick;
	uint64_t finalTick =  CL::ASmirrorData_cptr->networkParams.mainLoopTicks;							
	uint64_t ticksRan = finalTick - initialTick;

	//When you pause, timing info from the last step is not yet sent, although step is, so:
	ticksRan++;
	if (ticksRan <= 1) {
		LOG_ERROR("Ran for one or less ticks: can't test");
		return false;
	}

	double totalMultiplier = 
				CL::ASmirrorData_cptr->networkParams.accumulatedMultiplier;
	if (totalMultiplier == 0) {
		LOG_ERROR("Network registered no time running: can't test");
		return false;
	}

	//since we added one "lost" step:
	double adjustedTotalMultiplier = totalMultiplier*((double)ticksRan/(ticksRan - 1)); 

	float absoluteDifference = (float)fabs(millisToRun - (ticksRan * AS_MILLISECONDS_PER_STEP));
	float propotionalAbsoluteDifference = absoluteDifference/millisToRun;
	float graceFactor = 0.015f;
	if (propotionalAbsoluteDifference > graceFactor) {
		LOG_WARN("Didn't run the expected time/ticks. Will proceed, but may fail");
	}

	//Did anything change which shouldn't have changed?
	bool eitherDecisionsOrActionsAreOn = CL::ASmirrorData_cptr->networkParams.makeDecisions
		                              || CL::ASmirrorData_cptr->networkParams.processActions;
	if(eitherDecisionsOrActionsAreOn){
		LOG_ERROR("Network went back to deciding/acting after that was disabled. Will fail.");
		AS::quit();
		return false;
	}

	if (gaState_ptr->at(0).connectedGAs.getField() != neighboursOfFirstGA.getField()) {
		LOG_ERROR("Unexpected changes happened TO GA0, maybe through actions or decisions. Will fail.");
		AS::quit();
		return false;
	}

	if (gaState_ptr->at(quantityGAs - 1).connectedGAs.getField() != neighboursOfLastGA.getField()) {
		LOG_ERROR("Unexpected changes happened to last GA, maybe through actions or decisions. Will fail.");
		AS::quit();
		return false;
	}

	bool changed = laState_ptr->at(0).parameters.strenght.externalGuard != externalGuardFirstLA;
	changed |= incomeFirstLA !=  laState_ptr->at(0).parameters.resources.updateRate;
	changed |= incomeLastLA != laState_ptr->at(quantityLAs - 1).parameters.resources.updateRate;
	changed |= strenghtFirstLA !=  laState_ptr->at(0).parameters.strenght.current;
	changed |= strenghtLastLA != laState_ptr->at(quantityLAs - 1).parameters.strenght.current;
	changed |= thresholdFirstLA !=  laState_ptr->at(0).parameters.strenght.thresholdToCostUpkeep;
	changed |= thresholdLastLA != laState_ptr->at(quantityLAs - 1).parameters.strenght.thresholdToCostUpkeep;

	if (changed) {
		LOG_ERROR("Unexpected changes happened to LAs, maybe through actions or decisions. Will fail.");
		AS::quit();
		return false;
	}

	//TODO: Check that diplomatic relations haven't changed either
		
	//Now we calculate the expected results, first for the LAs:
	
	float defaultUpkeep = AS::calculateUpkeep(DEFAULT_LA_STRENGHT, DEFAULT_REINFORCEMENT,
												     DEFAULT_LA_STR_THRESHOLD_FOR_UPKEEP);

	float LAtradeCoeficient = 1.0f/(MAX_LA_NEIGHBOURS/DEFAULT_LA_NEIGHBOUR_QUOTIENT); 

	float tradePerSecondFromOtherLAs = 
		AS::calculateTradeIncomePerSecondFromResources(LAtradeCoeficient, DEFAULT_LA_INCOME,
			                                                                  defaultUpkeep);
	float tradeFromOtherLAs = tradePerSecondFromOtherLAs * (float)adjustedTotalMultiplier;

	float expectedTradeFirstLA = tradeFromOtherLAs 
								 * MAX_LA_NEIGHBOURS/DEFAULT_LA_NEIGHBOUR_QUOTIENT;

	float expectedUpkeepFirstLA = AS::calculateUpkeep(DEFAULT_LA_STRENGHT, 
		                                              externalGuardFirstLA,
												      DEFAULT_LA_STR_THRESHOLD_FOR_UPKEEP);

	float expectedLiquidFirstLA = DEFAULT_LA_INCOME - expectedUpkeepFirstLA;
	float expectedTotalIncomeFirstLAMinusTrade = 
							expectedLiquidFirstLA * (float)adjustedTotalMultiplier;
	
	float expectedTotalResourcesFirstLA = startingResourcesFirstLA +
						expectedTradeFirstLA + expectedTotalIncomeFirstLAMinusTrade;

	float expectedTaxesFirstLA = (float)(GA_TAX_RATE_PER_SECOND * adjustedTotalMultiplier
		                  * (expectedTotalResourcesFirstLA + startingResourcesFirstLA)/2);
	expectedTaxesFirstLA -= (float)(GA_TAX_RATE_PER_SECOND * adjustedTotalMultiplier
		                            * expectedTaxesFirstLA); //second order effect
	expectedTotalResourcesFirstLA -= expectedTaxesFirstLA; 
	
	float tradePerSecondFromFirstLA = 
		AS::calculateTradeIncomePerSecondFromResources(LAtradeCoeficient, DEFAULT_LA_INCOME,
			                                                          expectedUpkeepFirstLA);
	float tradeFromFirstLA = tradePerSecondFromFirstLA * (float)adjustedTotalMultiplier;

	//int quantityOtherLAs = (MAX_LA_NEIGHBOURS/DEFAULT_LA_NEIGHBOUR_QUOTIENT) - 1;
	//float expectedTradeLastLA = quantityOtherLAs*tradeFromOtherLAs + tradeFromFirstLA;
	//Change in blocks means last LA is not a partner of the first LA anymore, so:
	float expectedTradeLastLA = 
		(MAX_LA_NEIGHBOURS/DEFAULT_LA_NEIGHBOUR_QUOTIENT) * tradeFromOtherLAs;

	float defaultLiquidIncome = DEFAULT_LA_INCOME - defaultUpkeep;

	float expectedTotalIncomeLastLAMinusTrade = 
							(float)(defaultLiquidIncome * adjustedTotalMultiplier);
	//zeroth order on taxes:
	float expectedTotalResourcesLastLA = DEFAULT_LA_RESOURCES + 
							expectedTotalIncomeLastLAMinusTrade + expectedTradeLastLA;
	//first order on taxes:
	float expectedTaxesLastLA = (float)(GA_TAX_RATE_PER_SECOND * adjustedTotalMultiplier
								* (expectedTotalResourcesLastLA + DEFAULT_LA_RESOURCES)/2);
	//second order on taxes:
	expectedTaxesLastLA -= (float)(GA_TAX_RATE_PER_SECOND * adjustedTotalMultiplier
						  * expectedTaxesLastLA);
	expectedTotalResourcesLastLA -= expectedTaxesLastLA; 
	
	//Then the GA expectations:

	//GA income depends on connected LAs and GAs totals
	//First GA by default has LA0 and the next few, which by default do not trade with LA0
	//So we can calculate a standard LAs expected tax and add connectedLAs-1 of that to LA0s:
	float expectedTradeDefaultLA = 
			(float)(TRADE_FACTOR_LA_PER_SECOND * adjustedTotalMultiplier * defaultLiquidIncome);
	float expectedTotalIncomeDefaultLAMinusTrade =
									(float)(defaultLiquidIncome * adjustedTotalMultiplier);
	float expectedTotalResourcesDefaultLA =  DEFAULT_LA_RESOURCES + 
			expectedTotalIncomeDefaultLAMinusTrade + expectedTradeDefaultLA;
	float expectedTaxesDefaultLA = (float)(GA_TAX_RATE_PER_SECOND * adjustedTotalMultiplier
								* (expectedTotalResourcesDefaultLA + DEFAULT_LA_RESOURCES)/2);
	
	int connectedLAsGA0 = gaState_ptr->at(0).localAgentsBelongingToThis.howManyAreOn();
	int connectedLAsLastGA = gaState_ptr->at(quantityGAs - 1).localAgentsBelongingToThis.howManyAreOn();

	float firstGAtaxGain = (connectedLAsGA0 - 1)*expectedTaxesDefaultLA 
		                   + expectedTaxesFirstLA;

	//For trade, we consider full trade with a GA with all standard taxes;
	//Note that by doing this we ignore second and higher order GA trade effects;
	float defaultGAtaxGain = connectedLAsGA0 * expectedTaxesDefaultLA;
	
	//GA's trade value is proportional to the tax they received last tick
	//time multiplier is already applied there
	
	float firstGAtradeGainFirstOrder = (float)(defaultGAtaxGain * TRADE_FACTOR_GA 
		                               * adjustedTotalMultiplier);

	float expectedTotalResourcesFirstGA = 
			startingResourcesFirstGA + firstGAtaxGain + firstGAtradeGainFirstOrder;

	//For the last GA we do the same, but using the last LA, and there's no trade;
	float lastGAtaxGain = (connectedLAsLastGA - 1)*expectedTaxesDefaultLA 
		                  + expectedTaxesLastLA;
	float expectedTotalResourcesLastGA = DEFAULT_GA_RESOURCES + lastGAtaxGain;
	
	//And finally check them:
	//TODO: notice that we have a 0.015f timeMultiplier grace factor
	float epsLA = 0.6f; //TODO: actual reasonable margin, this is a guess
	bool aux;
	aux = (fabs(expectedTotalResourcesFirstLA - laState_ptr->at(0).parameters.resources.current) <= epsLA);
	if(!aux){ 
		LOG_ERROR("First LA doesn't match expected"); 
		if (print) {
			printf("Expected: %f, Read: %f\n", expectedTotalResourcesFirstLA,
				laState_ptr->at(0).parameters.resources.current);
		}
	}
	result &= aux;
	aux = (fabs(expectedTotalResourcesLastLA - laState_ptr->at(quantityLAs - 1).parameters.resources.current) <= epsLA);	
	if(!aux){ 
		LOG_ERROR("Last LA doesn't match expected"); 
		if (print) {
			printf("Expected: %f, Read: %f\n", expectedTotalResourcesLastLA,
				laState_ptr->at(quantityLAs - 1).parameters.resources.current);
		}
	}
	result &= aux;

	float epsGA = 0.1f;
	aux = (fabs(expectedTotalResourcesFirstGA -  gaState_ptr->at(0).parameters.GAresources) <= epsGA);
	if(!aux){ 
		LOG_ERROR("First GA doesn't match expected"); 
		if (print) {
			printf("Expected: %f, Read: %f\n", expectedTotalResourcesFirstGA,
				gaState_ptr->at(0).parameters.GAresources);
		}
	}
	result &= aux;
	aux = (fabs(expectedTotalResourcesLastGA - gaState_ptr->at(quantityGAs - 1).parameters.GAresources) <= epsGA);	
	if(!aux){ 
		LOG_ERROR("Last GA doesn't match expected"); 
		if (print) {
			printf("Expected: %f, Read: %f\n", expectedTotalResourcesLastGA,
				gaState_ptr->at(quantityGAs - 1).parameters.GAresources);
		}
	}
	result &= aux;
	
	//And then save and show the results and wrap things up:
	bool couldSave = AS::saveNetworkToFile(updateTestOutputFilename, true, false, true);

	if (print) {
		printf("Ran for %llu ticks, total multiplier: %f (adj: %f)\n", 
			       ticksRan, totalMultiplier, adjustedTotalMultiplier);
		printf("LA %d: curr: %f (starting: %f, diff: %f);\nExpected: total liquid income: %f, total trade: %f, total taxes: %f;\n\tstr: %f (guard: %f, thresh: %f)\n",
			0,  laState_ptr->at(0).parameters.resources.current, startingResourcesFirstLA,
			((double)laState_ptr->at(0).parameters.resources.current - startingResourcesFirstLA),
			expectedTotalIncomeFirstLAMinusTrade, expectedTradeFirstLA, expectedTaxesFirstLA,
			laState_ptr->at(0).parameters.strenght.current, 
			laState_ptr->at(0).parameters.strenght.externalGuard,
			laState_ptr->at(0).parameters.strenght.thresholdToCostUpkeep);
		printf("LA %d: curr: %f (starting: %f, diff: %f);\nExpected: total liquid income: %f, total trade: %f, total taxes: %f;\n\tstr: %f (guard: %f, thresh: %f)\n",
			(quantityLAs - 1),  laState_ptr->at(quantityLAs - 1).parameters.resources.current, DEFAULT_LA_RESOURCES,
			((double)laState_ptr->at(quantityLAs - 1).parameters.resources.current - DEFAULT_LA_RESOURCES),
			expectedTotalIncomeLastLAMinusTrade, expectedTradeLastLA, expectedTaxesLastLA,
			laState_ptr->at(quantityLAs - 1).parameters.strenght.current, 
			laState_ptr->at(quantityLAs - 1).parameters.strenght.externalGuard,
			laState_ptr->at(quantityLAs - 1).parameters.strenght.thresholdToCostUpkeep);
		printf("\nGA %d: curr: %f (starting: %f, diff: %f, expected taxes: %f, expected trade: %f)\n",
			0, gaState_ptr->at(0).parameters.GAresources, startingResourcesFirstGA,
			((double)gaState_ptr->at(0).parameters.GAresources - startingResourcesFirstGA),
			firstGAtaxGain, firstGAtradeGainFirstOrder);
		printf("GA %d: curr: %f (starting: %f, diff: %f, expected taxes: %f, expected trade: %f)\n",
			(quantityGAs - 1), gaState_ptr->at(quantityGAs - 1).parameters.GAresources, startingResourcesLastGA,
			((double)gaState_ptr->at(quantityGAs - 1).parameters.GAresources - startingResourcesLastGA),
			lastGAtaxGain, 0.0f);
	}

	if (!result) {
		LOG_ERROR("Final results don't match expected. Test failed");
	}

	aux = AS::quit();
	if (!aux) {
		LOG_ERROR("Failed to quit test network or main loop found errors. Aborting test");
		return false;
	}

	if (!couldSave) {
		LOG_ERROR("Saving of updated network failed. Test will fail.");
		return false;
	}

	if(result) { LOG_INFO("Test passed!"); }
	return result;
}

#define TST_TICKS_MAINLOOP_ERRORS (AS_TOTAL_CHOPS*25)
bool testMainLoopErrors(std::string filename) {
	
	LOG_DEBUG("Will load a network, run for several ticks, stop it, and check for errors\n", 1);
	GETCHAR_PAUSE;
	bool result = AS::loadNetworkFromFile(filename, false);
	if (!result) {
		LOG_ERROR("Failed to Load test network. Aborting test");
		return false;
	}

	AS::run();
	
	int ticks = TST_TICKS_MAINLOOP_ERRORS;
	std::chrono::microseconds microsToSleep(ticks*AS_MILLISECONDS_PER_STEP*MICROS_IN_A_MILLI);
	AZ::hybridBusySleepForMicros(microsToSleep);

	result = AS::quit();

	if(!result){ LOG_ERROR("Main Loop raised errors or failed while quitting"); }
	else { LOG_INFO("Main Loop ran and quit without raising any errors"); }

	return result;
}

bool testPause(bool printLog, int pauseUnpauseCycles) {

	LOG_DEBUG("Will test pausing and resuming the main loop\n", 1);
	GETCHAR_PAUSE;

	if (!AS::isMainLoopRunning()) {
		LOG_ERROR("This test needs main loop to already be running. Aborting");
		return false;
	}

	const CL::mirror_t* mp = CL::ASmirrorData_cptr;
	if (mp == NULL) {
		LOG_ERROR("Failed to acquire const pointer to mirror Data");
		return false;
	}

	for(int i = 0; i < pauseUnpauseCycles; i++){

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
			printf("Cycle: %d\n", i);
			return false;
		}
		else if ((ticksNow - ticksAfterPause) < (periodsToWaitAfterUnpause/pauseGraceFactor)) {
			LOG_ERROR("AS is ticking too slow after unpausing");
			if (printLog) {
				printf("expected to tick about %d times, but ticked %lld\n",
					periodsToWaitAfterUnpause, ticksNow - ticksAfterPause);
			}
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

	LOG_DEBUG("Will try to send data through CL's Client Data Handler\n", 1);
	GETCHAR_PAUSE;

	if (!AS::isMainLoopRunning()) {
		LOG_ERROR("This test expects the mainLoop to be running");
		return false;
	}

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

	result = AS::saveNetworkToFile(fileName, false);
	if (!result) {
		LOG_ERROR("Failed to save network. Test will pass, but fix this to actually check changes");
	}
	else {
		LOG_INFO("Network saved to ClientDataHandler_*customFilename*");
	}

	return result;
}

bool testClientDataHAndlerInitialization(void) {
	
	LOG_DEBUG("Will test ClientDataHandler acquisition\n", 1);
	GETCHAR_PAUSE;

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

	LOG_DEBUG("Will check if reader thread's results are as expected. May need to wait for execution to finish\n",1);
	GETCHAR_PAUSE;

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

	LOG_DEBUG("Will spawn a new thread which will try to read a few times from CL the number of ticks while AS's main loop runs\n", 1);
	GETCHAR_PAUSE;

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
	
	LOG_DEBUG("TA will try to change some data in CL so AS can check it...\n",1);
	GETCHAR_PAUSE;
	
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

	bool result = cp->networkParameters.changeCommentTo(newComment.c_str());
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

	LOG_TRACE("Will issue new shouldMakeDecisions to first LA");

	cp->LAdecision.changeShouldMakeDecisions(0, false);

	LOG_INFO("All test changes issued...");

	return AS::testGotNewValuesFromASthroughCL();
}

bool testReadingCLdataFromTA(void) {

	LOG_DEBUG("TA will try to read data from CL...\n", 1);
	GETCHAR_PAUSE;

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

	bool resultAux = (firstGAaction.phaseTiming.elapsed == 0);
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

	LOG_DEBUG("TA will querry wether CL's Data Controllers are properly initialized...\n", 1);
	GETCHAR_PAUSE;

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

	int GAquantity = CL::ASmirrorData_cptr->networkParams.numberGAs - 1; //last doesn't count
	int LAquantity = CL::ASmirrorData_cptr->networkParams.numberLAs;
	int maxActions = CL::ASmirrorData_cptr->networkParams.maxActions;

	failed += 1*(capLAact != (LAquantity * maxActions * sizeLAact));
	failed += 2*(capGAact != (GAquantity * maxActions * sizeGAact));

	//TODO-CRITICAL: GA's are counting the "phantom" last one
	failed += 4*(capLAcold != (MAX_LA_QUANTITY* sizeLAcold));
	failed += 8*(capGAcold != (MAX_GA_QUANTITY* sizeGAcold));
	failed += 16*(capLAstate != (MAX_LA_QUANTITY* sizeLAstate));
	failed += 32*(capGAstate != (MAX_GA_QUANTITY*sizeGAstate));
	failed += 64*(capLAdecs != (MAX_LA_QUANTITY* sizeLAdecs));
	failed += 128*(capGAdecs != (MAX_GA_QUANTITY* sizeGAdecs));

	if (failed) {
		LOG_ERROR("Some CL data controller capacities don't match the expected!");

		#if (defined AS_DEBUG) || VERBOSE_RELEASE
			printf("Failed some of the 8 checks. Failures id: %d\nsizeLAact; %zi, sizeGAact: %zi\n", 
															failed, sizeLAact, sizeGAact);
			printf("capLAact: %zi, expected: %zi - capGAact: %zi, expected: %zi\n",
				capLAact, (LAquantity * maxActions * sizeLAact), capGAact,
				(GAquantity * maxActions * sizeGAact));
			printf("Note: may have failed some other test related to agent data instead\n");
		#endif // AS_DEBUG

		GETCHAR_PAUSE;
		return false;
	}
	
	LOG_INFO("CL data controller capacities are as expected. CL Initialization seems ok : )");
	return true;
}

bool testMockData(void) {

	LOG_DEBUG("Sanity checks for the communication between APP and DLLs (with mock initialization)\n", 1);
	GETCHAR_PAUSE;

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
	//TODO: sayHello()s should return char*m to be tested and printed
	//This way we can programatically test the results
	//The strings should be predifined

	LOG_DEBUG("We will test basic communication with the dlls\n",1);
	GETCHAR_PAUSE;

	AS::sayHello();

	CL::sayHelloExternal();
	
	LOG_INFO("AS, CL-internal and CL-external should have said hello above : )");
}