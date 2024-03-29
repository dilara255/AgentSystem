
//TODO: BUG: CRITICAL: FIX: Proper bounds checking everywhere

#include "systems/clientDataHandler.hpp"

namespace CL{

	//TODO: Update methods to use buildAndPushChangeAndAcquireMutex
	
	//- Checks that the agent is within bounds (if passed the right size);
	//- Creates and pushes the changeDataInfo with agentID and boundTransferFunction;
	//- Acquires the relevant mutex before pushing and KEEPS IT LOCKED;
	//- Returns true if everything worked, false if anything failed;
	//WARNING: if this returns true, THE CALLER is responsible for RELEASING THE LOCK
	bool buildAndPushChangeAndAcquireMutex(uint32_t agentID, int size, 
		std::vector<CL::ClientData::changedDataInfo_t>* changesVector_ptr,
		std::mutex** mutex_ptr_ptr, CL::ClientData::Handler* handler_ptr,
		CL::ClientData::transferFunc_t boundTransferFunction) {

		//Some sanity checking:
		if ( (agentID >= (uint32_t)size) || (agentID < 0) ) {
			LOG_ERROR("Tried to change data for agentID out of range");
			return false;
		}

		//Build the changeDataInfo:
		CL::ClientData::changedDataInfo_t change;
		change.agentID = agentID;
		change.getNewData_fptr = boundTransferFunction;

		//Lock in order to push the change:
		*mutex_ptr_ptr = handler_ptr->acquireMutex();
		if (*mutex_ptr_ptr == NULL) {
			LOG_ERROR("Aborting Client Data Change");
			return false;
		}

		//Store the callback function to transfer the change:
		changesVector_ptr->push_back(change);

		//Will return still locked: the caller will presumably send the changed data
		return true;
	}

//NETWORK

	bool CL_API ClientData::NetworkParameterDataHandler::changeAll(networkParameters_t* newValue_ptr)
	{
		return false;
	}


	bool CL_API ClientData::NetworkParameterDataHandler::changeCommentTo(std::string newValue)
	{

		if (newValue.size() >= COMMENT_LENGHT) {
			LOG_ERROR("Client is trying to pass new network comment line which is too large!");
			return false;
		}

		//Get the callback ready (so we hold the mutex for as little as possible):
		changedDataInfo_t change;
		change.agentID = 0; //doesn't matter for network parameters

		auto callback = std::bind(&CL::ClientData::NetworkParameterDataHandler::transferComment,
			this, std::placeholders::_1, std::placeholders::_2);

		change.getNewData_fptr = callback;

		std::mutex* mutex_ptr = m_parentHandlerPtr->acquireMutex();
		if (mutex_ptr == NULL) {
			LOG_ERROR("Aborting Client Data Change");
			return false;
		}

		//Actually changes the value:
		strcpy(m_data_ptr->comment, newValue.c_str());

		//Store the callback function to transfer the change:
		m_changesVector_ptr->push_back(change);

		mutex_ptr->unlock();

		return true;
	}

	bool CL_API ClientData::NetworkParameterDataHandler::changeIsNetworkInitializedTo(bool newValue)
	{
		return false;
	}

	bool CL_API ClientData::NetworkParameterDataHandler::changePaceTo(float newValue)
	{
		return false;
	}
	
	bool CL_API ClientData::NetworkParameterDataHandler::changeLastMainLoopStartingTickTo(uint64_t newValue)
	{
		return false;
	}

	bool CL_API ClientData::NetworkParameterDataHandler::changeAccumulatedMultiplierTo(double newValue)
	{
		return false;
	}

	bool CL_API ClientData::NetworkParameterDataHandler::changeLastStepTimeMicrosTo(std::chrono::microseconds newValue)
	{
		return false;
	}

	bool CL_API ClientData::NetworkParameterDataHandler::changeLastStepHotMicrosTo(std::chrono::microseconds newValue)
	{
		return false;
	}
	
	bool CL_API ClientData::NetworkParameterDataHandler::changeMainLoopTicksTo(uint64_t newValue)
	{
		return false;
	}

