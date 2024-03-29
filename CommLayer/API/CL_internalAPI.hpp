#pragma once

#include "core.hpp"
#include "CL_core.hpp"

#include "../include/data/mirrorDataControllers.hpp"
#include "../include/data/agentDataControllers.hpp"

namespace CL {
	CL_API bool init(const AS::networkParameters_t* params_cptr);

	CL_API bool isClintDataInitialized();

	//If the handler already exists, will delete and re-instantiate and initialize it.
	//Is called whenever a network is loaded.
	CL_API bool createClientDataHandler(AS::networkParameters_t params);

	//Recepient fields with corresponding Client Data changes will be overwritten by the changes.
	//Will block Client Data during this process.
	CL_API bool getNewClientData(AS::networkParameters_t* paramsRecepient_ptr,
								 AS::dataControllerPointers_t* agentDataRecepient_ptr,
		                         AS::ActionDataController* actionsRecepient_ptr,
								 bool silent);

	//Clears the old data and receives 
	CL_API bool replaceMirrorData(const AS::networkParameters_t* params,
							  const std::vector <AS::actionData_t>* actionsLAs_cptr,
							  const std::vector <AS::actionData_t>* actionsGAs_cptr,
							  const std::vector <LA::coldData_t>* coldDataLAs_cptr,
							  const std::vector <LA::stateData_t>* stateLAs_cptr,
							  const std::vector <LA::decisionData_t>* decisionLAs_cptr,
							  const std::vector <GA::coldData_t>* coldDataGAs_cptr,
 							  const std::vector <GA::stateData_t>* stateGAs_cptr,
  							  const std::vector <GA::decisionData_t>* decisionGAs_cptr,
						      const AS::Decisions::networksDecisionsReflection_t* decisionReflection_cptr,
					          bool silent);

	//Used for synchronization (sp?), ie, for saving. 
	//Acquires mutex and returns pointer to it. NULL on failure. Remember to unlock!
	CL_API std::mutex* blockClientData();

	//****For Testing****
	CL_API void sanityTest(int ASinitTestNumber, int tstArraySize);
	CL_API void sayHelloInternal();
	CL_API int* getTestArrayPtr();
	CL_API bool hasTstArrayInitialized();
	CL_API void setTstArrayHasInitialized(bool hasInitialized);
	/*DEPRECATED
	CL_API bool sendDataChangedForTest(char* recipientString, GA::coldData_t* recepientGAcold,
	                     GA::stateData_t* recepientGAstate, LA::stateData_t* recipientLAstate,
		          LA::decisionData_t* recipientLAdecision, AS::actionData_t* recepientAction);
	*/
	//*******************
}