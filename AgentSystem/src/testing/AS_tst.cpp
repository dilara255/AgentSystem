/*
TO DO: tests:

- Test clearing a network (sizes should equal zero);

- Test AS::Action class methods;

- Load default values network it, test some values (and data objects);
- Change those values, save new network with them, run prevous tests again for this file;

- Full test from loaded against defaults/expected and between files;

- Add all of these to the test sequence;
*/

#include "miscStdHeaders.h"
#include "core.hpp"

#include "AS_API.hpp"
#include "CL_internalAPI.hpp"

#include "systems/AScoordinator.hpp"
#include "data/agentDataStructures.hpp"
#include "data/agentClasses.hpp"
#include "network/fileManager.hpp"

#include "testing/AS_tst.hpp"

int initTestNumber;
int* AStestArray_ptr;
int* CLtestArray_ptr;

//TO DO: Add reasonable return values to all tests : )
namespace AS {

	extern ActionSystem actionSystem;
	extern dataControllerPointers_t agentDataControllerPtrs;
	extern networkParameters_t* currentNetworkParams_ptr;

	bool testGotNewValuesFromASthroughCL() {
		
		GA::coldData_t newGAcoldData;
		GA::stateData_t newGAstate;
		LA::stateData_t newLAdstate;
		LA::decisionData_t newLAdecision;
		AS::actionData_t newLAaction;

		bool result = CL::sendDataChangedForTest(&currentNetworkParams_ptr->comment[0],
			&newGAcoldData, &newGAstate, &newLAdstate, &newLAdecision, &newLAaction);
		if (!result) {
			LOG_ERROR("Failed to receive modified data from CL");
			return false;
		}

		int failed = 0;

		if (currentNetworkParams_cptr->comment[0] != TST_COMMENT_LETTER_CHANGE) {
			failed++; 
			LOG_TRACE("Value read is not as expected"); }
		
		if (newGAcoldData.id != TST_GA_ID) { 
			failed++; 
			LOG_TRACE("Value read is not as expected"); }

		agentDataControllerPtrs.GAcoldData_ptr->popLastAgent();
		agentDataControllerPtrs.GAcoldData_ptr->addAgentData(newGAcoldData);

		if (newGAstate.connectedGAs.getField() != TST_GA_CONNECTIONS) {
			failed++;
			LOG_TRACE("Value read is not as expected"); }

		agentDataControllerPtrs.GAstate_ptr->popLastAgent();
		agentDataControllerPtrs.GAstate_ptr->addAgentData(newGAstate);

		if (newLAdstate.parameters.strenght.externalGuard != (float)TST_LA_REINFORCEMENT) { 
			failed++; 
			LOG_TRACE("Value read is not as expected"); }

		agentDataControllerPtrs.LAstate_ptr->popLastAgent();
		agentDataControllerPtrs.LAstate_ptr->addAgentData(newLAdstate);
		
		int ctmp = TST_CHANGED_CATEGORY; int mtmp = TST_CHANGED_MODE;
		if (newLAdecision.offsets.incentivesAndConstraintsFromGA[ctmp][mtmp]
																!= (float)TST_LA_OFFSET) {
			failed++; 
			LOG_TRACE("Value read is not as expected!"); }

		agentDataControllerPtrs.LAdecision_ptr->popLastAgent();
		agentDataControllerPtrs.LAdecision_ptr->addAgentData(newLAdecision);

		if (newLAaction.details.processingAux != TST_LAST_ACTION_AUX) { 
			failed++; 
			LOG_TRACE("Value read is not as expected"); }

		actionSystem.data.popBackLAaction();
		actionSystem.data.pushBackLAaction(newLAaction);

		if (failed) {
			LOG_ERROR("Some of the values modified by the TA werent read back from the CL as expected");
		
			#ifdef AS_DEBUG
				printf("\n%i out of 6 failed. Test action aux: %i - expected %i", failed,
					newLAaction.details.processingAux, TST_LAST_ACTION_AUX);
				printf("\nGA connection data: %i - expected %i", newGAstate.connectedGAs.getField(),
					TST_GA_CONNECTIONS);
				printf(" | comment first letter: %c - expected %c", currentNetworkParams_cptr->comment[0],
																   TST_COMMENT_LETTER_CHANGE);
				printf("\nGA id: %i - expected %i", newGAcoldData.id, TST_GA_ID);
				printf(" | LA reinforcement : %f - expected %f",
							newLAdstate.parameters.strenght.externalGuard, TST_LA_REINFORCEMENT);
				printf("\nLA disposition offset: %f - expected %f\n",
					newLAdecision.offsets.incentivesAndConstraintsFromGA[ctmp][mtmp], TST_LA_OFFSET);
				getchar();
			#endif // AS_DEBUG
			return false;
		}

		LOG_INFO("Numbers modified by the TA read back from the CL as expected and updated on AS (see saved file to check this)");
		return true;
	}

