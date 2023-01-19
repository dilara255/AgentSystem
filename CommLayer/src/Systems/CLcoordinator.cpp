#include "miscStdHeaders.h"

#include "logAPI.hpp"
#include "AS_internal.hpp"

#include "CL_externalAPI.hpp"
#include "CL_internalAPI.hpp"

#include "systems/dataMirror.hpp"
#include "systems/clientDataHandler.hpp"

namespace CL {

	DataMirrorSystem mirror; //TO DO: change to ASmirror
	mirror_t* mirrorData_ptr; //WARNING: bypasses mirror instance
	//TO DO: fix, this is just for initial testing

	CL::ClientData::Handler* clientData_ptr = NULL;

	bool init() {
		LOG_INFO("initializing CL...");

		bool result = mirror.initialize(&mirrorData_ptr);
		if (!result) {
			LOG_CRITICAL("CL INITIALIZATION FAILED!");
			return false;
		}

		if (!mirror.isInitialized()) {
			LOG_CRITICAL("CL INITIALIZATION FAILED! (instance lost maybe?)");
			return false;
		}

		if ((!mirrorData_ptr->agentMirrorPtrs.haveBeenCreated) ||
			(!mirrorData_ptr->actionMirror.isInitialized())) {
			LOG_CRITICAL("CL INITIALIZATION FAILED! (pointer copy / container initialization)");
			return false;
		}

		LOG_INFO("CL INITIALIZED!");
		return true;
	}

	//Also creates and initializes its components. Does quite a bit of heap allocation.
	//TO DO: An "update" method could avoid it's use on AS's loading.
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
			//^This little dance is just in case Client tries to acces while we're deleting
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

	ClientData::Handler* getDataHandlerPtr() {
		if ((clientData_ptr == NULL) || (!clientData_ptr->hasInitialized())) {
			LOG_ERROR("Trying to get unititalized or inexistent Client Data Handler");
			return NULL;
		}

		return clientData_ptr;
	}

	bool blockClientDataForAmoment() {
		if (clientData_ptr == NULL) {
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
						  bool shouldMainLoopBeRunning) {
		
		CL::ClientData::ASdataControlPtrs_t recepientPtrs;

		recepientPtrs.params_ptr = paramsRecepient_ptr;
		recepientPtrs.agentData_ptr = agentDataRecepient_ptr;
		recepientPtrs.actions_ptr = actionsRecepient_ptr;

		return clientData_ptr->sendNewClientData(recepientPtrs, shouldMainLoopBeRunning);
	}

	bool acceptReplacementData(const AS::networkParameters_t* params,
					   const std::vector <AS::actionData_t>* actionsLAs_cptr,
					   const std::vector <AS::actionData_t>* actionsGAs_cptr,
					   const std::vector <LA::coldData_t>* coldDataLAs_cptr,
					   const std::vector <LA::stateData_t>* stateLAs_cptr,
					   const std::vector <LA::decisionData_t>* decisionLAs_cptr,
					   const std::vector <GA::coldData_t>* coldDataGAs_cptr,
					   const std::vector <GA::stateData_t>* stateGAs_cptr,
					   const std::vector <GA::decisionData_t>* decisionGAs_cptr) {

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
		bool result = mirror.clearAllData();
		
		result &= mirror.receiveReplacementParams(params);
		result &= mirror.receiveReplacementAgentData(agentDataPtrs);
		result &= mirror.receiveReplacementActionData(actionPtrs);

		if (!result) { 
			LOG_ERROR("Aborting transfer, will clear mirror"); 
			mirror.clearAllData();
		}

		mirror.updateHasData();
		if (!mirror.hasData()) {
			LOG_ERROR("Mirror System says some data controllers read as not initialized");				
			return false;
		}

		//LOG_TRACE("Data received...");
		return result;
	}
}