	bool CL_API ClientData::NetworkParameterDataHandler::changeMaxActionsTo(int newValue)
	{
		return false;
	}

	bool CL_API ClientData::NetworkParameterDataHandler::changeMaxLAneighboursTo(int newValue)
	{
		return false;
	}

	bool CL_API ClientData::NetworkParameterDataHandler::changeNameTo(std::string newValue)
	{
		return false;
	}

	bool CL_API ClientData::NetworkParameterDataHandler::changeNumberGAsTo(int newValue)
	{
		return false;
	}

	bool CL_API ClientData::NetworkParameterDataHandler::changeNumberLAsTo(int newValue)
	{
		return false;
	}

	bool CL_API ClientData::NetworkParameterDataHandler::changeSeedsTo(uint64_t seed0, 
		                                uint64_t see1, uint64_t seed2, uint64_t seed3)
	{
		return false;
	}

	bool CL_API ClientData::NetworkParameterDataHandler::changeMakeDecisionsTo(bool newValue)
	{
		//This defines the functions to be used to tansfer the data to the AS:
		auto boundTransferFunction = 
			std::bind(&CL::ClientData::NetworkParameterDataHandler::transferMakeDecisions,
			this, std::placeholders::_1, std::placeholders::_2);

		//This prepares the change data. It shouldn't need to be changed:
		std::mutex* mutex_ptr;
		#pragma warning(push)
		#pragma warning(disable : 4267) //TODO: try to understand the warning : p
		bool result = buildAndPushChangeAndAcquireMutex(0, 1,
			m_changesVector_ptr, &mutex_ptr, m_parentHandlerPtr, boundTransferFunction);
		#pragma warning(pop)
		
		//The data to be transfered is actually recorded here:
		if (result) {
			m_data_ptr->makeDecisions = newValue;
		}

		//Then we clean up and exit:
		if (mutex_ptr != NULL) {
			mutex_ptr->unlock();
		}
		return result;
	}

	bool CL_API ClientData::NetworkParameterDataHandler::changeProcessActionsTo(bool newValue)
	{
		//This defines the functions to be used to tansfer the data to the AS:
		auto boundTransferFunction = 
			std::bind(&CL::ClientData::NetworkParameterDataHandler::transferProcessActions,
			this, std::placeholders::_1, std::placeholders::_2);

		//This prepares the change data. It shouldn't need to be changed:
		std::mutex* mutex_ptr;
		#pragma warning(push)
		#pragma warning(disable : 4267) //TODO: try to understand the warning : p
		bool result = buildAndPushChangeAndAcquireMutex(0, 1,
			m_changesVector_ptr, &mutex_ptr, m_parentHandlerPtr, boundTransferFunction);
		#pragma warning(pop)
		
		//The data to be transfered is actually recorded here:
		if (result) {
			m_data_ptr->processActions = newValue;
		}

		//Then we clean up and exit:
		if (mutex_ptr != NULL) {
			mutex_ptr->unlock();
		}
		return result;
	}

//ACTIONS

	bool CL_API CL::ClientData::ActionsHandler::changeAll(bool isGlobal, uint32_t agentID, uint32_t actionID, actionData_t newValue)
	{
	return false;
	}



//ACTION_IDS

	bool CL_API ClientData::ActionIDsHandler::changeAll(bool isGlobal, uint32_t agentID, uint32_t actionID, AS::ids_t newValue)
	{
		return false;
	}



	bool CL_API ClientData::ActionIDsHandler::changeActiveTo(bool isGlobal, uint32_t agentID, uint32_t actionID, uint32_t newValue)
	{
		return false;
	}

	bool CL_API ClientData::ActionIDsHandler::changeCategoryTo(bool isGlobal, uint32_t agentID, uint32_t actionID, uint32_t newValue)
	{
		return false;
	}

	bool CL_API ClientData::ActionIDsHandler::changeModeTo(bool isGlobal, uint32_t agentID, uint32_t actionID, uint32_t newValue)
	{
		return false;
	}

	bool CL_API ClientData::ActionIDsHandler::changeOriginTo(bool isGlobal, uint32_t agentID, uint32_t actionID, uint32_t newValue)
	{
		return false;
	}

