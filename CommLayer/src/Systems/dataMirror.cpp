#include "miscStdHeaders.h"

#include "AS_internal.hpp"

#include "CL_internalAPI.hpp"

#include "systems/dataMirror.hpp"

namespace CL {

	bool DataMirrorSystem::initialize(mirror_t** mirror_ptr_ptr) {
		
		//m_isInitialized = CL::createAgentDataControllers(&data.agentControllerPtrs);
		(*mirror_ptr_ptr) = &data;
		
		bool result = data.actionMirror.initialize();
		if (!result) {
			LOG_CRITICAL("Couldn't initialize Data Mirror System!");
			return false;
		}

		m_isInitialized = (AS::GLOBAL == 1); //tests if enums are working or something
		return m_isInitialized;
	}

	bool DataMirrorSystem::clearAllData() {
		if (!m_isInitialized) {
			LOG_ERROR("Can't clear mirror data - the instance is not initialized");
			return false;
		}

		data.agentMirrorPtrs.GAcoldData_ptr->clearData();
		data.agentMirrorPtrs.GAstate_ptr->clearData();
		data.agentMirrorPtrs.GAdecision_ptr->clearData();

		data.agentMirrorPtrs.LAcoldData_ptr->clearData();
		data.agentMirrorPtrs.LAstate_ptr->clearData();
		data.agentMirrorPtrs.LAdecision_ptr->clearData();
		
		data.agentMirrorPtrs.haveData = false;

		data.actionMirror.clearData();
		
		data.networkParams.isNetworkInitialized = false;
		
		m_hasData = false;

		return true;
	}

	bool DataMirrorSystem::createAgentDataControllers() {
		LOG_TRACE("Trying to create Agent Data Controllers\n");

		if (data.agentMirrorPtrs.haveBeenCreated) {
			LOG_WARN("Data Controllers already exist: aborting re-creation\n");
			return false;
		}
		
		data.agentMirrorPtrs.LAcoldData_ptr = new ColdDataControllerLA();
		data.agentMirrorPtrs.LAstate_ptr = new StateControllerLA();
		data.agentMirrorPtrs.LAdecision_ptr = new DecisionSystemLA();
		data.agentMirrorPtrs.GAcoldData_ptr = new ColdDataControllerGA();
		data.agentMirrorPtrs.GAstate_ptr = new StateControllerGA();
		data.agentMirrorPtrs.GAdecision_ptr = new DecisionSystemGA();

		data.agentMirrorPtrs.haveBeenCreated = true;

		LOG_INFO("Data Controllers created\n");
		return true;
	}

	bool DataMirrorSystem::updateHasData() {
		m_hasData &= data.networkParams.isNetworkInitialized;
		m_hasData &= data.agentMirrorPtrs.haveData;
		return m_hasData &= data.actionMirror.hasData();
	}

	bool DataMirrorSystem::receiveParams(const AS::networkParameters_t* params_cptr) {

		LOG_TRACE("Receiving new parameters");

		data.networkParams.isNetworkInitialized = params_cptr->isNetworkInitialized;
		data.networkParams.maxActions = params_cptr->maxActions;
		data.networkParams.maxLAneighbours = params_cptr->maxLAneighbours;
		data.networkParams.numberGAs = params_cptr->numberGAs;
		data.networkParams.numberLAs = params_cptr->numberLAs;
		
		size_t nameSize = NAME_LENGHT * sizeof(char);
		int result = strcpy_s(data.networkParams.name, nameSize, params_cptr->name);
		if (!result) { //strcpy_s returns 0 on success
			LOG_ERROR("Failed to receive network name...");
			return false;
		}

		size_t commentSize = COMMENT_LENGHT * sizeof(char);
		result = strcpy_s(data.networkParams.comment, commentSize, params_cptr->comment);
		if (!result) { //strcpy_s returns 0 on success
			LOG_ERROR("Failed to receive network comment line...");
			return false;
		}

		return true;
	}

	bool DataMirrorSystem::receiveAgentData(const CL::agentToMirrorVectorPtrs_t dataPtrs) {

		LOG_TRACE("Receiving new agent data...");

		if (!data.agentMirrorPtrs.haveBeenCreated) {
			LOG_ERROR("Can't receive agent Data before controllers are created!");
			return false;
		}

		data.agentMirrorPtrs.LAcoldData_ptr->data = *dataPtrs.coldDataLAs_cptr;
		data.agentMirrorPtrs.LAstate_ptr->data = *dataPtrs.stateLAs_cptr;
		data.agentMirrorPtrs.LAdecision_ptr->data = *dataPtrs.decisionLAs_cptr;

		data.agentMirrorPtrs.GAcoldData_ptr->data = *dataPtrs.coldDataGAs_cptr;
		data.agentMirrorPtrs.GAstate_ptr->data = *dataPtrs.stateGAs_cptr;
		data.agentMirrorPtrs.GAdecision_ptr->data = *dataPtrs.decisionGAs_cptr;

		data.agentMirrorPtrs.haveData = true;

        return true;
	}

	bool DataMirrorSystem::receiveActionData(actionToMirrorVectorPtrs_t actionPtrs) {

		LOG_TRACE("Receiving new action data...");

		if (!data.actionMirror.isInitialized()) {
			LOG_ERROR("Can't receive action Data before controllers are initialized!");
			return false;
		}

		data.actionMirror.dataLAs = *actionPtrs.actionsLAs_cptr;
		data.actionMirror.dataGAs = *actionPtrs.actionsGAs_cptr;

		return true;
	}

	bool DataMirrorSystem::transferParams(AS::networkParameters_t* recepient_ptr) const {

		LOG_WARN("This function is not implemented yet - returning false...");
		return false;
	}

	bool DataMirrorSystem::transferAgentData(CL::agentMirrorControllerPtrs_t* recepient_ptr) const {

		LOG_WARN("This function is not implemented yet - returning false...");
        return false;
	}

	bool DataMirrorSystem::transferActionData(CL::ActionMirrorController* recepient_ptr) const {

		LOG_WARN("This function is not implemented yet - returning false...");
		return false;
	}
}