	bool testDataTransferFromAStoCL(void) {

		LOG_INFO("Will Try to transfer data from AS to CL...");

		bool result = AS::sendReplacementDataToCL();
		if (!result) {
			LOG_CRITICAL("OOH NOOOES : (");
		}
		
		LOG_INFO("Data sent!");
		return result;
	}

	bool testAgentDataClassCreation() {
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
		LAagentDataObject.m_decisionData.offsets.personality[0][0] = testOffset;
		LAagentDataObject.m_state.GAid = testGAid;

		GAagentDataObject.m_coldData.id = testGAid;
		GAagentDataObject.m_decisionData.personality[0] = testPersonality;
		GAagentDataObject.m_state.onOff = testGAonOFF;

		bool result = true;

		result &= (LAagentDataObject.m_coldData.id == testLAid);
		result &= (LAagentDataObject.m_decisionData.offsets.personality[0][0]
			                                                              == testOffset);
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

		return result;
	}
}

bool AS::testContainersAndAgentObjectCreation() 
{
	LOG_TRACE("Going to test AS data containers and Agent Object instantiation...");
	bool result = AS::testDataContainerCapacity(AS::agentDataControllers_cptr);
	result &= AS::testAgentDataClassCreation();
	return result;
}

bool AS::testFileCreation(std::string nameNoDefaults, std::string nameWithDefaults) {
	
	int result = AS::createEmptyNetworkFile(nameNoDefaults, nameNoDefaults, TST_NUMBER_LAS, 
						    TST_NUMBER_GAS, MAX_LA_NEIGHBOURS, MAX_ACTIONS_PER_AGENT, false);

	result *= AS::createEmptyNetworkFile(nameWithDefaults, nameWithDefaults, TST_NUMBER_LAS, 
			                 TST_NUMBER_GAS, MAX_LA_NEIGHBOURS, MAX_ACTIONS_PER_AGENT, true);

	if (result) {
		LOG_INFO("Test Empty Network Files created with and without defaults");
	}
	else {
		LOG_CRITICAL("Test Empty Network File Creation Failed (check if they already exist)");
	}

	return result;
}

void AS::CLsanityTest() {
	initTestNumber = AS_TST_INIT_EXPECTED_NUMBER;

	CL::sanityTest(initTestNumber, TST_ARRAY_SIZE);

	AS::initTstArray();

	CLtestArray_ptr = CL::getTestArrayPtr();

	AS::transferData(CLtestArray_ptr);

	//TO DO: ADD RESULTS FFS
}

void AS::initTstArray() {

	LOG_TRACE("Will initialize Test Array",1);

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
		printf("Copied Test Array to CL (%p):\nCLtstArray[0]: %i\nCLtstArray[1]: %i\n",
							 CLtestArray_ptr, CLtestArray_ptr[0], CLtestArray_ptr[1]);
	#endif

	return;
}

void AS::sayHello() {

	printf("\nHi, I'm AS, running on %s %s, in %s mode\n", 
		   CURR_SYSTEM, CURR_ARCHTECTURE, COMPILE_CFG);

	CL::sayHelloInternal();
	
	return;
}