	bool CL_API ClientData::ActionIDsHandler::changeScopeTo(bool isGlobal, uint32_t agentID, uint32_t actionID, uint32_t newValue)
	{
		return false;
	}

	bool CL_API ClientData::ActionIDsHandler::changeTargetTo(bool isGlobal, uint32_t agentID, uint32_t actionID, uint32_t newValue)
	{
		return false;
	}



//ACTION_TICK_INFO

	bool CL_API ClientData::ActionTickInfoHandler::changeAll(bool isGlobal, uint32_t agentID, uint32_t actionID, AS::timeInfo_t newValue)
		{
			return false;
		}

	bool CL_API ClientData::ActionTickInfoHandler::changeInitialTo(bool isGlobal, uint32_t agentID, uint32_t actionID, uint32_t newValue)
	{
		return false;
	}

	bool CL_API ClientData::ActionTickInfoHandler::changeLastProcessedTo(bool isGlobal, uint32_t agentID, uint32_t actionID, uint32_t newValue)
	{
		return false;
	}



//ACTION_DETAILS

	bool CL_API ClientData::ActionDetailsHandler::changeAll(bool isGlobal, uint32_t agentID, uint32_t actionID, AS::details_t newValue)
	{
		return false;
	}

	bool CL_API ClientData::ActionDetailsHandler::changeIntensityTo(bool isGlobal, uint32_t agentID, uint32_t actionID, float newValue)
	{
		return false;
	}

	bool CL_API ClientData::ActionDetailsHandler::changeAuxTo(bool isGlobal, uint32_t agentID, uint32_t actionID, float newValue)
	{
		int maxActionsPerAgent = m_parentHandlerPtr->m_maxActions;
		int index = (agentID)*maxActionsPerAgent + actionID;

		//TODO: extract
		if ((uint32_t)index >= m_data_ptr->size()) {
			LOG_ERROR("Tried to change data for Index out of range");
			
			#if (defined AS_DEBUG) || VERBOSE_RELEASE
				printf("Data has capacity %zu, index was %u\n", 
												m_data_ptr->size(), index);
			#endif // AS_DEBUG

			return false;
		}

		//TODO: EXTRACT
		uint32_t effectiveID = 0;
		effectiveID += ((int)isGlobal << (uint32_t)AS::actionIDtoUnsigned::SCOPE_SHIFT);
		effectiveID += (agentID << (uint32_t)AS::actionIDtoUnsigned::AGENT_SHIFT);
		effectiveID += (actionID << (uint32_t)AS::actionIDtoUnsigned::ACTION_SHIFT);

		changedDataInfo_t change;

		change.agentID = effectiveID;

		//Get the callback ready (so we hold the mutex for as little as possible):
		auto callback = std::bind(&CL::ClientData::ActionDetailsHandler::transferAux,
			this, std::placeholders::_1, std::placeholders::_2);

		change.getNewData_fptr = callback;

		std::mutex* mutex_ptr = m_parentHandlerPtr->acquireMutex();
		if (mutex_ptr == NULL) {
			LOG_ERROR("Aborting Client Data Change");
			return false;
		}

		//Actually changes the value:
		m_data_ptr->at(index).details.processingAux = newValue;

		//Store the callback function to transfer the change:
		m_changesVector_ptr->push_back(change);

		mutex_ptr->unlock();

		return true;
	}



//LOCAL_AGENT: COLD

		bool CL_API ClientData::LAcoldDataHandler::changeAll(uint32_t agentID, LA::coldData_t* newValue_ptr)
	{
		return false;
	}


	bool CL_API ClientData::LAcoldDataHandler::changeID(uint32_t agentID, uint32_t newValue)
	{
		return false;
	}

	bool CL_API ClientData::LAcoldDataHandler::changeName(uint32_t agentID, std::string newValue)
	{
		return false;
	}
	


//LOCAL_AGENT: STATE

	bool CL_API CL::ClientData::LAstateHandler::changeAll(uint32_t agentID, LA::stateData_t * newValue_ptr)
	{
		return false;
	}


