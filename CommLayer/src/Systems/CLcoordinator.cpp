#include "miscStdHeaders.h"

#include "logAPI.hpp"
#include "AS_internal.hpp"

#include "CL_externalAPI.hpp"
#include "CL_internalAPI.hpp"

#include "systems/dataMirror.hpp"

namespace CL {

	DataMirrorSystem mirror;

	mirror_t* mirrorData_ptr; //WARNING: bypasses mirror instance
	//TO DO: fix, this is just for initial testing

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