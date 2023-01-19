#include "systems/clientDataHandler.hpp"

namespace CL {
	
	//Initializations:

	ClientData::Handler::Handler(AS::networkParameters_t params) {
		LOG_TRACE("Will construct and initialize Client Data Handler on the CL...");

		bool result = m_mirrorSystem.initialize(&m_data_ptr);

		int referenceNetworkSize = (params.numberGAs + params.numberLAs) * params.maxActions;
		m_changes.reserve(referenceNetworkSize);

		LOG_TRACE("Changes vector size set. Will initialize Handlers for each Data Category...");

		result &= networkParameters.initialize(this, &m_data_ptr->networkParams, 
			                                   &m_changes);

		agentMirrorControllerPtrs_t ap = m_data_ptr->agentMirrorPtrs;
		
		result &= LAcold.initialize(this, ap.LAcoldData_ptr, &m_changes);
		result &= LAstate.initialize(this, ap.LAstate_ptr, &m_changes);
		result &= LAdecision.initialize(this, ap.LAdecision_ptr, &m_changes);

		result &= GAcold.initialize(this, ap.GAcoldData_ptr, &m_changes);
		result &= GAstate.initialize(this, ap.GAstate_ptr, &m_changes);
		result &= GAdecision.initialize(this, ap.GAdecision_ptr, &m_changes);
		
		result &= LAaction.initialize(this, &m_data_ptr->actionMirror.dataLAs, &m_changes);
		result &= GAaction.initialize(this, &m_data_ptr->actionMirror.dataGAs, &m_changes);

		m_initialized = result;

		if (!m_initialized) {
			LOG_CRITICAL("Something went wrong initializing Client Data Handler on the CL. May catch fire.");
		}
		else {
			LOG_INFO("Client Data Handler constructed and initialized on the CL");
		}
	}

	bool ClientData::LAstateHandler::initialize(ClientData::Handler* parentHandlerPtr,
										StateControllerLA* data_ptr,
										std::vector <changedDataInfo_t>* changesVector_ptr) {
		
		LOG_TRACE("Initializing LAstate Handler");

		if ((parentHandlerPtr == NULL) || (data_ptr == NULL) || (changesVector_ptr == NULL)) {
			LOG_ERROR("LA State Client Data Handler failed to initialize - received null vectors");
			return false;
		}

		m_parentHandlerPtr = parentHandlerPtr;
		m_data_ptr = data_ptr;
		m_changesVector_ptr = changesVector_ptr;

		LOG_TRACE("Initializing Handlers for the fields of LAstate");
		bool result = parameters.initialize(parentHandlerPtr, data_ptr, changesVector_ptr);
		//TO DO: the rest of the initialization

		if (!result) {
			LOG_ERROR("LA State Client Data Handler failed to initialize some of its components");
			return false;
		}
		else {
			LOG_INFO("LA State Client Data Handler initialized");
			return true;
		}
	}

	bool ClientData::LAparametersHandler::initialize(ClientData::Handler* parentHandlerPtr,
										  StateControllerLA* data_ptr,
										  std::vector <changedDataInfo_t>* changesVector_ptr) {
		
		LOG_TRACE("- Initializing LA Parameters Handler");

		if ((parentHandlerPtr == NULL) || (data_ptr == NULL) || (changesVector_ptr == NULL)) {
			LOG_ERROR("- LA Parameters Client Data Handler failed to initialize - received null vectors");
			return false;
		}

		m_parentHandlerPtr = parentHandlerPtr;
		m_data_ptr = data_ptr;
		m_changesVector_ptr = changesVector_ptr;

		LOG_TRACE("- Initializing Handlers for the fields of LA Parameters");
		bool result = resources.initialize(parentHandlerPtr, data_ptr, changesVector_ptr);
		//TO DO: the rest of the initialization
		
		if (!result) {
			LOG_ERROR("- LA Parameters Client Data Handler failed to initialize some of its components");
			return false;
		}
		else {
			LOG_INFO("- LA Parameters Client Data Handler initialized");
			return true;
		}	
	}

	bool ClientData::LAresourcesHandler::initialize(ClientData::Handler* parentHandlerPtr,
		                                 StateControllerLA* data_ptr,
		                                 std::vector <changedDataInfo_t>* changesVector_ptr){
		
		LOG_TRACE("-- Initializing LA Resources Handler");
		
		if ((parentHandlerPtr == NULL) || (data_ptr == NULL) || (changesVector_ptr == NULL)) {
			LOG_ERROR("-- LA Resources Client Data Handler failed to initialize - received null vectors");
			return false;
		}
		
		m_parentHandlerPtr = parentHandlerPtr;
		m_data_ptr = data_ptr;
		m_changesVector_ptr = changesVector_ptr;

		LOG_INFO("-- LA Resources Client Data Handler initialized");
		return true;
	}