	bool CL_API CL::ClientData::LAstateHandler::changeGAid(uint32_t agentID, uint32_t newValue)
	{
		return false;
	}

	bool CL_API CL::ClientData::LAstateHandler::changeOnOFF(uint32_t agentID, bool newValue)
	{
		return false;
	}

	bool CL_API CL::ClientData::LAstateHandler::changeUnderAttack(uint32_t agentID, int newValue)
	{
		return false;
	}

//STATE: PARAMS: RELATIONS
	
	bool CL_API ClientData::LArelationsHandler::changeAll(uint32_t agentID, AS::LAneighborRelations_t* newValue_ptr)
	{
		return false;
	}


	bool CL_API ClientData::LArelationsHandler::changeStance(uint32_t ofAgentID, uint32_t towardsAgentID, int newValue)
	{
		return false;
	}

	bool CL_API ClientData::LArelationsHandler::changeDisposition(uint32_t ofAgentID, uint32_t towardsAgentID, float newValue)
	{
		return false;
	}

	bool CL_API ClientData::LArelationsHandler::changeLastStepDisposition(uint32_t ofAgentID, uint32_t towardsAgentID, float newValue)
	{
		return false;
	}



//STATE: LOCATION

	bool CL_API ClientData::LocationAndConnectionsHandler::changeAll(uint32_t agentID, AS::LAlocationAndConnectionData_t* newValue_ptr)
	{
		return false;
	}

	bool CL_API ClientData::LocationAndConnectionsHandler::changeConnectedNeighbours(uint32_t AgentID, AS::LAflagField_t* newvalue_ptr)
	{
		return false;
	}



//LOCATION: POSITION

	bool CL_API ClientData::PositionHandler::changeAll(uint32_t agentID, AS::pos_t newValue)
	{
		return false;
	}

	bool CL_API ClientData::PositionHandler::changeX(uint32_t AgentID, float newValue)
	{
		return false;
	}

	bool CL_API ClientData::PositionHandler::changeY(uint32_t AgentID, float newValue)
	{
		return false;
	}



//STATE: PARAMS: 
	
	bool CL_API CL::ClientData::LAparametersHandler::changeAll(uint32_t agentID, AS::LAparameters_t newValue)
	{
		return false;
	}

//PARAMS: RESOURCES

	bool CL_API CL::ClientData::LAresourcesHandler::changeAll(uint32_t agentID, AS::resources_t newValue)
	{
		return false;
	}


	bool ClientData::LAresourcesHandler::changeCurrentTo(uint32_t agentID, float newValue) {
		
		//TODO: extract
		if ( (agentID >= m_data_ptr->data.size()) || (agentID < 0) ) {
			LOG_ERROR("Tried to change data for agentID out of range");
			
			#if (defined AS_DEBUG) || VERBOSE_RELEASE
				printf("Data has capacity %zu, aid was %u\n", 
												m_data_ptr->data.size(), agentID);
			#endif // AS_DEBUG

			return false;
		}

		//
		changedDataInfo_t change;
		change.agentID = agentID;

		//Get the callback ready (so we hold the mutex for as little as possible):
		auto callback = std::bind(&CL::ClientData::LAresourcesHandler::transferCurrent,
			this, std::placeholders::_1, std::placeholders::_2);

		change.getNewData_fptr = callback;

		std::mutex* mutex_ptr = m_parentHandlerPtr->acquireMutex();
		if (mutex_ptr == NULL) {
			LOG_ERROR("Aborting Client Data Change");
			return false;
		}

		//Actually changes the value:
		m_data_ptr->data[agentID].parameters.resources.current = newValue;

		//Store the callback function to transfer the change:
		m_changesVector_ptr->push_back(change);

		mutex_ptr->unlock();

		return true;
	}

	bool CL_API CL::ClientData::LAresourcesHandler::changeIncomeTo(uint32_t agentID, float newValue)
	{
		return false;
	}

	bool CL_API CL::ClientData::LAresourcesHandler::changeTaxRateTo(uint32_t agentID, float newValue)
	{
		return false;
	}

