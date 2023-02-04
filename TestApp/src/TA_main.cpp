#include "miscStdHeaders.h"
#include "core.hpp"

#include "AS_API.hpp"
#include "CL_ExternalAPI.hpp"

void testSayHello(void);
bool testMockData(void);
bool testFromTAifCLhasInitialized(void);
bool testReadingCLdataFromTA(void);
bool testChangingCLdataFromTAandRetrievingFromAS(void);
bool testReadingTickDataWhileASmainLoopRuns_start(void);
bool testReadingTickDataWhileASmainLoopRuns_end(void);
bool testSendingClientDataAndSaving(void);
bool testClientDataHAndlerInitialization(void);

#define MS_DEBUG_MALLOC_INIT_VALUE (-842150451) //WARNING: not portable, but used only for testing
#define BASIC_INIT_COMM_TESTS 4
#define SPECIFIC_DATA_FUNCTIONALITY_TESTS 7
#define SPECIFIC_THREADED_LOOP_TESTS 7
#define TOTAL_TESTS (BASIC_INIT_COMM_TESTS+SPECIFIC_DATA_FUNCTIONALITY_TESTS+SPECIFIC_THREADED_LOOP_TESTS)

std::thread reader;//to test realtime reading of data trough CL as AS runs
uint64_t g_ticksRead[TST_TIMES_TO_QUERRY_TICK]; 

CL::ClientData::Handler* cdHandler_ptr;