	//Actual Data handling:

	bool ClientData::Handler::processChange(ClientData::changedDataInfo_t change, 
		                                    ASdataControlPtrs_t recepientPtrs) {
		
		if (change.getNewData_fptr == NULL) {
			LOG_ERROR("Change has invalid processing function pointer, can't transfer data");
			return false;
		}

		//TODO-CITICAL: Check that this works, consideting the fptr is to a private method
		return change.getNewData_fptr(change.agentID, recepientPtrs);
	}

	bool ClientData::Handler::sendNewClientData(ASdataControlPtrs_t recepientPtrs, 
		                                        bool shouldMainLoopBeRunning) {

		int size = (int)m_changes.size();

		std::mutex* mutex_ptr = acquireMutex();
		if (mutex_ptr == NULL) {
			LOG_ERROR("Aborting Client Data transfer, couldn't acquire mutex");
			return false;
		}

		#ifdef AS_DEBUG
			if (!shouldMainLoopBeRunning) {
				printf("Client Changes to process: %d\n", size);
			}
		#endif // AS_DEBUG

		bool result = true;
		for (int i = 0; i < size; i++) {
			result &= processChange(m_changes[i], recepientPtrs);
		}

		if (!result) {
			LOG_ERROR("Failed to process some changes issue by the Client");
		}

		mutex_ptr->unlock();

		return result;
	}

	bool ClientData::LAresourcesHandler::transferCurrent(uint32_t agentID, 
													ASdataControlPtrs_t recepientPtrs){
		
		//TO DO: extract functions
		if (agentID > m_data_ptr->data.size()) {
			LOG_ERROR("Trying to get changes from agent outside of Client Data range");
			return false;
		}
		if (agentID > recepientPtrs.agentData_ptr->LAstate_ptr->getDirectDataPtr()->size()) {
			LOG_ERROR("Trying to get changes from agent outside of AS Data range");
			return false;
		}

		LA::StateController* LAdata_ptr = recepientPtrs.agentData_ptr->LAstate_ptr;
		std::vector <LA::stateData_t>* dataVector_ptr = LAdata_ptr->getDirectDataPtr();
		LA::stateData_t* agentData_ptr = &(dataVector_ptr->at(agentID));
		
		agentData_ptr->parameters.resources.current =
			m_data_ptr->data.at(agentID).parameters.resources.current;
		
		return true;
	}

	bool ClientData::LAresourcesHandler::changeCurrentTo(uint32_t agentID, float newValue) {
		
		//TO DO: extract
		if (agentID >= m_data_ptr->data.size()) {
			LOG_ERROR("Tried to change data for agentID out of range");
			
			#ifdef AS_DEBUG
				printf("Data has capacity %zu, aid was %u\n", 
					                            m_data_ptr->data.size(), agentID);
			#endif // AS_DEBUG

			return false;
		}

		std::mutex* mutex_ptr = m_parentHandlerPtr->acquireMutex();

		if (mutex_ptr == NULL) {
			LOG_ERROR("Aborting Client Data Change");
			return false;
		}

		//Actually changes the value:
		m_data_ptr->data[agentID].parameters.resources.current = newValue;

		//Store the callback function to transfer the change:
		changedDataInfo_t change;
		change.agentID = agentID;

		auto callback = std::bind(&CL::ClientData::LAresourcesHandler::transferCurrent,
			this, std::placeholders::_1, std::placeholders::_2);

		change.getNewData_fptr = callback;

		m_changesVector_ptr->push_back(change);

		mutex_ptr->unlock();

		return true;
	}

	std::mutex* ClientData::Handler::acquireMutex() {
		int tries = 0;
		bool acquired = false;
		while (!acquired && (tries < MAX_MUTEX_TRY_LOCKS)) {
			acquired = m_mutex.try_lock();
			tries++;
			std::this_thread::sleep_for(
				std::chrono::microseconds(SLEEP_TIME_WAITING_MUTEX_MICROS));
		}
		if (!acquired) {
			LOG_ERROR("lock acquisition timed out!");
			return NULL;
		}
		
		return &m_mutex;
	}

}