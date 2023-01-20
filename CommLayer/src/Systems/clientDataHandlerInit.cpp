#include "systems/clientDataHandler.hpp"

namespace CL {

	//Initializations:

//HANDLER

	ClientData::Handler::Handler(AS::networkParameters_t params) {
		LOG_TRACE("Will construct and initialize Client Data Handler on the CL...");

		bool result = m_mirrorSystem.initialize(&m_data_ptr,
			                                   (const AS::networkParameters_t*)&params);

		m_LAquantity = params.numberLAs;
		m_GAquantity = params.numberGAs - 1;
		if(m_GAquantity < 0) m_GAquantity = 0;

		m_maxActions = params.maxActions;
		m_referenceNetworkSize = ((m_GAquantity) + m_LAquantity) * m_maxActions;
		m_changes.reserve(m_referenceNetworkSize);

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
		
		LOG_CRITICAL("CHECK SIZES!");
		printf("\n\nLAs: %d, GAs: %d, MaxAc: %d, refSize: %d\n", m_LAquantity, m_GAquantity,
			                                         m_maxActions, m_referenceNetworkSize);
		getchar();
	}



//BASE

	bool ClientData::BaseSubHandler::initialize(ClientData::Handler* parentHandlerPtr,
		                                std::vector <changedDataInfo_t>* changesVector_ptr) {

		if ((parentHandlerPtr == NULL) || (changesVector_ptr == NULL)) {
			LOG_ERROR("Base Data Handler failed to initialize - received null pointers");
			return false;
		}

		m_parentHandlerPtr = parentHandlerPtr;
		m_changesVector_ptr = changesVector_ptr;

		return true;
	}



//NETWORK

	bool ClientData::NetworkParameterDataHandler::initialize(
					    ClientData::Handler* parentHandlerPtr, networkParameters_t* data_ptr,
						                  std::vector <changedDataInfo_t>* changesVector_ptr) {

		LOG_TRACE("Initializing Network Parameters Data Handler");

		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("Data Handler failed to initialize - received null data pointer");
			return false;
		}

		m_data_ptr = data_ptr;

		LOG_INFO("Handler initialized");
		return true;
	}



//ACTION

	bool ClientData::ActionsHandler::initialize(ClientData::Handler* parentHandlerPtr, 
								   	             std::vector <actionData_t>* data_ptr,
								   std::vector <changedDataInfo_t>* changesVector_ptr) {
		
		LOG_TRACE("Initializing Action Data Handler");

		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("Data Handler failed to initialize - received null data pointer");
			return false;
		}
		
		m_data_ptr = data_ptr;		
		
		LOG_TRACE("Initializing Field Handlers");
		result = IDs.initialize(parentHandlerPtr, data_ptr, changesVector_ptr);
		result &= details.initialize(parentHandlerPtr, data_ptr, changesVector_ptr);
		result &= tickInfo.initialize(parentHandlerPtr, data_ptr, changesVector_ptr);

		if (!result) {
			LOG_ERROR("Handler failed to initialize some of its components");
			return false;
		}
		else {
			LOG_INFO("Handler initialized");
			return true;
		}
	}

