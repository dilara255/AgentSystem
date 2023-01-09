#include "miscStdHeaders.h"
#include "core.hpp"

#include "CL_internalAPI.hpp"
#include "CL_externalAPI.hpp"

#include "CL_tst.hpp"

//TO DO: TEST DATA STRUCTURE
initTestNumbers_t initNumbers;
int* testArray_ptr;
bool tstArrayInitialized;

void CL::init() {
	LOG_INFO("CL Mock initialization Called");
}

void CL::initTest(int ASnumber, int tstArraySize) {
	LOG_TRACE("Will initialize basic CL/AS communication test");

	initNumbers.ASnumber = ASnumber;
	initNumbers.CLnumber = CL_TST_INIT_EXPECTED_NUMBER;

	testArray_ptr = (int*)malloc(tstArraySize * sizeof(int));
	tstArrayInitialized = false;

	LOG_INFO("Initialized");

	return;
}

bool CL::hasTstArrayInitialized() {
	return tstArrayInitialized;
}

void CL::setTstArrayHasInitialized(bool hasInitialized) {
	tstArrayInitialized = hasInitialized;
}

int* CL::getTestArrayPtr() {
	if (!tstArrayInitialized) LOG_WARN("Test array not initialized yet");
	if (!testArray_ptr) LOG_WARN("Test array ptr null. Will crash : )");
	
	return testArray_ptr;
}

int CL::getASnumber() {
	return initNumbers.ASnumber;
}

int CL::getCLnumber() {
	return initNumbers.CLnumber;
}

void CL::sayHelloInternal() {
	printf("\nAnd I'm CL-internal, running on %s %s, in %s mode\n",
		CURR_SYSTEM, CURR_ARCHTECTURE, COMPILE_CFG);
}

void CL::sayHelloExternal() {
	printf("\nAnd I'm CL-external, running on %s %s, in %s mode\n",
		CURR_SYSTEM, CURR_ARCHTECTURE, COMPILE_CFG);
}