	bool CL_API CL::ClientData::LAresourcesHandler::changeTradeRateTo(uint32_t agentID, float newValue)
	{
		return false;
	}

//PARAMS: STRENGHT

	
	bool CL_API ClientData::LAstrenghtHandler::changeAll(uint32_t agentID, AS::strenght_t newValue)
	{
		return false;
	}


	bool CL_API ClientData::LAstrenghtHandler::changeCurrent(uint32_t agentID, float newValue)
	{
		return false;
	}

	bool CL_API ClientData::LAstrenghtHandler::changeCurrentUpkeep(uint32_t agentID, float newValue)
	{
		return false;
	}

	bool CL_API ClientData::LAstrenghtHandler::changeGuard(uint32_t agentID, float newValue)
	{
		//TODO: extract
		if ( (agentID >= m_data_ptr->data.size())  || (agentID < 0)) {
			LOG_ERROR("Tried to change data for agentID out of range");
			
			#if (defined AS_DEBUG) || VERBOSE_RELEASE
				printf("Data has capacity %zu, aid was %u\n", 
												m_data_ptr->data.size(), agentID);
			#endif // AS_DEBUG

			return false;
		}

		//
		changedDataInfo_t change;
		change.agentID = agentID;

		//Get the callback ready (so we hold the mutex for as little as possible):
		auto callback = std::bind(&CL::ClientData::LAstrenghtHandler::transferGuard,
			this, std::placeholders::_1, std::placeholders::_2);

		change.getNewData_fptr = callback;

		std::mutex* mutex_ptr = m_parentHandlerPtr->acquireMutex();
		if (mutex_ptr == NULL) {
			LOG_ERROR("Aborting Client Data Change");
			return false;
		}

		//Actually changes the value:
		m_data_ptr->data[agentID].parameters.strenght.externalGuard = newValue;

		//Store the callback function to transfer the change:
		m_changesVector_ptr->push_back(change);

		mutex_ptr->unlock();

		return true;
	}

	bool CL_API ClientData::LAstrenghtHandler::changeThresholdForUpkeed(uint32_t agentID, float newValue)
	{
		return false;
	}

	bool CL_API ClientData::LAstrenghtHandler::changeOnAttacks(uint32_t agentID, float newValue)
	{
		return false;
	}

	bool CL_API ClientData::LAstrenghtHandler::changeAttritionLossRate(uint32_t agentID, float newValue)
	{
		return false;
	}

//LOCAL_AGENT: DECISION

	bool CL_API CL::ClientData::LAdecisionDataHandler::changeAll(uint32_t agentID, LA::decisionData_t * newValue_ptr)
	{
		return false;
	}

	bool CL_API CL::ClientData::LAdecisionDataHandler::changeShouldMakeDecisions(uint32_t agentID, bool should)
	{
		//This defines the functions to be used to tansfer the data to the AS:
		auto boundTransferFunction = 
			std::bind(&CL::ClientData::LAdecisionDataHandler::transferShouldMakeDecisions,
			this, std::placeholders::_1, std::placeholders::_2);

		//This prepares the change data. It shouldn't need to be changed:
		std::mutex* mutex_ptr;
		#pragma warning(push)
		#pragma warning(disable : 4267) //TODO: try to understand the warning : p
		bool result = buildAndPushChangeAndAcquireMutex(agentID, m_data_ptr->data.size(),
			m_changesVector_ptr, &mutex_ptr, m_parentHandlerPtr, boundTransferFunction);
		#pragma warning(pop)
		
		//The data to be transfered is actually recorded here:
		if (result) {
			m_data_ptr->data[agentID].shouldMakeDecisions = should;
		}

		//Then we clean up and exit:
		if (mutex_ptr != NULL) {
			mutex_ptr->unlock();
		}
		return result;
	}

