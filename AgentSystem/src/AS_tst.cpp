#include "miscStdHeaders.h"
#include "core.hpp"

#include "AS_API.hpp"
#include "CL_internalAPI.hpp"

#include "agentData/dataStructures.hpp"
#include "agentData/agentClasses.hpp"

#include "AS_tst.hpp"

int initTestNumber;
int* AStestArray_ptr;
int* CLtestArray_ptr;

//TO DO: Separate from testing
void AS::initializeASandCL() {

	LOG_INFO("Loggers Initialized");

	initTestNumber = AS_TST_INIT_EXPECTED_NUMBER;

	CL::init(initTestNumber, TST_ARRAY_SIZE);

	LOG_TRACE("\nWill initialize Test Array");
	AS::initTstArray();

	CLtestArray_ptr = CL::getTestArrayPtr();

	AS::transferData(CLtestArray_ptr);

	getchar();

	createAgentDataControllers(MAX_LA_QUANTITY, MAX_GA_QUANTITY);

	LA::coldData_t LAcoldData = LA::coldData_t();
	LA::stateData_t LAstate = LA::stateData_t();
	LA::decisionData_t LAdecisionData = LA::decisionData_t();
	GA::coldData_t GAcoldData = GA::coldData_t();
	GA::stateData_t GAstate = GA::stateData_t();
	GA::decisionData_t GAdecisionData = GA::decisionData_t();

	AS::LAdata LAagentDataObject(LAcoldData, LAstate, LAdecisionData);
	AS::GAdata GAagentDataObject(GAcoldData, GAstate, GAdecisionData);

	LOG_INFO("Initialized");

	return;
}

void AS::initTstArray() {

	AStestArray_ptr = (int*)malloc(TST_ARRAY_SIZE * sizeof(int));
	
	AStestArray_ptr[0] = AS_TST_EXPECTED_ARR0;
	AStestArray_ptr[1] = AS_TST_EXPECTED_ARR1;

#ifdef AS_DEBUG
	printf("AStstArray[0]: %i\nAStstArray[1]: %i\n",
		   AStestArray_ptr[0], AStestArray_ptr[1]);
#endif

	LOG_TRACE("AS Test Array initialized\n");

	return;
}

void AS::transferData(int* CLtestArray_ptr) {
	if (!CLtestArray_ptr) LOG_WARN("Null ptr. Will crash");

	LOG_TRACE("Will transfer data to CL\n");

#ifdef AS_DEBUG
	printf("AStstArray[0]: %i\nAStstArray[1]: %i\n",
		   AStestArray_ptr[0], AStestArray_ptr[1]);
#endif

	memcpy(CLtestArray_ptr, AStestArray_ptr, TST_ARRAY_SIZE * sizeof(int));
	CL::setTstArrayHasInitialized(true);

#ifdef AS_DEBUG
	printf("Copied Test Array to CL (%i):\nCLtstArray[0]: %i\nCLtstArray[1]: %i\n",
		   (int)CLtestArray_ptr, CLtestArray_ptr[0], CLtestArray_ptr[1]);
#endif

	return;
}

void AS::sayHello() {

	printf("\nHi, I'm AS, running on %s %s, in %s mode\n", 
		   CURR_SYSTEM, CURR_ARCHTECTURE, COMPILE_CFG);

	CL::sayHelloInternal();
	
	return;
}