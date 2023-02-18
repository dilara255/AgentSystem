/*
TODO: tests:

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
#include "CL_externalAPI.hpp"

#include "systems/AScoordinator.hpp"
#include "data/agentDataStructures.hpp"
#include "data/agentClasses.hpp"
#include "network/fileManager.hpp"

#include "systems/AScoordinator.hpp"
#include "testing/AS_tst.hpp"

int initTestNumber;
int* AStestArray_ptr;
int* CLtestArray_ptr;

bool setLAneighbourIDsAndFirst(AS::LAlocationAndConnectionData_t* data_ptr, int numberLAs);
bool setGAneighboursAndLAsIDs(AS::GAflagField_t* connectedGAs_ptr, int numberEffectiveGAs,
                                                        int neighbourIDs[MAX_GA_QUANTITY],
                                       AS::LAflagField_t* connectedLAs_ptr, int numberLAs,
                                                               int laIDs[MAX_LA_QUANTITY]);

//TODO: Add reasonable return values to all tests : )
namespace AS {

	extern ActionSystem actionSystem;
	extern dataControllerPointers_t agentDataControllerPtrs;
	extern networkParameters_t* currentNetworkParams_ptr;

	//TODO: some of the sub-tests can be extracted into FlagField functionality and tests
	bool testNeighbourIDsetting() {
		LOG_WARN("Will test setting neighbour IDs for LAs and GAs based on connections");
		
		//First we will test the LA side: 
		AS::LAlocationAndConnectionData_t connectionData;

		const int blocks = (MAX_LA_QUANTITY / (sizeof(uint32_t)*BITS_IN_A_BYTE));
		if (blocks != 4) {
			LOG_ERROR("Expected a different block size: MAX_LA_QUANTITY not supported by this teste. Will fail)");
			return false;
		}

		uint32_t connectedLAs[blocks];
		connectedLAs[0] = 0b01001011011000011110111110011100;
		connectedLAs[1] = 0b01010010011000101010110111010001;
		connectedLAs[2] = 0b10100101101000011001100001110011;
		connectedLAs[3] = 0b11100011001010011001011100100110;

		AZ::FlagField128 effectiveLAsMaskFlags;
		for(int i = 0; i < blocks; i++){
			effectiveLAsMaskFlags.loadField(0,i);
		}		
		for (int i = 0; i < TST_NUMBER_LAS; i++) {
			effectiveLAsMaskFlags.setBitOn(i);
		}

		for(int i = 0; i < blocks; i++){
			uint32_t effectiveBlock = connectedLAs[i] & effectiveLAsMaskFlags.getField(i);
			connectionData.connectedNeighbors.loadField(effectiveBlock,i);
		}
		int totalConnected = connectionData.connectedNeighbors.howManyAreOn();
		int* totalConnected_ptr = &totalConnected;

		bool result = setLAneighbourIDsAndFirst(&connectionData, MAX_LA_QUANTITY);
		if (!result) {
			LOG_ERROR("Failed setting neighbour IDs and first connected");
		}
		
		if (connectionData.numberConnectedNeighbors != totalConnected) {
			LOG_ERROR("LA's total connected neighbours is not registered as expected");
			return false;
		}

		int firstNeighbour = -1;
		int i = 0;
		while (firstNeighbour == -1) {
			if (connectionData.connectedNeighbors.isBitOn(i)) {
				firstNeighbour = i;
			}
			i++;
		}
		if (connectionData.firstConnectedNeighborId != firstNeighbour) {
			LOG_ERROR("LA's First Neighbour ID not as expected");
			result = false;
		}

		int found = 0;
		bool neighbourIDsOk = true;
		for (int i = 0; i < TST_NUMBER_LAS; i++) {
			if (connectionData.connectedNeighbors.isBitOn(i)) {
				neighbourIDsOk &= (connectionData.neighbourIDs[found] == i);
				found++;
			}
			if(found == MAX_LA_NEIGHBOURS) { break; }
		}
		if (!neighbourIDsOk) {
			LOG_ERROR("LA neighbours IDs don't match expected");
			result = false;
		}

		//And then the GA side: 
		GA::stateData_t GAstate;

		AS::GAflagField_t* connectedGAs_ptr = &GAstate.connectedGAs;
		AS::LAflagField_t* connectedLAs_ptr = &GAstate.localAgentsBelongingToThis;

		int numberEffectiveGAs = (TST_NUMBER_GAS - 1);
		AZ::FlagField32 effectiveGAsMaskFlags;
		effectiveGAsMaskFlags.loadField(0);
		for (int i = 0; i < numberEffectiveGAs; i++) {
			effectiveGAsMaskFlags.setBitOn(i);
		}
		uint32_t effectiveGAsMask = effectiveGAsMaskFlags.getField();

		GAstate.connectedGAs.loadField(connectedLAs[0] & effectiveGAsMask);

		for(int i = 0; i < blocks; i++){
			uint32_t effectiveBlock = connectedLAs[i] & effectiveLAsMaskFlags.getField(i);
			GAstate.localAgentsBelongingToThis.loadField(effectiveBlock,i);
		}

		int* neighbourIDs = &GAstate.neighbourIDs[0];
		int* laIDs = &GAstate.laIDs[0];		
		
		result = setGAneighboursAndLAsIDs(connectedGAs_ptr, numberEffectiveGAs, neighbourIDs, 
                                                     connectedLAs_ptr, TST_NUMBER_LAS, laIDs);
		if (!result) {
			LOG_ERROR("Failed setting neighbour and LA IDs");
		}

		//Cheking GA neighbor IDs:

		found = 0;
		neighbourIDsOk = true;
		for (int i = 0; i < numberEffectiveGAs; i++) {
			if (GAstate.connectedGAs.isBitOn(i)) {
				neighbourIDsOk &= (GAstate.neighbourIDs[found] == i);
				found++;
			}
		}
		if (!neighbourIDsOk) {
			LOG_ERROR("LA neighbours IDs don't match expected");
			result = false;
		}
		if (found != GAstate.connectedGAs.howManyAreOn()) {
			LOG_ERROR("Didn't find as many GA neighbours as expected");
			result = false;
		}

		//Cheking GA connections with its LAs: ID matches:
		
		found = 0;
		neighbourIDsOk = true;
		for (int i = 0; i < (sizeof(uint32_t)*BITS_IN_A_BYTE*blocks); i++) {
			if (GAstate.localAgentsBelongingToThis.isBitOn(i)) {
				neighbourIDsOk &= (GAstate.laIDs[found] == i);
				found++;
			}
		}
		if (!neighbourIDsOk) {
			LOG_ERROR("LA neighbours IDs don't match expected");
			result = false;
		}
		if (found != GAstate.localAgentsBelongingToThis.howManyAreOn()) {
			LOG_ERROR("Didn't find as many connected LA as expected");
			printf("\nexpected: %d, found: %d\n", 
				GAstate.localAgentsBelongingToThis.howManyAreOn(), found);
			result = false;
		}

		return result;
	}

	bool testMainLoopStopAndRestart() {
		if(!isMainLoopRunning()) {return false;}
		if (!stop()) {
			LOG_ERROR("Couldn't stop main loop (or it wasn't running)");
			return false;
		}
		if(!run()){
			LOG_ERROR("Couldn't start main loop (or it was already running)");
			return false;
		}
		return true;
	}

	bool testGotNewValuesFromASthroughCL() {
		
		LOG_WARN("Will test changes issued by the Client to the AS through the CL...");

		if ((&actionSystem == NULL) || (!agentDataControllerPtrs.haveBeenCreated) ||
			(!agentDataControllerPtrs.haveData) || (currentNetworkParams_ptr == NULL) ||
			(!currentNetworkParams_ptr->isNetworkInitialized)) {
			LOG_ERROR("Something is wrong with the pointers being used. Aborting test");
			return false;
		}

		LOG_TRACE("Will first transfer any unprocessed Client data to the AS...");

		bool result = CL::getNewClientData(currentNetworkParams_ptr,
			                               &agentDataControllerPtrs, 
			                               &(actionSystem.data),
			                               false);
		if (!result) {
			LOG_ERROR("Test Failed to get Client Changes. Will abort");
			return false;
		}

		int failed = 0;

		LOG_TRACE("Done. Will check transfered values. First, the network comment...");

		if (currentNetworkParams_cptr->comment[0] != TST_COMMENT_LETTER_CHANGE) {
			failed++; 
			LOG_TRACE("Value read is not as expected"); 
		}
		
		AS::networkParameters_t* pp = currentNetworkParams_ptr;

		//Only at most max GAs - 1 have data: the "last" is reserved for "no GA", so:
		int lastGA = pp->numberGAs - 1 - 1;

		AS::dataControllerPointers_t mp = agentDataControllerPtrs;

		LOG_TRACE("Will read Last GA's id...");

		uint32_t idRead = mp.GAcoldData_ptr->getDataCptr()->at(lastGA).id;

		if (idRead != TST_GA_ID) { 
			failed++; 
			LOG_TRACE("Value read is not as expected"); 
		}

		LOG_TRACE("Will read Last GA's GA connection data...");

		AS::GAflagField_t connectedRead = 
			                        mp.GAstate_ptr->getDataCptr()->at(lastGA).connectedGAs;

		if (connectedRead.getField() != TST_GA_CONNECTIONS) {
			failed++;
			LOG_TRACE("Value read is not as expected"); 
		}

		int lastLA = pp->numberLAs - 1;

		LOG_TRACE("Will read Last LA's guard...");

		float guardRead = mp.LAstate_ptr->getDataCptr()->at(lastLA).parameters.strenght.externalGuard;
		if (guardRead != (float)TST_LA_REINFORCEMENT) { 
			failed++; 
			LOG_TRACE("Value read is not as expected"); 
		}

		LOG_TRACE("Will read Last LA's GA decision offsets...");

		const AS::LAdecisionOffsets_t* offsets_ptr =
	      &mp.LAdecision_ptr->getDataCptr()->at(lastLA).offsets.incentivesAndConstraintsFromGA;

		int ctmp = TST_CHANGED_CATEGORY; int mtmp = TST_CHANGED_MODE;

		float offsetRead = (*offsets_ptr)[ctmp][mtmp];	
		
		if (offsetRead != (float)TST_LA_OFFSET) {
			failed++; 
			LOG_TRACE("Value read is not as expected!"); 
		}

		LOG_TRACE("Will read Last LA's las actions's AUX...");

		int index = lastLA * pp->maxActions + pp->maxActions - 1;
		float auxRead = actionSystem.data.getActionsLAsCptr()->at(index).details.processingAux;
		if (auxRead != TST_LAST_ACTION_AUX) { 
			failed++; 
			LOG_TRACE("Value read is not as expected"); 
		}

		LOG_TRACE("Checking results...");

		if (failed) {
			LOG_ERROR("Some of the values modified by the TA werent read back from the CL as expected");
		
			#if (defined AS_DEBUG) || VERBOSE_RELEASE
				printf("\n%d out of 6 failed. Test action aux: %f - expected %f ", failed,
					                                         auxRead, TST_LAST_ACTION_AUX);
				printf("\nGA connection data: %d - expected %d ", connectedRead.getField(),
					                                                    TST_GA_CONNECTIONS);
				printf(" | comment first letter: %c - expected %c", currentNetworkParams_cptr->comment[0],
																   TST_COMMENT_LETTER_CHANGE);
				printf("\nGA id: %d - expected %d ", idRead, TST_GA_ID);
				printf(" | LA reinforcement : %f - expected %f", guardRead, TST_LA_REINFORCEMENT);
				printf("\nLA disposition offset: %f - expected %f\n", offsetRead, TST_LA_OFFSET);
				GETCHAR_PAUSE;
			#endif // AS_DEBUG
			return false;
		}

		LOG_INFO("Members modified by the Client read back from the CL as expected and updated on AS (see saved file to check this)");
		return true;
	}

	bool testDataTransferFromAStoCL(void) {

		LOG_WARN("Will Try to transfer data from AS to CL...");

		if (!CL::isASdataPointerInitialized()) {
			LOG_ERROR("As mirror on CL is not yet initialized.");
			return false;
		}

		bool result = AS::sendReplacementDataToCL(false);
		if (!result) {
			LOG_CRITICAL("OOH NOOOES : (");
			return false;
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

		//TODO: implement actual testing : )
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
	LOG_WARN("Going to test AS data containers and Agent Object instantiation...");
	bool result = AS::testDataContainerCapacity(AS::agentDataControllers_cptr);
	result &= AS::testAgentDataClassCreation();
	return result;
}

bool AS::testFileCreation(std::string nameNoDefaults, std::string nameWithDefaults) {
	LOG_WARN("Will test file creation, with and withouth defaults");

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

	//TODO: ADD RESULTS FFS
}

void AS::initTstArray() {

	LOG_TRACE("Will initialize Test Array",1);

	AStestArray_ptr = (int*)malloc(TST_ARRAY_SIZE * sizeof(int));
	
	AStestArray_ptr[0] = AS_TST_EXPECTED_ARR0;
	AStestArray_ptr[1] = AS_TST_EXPECTED_ARR1;

	#if (defined AS_DEBUG) || VERBOSE_RELEASE
		printf("AStstArray[0]: %d \nAStstArray[1]: %d \n",
			   AStestArray_ptr[0], AStestArray_ptr[1]);
	#endif

	LOG_TRACE("AS Test Array initialized\n");

	return;
}

void AS::transferData(int* CLtestArray_ptr) {
	if (!CLtestArray_ptr){
		LOG_WARN("Null ptr. Will crash");
	}

	LOG_TRACE("Will transfer data to CL\n");

	#if (defined AS_DEBUG) || VERBOSE_RELEASE
		printf("AStstArray[0]: %d \nAStstArray[1]: %d \n",
			   AStestArray_ptr[0], AStestArray_ptr[1]);
	#endif

	memcpy(CLtestArray_ptr, AStestArray_ptr, TST_ARRAY_SIZE * sizeof(int));
	CL::setTstArrayHasInitialized(true);

	#if (defined AS_DEBUG) || VERBOSE_RELEASE
		printf("Copied Test Array to CL (%p):\nCLtstArray[0]: %d \nCLtstArray[1]: %d \n",
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