	bool CL_API CL::ClientData::LAdecisionDataHandler::changeInfiltrationOnAll(uint32_t agentID, float newInfiltrationToAll)
	{
		//This defines the functions to be used to tansfer the data to the AS:
		auto boundTransferFunction = 
			std::bind(&CL::ClientData::LAdecisionDataHandler::transferInfiltrationOnAll,
			this, std::placeholders::_1, std::placeholders::_2);

		//This prepares the change data. It shouldn't need to be changed:
		std::mutex* mutex_ptr;
		#pragma warning(push)
		#pragma warning(disable : 4267) //TODO: try to understand the warning : p
		bool result = buildAndPushChangeAndAcquireMutex(agentID, m_data_ptr->data.size(),
			m_changesVector_ptr, &mutex_ptr, m_parentHandlerPtr, boundTransferFunction);
		#pragma warning(pop)
		
		//The data to be transfered is actually recorded here:
		if (result) {
			for (int i = 0; i < MAX_LA_NEIGHBOURS; i++) {
				m_data_ptr->data[agentID].infiltration[i] = newInfiltrationToAll;
			}			
		}

		//Then we clean up and exit:
		if (mutex_ptr != NULL) {
			mutex_ptr->unlock();
		}
		return result;
	}


//DECISION: PERSONALITY

	bool CL_API CL::ClientData::LApersonalityHandler::changeAll(uint32_t agentID, AS::LApersonalityAndGAinfluence_t * newValue_ptr)
	{
		return false;
	}


	bool CL_API CL::ClientData::LApersonalityHandler::changeGAoffsets(uint32_t agentID, AS::LAdecisionOffsets_t * newValue_ptr)
	{
		//TODO: extract
		if ( (agentID >= m_data_ptr->data.size()) || (agentID < 0) ) {
			LOG_ERROR("Tried to change data for agentID out of range");
			
			#if (defined AS_DEBUG) || VERBOSE_RELEASE
				printf("Data has capacity %zu, aid was %u\n", 
												m_data_ptr->data.size(), agentID);
			#endif // AS_DEBUG

			return false;
		}

		//
		changedDataInfo_t change;
		change.agentID = agentID;

		//Get the callback ready (so we hold the mutex for as little as possible):
		auto callback = std::bind(&CL::ClientData::LApersonalityHandler::transferGAoffsets,
			this, std::placeholders::_1, std::placeholders::_2);

		change.getNewData_fptr = callback;

		std::mutex* mutex_ptr = m_parentHandlerPtr->acquireMutex();
		if (mutex_ptr == NULL) {
			LOG_ERROR("Aborting Client Data Change");
			return false;
		}

		//Actually changes the value:
		AS::LAdecisionOffsets_t* NewOffsets_ptr =
	        &m_data_ptr->data[agentID].offsets.incentivesAndConstraintsFromGA;	

		for (int i = 0; i < (int)AS::actCategories::TOTAL; i++) {
			for (int j = 0; j < (int)AS::actModes::TOTAL; j++) {
				(*NewOffsets_ptr)[i][j] = (*newValue_ptr)[i][j];
			}
		}		

		//Store the callback function to transfer the change:
		m_changesVector_ptr->push_back(change);

		mutex_ptr->unlock();

		return true;
	}

	bool CL_API CL::ClientData::LApersonalityHandler::changePersonality(uint32_t agentID, AS::LAdecisionOffsets_t * newValue_ptr)
	{
		return false;
	}
	


//GLOBAL_AGENT: COLD
	
	bool CL_API ClientData::GAcoldDataHandler::changeAll(uint32_t agentID, GA::coldData_t* newValue_ptr)
	{
		return false;
	}


	bool CL_API ClientData::GAcoldDataHandler::changeID(uint32_t agentID, uint32_t newValue)
	{
		//TODO: extract
		if ( (agentID >= m_data_ptr->data.size()) || (agentID < 0) ) {
			LOG_ERROR("Tried to change data for agentID out of range");
			
			#if (defined AS_DEBUG) || VERBOSE_RELEASE
				printf("Data has capacity %zu, aid was %u\n", 
												m_data_ptr->data.size(), agentID);
			#endif // AS_DEBUG

			return false;
		}

		//
		changedDataInfo_t change;
		change.agentID = agentID;

		//Get the callback ready (so we hold the mutex for as little as possible):
		auto callback = std::bind(&CL::ClientData::GAcoldDataHandler::transferID,
			this, std::placeholders::_1, std::placeholders::_2);

		change.getNewData_fptr = callback;

		std::mutex* mutex_ptr = m_parentHandlerPtr->acquireMutex();
		if (mutex_ptr == NULL) {
			LOG_ERROR("Aborting Client Data Change");
			return false;
		}

		//Actually changes the value:
		m_data_ptr->data[agentID].id = newValue;

		//Store the callback function to transfer the change:
		m_changesVector_ptr->push_back(change);

		mutex_ptr->unlock();

		return true;
	}

