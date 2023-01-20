#include "systems/clientDataHandler.hpp"

namespace CL {

	//MUTEX HANDLING
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
	

	//DISPATCHER - ALL
	bool ClientData::Handler::sendNewClientData(ASdataControlPtrs_t recepientPtrs, 
		                                                              bool silent) {

		int size = (int)m_changes.size();

		#ifdef AS_DEBUG
			if (!silent) {
				LOG_TRACE("Will acquire mutex...");
			}
		#endif // AS_DEBUG

		std::mutex* mutex_ptr = acquireMutex();
		if (mutex_ptr == NULL) {
			LOG_ERROR("Aborting Client Data transfer, couldn't acquire mutex");
			return false;
		}

		#ifdef AS_DEBUG
			if (!silent) {
				printf("Client Changes to process: %d\n", size);
			}
		#endif // AS_DEBUG

		bool result = true;
		bool tmp;
		for (int i = 0; i < size; i++) {

			#ifdef AS_DEBUG
				if (!silent) {
					printf("Change %d: %d, %s\n", i, m_changes[i].agentID,
						                          m_changes[i].getNewData_fptr);
				}
			#endif // AS_DEBUG
			
			tmp = processChange(m_changes[i], recepientPtrs);
			result &=tmp;

			#ifdef AS_DEBUG
				if(!tmp && !silent) {
					LOG_WARN("Failed to process change!");
				}
			#endif // AS_DEBUG
		}

		if (!result) {
			LOG_ERROR("Failed to process some changes issue by the Client");
		}

		#ifdef AS_DEBUG
			if (!silent) {
				LOG_INFO("All changes processed! Will release lock...");
			}
		#endif // AS_DEBUG

		mutex_ptr->unlock();

		return result;
	}


	//DISPATCHER - EACH
	bool ClientData::Handler::processChange(ClientData::changedDataInfo_t change, 
		                                    ASdataControlPtrs_t recepientPtrs) {
		
		if (change.getNewData_fptr == NULL) {
			LOG_ERROR("Change has invalid processing function pointer, can't transfer data");
			return false;
		}

		//TODO-CITICAL: Check that this works, consideting the fptr is to a private method
		return change.getNewData_fptr(change.agentID, recepientPtrs);
	}


//NETWORK

