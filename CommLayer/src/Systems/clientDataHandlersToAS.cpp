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
		return false;
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
		return false;
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
		return false;
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
		return false;
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
		return false;
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


	bool ClientData::GAstateHandler::transferGAresources(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
	{
		return false;
	}

	bool ClientData::GAstateHandler::transferLAstrenghtTotal(uint32_t agentID, ASdataControlPtrs_t recepientPtrs)
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
