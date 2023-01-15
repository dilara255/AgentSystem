#include "miscStdHeaders.h"
#include "core.hpp"

#include "AS_API.hpp"
#include "CL_ExternalAPI.hpp"

void testSayHello(void);
bool testMockData(void);
bool testFromTAifCLhasInitialized(void);
bool testReadingCLdataFromTA(void);
bool testChangingCLdataFromTAandRetrievingFromAS(void);

#define MS_DEBUG_MALLOC_INIT_VALUE (-842150451) //WARNING: not portable, but used only for testing
#define BASIC_INIT_COMM_TESTS 4
#define SPECIFIC_DATA_FUNCTIONALITY_TESTS 7
#define SPECIFIC_THREADED_LOOP_TESTS 3
#define TOTAL_TESTS (BASIC_INIT_COMM_TESTS+SPECIFIC_DATA_FUNCTIONALITY_TESTS+SPECIFIC_THREADED_LOOP_TESTS)

#ifdef AS_DEBUG
	#define GETCHAR_PAUSE getchar();
#else
	#define GETCHAR_PAUSE puts("\n");
#endif // AS_DEBUG


//TO DO: make all tests return bool and count results, than match with expected
int main(void) {
	
	testSayHello();

	LOG_INFO("AS, CL-internal and CL-external should have said hello above : )");
	LOG_TRACE("This will run a few batteries of tests..."); GETCHAR_PAUSE

	LOG_INFO("Basic App, AS and CL communicaton and data storage tests:\n\n",2); GETCHAR_PAUSE
	int resultsBattery1 = (int)testMockData(); GETCHAR_PAUSE
	
	LOG_TRACE("Actual initialization tests...");
	resultsBattery1 += (int)AS::initializeASandCL(); GETCHAR_PAUSE

	resultsBattery1 += (int)AS::testContainersAndAgentObjectCreation(); GETCHAR_PAUSE

	resultsBattery1 += (int)testFromTAifCLhasInitialized(); GETCHAR_PAUSE

	if (resultsBattery1 != BASIC_INIT_COMM_TESTS) {
		LOG_CRITICAL("Not all of these tests passed:");
		printf("%i out of %i failed", BASIC_INIT_COMM_TESTS - resultsBattery1, BASIC_INIT_COMM_TESTS);
		GETCHAR_PAUSE
	}
	else {
		LOG_INFO("All of these tests passed!"); GETCHAR_PAUSE
	}

	LOG_INFO("Specific functionality tests (DATA manipulation):\n\n",2); GETCHAR_PAUSE
	
	int resultsBattery2 = (int)AS::testFileCreation(fileNameNoDefaults, fileNameWithDefaults); 
	GETCHAR_PAUSE

	resultsBattery2 += (int)AS::loadNetworkFromFile(fileNameWithDefaults); GETCHAR_PAUSE

	resultsBattery2 += (int)AS::saveNetworkToFile(); GETCHAR_PAUSE

	resultsBattery2 += (int)AS::testDataTransferFromAStoCL(); GETCHAR_PAUSE

	resultsBattery2 += (int)testReadingCLdataFromTA(); GETCHAR_PAUSE

	resultsBattery2 += (int)testChangingCLdataFromTAandRetrievingFromAS(); GETCHAR_PAUSE

	resultsBattery2 += (int)AS::saveNetworkToFile(customFilename, true); GETCHAR_PAUSE
	
	if (resultsBattery2 != SPECIFIC_DATA_FUNCTIONALITY_TESTS) {
		LOG_CRITICAL("Not all of these tests passed:");
		printf("%i out of %i failed", SPECIFIC_DATA_FUNCTIONALITY_TESTS - resultsBattery2, 
			                                             SPECIFIC_DATA_FUNCTIONALITY_TESTS);
		GETCHAR_PAUSE
	}
	else {
		LOG_INFO("All of these tests passed!"); GETCHAR_PAUSE
	}

	LOG_INFO("Specific functionality tests (THREADED LOOPs):\n\n",2); GETCHAR_PAUSE

	int resultsBattery3 = (int)AS::loadNetworkFromFile(customFilename, true);
	GETCHAR_PAUSE;
	
	resultsBattery3 += (int)AS::saveNetworkToFile(customFilename, true); GETCHAR_PAUSE;

	resultsBattery3 += (int)AS::quit(); GETCHAR_PAUSE;

	if (resultsBattery3 != SPECIFIC_THREADED_LOOP_TESTS) {
		LOG_CRITICAL("Not all of these tests passed:");
		printf("%i out of %i failed", SPECIFIC_THREADED_LOOP_TESTS - resultsBattery2,
			SPECIFIC_THREADED_LOOP_TESTS);
		GETCHAR_PAUSE
	}
	else {
		LOG_INFO("All of these tests passed!"); GETCHAR_PAUSE
	}

	LOG_TRACE("Tests ended...\n",1); GETCHAR_PAUSE

	int totalPassed = resultsBattery1 + resultsBattery2 + resultsBattery3;
	if (totalPassed == TOTAL_TESTS) {
		LOG_INFO("All automatically checked tests passed!"); GETCHAR_PAUSE
	}
	else {
		LOG_CRITICAL("Not all tests were passed (as far as we checked)!");
		printf("%i out of %i failed\n", TOTAL_TESTS - totalPassed, TOTAL_TESTS);
		GETCHAR_PAUSE
	}

	LOG_WARN("Check that you have (at least): one network file with format specifiers,\none with default values and one with modified values:");
	printf("\t-The one with specifiers is %s\n\t-%s should have the defaults\n\t-%s received modifications from TA\n",
		                              fileNameNoDefaults, fileNameWithDefaults, customFilename);
	printf("\nThe modified file has different data:\n\t-The comment's first letter should be a %c;\n\t-Ticks should be the number of times mainLoopTrhead looped before last save;\n",TST_COMMENT_LETTER_CHANGE);
	printf("\t-Last GA`s id = %i and connected GAs = %i;\n", TST_GA_ID, TST_GA_CONNECTIONS);
	printf("\t-Last LA`s reinforcement = % f, offset[%i][%i][1] = % f and last actions aux = % i.\n",
		TST_LA_REINFORCEMENT, TST_CHANGED_CATEGORY, TST_CHANGED_MODE, TST_LA_OFFSET, TST_LAST_ACTION_AUX);
	getchar();

	LOG_INFO("Done! Enter to exit"); getchar();
	
	return (1 + (totalPassed - TOTAL_TESTS));
}

