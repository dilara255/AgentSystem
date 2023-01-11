#include "miscStdHeaders.h"

#include "AS_internal.hpp"

#include "CL_internalAPI.hpp"

namespace CL {

	bool DataMirror::initialize() {
		
		//m_isInitialized = AS::createAgentDataControllers(&data.agentControllerPtrs);
		m_isInitialized = true;
		return m_isInitialized;
	}

	bool DataMirror::clearAllData() {
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

	bool DataMirror::updateHasData() {
		m_hasData = true;
		/*
		m_hasData &= data.networkParams.isNetworkInitialized;
		m_hasData &= data.agentControllerPtrs.haveData;
		m_hasData &= data.action.data.hasData();
		*/
		return m_hasData;
	}

	bool DataMirror::receiveParams(const AS::networkParameters_t* params_cptr) {

		return true;
	}

	bool DataMirror::receiveAgentData(const AS::dataControllerPointers_t dataPtrs) {

		return true;
	}

	bool DataMirror::receiveActionData(const AS::ActionDataController* actions_cptr) {

		return true;
	}


	bool DataMirror::transferParams(AS::networkParameters_t* recepient_ptr) const {

		return true;
	}

	bool DataMirror::transferAgentData(AS::dataControllerPointers_t* recepient_ptr) const {

		return true;
	}

	bool DataMirror::transferActionData(AS::ActionDataController* recepient_ptr) const {

		return true;
	}
}


