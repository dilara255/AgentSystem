#include "miscStdHeaders.h"

#include "AS_internal.hpp"

#include "CL_internalAPI.hpp"

#include "systems/dataMirror.hpp"

namespace CL {

	bool DataMirrorSystem::initialize() {
		
		//m_isInitialized = CL::createAgentDataControllers(&data.agentControllerPtrs);
		m_isInitialized = (AS::GLOBAL == 1);
		return m_isInitialized;
	}

	bool DataMirrorSystem::clearAllData() {
		if (!m_isInitialized) {
			LOG_ERROR("Can't clear mirror data - the instance is not initialized");
			return false;
		}

		/*
		data.agentControllerPtrs.GAcoldData_ptr->clearData();
		data.agentControllerPtrs.GAstate_ptr->clearData();
		data.agentControllerPtrs.GAdecision_ptr->clearData();

		data.agentControllerPtrs.LAcoldData_ptr->clearData();
		data.agentControllerPtrs.LAstate_ptr->clearData();
		data.agentControllerPtrs.LAdecision_ptr->clearData();
		
		data.agentControllerPtrs.haveData = false;

		data.action.data.clearData();

		data.networkParams.isNetworkInitialized = false;
		*/
		m_hasData = false;

		return true;
	}

	bool DataMirrorSystem::updateHasData() {
		m_hasData = true;
		/*
		m_hasData &= data.networkParams.isNetworkInitialized;
		m_hasData &= data.agentControllerPtrs.haveData;
		m_hasData &= data.action.data.hasData();
		*/
		return m_hasData;
	}

	bool DataMirrorSystem::receiveParams(const AS::networkParameters_t* params_cptr) {

		LOG_WARN("This function is not implemented yet - returning false...");
		return false;
	}

	bool DataMirrorSystem::receiveAgentData(const CL::agentMirrorControllerPtrs_t dataPtrs) {

		LOG_WARN("This function is not implemented yet - returning false...");
        return false;
	}

	bool DataMirrorSystem::receiveActionData(const CL::ActionMirrorController* actions_cptr) {

		LOG_WARN("This function is not implemented yet - returning false...");
		return false;
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