//ACTION FIELDS



	//IDs
	bool ClientData::ActionIDsHandler::initialize(ClientData::Handler* parentHandlerPtr,
										           std::vector <actionData_t>* data_ptr,
								     std::vector <changedDataInfo_t>* changesVector_ptr) {
		
		LOG_TRACE("- Initializing Action IDs Client Data Handler");

		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("- Data Handler failed to initialize - received null data pointer");
			return false;
		}

		m_data_ptr = data_ptr;		

		LOG_INFO("- Handler initialized");
		return true;
	}



	//TICK_INFO
	bool ClientData::ActionTickInfoHandler::initialize(ClientData::Handler* parentHandlerPtr,
										                std::vector <actionData_t>* data_ptr,
								          std::vector <changedDataInfo_t>* changesVector_ptr) {
		
		LOG_TRACE("- Initializing Action Tick Info Client Data Handler");

		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("- Data Handler failed to initialize - received null data pointer");
			return false;
		}

		m_data_ptr = data_ptr;		

		LOG_INFO("- Handler initialized");
		return true;
	}
	


	//DETAILS
	bool ClientData::ActionDetailsHandler::initialize(ClientData::Handler* parentHandlerPtr,
										               std::vector <actionData_t>* data_ptr,
								         std::vector <changedDataInfo_t>* changesVector_ptr) {
		
		LOG_TRACE("- Initializing Action Details Client Data Handler");

		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("- Data Handler failed to initialize - received null data pointer");
			return false;
		}
		
		m_data_ptr = data_ptr;		

		LOG_INFO("- Handler initialized");

		LOG_CRITICAL("CHECK SIZE!");
		printf("capacity: %zu, size: %zu", m_data_ptr->capacity(), m_data_ptr->size());
		getchar();

		return true;
	}

//LOCAL_AGENT_DATA_CATEGORIES
	

	//COLD
	bool ClientData::LAcoldDataHandler::initialize(ClientData::Handler* parentHandlerPtr,
										ColdDataControllerLA* data_ptr,
										std::vector <changedDataInfo_t>* changesVector_ptr) {
		
		LOG_TRACE("Initializing LAcold Handler");

		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("Data Handler failed to initialize - received null data pointer");
			return false;
		}

		m_data_ptr = data_ptr;

		LOG_INFO("Handler initialized");
		return true;
	}
	
	

	//STATE
	bool ClientData::LAstateHandler::initialize(ClientData::Handler* parentHandlerPtr,
										StateControllerLA* data_ptr,
										std::vector <changedDataInfo_t>* changesVector_ptr) {
		
		LOG_TRACE("Initializing LAstate Handler");

		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("Data Handler failed to initialize - received null data pointer");
			return false;
		}

		m_data_ptr = data_ptr;		

		LOG_TRACE("Initializing Field Handlers");
		result = location.initialize(parentHandlerPtr, data_ptr, changesVector_ptr);
		result &= parameters.initialize(parentHandlerPtr, data_ptr, changesVector_ptr);
		result &= relations.initialize(parentHandlerPtr, data_ptr, changesVector_ptr);

		if (!result) {
			LOG_ERROR("Handler failed to initialize some of its components");
			return false;
		}
		else {
			LOG_INFO("Handler initialized");
			return true;
		}
	}

	

	//DECISION
	bool ClientData::LAdecisionDataHandler::initialize(ClientData::Handler* parentHandlerPtr,
										DecisionSystemLA* data_ptr,
										std::vector <changedDataInfo_t>* changesVector_ptr) {
		
		LOG_TRACE("Initializing LA Decision Handler");

		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("Data Handler failed to initialize - received null data pointer");
			return false;
		}

		m_data_ptr = data_ptr;		

		LOG_TRACE("Initializing Field Handlers");
		result = personality.initialize(parentHandlerPtr, data_ptr, changesVector_ptr);

		if (!result) {
			LOG_ERROR("Handler failed to initialize some of its components");
			return false;
		}
		else {
			LOG_INFO("Handler initialized");
			return true;
		}
	}
	
