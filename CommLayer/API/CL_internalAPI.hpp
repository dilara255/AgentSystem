#pragma once

#include "core.hpp"

#include "../include/data/mirrorDataControllers.hpp"

namespace CL {
	CL_API bool init();
	CL_API bool acceptReplacementData(const AS::networkParameters_t* params,
							  const std::vector <AS::actionData_t>* actionsLAs_cptr,
							  const std::vector <AS::actionData_t>* actionsGAs_cptr,
							  const std::vector <LA::coldData_t>* coldDataLAs_cptr,
							  const std::vector <LA::stateData_t>* stateLAs_cptr,
							  const std::vector <LA::decisionData_t>* decisionLAs_cptr,
							  const std::vector <GA::coldData_t>* coldDataGAs_cptr,
 							  const std::vector <GA::stateData_t>* stateGAs_cptr,
  							  const std::vector <GA::decisionData_t>* decisionGAs_cptr);

	//****For Testing****
	CL_API void sanityTest(int ASinitTestNumber, int tstArraySize);
	CL_API void sayHelloInternal();
	CL_API int* getTestArrayPtr();
	CL_API bool hasTstArrayInitialized();
	CL_API void setTstArrayHasInitialized(bool hasInitialized);
	CL_API bool sendDataChangedForTest(char* recipientString, GA::coldData_t* recepientGAcold,
		GA::stateData_t* recepientGAstate, LA::stateData_t* recipientLAstate,
		LA::decisionData_t* recipientLAdecision, AS::actionData_t* recepientAction);
	//*******************
}