int main(void) {

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
	
	if (resultsBattery2 != SPECIFIC_DATA_FUNCTIONALITY_TESTS) {
		LOG_CRITICAL("Not all of these tests passed:");
		printf("%d out of %d failed", SPECIFIC_DATA_FUNCTIONALITY_TESTS - resultsBattery2, 
			                                             SPECIFIC_DATA_FUNCTIONALITY_TESTS);
		GETCHAR_PAUSE;
	}
	else {
		LOG_INFO("All of these tests passed!"); GETCHAR_PAUSE;
	}

	LOG_INFO("Specific functionality tests (THREADED LOOPs):\n",1); GETCHAR_PAUSE;

	LOG_WARN("Will load the previously modified network");
	int resultsBattery3 = (int)AS::loadNetworkFromFile(customFilename, true);
	GETCHAR_PAUSE;

	resultsBattery3 += (int)testReadingTickDataWhileASmainLoopRuns_start(); GETCHAR_PAUSE;
	
	resultsBattery3 += (int)testReadingTickDataWhileASmainLoopRuns_end(); GETCHAR_PAUSE;

	resultsBattery3 += (int)AS::saveNetworkToFile(customFilename, true); GETCHAR_PAUSE;

	resultsBattery3 += (int)testClientDataHAndlerInitialization(); GETCHAR_PAUSE;

	resultsBattery3 += (int)testSendingClientDataAndSaving(); GETCHAR_PAUSE;

	resultsBattery3 += (int)AS::quit(); GETCHAR_PAUSE;

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

	int totalPassed = resultsBattery1 + resultsBattery2 + resultsBattery3;
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

bool testSendingClientDataAndSaving(void) {
	LOG_WARN("Will try to send data through CL's Client Data Handler");

	uint32_t id = CL::ASmirrorData_ptr->networkParams.numberLAs - 1;
	
	LOG_TRACE("Calculated last LA's ID from AS mirror data");

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

	cdHandler_ptr = CL::getClientDataHandlerPtr();

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

	g_ticksRead[0] = CL::ASmirrorData_ptr->networkParams.mainLoopTicks;

	for (int i = 1; i < numberTicks; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(TST_TA_QUERY_FREQUENCY_MS));
		g_ticksRead[i] = CL::ASmirrorData_ptr->networkParams.mainLoopTicks;
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
	
	if (reader.joinable()) {
		reader.join();
	}
	else {
		LOG_WARN("Reader thread seem innactive. If it wasn't created, this test will make no sense!");
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
	bool result = (fails < TST_TIMES_TO_QUERRY_TICK / TST_TICK_COUNT_SAFETY_FACTOR);

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
	for (int i = 0; i < TST_TIMES_TO_QUERRY_TICK; i++) {
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

	const CL::mirror_t* mp = CL::ASmirrorData_ptr;
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

	for (int i = 0; i < AS::TOTAL_CATEGORIES; i++) {
		for (int j = 0; j < AS::TOTAL_MODES; j++) {

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

	bool result = CL::ASmirrorData_ptr->agentMirrorPtrs.haveData;
	result &= CL::ASmirrorData_ptr->networkParams.isNetworkInitialized;
	result &= CL::ASmirrorData_ptr->actionMirror.hasData();
	if (!result) {
		LOG_ERROR("CL Mirror data controllers say they have no data!");
		return false;
	}
	LOG_TRACE("Data Flags ok. Will try to read data...");

	result = (CL::ASmirrorData_ptr->networkParams.numberGAs == TST_NUMBER_GAS);
	result &= (CL::ASmirrorData_ptr->networkParams.numberLAs == TST_NUMBER_LAS);
	if (!result) {
		LOG_ERROR("Agent amounts were not as expected according to CL. Aborting test.");
		return false;
	}

	//strcmp returns 0 on perfect match
	if (strcmp(CL::ASmirrorData_ptr->networkParams.name, fileNameWithDefaults) != 0) {
		LOG_ERROR("Network name doesn't match expected.");
		result = false;
	}
	
	AS::actionData_t firstGAaction =
		CL::ASmirrorData_ptr->actionMirror.dataGAs[0];
	AS::actionData_t lastLAaction =
		CL::ASmirrorData_ptr->actionMirror.dataLAs[(TST_NUMBER_LAS * MAX_ACTIONS_PER_AGENT) - 1];

	bool resultAux = (firstGAaction.ticks.initial == DEFAULT_FIRST_TICK);
	resultAux &= (lastLAaction.details.processingAux == DEFAULT_ACTION_AUX);
	if (!resultAux) {
		LOG_ERROR("Test action data doesn't match expected.");
		result = false;
	}

	GA::coldData_t firstGAcold = CL::ASmirrorData_ptr->agentMirrorPtrs.GAcoldData_ptr->data[0];
	LA::decisionData_t lastLAdecision = CL::ASmirrorData_ptr->agentMirrorPtrs.LAdecision_ptr->data[TST_NUMBER_LAS-1];

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

	bool result = CL::ASmirrorData_ptr->actionMirror.isInitialized();
	result &= CL::ASmirrorData_ptr->agentMirrorPtrs.haveBeenCreated;
	if (!result) {
		LOG_ERROR("CL Mirror data controllers not initialized or bad pointer");
		return false;
	}
	LOG_TRACE("Initialization flags ok. Checking capacities...");
	
	//Dividing by sizeof to get back to quantity of items, which is easier to check expected
	size_t capLAact = CL::ASmirrorData_ptr->actionMirror.capacityForDataInBytesLAs();
	size_t sizeLAact = sizeof(CL::ASmirrorData_ptr->actionMirror.dataLAs[0]);

	size_t capGAact = CL::ASmirrorData_ptr->actionMirror.capacityForDataInBytesGAs();
	size_t sizeGAact = sizeof(CL::ASmirrorData_ptr->actionMirror.dataGAs[0]);

	size_t capLAcold = CL::ASmirrorData_ptr->agentMirrorPtrs.LAcoldData_ptr->capacityForDataInBytes();
	size_t sizeLAcold = sizeof(CL::ASmirrorData_ptr->agentMirrorPtrs.LAcoldData_ptr->data[0]);

	size_t capGAcold = CL::ASmirrorData_ptr->agentMirrorPtrs.GAcoldData_ptr->capacityForDataInBytes();
	size_t sizeGAcold = sizeof(CL::ASmirrorData_ptr->agentMirrorPtrs.GAcoldData_ptr->data[0]);

	size_t capLAstate = CL::ASmirrorData_ptr->agentMirrorPtrs.LAstate_ptr->capacityForDataInBytes();
	size_t sizeLAstate = sizeof(CL::ASmirrorData_ptr->agentMirrorPtrs.LAstate_ptr->data[0]);

	size_t capGAstate = CL::ASmirrorData_ptr->agentMirrorPtrs.GAstate_ptr->capacityForDataInBytes();
	size_t sizeGAstate = sizeof(CL::ASmirrorData_ptr->agentMirrorPtrs.GAstate_ptr->data[0]);

	size_t capLAdecs = CL::ASmirrorData_ptr->agentMirrorPtrs.LAdecision_ptr->capacityForDataInBytes();
	size_t sizeLAdecs = sizeof(CL::ASmirrorData_ptr->agentMirrorPtrs.LAdecision_ptr->data[0]);

	size_t capGAdecs = CL::ASmirrorData_ptr->agentMirrorPtrs.GAdecision_ptr->capacityForDataInBytes();
	size_t sizeGAdecs = sizeof(CL::ASmirrorData_ptr->agentMirrorPtrs.GAdecision_ptr->data[0]);

	int failed = 0;

	int GAquantity = CL::ASmirrorData_ptr->networkParams.numberGAs; //already corrected
	int LAquantity = CL::ASmirrorData_ptr->networkParams.numberLAs;
	int maxActions = CL::ASmirrorData_ptr->networkParams.maxActions;

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