	bool CL_API ClientData::GAcoldDataHandler::changeName(uint32_t agentID, std::string newValue)
	{
		return false;
	}
	


//GLOBAL_AGENT: STATE
		

	bool CL_API ClientData::GAstateHandler::changeAll(uint32_t agentID, GA::stateData_t* newValue_ptr)
	{
		return false;
	}

	//****************************************************************
	//TODO-CRITICAL-BUG-WARNING:
	//===> NEIGHBOR CONNECTION CHANGES HAVE TO BE SYMMETRICAL, ALWAYS;
	//****************************************************************
	bool CL_API ClientData::GAstateHandler::changeConnectedGAs(uint32_t agentID, 
		                                        AS::GAflagField_t* newValue_ptr)
	{
		//TODO: BUG: CRITICAL: FIX: Bounds checking
		//This defines the functions to be used to tansfer the data to the AS:
		auto boundTransferFunction = 
			std::bind(&CL::ClientData::GAstateHandler::transferConnectedGAs,
			this, std::placeholders::_1, std::placeholders::_2);

		//This prepares the change data. It shouldn't need to be changed:
		std::mutex* mutex_ptr;
		#pragma warning(push)
		#pragma warning(disable : 4267) //TODO: try to understand the warning : p
		bool result = buildAndPushChangeAndAcquireMutex(agentID, m_data_ptr->data.size(),
			m_changesVector_ptr, &mutex_ptr, m_parentHandlerPtr, boundTransferFunction);
		#pragma warning(pop)
		
		//The data to be transfered is actually recorded here:
		if (result) {
			m_data_ptr->data[agentID].connectedGAs.loadField(newValue_ptr->getField());
		}

		//Then we clean up and exit:
		if (mutex_ptr != NULL) {
			mutex_ptr->unlock();
		}
		return result;
	}

	bool CL_API ClientData::GAstateHandler::changeLocalAgentsBelongingToThis(uint32_t agentID, AS::LAflagField_t* newValue_ptr)
	{
		return false;
	}

	bool CL_API ClientData::GAstateHandler::changeOnOff(uint32_t agentID, bool newValue)
	{
		return false;
	}



//STATE: RELATIONS
	
	bool CL_API ClientData::GArelationsHandler::changeAll(uint32_t agentID, AS::GAneighborRelations_t* newValue_ptr)
	{
		return false;
	}


	bool CL_API ClientData::GArelationsHandler::changeStanceToNeighbour(uint32_t agentID, uint32_t neighbourID, int newValue)
	{
		return false;
	}

	bool CL_API ClientData::GArelationsHandler::changeDispositionToNeighbour(uint32_t agentID, uint32_t neighbourID, int newValue)
	{
		return false;
	}

	bool CL_API ClientData::GArelationsHandler::changeLastStepDispositionToNeighbour(uint32_t agentID, uint32_t neighbourID, int newValue)
	{
		return false;
	}



//STATE: PARAMS

	bool CL_API ClientData::GAparametersHandler::changeAll(uint32_t agentID, AS::GAparameterTotals_t newValue)
	{
		return false;
	}