bool testChangingCLdataFromTAandRetrievingFromAS(void) {

	LOG_INFO("TA will try to change some data in CL so AS can check it...");

	CL::mirrorData_ptr->networkParams.comment[0] = TST_COMMENT_LETTER_CHANGE;

	GA::coldData_t newGAcoldData = CL::mirrorData_ptr->agentMirrorPtrs.GAcoldData_ptr->data.back();
	CL::mirrorData_ptr->agentMirrorPtrs.GAcoldData_ptr->data.pop_back();
	newGAcoldData.id = TST_GA_ID;
	CL::mirrorData_ptr->agentMirrorPtrs.GAcoldData_ptr->data.push_back(newGAcoldData);

	GA::stateData_t newGAstate = CL::mirrorData_ptr->agentMirrorPtrs.GAstate_ptr->data.back();
	CL::mirrorData_ptr->agentMirrorPtrs.GAstate_ptr->data.pop_back();
	newGAstate.connectedGAs.loadField(TST_GA_CONNECTIONS);
	CL::mirrorData_ptr->agentMirrorPtrs.GAstate_ptr->data.push_back(newGAstate);

	LA::stateData_t newLAdstate = CL::mirrorData_ptr->agentMirrorPtrs.LAstate_ptr->data.back();
	CL::mirrorData_ptr->agentMirrorPtrs.LAstate_ptr->data.pop_back();
	newLAdstate.parameters.strenght.externalGuard = TST_LA_REINFORCEMENT;
	CL::mirrorData_ptr->agentMirrorPtrs.LAstate_ptr->data.push_back(newLAdstate);


	LA::decisionData_t newLAdecision = CL::mirrorData_ptr->agentMirrorPtrs.LAdecision_ptr->data.back();
	CL::mirrorData_ptr->agentMirrorPtrs.LAdecision_ptr->data.pop_back();
	newLAdecision.offsets.incentivesAndConstraintsFromGA[TST_CHANGED_CATEGORY][TST_CHANGED_MODE] 
		                                                               = (float)TST_LA_OFFSET;
	CL::mirrorData_ptr->agentMirrorPtrs.LAdecision_ptr->data.push_back(newLAdecision);

	AS::actionData_t newLAaction = CL::mirrorData_ptr->actionMirror.dataLAs.back();
	CL::mirrorData_ptr->actionMirror.dataLAs.pop_back();
	newLAaction.details.processingAux = TST_LAST_ACTION_AUX;
	CL::mirrorData_ptr->actionMirror.dataLAs.push_back(newLAaction);

	return AS::testGotNewValuesFromASthroughCL();
}

