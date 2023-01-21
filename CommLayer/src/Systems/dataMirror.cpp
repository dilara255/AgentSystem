#include "miscStdHeaders.h"

#include "AS_internal.hpp"

#include "CL_internalAPI.hpp"

#include "systems/dataMirror.hpp"

namespace CL {

	//TO DO: FIX: "isASdataPointerInitialized()" is failing after this!
	//Maybe it's looking at CLcoordinator's globals but not this?
	//Pass pointer to relevant globals here and mark their status!
	bool DataMirrorSystem::initialize(mirror_t** mirrorData_ptr_ptr,
		                              const AS::networkParameters_t* params_cptr) {
			
		LOG_TRACE("Initializing Data Mirror System...");

		//this is just to make sure AS is seen here
		if (!AS::GLOBAL == 1) {
			LOG_CRITICAL("Couldn't read critical enum - Initialization failed...");
			return false;
		}

		//TODO-CRITICAL: Extract. Note: CL doesn't link to AS stuff. Deal with that first
		strcpy(data.networkParams.comment, params_cptr->comment);
		data.networkParams.isNetworkInitialized = params_cptr->isNetworkInitialized;
		data.networkParams.lastMainLoopStartingTick = params_cptr->lastMainLoopStartingTick;
		data.networkParams.mainLoopTicks = params_cptr->mainLoopTicks;
		data.networkParams.maxActions = params_cptr->maxActions;
		data.networkParams.maxLAneighbours = params_cptr->maxLAneighbours;
		strcpy(data.networkParams.name, params_cptr->name);
		data.networkParams.numberGAs = params_cptr->numberGAs;
		data.networkParams.numberLAs = params_cptr->numberLAs;

		bool result = data.actionMirror.initialize(params_cptr);
		if (!result) {
			LOG_CRITICAL("Couldn't initialize Action Mirror System!");
			return false;
		}
		
		m_isInitialized = true; //provisionally

		result = createAgentDataControllers(params_cptr);
		if (!result) {
			LOG_CRITICAL("Couldn't create AgentDataControllers. Aborting...");
			m_isInitialized = false;
			m_hasData = false;
			return false;
		}

		(*mirrorData_ptr_ptr) = &data;
		data_cptr = (const mirror_t*)&data;

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

	bool DataMirrorSystem::createAgentDataControllers(const AS::networkParameters_t* params_cptr) {
		LOG_TRACE("Trying to create Agent Data Controllers");

		if (!m_isInitialized) {
			LOG_ERROR("Can't create controllers before initializing the Data Mirror System");
			return false;
		}

		if (data.agentMirrorPtrs.haveBeenCreated) {
			LOG_WARN("Data Controllers already exist: aborting re-creation (returns true)");
			return true;
		}

		if (params_cptr == NULL) {
			LOG_ERROR("Network parameters pointer is null or network not initialized. Aborting");
			return false;
		}
		
		CL::agentMirrorControllerPtrs_t* amp = &data.agentMirrorPtrs;

		amp->LAcoldData_ptr = new ColdDataControllerLA();
		amp->LAstate_ptr = new StateControllerLA();
		amp->LAdecision_ptr = new DecisionSystemLA();
		amp->GAcoldData_ptr = new ColdDataControllerGA();
		amp->GAstate_ptr = new StateControllerGA();
		amp->GAdecision_ptr = new DecisionSystemGA();

		amp->haveBeenCreated = true;

		LOG_TRACE("Agent Data Controllers created. Will initialize their capacities");

		amp->LAcoldData_ptr->data.reserve(MAX_LA_QUANTITY);
		amp->LAstate_ptr->data.reserve(MAX_LA_QUANTITY);
		amp->LAdecision_ptr->data.reserve(MAX_LA_QUANTITY);

		LA::coldData_t coldStubLA;
		LA::stateData_t stateStubLA;
		LA::decisionData_t decisionStubLA;

		int numberLAs = params_cptr->numberLAs;

		for (int i = 0; i < numberLAs; i++) {
			amp->LAcoldData_ptr->data.push_back(coldStubLA);
			amp->LAstate_ptr->data.push_back(stateStubLA);
			amp->LAdecision_ptr->data.push_back(decisionStubLA);
		}
		
		bool hasPushed = amp->LAcoldData_ptr->data.size() == numberLAs;
		hasPushed &= amp->LAstate_ptr->data.size() == numberLAs;
		hasPushed &= amp->LAdecision_ptr->data.size() == numberLAs;

		amp->GAcoldData_ptr->data.reserve(MAX_GA_QUANTITY);
		amp->GAstate_ptr->data.reserve(MAX_GA_QUANTITY);
		amp->GAdecision_ptr->data.reserve(MAX_GA_QUANTITY);

		GA::coldData_t coldStubGA;
		GA::stateData_t stateStubGA;
		GA::decisionData_t decisionStubGA;

		int numberGAs = params_cptr->numberGAs - 1; //las GA is reserved for "no GA"
		if(numberGAs < 0) numberGAs = 0;
		for (int i = 0; i < numberGAs; i++) {
			amp->GAcoldData_ptr->data.push_back(coldStubGA);
			amp->GAstate_ptr->data.push_back(stateStubGA);
			amp->GAdecision_ptr->data.push_back(decisionStubGA);
		}

		hasPushed &= amp->GAcoldData_ptr->data.size() == numberGAs;
		hasPushed &= amp->GAstate_ptr->data.size() == numberGAs;
		hasPushed &= amp->GAdecision_ptr->data.size() == numberGAs;

		if (!hasPushed) {
			LOG_ERROR("Didn't create the right amount of agentData stubs on containers");
			return false;
		}

		LOG_INFO("Data Controllers created and capacity reserved\n");
		return true;
	}

	bool DataMirrorSystem::updateHasData() {

		m_hasData = true;
		m_hasData &= isNetworkInitialized();
		m_hasData &= hasAgentData();
		m_hasData &= hasActionData();

		m_hasData &= data.actionMirror.hasData();
			
		return m_hasData;
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
			#if (defined AS_DEBUG) || VERBOSE_RELEASE
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


