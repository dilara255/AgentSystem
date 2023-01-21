#include "miscStdHeaders.h"

#include "logAPI.hpp"
#include "AS_internal.hpp"

#include "CL_externalAPI.hpp"
#include "CL_internalAPI.hpp"

#include "systems/dataMirror.hpp"
#include "systems/clientDataHandler.hpp"

namespace CL {

	DataMirrorSystem ASmirror;
	const mirror_t* ASmirrorData_ptr = NULL;

	CL::ClientData::Handler* clientData_ptr = NULL;

	bool isClientDataPointerInitialized() {
		bool clientDataPtrIsNull = (clientData_ptr == NULL);
		bool clientDataHandlerIsInitialized = (clientData_ptr->hasInitialized());
		if (clientDataPtrIsNull || !clientDataHandlerIsInitialized) {
			#if (defined AS_DEBUG) || VERBOSE_RELEASE
				printf("Client Data Handler ptr null? %d. Handler initialized? %d.\n",
								    (int)clientDataPtrIsNull, (int)clientDataHandlerIsInitialized);
			#endif
			return false;
		}
		
		return true;
	}

	bool isASdataPointerInitialized() {
		bool mirroPtrIsNUll = (ASmirrorData_ptr == NULL);
		bool ASmirroIsInitialized = (ASmirror.isInitialized());
		if (mirroPtrIsNUll || !ASmirroIsInitialized) {
			#if (defined AS_DEBUG) || VERBOSE_RELEASE
				printf("AS mirror data ptr null? %d. AS mirror initialized? %d.\n",
								    (int)mirroPtrIsNUll, (int)ASmirroIsInitialized);
			#endif
			return false;
		}
		return true;
	}

	//Initializes the CL, including ASmirror. 
	//WARNING: Client Data only initializes on network load!
	bool init(const AS::networkParameters_t* params_cptr) {
		LOG_INFO("initializing CL...");

		mirror_t* tempASmirrorData_ptr;

		bool result = ASmirror.initialize(&tempASmirrorData_ptr, params_cptr);
		if (!result) {
			LOG_CRITICAL("ASmirror failed to initialize: CL INITIALIZATION FAILED!");
			return false;
		}

		ASmirrorData_ptr = (const mirror_t*)tempASmirrorData_ptr;
		if (ASmirrorData_ptr == NULL) {
			LOG_CRITICAL("Failed to set pointer to AS mirror data. CL Initialization failed");
			return false;
		}

		if (!ASmirror.isInitialized()) {
			LOG_CRITICAL("ASmirror reads as not initialized (instance lost maybe?) - CS failed initialization");
			return false;
		}

		if ((!ASmirrorData_ptr->agentMirrorPtrs.haveBeenCreated) ||
			(!ASmirrorData_ptr->actionMirror.isInitialized())) {
			LOG_CRITICAL("As Mirror pointer copy or container initialization failed - CL initialization failed");
			return false;
		}

		LOG_INFO("CL INITIALIZED!");
		return true;
	}

	//Also creates and initializes its components. Does quite a bit of heap allocation.
	//TO DO: An "update" method could avoid its use on AS's loading.
	bool createClientDataHandler(AS::networkParameters_t params) {

		LOG_TRACE("Will try to create Client Data Handler...");

		CL::ClientData::Handler* tempHandler_ptr = new CL::ClientData::Handler(params);

		if (tempHandler_ptr == NULL) {
			LOG_CRITICAL("New Client Data Handler Pointer is Null. Will abort creation");
			return false;
		}

		if (!tempHandler_ptr->hasInitialized()) {
			LOG_ERROR("New Client Data Handler failed to initialize. Will delete and abort creation");
			delete tempHandler_ptr;
			tempHandler_ptr = NULL;
			return false;
		}

		if (clientData_ptr != NULL) {
			LOG_TRACE("Handler already exists, will delete old and substitute");
			CL::ClientData::Handler* tempHandler2_ptr = clientData_ptr;
			clientData_ptr = tempHandler_ptr;
			delete tempHandler2_ptr;
			tempHandler2_ptr = NULL;
			tempHandler_ptr = NULL;
			//^This little dance is just in case Client tries to access while we're deleting
		}
		else {
			clientData_ptr = tempHandler_ptr;
			tempHandler_ptr = NULL;
		}	

		LOG_TRACE("New handler created");

		bool result = clientData_ptr->hasInitialized();

		if (!result) {
			LOG_CRITICAL("Failed to initialize Client Data Handler instance!");
			return false;
		}
		else {
			LOG_INFO("Client Data Handler initialized");
			return true;
		}
	}

