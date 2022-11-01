#include "miscStdHeaders.h"
#include "core.hpp"

#include "AS_API.hpp"
#include "CL_ExternalAPI.hpp"

void test1SayHello(void);
void test2Initialization(void);
void test3PointersToMockData(void);

int main(void) {
	
	test2Initialization();
	test3PointersToMockData();

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
		if (tstArray_ptr[0] == -842150451) { //MS debuging malloc initializes to this
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