bool testReadingCLdataFromTA(void) {

	LOG_TRACE("TA will try to read data from CL...");

	bool result = CL::mirrorData_ptr->agentMirrorPtrs.haveData;
	result &= CL::mirrorData_ptr->networkParams.isNetworkInitialized;
	result &= CL::mirrorData_ptr->actionMirror.hasData();
	if (!result) {
		LOG_ERROR("CL Mirror data controllers say they have no data!");
		return false;
	}
	LOG_TRACE("Data Flags ok. Will try to read data...");

	result = (CL::mirrorData_ptr->networkParams.numberGAs == TST_NUMBER_GAS);
	result &= (CL::mirrorData_ptr->networkParams.numberLAs == TST_NUMBER_LAS);
	if (!result) {
		LOG_ERROR("Agent amounts were not as expected according to CL. Aborting test.");
		return false;
	}

	//strcmp returns 0 on perfect match
	if (strcmp(CL::mirrorData_ptr->networkParams.name, fileNameWithDefaults) != 0) {
		LOG_ERROR("Network name doesn't match expected.");
		result = false;
	}
	
	AS::actionData_t firstGAaction =
		CL::mirrorData_ptr->actionMirror.dataGAs[0];
	AS::actionData_t lastLAaction =
		CL::mirrorData_ptr->actionMirror.dataLAs[(TST_NUMBER_LAS * MAX_ACTIONS_PER_AGENT) - 1];

	bool resultAux = (firstGAaction.ticks.initial == DEFAULT_FIRST_TICK);
	resultAux &= (lastLAaction.details.processingAux == DEFAULT_ACTION_AUX);
	if (!resultAux) {
		LOG_ERROR("Test action data doesn't match expected.");
		result = false;
	}

	GA::coldData_t firstGAcold = CL::mirrorData_ptr->agentMirrorPtrs.GAcoldData_ptr->data[0];
	LA::decisionData_t lastLAdecision = CL::mirrorData_ptr->agentMirrorPtrs.LAdecision_ptr->data[TST_NUMBER_LAS-1];

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

	LOG_TRACE("TA will querry wether CL's Data Controller are properly initialized...");

	bool result = CL::mirrorData_ptr->actionMirror.isInitialized();
	result &= CL::mirrorData_ptr->agentMirrorPtrs.haveBeenCreated;
	if (!result) {
		LOG_ERROR("CL Mirror data controllers not initialized or bad pointer");
		return false;
	}
	LOG_TRACE("Initialization flags ok. Checking capacities...");
	
	//Dividing by sizeof to get back to quantity of items, which is easier to check expected
	size_t capLAact = CL::mirrorData_ptr->actionMirror.capacityForDataInBytesLAs();
	size_t sizeLAact = sizeof(CL::mirrorData_ptr->actionMirror.dataLAs[0]);

	size_t capGAact = CL::mirrorData_ptr->actionMirror.capacityForDataInBytesGAs();
	size_t sizeGAact = sizeof(CL::mirrorData_ptr->actionMirror.dataGAs[0]);

	size_t capLAcold = CL::mirrorData_ptr->agentMirrorPtrs.LAcoldData_ptr->capacityForDataInBytes();
	size_t sizeLAcold = sizeof(CL::mirrorData_ptr->agentMirrorPtrs.LAcoldData_ptr->data[0]);

	size_t capGAcold = CL::mirrorData_ptr->agentMirrorPtrs.GAcoldData_ptr->capacityForDataInBytes();
	size_t sizeGAcold = sizeof(CL::mirrorData_ptr->agentMirrorPtrs.GAcoldData_ptr->data[0]);

	size_t capLAstate = CL::mirrorData_ptr->agentMirrorPtrs.LAstate_ptr->capacityForDataInBytes();
	size_t sizeLAstate = sizeof(CL::mirrorData_ptr->agentMirrorPtrs.LAstate_ptr->data[0]);

	size_t capGAstate = CL::mirrorData_ptr->agentMirrorPtrs.GAstate_ptr->capacityForDataInBytes();
	size_t sizeGAstate = sizeof(CL::mirrorData_ptr->agentMirrorPtrs.GAstate_ptr->data[0]);

	size_t capLAdecs = CL::mirrorData_ptr->agentMirrorPtrs.LAdecision_ptr->capacityForDataInBytes();
	size_t sizeLAdecs = sizeof(CL::mirrorData_ptr->agentMirrorPtrs.LAdecision_ptr->data[0]);

	size_t capGAdecs = CL::mirrorData_ptr->agentMirrorPtrs.GAdecision_ptr->capacityForDataInBytes();
	size_t sizeGAdecs = sizeof(CL::mirrorData_ptr->agentMirrorPtrs.GAdecision_ptr->data[0]);

	int failed = 0;

	failed += capLAact != (MAX_LA_QUANTITY * MAX_ACTIONS_PER_AGENT * sizeLAact);
	failed += capGAact != (MAX_GA_QUANTITY * MAX_ACTIONS_PER_AGENT * sizeGAact);

	failed += capLAcold != (MAX_LA_QUANTITY* sizeLAcold);
	failed += capGAcold != (MAX_GA_QUANTITY* sizeGAcold);
	failed += capLAstate != (MAX_LA_QUANTITY* sizeLAstate);
	failed += capGAstate != (MAX_GA_QUANTITY*sizeGAstate);
	failed += capLAdecs != (MAX_LA_QUANTITY* sizeLAdecs);
	failed += capGAdecs != (MAX_GA_QUANTITY* sizeGAdecs);

	if (failed) {
		LOG_ERROR("Some CL data controller capacities don't match the expected!");

		#ifdef AS_DEBUG
			printf("Failed on %i out of 8 checks\nsizeLAact; %zi, sizeGAact: %zi\n", 
															failed, sizeLAact, sizeGAact);
			printf("capLAact: %zi, expected: %zi - capGAact: %zi, expected: %zi\n",
				capLAact, (MAX_LA_QUANTITY * MAX_ACTIONS_PER_AGENT * sizeLAact), capGAact,
				(MAX_GA_QUANTITY * MAX_ACTIONS_PER_AGENT* sizeLAact));
		#endif // AS_DEBUG

		GETCHAR_PAUSE
		return false;
	}
	
	LOG_INFO("CL data controller capacities are as expected. CL Initialization seems ok : )");
	return true;
}

bool testMockData(void) {

	LOG_TRACE("Sanity checks for the communication between APP and DLLs (with mock initialization");

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

	#ifdef AS_DEBUG
		printf("TstArray[0]: %i\nTstArray[1]: %i\n",
			   tstArray_ptr[0], tstArray_ptr[1]);
	#endif

	return result;
}

void testSayHello(void) {
	
	AS::sayHello();

	CL::sayHelloExternal();
	
	GETCHAR_PAUSE
}