	bool CL::ClientData::NetworkParameterDataHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}


	bool ClientData::NetworkParameterDataHandler::transferComment(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		char* commentOnAS_ptr = recepientPtrs.params_ptr->comment;
		char* commentClient_ptr = m_data_ptr->comment;

		strcpy(commentOnAS_ptr, commentClient_ptr);
		
		return true;
	}

	bool ClientData::NetworkParameterDataHandler::transferIsNetworkInitialized(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::NetworkParameterDataHandler::transferLastMainLoopStartingTick(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::NetworkParameterDataHandler::transferMainLoopTicks(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::NetworkParameterDataHandler::transferMaxActions(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::NetworkParameterDataHandler::transferMaxLAneighbours(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::NetworkParameterDataHandler::transferName(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::NetworkParameterDataHandler::transferNumberGAs(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::NetworkParameterDataHandler::transferNumberLAs(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//ACTION
	
	bool CL::ClientData::ActionsHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//ACTION: IDs
	
	bool ClientData::ActionIDsHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}


	bool ClientData::ActionIDsHandler::transferActive(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::ActionIDsHandler::transferCategory(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::ActionIDsHandler::transferMode(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::ActionIDsHandler::transferOrigin(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::ActionIDsHandler::transferScope(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::ActionIDsHandler::transferTarget(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//ACTION: TICK_INFO
	
	bool ClientData::ActionTickInfoHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}


	bool ClientData::ActionTickInfoHandler::transferInitial(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::ActionTickInfoHandler::transferLastProcessed(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//ACTION: DETAILS

	bool ClientData::ActionDetailsHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}


	bool ClientData::ActionDetailsHandler::transferIntensity(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::ActionDetailsHandler::transferAux(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		//TO DO: Extract
		bool isGlobal = (bool)(agentID & (uint32_t)AS::actionIDtoUnsigned::SCOPE_MASK);
		
		uint32_t actionID = agentID & (uint32_t)AS::actionIDtoUnsigned::ACTION_MASK;
		actionID = actionID >> (uint32_t)AS::actionIDtoUnsigned::ACTION_SHIFT;

		agentID = agentID & (uint32_t)AS::actionIDtoUnsigned::AGENT_MASK;
		agentID = agentID >> (uint32_t)AS::actionIDtoUnsigned::AGENT_SHIFT;
	
		int maxActionsPerAgent = recepientPtrs.actions_ptr->getMaxActionsPerAgent();
		int index = (agentID)*maxActionsPerAgent + actionID; //IDs start at zero
		int minimumSize = (agentID+1)*maxActionsPerAgent;

		if (m_data_ptr->size() < (uint32_t)minimumSize) {
			LOG_ERROR("Trying to get changes from agent outside of Client Data range");
			return false;
		}
		if (recepientPtrs.actions_ptr->getDirectLAdataPtr()->size() < (uint32_t)minimumSize) {
			LOG_ERROR("Trying to get changes from agent outside of AS Data range");
			return false;
		}

		AS::actionData_t* ASaction_ptr;

		if(isGlobal){
			ASaction_ptr = &recepientPtrs.actions_ptr->getDirectGAdataPtr()->at(index);
		}
		else {
			ASaction_ptr = &recepientPtrs.actions_ptr->getDirectLAdataPtr()->at(index);
		}

		AS::actionData_t ClientAction = m_data_ptr->at(index);

		ASaction_ptr->details.processingAux = ClientAction.details.processingAux;
		
		return true;
	}




//LOCAL_AGENT
	




//LA_COLD

	bool ClientData::LAcoldDataHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}


	bool ClientData::LAcoldDataHandler::transferID(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::LAcoldDataHandler::transferName(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//LA_STATE
	
	bool ClientData::LAstateHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}


	bool ClientData::LAstateHandler::transferGAid(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::LAstateHandler::transferOnOFF(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//LA_STATE: RELATION

	bool ClientData::LArelationsHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}


	bool ClientData::LArelationsHandler::transferStance(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::LArelationsHandler::transferDisposition(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::LArelationsHandler::transferLastStepDisposition(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}
	


//LA_STATE: LOCATION

	bool ClientData::LocationAndConnectionsHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}


	bool ClientData::LocationAndConnectionsHandler::transferConnectedNeighbours(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//LOCATION: POSITION

	bool ClientData::PositionHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}


	bool ClientData::PositionHandler::transferX(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::PositionHandler::transferY(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//LA_STATE: PARAMS

	bool CL::ClientData::LAparametersHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//PARAMS: RESOURCES

	bool CL::ClientData::LAresourcesHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
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

	bool CL::ClientData::LAresourcesHandler::transferIncome(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//PARAMS: STRENGHT

	bool ClientData::LAstrenghtHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}


	bool ClientData::LAstrenghtHandler::transferCurrent(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::LAstrenghtHandler::transferCurrentUpkeep(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::LAstrenghtHandler::transferGuard(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
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
		
		agentData_ptr->parameters.strenght.externalGuard =
			m_data_ptr->data.at(agentID).parameters.strenght.externalGuard;
		
		return true;
	}

	bool ClientData::LAstrenghtHandler::transferThresholdForUpkeed(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//LA_DECISION

	bool CL::ClientData::LAdecisionDataHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//LA_DECISION: PERSONALITY

	bool ClientData::LApersonalityHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}


	bool ClientData::LApersonalityHandler::transferGAoffsets(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		//TO DO: extract functions
		if (agentID > m_data_ptr->data.size()) {
			LOG_ERROR("Trying to get changes from agent outside of Client Data range");
			return false;
		}
		if (agentID > recepientPtrs.agentData_ptr->LAstate_ptr->getDirectDataPtr()->size()) {
			LOG_ERROR("Trying to get changes from agent outside of AS Data range");
			return false;
		}

		AS::LAdecisionOffsets_t* ClientOffsets_ptr =
	        &m_data_ptr->data[agentID].offsets.incentivesAndConstraintsFromGA;	

		LA::decisionData_t* ASdecision_ptr =
			&recepientPtrs.agentData_ptr->LAdecision_ptr->getDirectDataPtr()->at(agentID);
		AS::LAdecisionOffsets_t* ASoffsets_ptr =
			&ASdecision_ptr->offsets.incentivesAndConstraintsFromGA;
		
		for (int i = 0; i < AS::TOTAL_CATEGORIES; i++) {
			for (int j = 0; j < AS::TOTAL_MODES; j++) {
				(*ASoffsets_ptr)[i][j] = (*ClientOffsets_ptr)[i][j];
			}
		}
		
		return true;	
	}

	bool ClientData::LApersonalityHandler::transferPersonality(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//GA_COLD

	bool ClientData::GAcoldDataHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}


	bool ClientData::GAcoldDataHandler::transferID(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		//TO DO: extract functions
		if (agentID > m_data_ptr->data.size()) {
			LOG_ERROR("Trying to get changes from agent outside of Client Data range");
			return false;
		}
		if (agentID > recepientPtrs.agentData_ptr->GAcoldData_ptr->getDirectDataPtr()->size()) {
			LOG_ERROR("Trying to get changes from agent outside of AS Data range");
			return false;
		}

		GA::ColdDataController* GAdata_ptr = recepientPtrs.agentData_ptr->GAcoldData_ptr;
		std::vector <GA::coldData_t>* dataVector_ptr = GAdata_ptr->getDirectDataPtr();
		GA::coldData_t* agentData_ptr = &(dataVector_ptr->at(agentID));

		agentData_ptr->id = m_data_ptr->data.at(agentID).id;

		return true;
	}

	bool ClientData::GAcoldDataHandler::transferName(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}
	


//GA_STATE
	
	bool ClientData::GAstateHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}


	bool ClientData::GAstateHandler::transferConnectedGAs(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		//TO DO: extract functions
		if (agentID > m_data_ptr->data.size()) {
			LOG_ERROR("Trying to get changes from agent outside of Client Data range");
			return false;
		}
		if (agentID > recepientPtrs.agentData_ptr->GAcoldData_ptr->getDirectDataPtr()->size()) {
			LOG_ERROR("Trying to get changes from agent outside of AS Data range");
			return false;
		}

		GA::StateController* GAstate_ptr = recepientPtrs.agentData_ptr->GAstate_ptr;
		std::vector <GA::stateData_t>* dataVector_ptr = GAstate_ptr->getDirectDataPtr();
		GA::stateData_t* agentData_ptr = &(dataVector_ptr->at(agentID));

		uint32_t newConnections = m_data_ptr->data.at(agentID).connectedGAs.getField();

		agentData_ptr->connectedGAs.loadField(newConnections);

		return true;
	}

	bool ClientData::GAstateHandler::transferLocalAgentsBelongingToThis(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//GA_STATE: RELATION
	
	bool ClientData::GArelationsHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}


	bool ClientData::GArelationsHandler::transferStanceToNeighbour(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::GArelationsHandler::transferDispositionToNeighbour(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::GArelationsHandler::transferLastStepDispositionToNeighbour(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//GA_STATE: GA_PARAMS
	
	bool ClientData::GAparametersHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}


	bool ClientData::GAparametersHandler::transferGAresources(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::GAparametersHandler::transferLAstrenghtTotal(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//GA_PARAMS: RESOURCE_TOTALS
		
	bool ClientData::GAresourcesHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::GAresourcesHandler::transferCurrent(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}
	bool ClientData::GAresourcesHandler::transferIncome(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}



//GA_DECISION

	bool ClientData::GAdecisionDataHandler::transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}


	bool ClientData::GAdecisionDataHandler::transferInfiltration(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::GAdecisionDataHandler::transferPersonality(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}
}