	CL_API ClientData::Handler* getClientDataHandlerPtr() {
		if (!isClientDataPointerInitialized()) {
			LOG_ERROR("Trying to get unititalized or inexistent Client Data Handler");
			return NULL;
		}

		return clientData_ptr;
	}

	bool blockClientDataForAmoment() {
		if (!isClientDataPointerInitialized()) {
			LOG_WARN("Client Data not initialized. Will proceed without blocking, but something may be wrong");
			return true;
		}

		std::mutex* mutex_ptr = clientData_ptr->acquireMutex();
		
		if (mutex_ptr == NULL) {
			LOG_ERROR("Client Data blocking failed!");
			return false;
		}

		mutex_ptr->unlock();
		return true;
	}

	bool getNewClientData(AS::networkParameters_t* paramsRecepient_ptr,
						  AS::dataControllerPointers_t* agentDataRecepient_ptr,
		 				  AS::ActionDataController* actionsRecepient_ptr,
						  bool silent) {
		
		if (!isClientDataPointerInitialized()) {
			if(!silent){
				LOG_ERROR("Can't get Client Data because the Handler is not initialized");
			}
			return false;
		}

		CL::ClientData::ASdataControlPtrs_t recepientPtrs;

		recepientPtrs.params_ptr = paramsRecepient_ptr;
		recepientPtrs.agentData_ptr = agentDataRecepient_ptr;
		recepientPtrs.actions_ptr = actionsRecepient_ptr;

		return clientData_ptr->sendNewClientData(recepientPtrs, silent);
	}

	bool acceptReplacementData(const AS::networkParameters_t* params,
					   const std::vector <AS::actionData_t>* actionsLAs_cptr,
					   const std::vector <AS::actionData_t>* actionsGAs_cptr,
					   const std::vector <LA::coldData_t>* coldDataLAs_cptr,
					   const std::vector <LA::stateData_t>* stateLAs_cptr,
					   const std::vector <LA::decisionData_t>* decisionLAs_cptr,
					   const std::vector <GA::coldData_t>* coldDataGAs_cptr,
					   const std::vector <GA::stateData_t>* stateGAs_cptr,
					   const std::vector <GA::decisionData_t>* decisionGAs_cptr,
					   bool silent) {

		if (!isASdataPointerInitialized()) {
			if(!silent){
				LOG_ERROR("Can't transfer data from AS because AS data mirror on CL is not initialized");
			}
			return false;
		}

		//LOG_TRACE("Will accept replacement data from the AS");

		CL::agentToMirrorVectorPtrs_t agentDataPtrs;

		agentDataPtrs.coldDataLAs_cptr = coldDataLAs_cptr;
		agentDataPtrs.stateLAs_cptr = stateLAs_cptr;
		agentDataPtrs.decisionLAs_cptr = decisionLAs_cptr;
		
		agentDataPtrs.coldDataGAs_cptr = coldDataGAs_cptr;
		agentDataPtrs.stateGAs_cptr = stateGAs_cptr;
		agentDataPtrs.decisionGAs_cptr = decisionGAs_cptr;

		CL::actionToMirrorVectorPtrs_t actionPtrs;

		actionPtrs.actionsLAs_cptr = actionsLAs_cptr;
		actionPtrs.actionsGAs_cptr = actionsGAs_cptr;

		//LOG_TRACE("Created Data bundles. Will clear current mirror data and transfer new...");
		bool result = ASmirror.clearAllData();
		
		result &= ASmirror.receiveReplacementParams(params);
		result &= ASmirror.receiveReplacementAgentData(agentDataPtrs);
		result &= ASmirror.receiveReplacementActionData(actionPtrs);

		if (!result) { 
			LOG_ERROR("Aborting transfer, will clear ASmirror"); 
			ASmirror.clearAllData();
		}

		ASmirror.updateHasData();
		if (!ASmirror.hasData()) {
			LOG_ERROR("Mirror System says some data controllers read as not initialized");				
			return false;
		}

		//LOG_TRACE("Data received...");
		return result;
	}
}