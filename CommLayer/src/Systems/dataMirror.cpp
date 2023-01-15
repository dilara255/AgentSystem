#include "miscStdHeaders.h"

#include "AS_internal.hpp"

#include "CL_internalAPI.hpp"

#include "systems/dataMirror.hpp"

namespace CL {

	bool DataMirrorSystem::initialize(mirror_t** mirror_ptr_ptr) {
			
		LOG_TRACE("Initializing Data Mirror System...");

		if (!AS::GLOBAL == 1) {
			LOG_CRITICAL("Couldn't read critical enum - Initialization failed...");
			return false;
		}

		bool result = data.actionMirror.initialize();
		if (!result) {
			LOG_CRITICAL("Couldn't initialize Data Mirror System!");
			return false;
		}

		m_isInitialized = true; //provisionally

		result = createAgentDataControllers();
		if (!result) {
			LOG_CRITICAL("Aborting...");
			m_isInitialized = false;
			m_hasData = false;
			return false;
		}

		(*mirror_ptr_ptr) = &data;

		LOG_INFO("Data Mirror System Initialized!");

		return true;
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

		if (!m_isInitialized) {
			LOG_ERROR("Can't create controllers before initializing the Data Mirror System");
			return false;
		}

		if (data.agentMirrorPtrs.haveBeenCreated) {
			LOG_WARN("Data Controllers already exist: aborting re-creation (returns true)\n");
			return true;
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

		m_hasData = true;
		m_hasData &= isNetworkInitialized();
		m_hasData &= hasAgentData();
		m_hasData &= hasActionData();
			
		return (m_hasData &= data.actionMirror.hasData());
	}

	bool DataMirrorSystem::receiveReplacementParams(const AS::networkParameters_t* params_cptr) {

		//LOG_TRACE("Receiving new parameters");

		data.networkParams.isNetworkInitialized = params_cptr->isNetworkInitialized;
		if (!data.networkParams.isNetworkInitialized) {
			LOG_WARN("Network being read says it wasnt initialized! Will proceed, but errors are to be expected");
		}
		data.networkParams.maxActions = params_cptr->maxActions;
		data.networkParams.maxLAneighbours = params_cptr->maxLAneighbours;
		data.networkParams.numberGAs = params_cptr->numberGAs;
		data.networkParams.numberLAs = params_cptr->numberLAs;
		data.networkParams.mainLoopTicks = params_cptr->mainLoopTicks;
		data.networkParams.lastMainLoopStartingTick = params_cptr->lastMainLoopStartingTick;
		
		size_t nameSize = NAME_LENGHT * sizeof(char);

		int result = strcpy_s(data.networkParams.name, nameSize, params_cptr->name);
		//strcpy_s returns 0 on success
		if (result != 0) {
			
			LOG_ERROR("Failed to receive network name...");
			#ifdef AS_DEBUG
				printf("name expected : % s | name read : % s\n", params_cptr->name,
														 data.networkParams.name);
			#endif // AS_DEBUG

			return false;
		}

		size_t commentSize = COMMENT_LENGHT * sizeof(char);
		result = strcpy_s(data.networkParams.comment, commentSize, params_cptr->comment);
		if (result != 0) { //strcpy_s returns 0 on success
			LOG_ERROR("Failed to receive network comment line...");
			return false;
		}

		return true;
	}

	bool DataMirrorSystem::receiveReplacementAgentData(const CL::agentToMirrorVectorPtrs_t dataPtrs) {

		//LOG_TRACE("Receiving new agent data...");

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

	bool DataMirrorSystem::receiveReplacementActionData(actionToMirrorVectorPtrs_t actionPtrs) {

		//LOG_TRACE("Receiving new action data...");

		if (!data.actionMirror.isInitialized()) {
			LOG_ERROR("Can't receive action Data before controllers are initialized!");
			return false;
		}

		data.actionMirror.dataLAs = *actionPtrs.actionsLAs_cptr;
		data.actionMirror.dataGAs = *actionPtrs.actionsGAs_cptr;

		data.actionMirror.setHasData(true);

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


