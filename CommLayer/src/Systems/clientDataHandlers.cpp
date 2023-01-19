#include "systems/clientDataHandler.hpp"

namespace CL {

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