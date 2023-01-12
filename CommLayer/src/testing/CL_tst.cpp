#include "miscStdHeaders.h"
#include "core.hpp"

#include "CL_internalAPI.hpp"
#include "CL_externalAPI.hpp"

#include "testing/CL_tst.hpp"

//TO DO: TEST DATA STRUCTURE
initTestNumbers_t initNumbers;
int* testArray_ptr;
bool tstArrayInitialized;

void CL::sanityTest(int ASnumber, int tstArraySize) {
	LOG_TRACE("Will initialize basic CL/AS communication test");

	initNumbers.ASnumber = ASnumber;
	initNumbers.CLnumber = CL_TST_INIT_EXPECTED_NUMBER;

	testArray_ptr = (int*)malloc(tstArraySize * sizeof(int));
	tstArrayInitialized = false;

	LOG_INFO("Initialized");

	return;
}

bool CL::sendDataChangedForTest(char* recipientString, GA::coldData_t* recepientGAcold,
	GA::stateData_t* recepientGAstate, LA::stateData_t* recipientLAstate,
	LA::decisionData_t* recipientLAdecision, AS::actionData_t* recepientAction) {

	recipientString[0] = mirrorData_ptr->networkParams.comment[0];

	*recepientGAcold = mirrorData_ptr->agentMirrorPtrs.GAcoldData_ptr->data.back();
	*recepientGAstate = mirrorData_ptr->agentMirrorPtrs.GAstate_ptr->data.back();
	*recipientLAstate = mirrorData_ptr->agentMirrorPtrs.LAstate_ptr->data.back();
	*recipientLAdecision = mirrorData_ptr->agentMirrorPtrs.LAdecision_ptr->data.back();
	*recepientAction = mirrorData_ptr->actionMirror.dataLAs.back();

	if (mirrorData_ptr->actionMirror.dataLAs.back().details.processingAux 
		                                                != TST_LAST_ACTION_AUX) {
		LOG_ERROR("Test last action aux is not as expected");
#ifdef AS_DEBUG
		printf("%i instead of %i",
			mirrorData_ptr->actionMirror.dataLAs.back().details.processingAux, TST_LAST_ACTION_AUX);
		getchar();
		return false;
#endif // AS_DEBUG
	}

	return true;
}

bool CL::hasTstArrayInitialized() {
	return tstArrayInitialized;
}

void CL::setTstArrayHasInitialized(bool hasInitialized) {
	tstArrayInitialized = hasInitialized;
}

int* CL::getTestArrayPtr() {
	if (!tstArrayInitialized) { LOG_WARN("Test array not initialized yet"); }
	if (!testArray_ptr) { LOG_WARN("Test array ptr null. Will crash : )"); }
	
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