	bool CL_API ClientData::GAparametersHandler::changeGAresourcesTo(uint32_t agentID, float newValue)
	{
		//TODO: BUG: CRITICAL: BOUNDS CHECKING
		
		//This defines the functions to be used to tansfer the data to the AS:
		auto boundTransferFunction = 
			std::bind(&CL::ClientData::GAparametersHandler::transferGAresources,
			this, std::placeholders::_1, std::placeholders::_2);

		//This prepares the change data. It shouldn't need to be changed:
		std::mutex* mutex_ptr;
		#pragma warning(push)
		#pragma warning(disable : 4267) //TODO: try to understand the warning : p
		bool result = buildAndPushChangeAndAcquireMutex(agentID, m_data_ptr->data.size(),
			m_changesVector_ptr, &mutex_ptr, m_parentHandlerPtr, boundTransferFunction);
		#pragma warning(pop)

		//Actually changes the value:
		if (result) {
			m_data_ptr->data[agentID].parameters.GAresources = newValue;
		}

		//Then we clean up and exit:
		if (mutex_ptr != NULL) {
			mutex_ptr->unlock();
		}
		return result;
	}

	bool CL_API ClientData::GAparametersHandler::changeLAstrenghtTotal(uint32_t agentID, float newValue)
	{
		return false;
	}



//PARAMS: RESOURCES

	bool CL_API ClientData::GAresourcesHandler::changeAll(uint32_t agentID, AS::resources_t newValue)
	{
		return false;
	}


	bool CL_API ClientData::GAresourcesHandler::changeCurrent(uint32_t agentID, float newValue)
	{
		return false;
	}

	bool CL_API ClientData::GAresourcesHandler::changeIncome(uint32_t agentID, float newValue)
	{
		return false;
	}
	


//GLOBAL_AGENT: DECISION
	
	bool CL_API ClientData::GAdecisionDataHandler::changeAll(uint32_t agentID, GA::decisionData_t* newValue_ptr)
	{
		return false;
	}


	bool CL_API ClientData::GAdecisionDataHandler::changePersonality(uint32_t agentID, AS::GApersonality* newValue_ptr)
	{
		return false;
	}

	bool CL_API CL::ClientData::GAdecisionDataHandler::changeShouldMakeDecisions(uint32_t agentID, bool should)
	{
		//This defines the functions to be used to tansfer the data to the AS:
		auto boundTransferFunction = 
			std::bind(&CL::ClientData::GAdecisionDataHandler::transferShouldMakeDecisions,
			this, std::placeholders::_1, std::placeholders::_2);

		//This prepares the change data. It shouldn't need to be changed:
		std::mutex* mutex_ptr;
		#pragma warning(push)
		#pragma warning(disable : 4267) //TODO: try to understand the warning : p
		bool result = buildAndPushChangeAndAcquireMutex(agentID, m_data_ptr->data.size(),
			m_changesVector_ptr, &mutex_ptr, m_parentHandlerPtr, boundTransferFunction);
		#pragma warning(pop)
		
		//The data to be transfered is actually recorded here:
		if (result) {
			m_data_ptr->data[agentID].shouldMakeDecisions = should;
		}

		//Then we clean up and exit:
		if (mutex_ptr != NULL) {
			mutex_ptr->unlock();
		}
		return result;
	}

	bool CL_API CL::ClientData::GAdecisionDataHandler::changeInfiltrationOnAll(uint32_t agentID, float newInfiltrationToAll)
	{
		//This defines the functions to be used to tansfer the data to the AS:
		auto boundTransferFunction = 
			std::bind(&CL::ClientData::GAdecisionDataHandler::transferInfiltrationOnAll,
			this, std::placeholders::_1, std::placeholders::_2);

		//This prepares the change data. It shouldn't need to be changed:
		std::mutex* mutex_ptr;
		#pragma warning(push)
		#pragma warning(disable : 4267) //TODO: try to understand the warning : p
		bool result = buildAndPushChangeAndAcquireMutex(agentID, m_data_ptr->data.size(),
			m_changesVector_ptr, &mutex_ptr, m_parentHandlerPtr, boundTransferFunction);
		#pragma warning(pop)
		
		//The data to be transfered is actually recorded here:
		if (result) {
			for (int i = 0; i < MAX_GA_QUANTITY; i++) {
				m_data_ptr->data[agentID].infiltration[i] = newInfiltrationToAll;
			}			
		}

		//Then we clean up and exit:
		if (mutex_ptr != NULL) {
			mutex_ptr->unlock();
		}
		return result;
	}
}



