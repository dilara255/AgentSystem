#include "miscStdHeaders.h"
#include "core.hpp"

#include "AS_API.hpp"
#include "CL_ExternalAPI.hpp"

void test1SayHello(void);
void test2Initialization(void);
void test3PointersToMockData(void);

#define MS_DEBUG_MALLOC_INIT_VALUE (-842150451) //WARNING: not portable, but used only for testing

//TO DO: make all tests return bool and count results, than match with expected
int main(void) {
	
	test1SayHello();
	getchar();

	LOG_WARN("\n\nWill try loading file before initialization... should fail");
	AS::loadNetworkFromFile(fileNameNoDefaults);
	LOG_WARN("Failing should happen above. Tests bellow should pass\n\n");
	getchar();

	//these tests poke at wether communication between the app and DLLs is working and 
	//data is being held as expected:
	test2Initialization();
	getchar();
	test3PointersToMockData();
	AS::testContainersAndAgentObjectCreation();
	getchar();

	//these are tests of specific functionality:
	AS::testFileCreation();
	getchar();
	AS::loadNetworkFromFile(fileNameNoDefaults);


	LOG_TRACE("Tests ended. Press return to exit");
	getchar();
	return 1;
}

void test3PointersToMockData(void) {

	int* tstArray_ptr = CL::getTestArrayPtr();
	if (tstArray_ptr[0] == AS_TST_EXPECTED_ARR0 && tstArray_ptr[1] == AS_TST_EXPECTED_ARR1) {
		LOG_INFO("AS test array numbers relayed correctly");
	}
	else {
		LOG_ERROR("AS test array numbers relayed INCORRECTLY");
		if (tstArray_ptr[0] == MS_DEBUG_MALLOC_INIT_VALUE) {
			LOG_TRACE("(it seems like no value was copied to CL's test array)");
		}
	}

#ifdef AS_DEBUG
	printf("TstArray[0]: %i\nTstArray[1]: %i\n",
		   tstArray_ptr[0], tstArray_ptr[1]);
#endif

	getchar();
	return;
}

void test2Initialization(void) {

	AS::initializeASandCL();

	AS::CLinitTest();

	int numberAS = CL::getASnumber();
	int numberCL = CL::getCLnumber();

	if (numberAS == AS_TST_INIT_EXPECTED_NUMBER) LOG_INFO("AS initialization test number is Right");
	else LOG_ERROR("AS initialization test number is WRONG");

	if (numberCL == CL_TST_INIT_EXPECTED_NUMBER) LOG_INFO("CL initialization test number is Right");
	else LOG_ERROR("CL initialization test number is WRONG");

	getchar();
	return;
}

void test1SayHello(void) {
	
	AS::sayHello();

	CL::sayHelloExternal();
	
	getchar();
	return;
}