//LA_STATE FIELDS
	


	//LOCATION
	bool ClientData::LocationAndConnectionsHandler::initialize(ClientData::Handler* parentHandlerPtr,
										  StateControllerLA* data_ptr,
										  std::vector <changedDataInfo_t>* changesVector_ptr) {
		
		LOG_TRACE("- Initializing LA Location and Connections Client Data Handler");

		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("- Data Handler failed to initialize - received null data pointer");
			return false;
		}

		m_data_ptr = data_ptr;		

		LOG_TRACE("- Initializing Field Handlers");
		result = position.initialize(parentHandlerPtr, data_ptr, changesVector_ptr);
		
		if (!result) {
			LOG_ERROR("- Handler failed to initialize some of its components");
			return false;
		}
		else {
			LOG_INFO("- Handler initialized");
			return true;
		}
	}



	//LA_PARAMETERS
	bool ClientData::LAparametersHandler::initialize(ClientData::Handler* parentHandlerPtr,
										  StateControllerLA* data_ptr,
										  std::vector <changedDataInfo_t>* changesVector_ptr) {
		
		LOG_TRACE("- Initializing LA Parameters Client Data Handler");

		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("- Data Handler failed to initialize - received null data pointer");
			return false;
		}

		m_data_ptr = data_ptr;		

		LOG_TRACE("- Initializing Field Handlers");
		result = resources.initialize(parentHandlerPtr, data_ptr, changesVector_ptr);
		result &= strenght.initialize(parentHandlerPtr, data_ptr, changesVector_ptr);
		
		if (!result) {
			LOG_ERROR("- Handler failed to initialize some of its components");
			return false;
		}
		else {
			LOG_INFO("- Handler initialized");
			return true;
		}
	}



	//RELATIONS
	bool ClientData::LArelationsHandler::initialize(ClientData::Handler* parentHandlerPtr,
										  StateControllerLA* data_ptr,
										  std::vector <changedDataInfo_t>* changesVector_ptr) {
		
		LOG_TRACE("- Initializing LA Relations Client Data Handler");

		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("- Data Handler failed to initialize - received null data pointer");
			return false;
		}

		m_data_ptr = data_ptr;		

		LOG_INFO("- Handler initialized");
		return true;
	}

//LOCAL_AGENT SUB-FIELDS
		


	//POSITION
	bool ClientData::PositionHandler::initialize(ClientData::Handler* parentHandlerPtr,
		                                                   StateControllerLA* data_ptr,
		                            std::vector <changedDataInfo_t>* changesVector_ptr){
		
		LOG_TRACE("-- Initializing LA Resources Handler");
		
		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("-- Data Handler failed to initialize - received null vectors");
			return false;
		}
		
		m_data_ptr = data_ptr;

		LOG_INFO("-- Data Handler initialized");
		return true;
	}


	//LA_RESOURCES
	bool ClientData::LAresourcesHandler::initialize(ClientData::Handler* parentHandlerPtr,
		                                 StateControllerLA* data_ptr,
		                                 std::vector <changedDataInfo_t>* changesVector_ptr){
		
		LOG_TRACE("-- Initializing LA Resources Handler");
		
		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("-- Data Handler failed to initialize - received null vectors");
			return false;
		}
		
		m_data_ptr = data_ptr;

		LOG_INFO("-- Data Handler initialized");
		return true;
	}



	//STRENGHT
	bool ClientData::LAstrenghtHandler::initialize(ClientData::Handler* parentHandlerPtr,
		                                                     StateControllerLA* data_ptr,
		                              std::vector <changedDataInfo_t>* changesVector_ptr){
		
		LOG_TRACE("-- Initializing LA Resources Handler");
		
		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("-- Data Handler failed to initialize - received null vectors");
			return false;
		}
		
		m_data_ptr = data_ptr;

		LOG_INFO("-- Data Handler initialized");
		return true;
	}



	//PERSONALITY
	bool ClientData::LApersonalityHandler::initialize(ClientData::Handler* parentHandlerPtr,
		                                 DecisionSystemLA* data_ptr,
		                                 std::vector <changedDataInfo_t>* changesVector_ptr){
		
		LOG_TRACE("-- Initializing LA Personality Handler");
		
		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("-- Data Handler failed to initialize - received null vectors");
			return false;
		}
		
		m_data_ptr = data_ptr;

		LOG_INFO("-- Data Handler initialized");
		return true;
	}
	
