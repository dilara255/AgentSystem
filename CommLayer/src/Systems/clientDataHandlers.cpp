#include "systems/clientDataHandler.hpp"

namespace CL {
	
	//Initializations:

	ClientData::Handler::Handler(AS::networkParameters_t params) {
		LOG_TRACE("Will construct and initialize Client Data Handler on the CL");

		bool result = m_mirrorSystem.initialize(&m_data_ptr);

		int referenceNetworkSize = (params.numberGAs + params.numberLAs) * params.maxActions;
		m_changes.reserve(referenceNetworkSize);

		result &= networkParameters.initialize(&m_mutex, &m_data_ptr->networkParams, 
			                                   &m_changes);

		agentMirrorControllerPtrs_t ap = m_data_ptr->agentMirrorPtrs;
		
		result &= LAcold.initialize(&m_mutex, ap.LAcoldData_ptr, &m_changes);
		result &= LAstate.initialize(&m_mutex, ap.LAstate_ptr, &m_changes);
		result &= LAdecision.initialize(&m_mutex, ap.LAdecision_ptr, &m_changes);

		result &= GAcold.initialize(&m_mutex, ap.GAcoldData_ptr, &m_changes);
		result &= GAstate.initialize(&m_mutex, ap.GAstate_ptr, &m_changes);
		result &= GAdecision.initialize(&m_mutex, ap.GAdecision_ptr, &m_changes);
		
		result &= LAaction.initialize(&m_mutex, &m_data_ptr->actionMirror.dataLAs, &m_changes);
		result &= GAaction.initialize(&m_mutex, &m_data_ptr->actionMirror.dataGAs, &m_changes);

		m_initialized = result;

		if (!m_initialized) {
			LOG_CRITICAL("Something went wrong initializing Client Data Handler on the CL. May catch fire.");
		}
		else {
			LOG_INFO("Client Data Handler constructed initialized on the CL");
		}
	}

	bool ClientData::LAstateHandler::initialize(std::mutex* mutex_ptr,
		StateControllerLA* data_ptr,
		std::vector <changedDataInfo_t>* changesVector_ptr) {
		if ((mutex_ptr == NULL) || (data_ptr == NULL) || (changesVector_ptr == NULL)) {
			LOG_ERROR("LA State Client Data Handler failed to initialize - received null vectors");
			return false;
		}

		m_mutex_ptr = mutex_ptr;
		m_data_ptr = data_ptr;
		m_changesVector_ptr = changesVector_ptr;

		bool result = parameters.initialize(mutex_ptr, data_ptr, changesVector_ptr);
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

	bool ClientData::LAparametersHandler::initialize(std::mutex* mutex_ptr,
										  StateControllerLA* data_ptr,
										  std::vector <changedDataInfo_t>* changesVector_ptr) {
		if ((mutex_ptr == NULL) || (data_ptr == NULL) || (changesVector_ptr == NULL)) {
			LOG_ERROR("LA Parameters Client Data Handler failed to initialize - received null vectors");
			return false;
		}

		m_mutex_ptr = mutex_ptr;
		m_data_ptr = data_ptr;
		m_changesVector_ptr = changesVector_ptr;

		bool result = resources.initialize(mutex_ptr, data_ptr, changesVector_ptr);
		//TO DO: the rest of the initialization
		
		if (!result) {
			LOG_ERROR("LA Parameters Client Data Handler failed to initialize some of its components");
			return false;
		}
		else {
			LOG_INFO("LA Parameters Client Data Handler initialized");
			return true;
		}	
	}

	bool ClientData::LAresourcesHandler::initialize(std::mutex* mutex_ptr, 
		                                 StateControllerLA* data_ptr,
		                                 std::vector <changedDataInfo_t>* changesVector_ptr){
		if ((mutex_ptr == NULL) || (data_ptr == NULL) || (changesVector_ptr == NULL)) {
			LOG_ERROR("LA Resources Client Data Handler failed to initialize - received null vectors");
			return false;
		}
		
		m_mutex_ptr = mutex_ptr;
		m_data_ptr = data_ptr;
		m_changesVector_ptr = changesVector_ptr;

		LOG_INFO("LA Resources Client Data Handler initialized");
		return true;
	}

	//Actual Data handling:
	bool ClientData::Handler::processChange(ClientData::changedDataInfo_t change, 
		                                    ASdataControlPtrs_t recepientPtrs) {
		
		if (!change.hasChanges) {
			LOG_WARN("Reached a Client Data Change marker which is innactive. Won't transfer");
			return false;
		}

		bool isLAstate = change.dataCategory == (int)AS::DataCategory::LA_STATE;
		if (isLAstate) {
			
			int id = change.agent;
			if (id > recepientPtrs.params_ptr->numberLAs) {
				LOG_ERROR("Agent Changed by the Client doesn't exist anymore in the AS. Won't transfer changes");
				return false;
			}

			auto dataPtr = recepientPtrs.agentData_ptr->LAstate_ptr->getDirectDataPtr();
			LA::stateData_t* state_ptr = &(*dataPtr)[id];

			switch (change.baseField) {

			case (int)LA::stateData_t::fields::PARAMETERS:

				AS::LAparameters_t* params_ptr = &(state_ptr->parameters);

				switch (change.subField[0]) {

				case (int)AS::LAparameters_t::fields::RESOURCES:

					AS::resources_t* resources_ptr = &(params_ptr->resources);

					switch (change.subField[1]) {

					case (int)AS::resources_t::fields::CURRENT:
						return LAstate.parameters.resources.transferCurrent(id, resources_ptr);
					}
				}
			}
		}

		LOG_ERROR("Couldn't parse which field has changes, won't transfer");
		return false;
	}

	bool ClientData::Handler::sendNewClientData(ASdataControlPtrs_t recepientPtrs) {

		int size = (int)m_changes.size();

		bool result = true;
		for (int i = 0; i < size; i++) {
			result = processChange(m_changes[i], recepientPtrs);
		}

		return result;
	}

	bool ClientData::LAresourcesHandler::transferCurrent(uint32_t agentID, 
		                                                 AS::resources_t* recepient){
		
		recepient->current = m_data_ptr->data[agentID].parameters.resources.current;
		return true;
	}

	bool ClientData::LAresourcesHandler::changeCurrentTo(uint32_t agentID, float newValue) {
		if (agentID > m_data_ptr->data.size()) {
			LOG_ERROR("Tried to change data for agentID out of range");
			return false;
		}

		//Extract this as a method! Maybe make mutex warper class
		int tries = 0;
		bool acquired = false;
		while (!acquired && (tries < MAX_MUTEX_TRY_LOCKS) ) {
			acquired = m_mutex_ptr->try_lock();
			tries++;
			std::this_thread::sleep_for(
				              std::chrono::microseconds(SLEEP_TIME_WAITING_MUTEX_MICROS));
		}
		if (!acquired) {
			LOG_ERROR("Client Data transfer failed - lock acquisition timed out!");
			return false;
		}

		m_data_ptr->data[agentID].parameters.resources.current = newValue;

		//Extract this too
		changedDataInfo_t change;
		change.agent = agentID;
		change.dataCategory = (int)AS::DataCategory::LA_STATE;
		change.baseField = (int)LA::stateData_t::fields::PARAMETERS;
		change.subField[0] = (int)AS::LAparameters_t::fields::RESOURCES;
		change.subField[1] = (int)AS::resources_t::fields::CURRENT;
		change.lastSubfield = 1;
		change.hasChanges = true;

		m_changesVector_ptr->push_back(change);

		m_mutex_ptr->unlock();

		return true;
	}

}