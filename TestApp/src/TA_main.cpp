#include "miscStdHeaders.h"
#include "core.hpp"

#include "AS_API.hpp"
#include "CL_ExternalAPI.hpp"

void test1SayHello(void);
bool test2Initialization(void);
bool test3PointersToMockData(void);

#define MS_DEBUG_MALLOC_INIT_VALUE (-842150451) //WARNING: not portable, but used only for testing
#define BASIC_INIT_COMM_TESTS 3
#define SPECIFIC_DATA_FUNCTIONALITY_TESTS 2
#define TOTAL_TESTS (BASIC_INIT_COMM_TESTS+SPECIFIC_DATA_FUNCTIONALITY_TESTS)

//TO DO: make all tests return bool and count results, than match with expected
int main(void) {
	
	test1SayHello();

	LOG_INFO("AS, CL-internal and CL-external should have said hello above...\n");
	getchar();

	LOG_WARN("\n\nWill try loading file before initialization... should fail");
	AS::loadNetworkFromFile(fileNameNoDefaults);
	LOG_WARN("Failing should happen above. Tests bellow should pass...\n\n");
	getchar();

	LOG_INFO("\n\nBasic App, AS and CL communicaton and data storage tests:\n\n");
	int resultsBattery1 = (int)test2Initialization();
	getchar();
	resultsBattery1 += (int)test3PointersToMockData();
	getchar();
	resultsBattery1 += (int)AS::testContainersAndAgentObjectCreation();
	getchar();
	if (resultsBattery1 != BASIC_INIT_COMM_TESTS) {
		LOG_CRITICAL("Not all of these tests passed:");
		printf("%i out of %i failed", BASIC_INIT_COMM_TESTS - resultsBattery1, BASIC_INIT_COMM_TESTS);
	}
	else {
		LOG_INFO("All of these tests passed!");
	}
	getchar();

	LOG_INFO("\n\nSpecific functionality tests (DATA manipulation):\n\n");
	int resultsBattery2 = (int)AS::testFileCreation();
	getchar();
	resultsBattery2 += (int)AS::loadNetworkFromFile(fileNameWithDefaults);
	getchar();
	if (resultsBattery2 != SPECIFIC_DATA_FUNCTIONALITY_TESTS) {
		LOG_CRITICAL("Not all of these tests passed:");
		printf("%i out of %i failed", SPECIFIC_DATA_FUNCTIONALITY_TESTS - resultsBattery2, 
			                     SPECIFIC_DATA_FUNCTIONALITY_TESTS);
	}
	else {
		LOG_INFO("All of these tests passed!");
	}
	getchar();

	int totalPassed = resultsBattery1 + resultsBattery2;
	if (totalPassed == TOTAL_TESTS) {
		LOG_INFO("All automatically checked tests passed!");
	}
	else {
		LOG_CRITICAL("Not all automatically checked tests were passed!");
		printf("%i out of %i failed\n", TOTAL_TESTS - totalPassed, TOTAL_TESTS);
	}

	LOG_TRACE("Tests ended. Press return to exit");
	getchar();
	return 1;
}

bool test3PointersToMockData(void) {
	bool result;

	int* tstArray_ptr = CL::getTestArrayPtr();
	if (tstArray_ptr[0] == AS_TST_EXPECTED_ARR0 && tstArray_ptr[1] == AS_TST_EXPECTED_ARR1) {
		LOG_INFO("AS test array numbers relayed correctly");
		result = true;
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

bool test2Initialization(void) {

	AS::initializeASandCL();

	AS::CLinitTest();

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

	return result;
}

void test1SayHello(void) {
	
	AS::sayHello();

	CL::sayHelloExternal();
	
	getchar();
	return;
}