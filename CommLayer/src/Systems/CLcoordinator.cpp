#include "miscStdHeaders.h"

#include "logAPI.hpp"
#include "AS_internal.hpp"

#include "CL_externalAPI.hpp"
#include "CL_internalAPI.hpp"

#include "systems/dataMirror.hpp"
#include "systems/clientDataHandler.hpp"

namespace CL {

	DataMirrorSystem ASmirror;
	const mirror_t* ASmirrorData_cptr = NULL;

	CL::ClientData::Handler* clientData_ptr = NULL;

	bool isClientDataPointerInitialized() {
	
		if (clientData_ptr == NULL) {
			#if (defined AS_DEBUG) || VERBOSE_RELEASE
				LOG_TRACE("Client Data ptr is NULL");
			#endif
			return false;
		}
		
		if (!clientData_ptr->hasInitialized()) {
			#if (defined AS_DEBUG) || VERBOSE_RELEASE
				LOG_TRACE("Client Data Handler not initialized");
			#endif
			return false;
		}
		
		return true;
	}

	bool isClintDataInitialized() {
		return isClientDataPointerInitialized();
	}

	bool isASdataPointerInitialized() {
		bool mirroPtrIsNUll = (ASmirrorData_cptr == NULL);
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

	//Initializes the CL, including ASmirror (with values sent by AS). 
	//WARNING: Client Data only initializes on network load!
	bool init(const AS::networkParameters_t* params_cptr) {
		LOG_INFO("initializing CL...");

		mirror_t* tempASmirrorData_ptr;

		bool result = ASmirror.initialize(&tempASmirrorData_ptr, params_cptr);
		if (!result) {
			LOG_CRITICAL("ASmirror failed to initialize: CL INITIALIZATION FAILED!");
			return false;
		}

		ASmirrorData_cptr = (const mirror_t*)tempASmirrorData_ptr;
		if (ASmirrorData_cptr == NULL) {
			LOG_CRITICAL("Failed to set pointer to AS mirror data. CL Initialization failed");
			return false;
		}

		if (!ASmirror.isInitialized()) {
			LOG_CRITICAL("ASmirror reads as not initialized (instance lost maybe?) - CS failed initialization");
			return false;
		}

		if ((!ASmirrorData_cptr->agentMirrorPtrs.haveBeenCreated) ||
			(!ASmirrorData_cptr->actionMirror.isInitialized())) {
			LOG_CRITICAL("As Mirror pointer copy or container initialization failed - CL initialization failed");
			return false;
		}

		LOG_INFO("CL INITIALIZED!");
		return true;
	}

	//Also creates and initializes its components. Does quite a bit of heap allocation.
	//TODO: An "update" method could avoid its use on AS's loading.
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
			return false;
		}

		if (clientData_ptr == NULL) {
			clientData_ptr = tempHandler_ptr;
		}	
		else{
			LOG_TRACE("Handler already exists, will substitute and delete old");

			//make sure no one is holding the lock before deletion:
			std::mutex* mutex_ptr = clientData_ptr->acquireMutex();
			if(mutex_ptr != NULL){
				CL::ClientData::Handler* tempHandler2_ptr = clientData_ptr;
				clientData_ptr = tempHandler_ptr;
				mutex_ptr->unlock();
				delete tempHandler2_ptr;
				LOG_TRACE("Substition and deletion done");
			}
			else {
				//couldn't secure mutex. Won't be able to delete old handler: delete new one.
				LOG_ERROR("Couldn't delete old Handler. Will keep it, discard the new one and fail");
				delete tempHandler_ptr;
				return false;
			}			
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

	std::mutex* blockClientData() {
		if (!isClientDataPointerInitialized()) {
			LOG_WARN("Client Data not initialized. Will proceed without blocking, but something may be wrong");
			return NULL;
		}

		std::mutex* mutex_ptr = clientData_ptr->acquireMutex();
		
		if (mutex_ptr == NULL) {
			LOG_ERROR("Client Data blocking failed!");
		}
		
		return mutex_ptr;
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

		bool result = clientData_ptr->sendNewClientData(recepientPtrs, silent);
		clientData_ptr->clearChanges();

		return result;
	}

	bool replaceMirrorData(const AS::networkParameters_t* params,
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

		//Create data bundles:
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

		//Clear old data: TODO: review wether this was actually necessary
		//bool result = ASmirror.clearAllData();
		
		//Actually send the data:
		bool result = true;
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