//GLOBAL_AGENT_DATA_CATEGORIES
	

	//COLD
	bool ClientData::GAcoldDataHandler::initialize(ClientData::Handler* parentHandlerPtr,
										ColdDataControllerGA* data_ptr,
										std::vector <changedDataInfo_t>* changesVector_ptr) {
		
		LOG_TRACE("Initializing GAcold Handler");

		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("Data Handler failed to initialize - received null data pointer");
			return false;
		}

		m_data_ptr = data_ptr;

		LOG_INFO("Handler initialized");
		return true;
	}
	
	

	//STATE
	bool ClientData::GAstateHandler::initialize(ClientData::Handler* parentHandlerPtr,
										StateControllerGA* data_ptr,
										std::vector <changedDataInfo_t>* changesVector_ptr) {
		
		LOG_TRACE("Initializing GAstate Handler");

		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("Data Handler failed to initialize - received null data pointer");
			return false;
		}

		m_data_ptr = data_ptr;		

		LOG_TRACE("Initializing Field Handlers");
		result &= parameters.initialize(parentHandlerPtr, data_ptr, changesVector_ptr);
		result &= relations.initialize(parentHandlerPtr, data_ptr, changesVector_ptr);

		if (!result) {
			LOG_ERROR("Handler failed to initialize some of its components");
			return false;
		}
		else {
			LOG_INFO("Handler initialized");
			return true;
		}
	}

	

	//DECISION
	bool ClientData::GAdecisionDataHandler::initialize(ClientData::Handler* parentHandlerPtr,
										DecisionSystemGA* data_ptr,
										std::vector <changedDataInfo_t>* changesVector_ptr) {
		
		LOG_TRACE("Initializing GA Decision Handler");

		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("Data Handler failed to initialize - received null data pointer");
			return false;
		}

		m_data_ptr = data_ptr;		
		
		LOG_INFO("Handler initialized");
		return true;
	}

//GLOBAL_AGENT_FIELDS



	//LA_PARAMETERS
	bool ClientData::GAparametersHandler::initialize(ClientData::Handler* parentHandlerPtr,
										  StateControllerGA* data_ptr,
										  std::vector <changedDataInfo_t>* changesVector_ptr) {
		
		LOG_TRACE("- Initializing GA Parameters Client Data Handler");

		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("- Data Handler failed to initialize - received null data pointer");
			return false;
		}

		m_data_ptr = data_ptr;		
		
		LOG_TRACE("- Initializing Field Handlers");
		result = LAresourceTotals.initialize(parentHandlerPtr, data_ptr, changesVector_ptr);
		
		if (!result) {
			LOG_ERROR("- Handler failed to initialize some of its components");
			return false;
		}
		else {
			LOG_INFO("- Handler initialized");
			return true;
		}
	}



	//GA_RELATIONS
	bool ClientData::GArelationsHandler::initialize(ClientData::Handler* parentHandlerPtr,
										 StateControllerGA* data_ptr,
										 std::vector <changedDataInfo_t>* changesVector_ptr) {

		LOG_TRACE("- Initializing GA Relations Client Data Handler");

		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("- Data Handler failed to initialize - received null data pointer");
			return false;
		}

		m_data_ptr = data_ptr;		

		LOG_INFO("- Handler initialized");
		return true;
	}

//GLOBAL_AGENT SUB-FIELDS
		


	//GA_RESOURCES
	bool ClientData::GAresourcesHandler::initialize(ClientData::Handler* parentHandlerPtr,
		                                 StateControllerGA* data_ptr,
		                                 std::vector <changedDataInfo_t>* changesVector_ptr){
		
		LOG_TRACE("-- Initializing GA Resources Handler");
		
		bool result = BaseSubHandler::initialize(parentHandlerPtr, changesVector_ptr);
		if (!result) { return false; }

		if (data_ptr == NULL) {
			LOG_ERROR("-- Data Handler failed to initialize - received null vectors");
			return false;
		}
		
		m_data_ptr = data_ptr;

		LOG_INFO("-- Data Handler initialized");
		return true;
	}
}