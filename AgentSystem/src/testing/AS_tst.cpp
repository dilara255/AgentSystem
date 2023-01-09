//TO DO: tests:
//- Test clearing a network (sizes should equal zero).
//- Create default values network;
//- Load from it, test some values (and data objects);
//- Change those values, save new network with them, run prevous tests again for this file;


#include "miscStdHeaders.h"
#include "core.hpp"

#include "AS_API.hpp"
#include "CL_internalAPI.hpp"

#include "systems/AScoordinator.hpp"
#include "data/agentDataStructures.hpp"
#include "data/agentClasses.hpp"
#include "fileManager.hpp"

#include "testing/AS_tst.hpp"

int initTestNumber;
int* AStestArray_ptr;
int* CLtestArray_ptr;

namespace AS {
	void testAgentDataClassCreation() {
		int testLAid = 0;
		float testOffset = 0.5;
		int testGAid = 1;
		int testPersonality = 99;
		bool testGAonOFF = true;
		
		LA::coldData_t LAcoldData = LA::coldData_t();
		LA::stateData_t LAstate = LA::stateData_t();
		LA::decisionData_t LAdecisionData = LA::decisionData_t();
		GA::coldData_t GAcoldData = GA::coldData_t();
		GA::stateData_t GAstate = GA::stateData_t();
		GA::decisionData_t GAdecisionData = GA::decisionData_t();

		AS::LAdata LAagentDataObject(LAcoldData, LAstate, LAdecisionData);
		AS::GAdata GAagentDataObject(GAcoldData, GAstate, GAdecisionData);

		//TO DO: implement actual testing : )
		//(load from default network and compare values with defaults)
		LAagentDataObject.m_coldData.id = testLAid;
		LAagentDataObject.m_decisionData.offsets.personality.offsets[0] = testOffset;
		LAagentDataObject.m_state.GAid = testGAid;

		GAagentDataObject.m_coldData.id = testGAid;
		GAagentDataObject.m_decisionData.personality[0] = testPersonality;
		GAagentDataObject.m_state.onOff = testGAonOFF;

		bool result = true;

		result &= (LAagentDataObject.m_coldData.id == testLAid);
		result &= (LAagentDataObject.m_decisionData.offsets.personality.offsets[0] == testOffset);
		result &= (LAagentDataObject.m_state.GAid == testGAid);

		result &= (GAagentDataObject.m_coldData.id == testGAid);
		result &= (GAagentDataObject.m_decisionData.personality[0] == testPersonality);
		result &= (GAagentDataObject.m_state.onOff == testGAonOFF);

		if (result) {
			LOG_INFO("Local and Global Agent Data Object instances have been created and accessed");
		}
		else {
			LOG_CRITICAL("Something went wrong with creation and or/ access to Local and/or Global Agent Data Object instances!");
		}
	}
}

void AS::testContainersAndAgentObjectCreation() 
{
	AS::testDataContainerCapacity(AS::agentDataControllers_cptr);
	AS::testAgentDataClassCreation();
}

void AS::testFileCreation() {
	std::string name = fileNameNoDefaults;
	int result = AS::createEmptyNetworkFile(name, name, TST_NUMBER_LAS, TST_NUMBER_GAS,
		MAX_LA_NEIGHBOURS, MAX_ACTIONS_PER_AGENT,
		false);

	std::string name2 = fileNameWithDefaults;
	result *= AS::createEmptyNetworkFile(name2, name2, TST_NUMBER_LAS, TST_NUMBER_GAS,
		MAX_LA_NEIGHBOURS, MAX_ACTIONS_PER_AGENT,
		true);

	if (result) {
		LOG_INFO("Test Empty Network Files created with and without defaults");
	}
	else {
		LOG_CRITICAL("Test Empty Network File Creation Failed (check if they already exist)");
	}
}

void AS::CLinitTest() {
	initTestNumber = AS_TST_INIT_EXPECTED_NUMBER;

	CL::initTest(initTestNumber, TST_ARRAY_SIZE);

	AS::initTstArray();

	CLtestArray_ptr = CL::getTestArrayPtr();

	AS::transferData(CLtestArray_ptr);

	//TO DO: ADD RESULTS FFS

	getchar();
}

void AS::initTstArray() {

	LOG_TRACE("\nWill initialize Test Array");

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