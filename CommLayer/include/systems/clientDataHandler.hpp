#pragma once

/*
//PLANNING:
//- TODO: Deal with minor 4 and then get back;
//- TODO-CRITICAL: Test pointer-math transfer idea;
//- TODO-CRITICAL: All Methods which are already stubbed or equivalent (depending on test above);
//- TO DO LATER:
//-- Relative methods were they'd make sense (addition and multiplication);
//-- Methods for "all agents" (including for  "entire structure");
*/

/*
Here we declare the class ClientDataHandler, its component classes and a helper type.
API: External: the Client will have direct acces to an instance of this class.
API: Internal: The AS will only have acces to a "CL::getNewClientData()" function.

ClientDataHandler:
- Takes data from the Client and stores it in a member mirror_t;
- Stores which changes have been made in a member vector of changedDataInfo_t;
-- Both are private. TODO: did the vector need to be warped for easier dll exporting?
- Retrieves changed data from the mirror_t;
- Handles mutex to the mirror_t;
- Clears the mirror_t after full retrieval from the AS.

The data handling classes:
- Form a composition hierarquy based on data categories and their respective fields;
- This hierarquy culminates on the ClientDataHandler class, which instantiates it's components;
- These classes mirror the structure of the actual data stored in mirror_t.

The type changedDataInfo_t holds information about a single change by the Client :
-It describes which field was changed, in a uniform way;
-A vector of these will guide the handler in deciding which data to retrieve for the AS;
-This vector will be cleared after that;
-Both the vector and the Client data on the CL will be locked during this process.

Usage:

By the CLIENT:

Example: given a CL::ClientData::Handler clientDataHandler, the Client CALLS:
	clientDataHandler.LAstate.parameters.resources.changeCurrentTo(agentID, newValue);

	What happens is:
		- clientDataHandler acquires mutex;
		- changeCurrentTo creates relevant element at the back of the changes vector:
			{LA_STATE, agentId, PARAMETERS, RESOURCES, CURRENT, true};
		- changeCurrentTo updates the data:
			data_ptr->data[agentID].parameters.resources.current = newValue;
		- clientDataHandler releases the mutex;

By the AS:

At each step, the AS CALLS CL::retrieveAndEraseClientChanges(&recepient), from CL_Internal API;
- This calls a method of the same name from the ClientData::Handler, which:
	- Acquires the mutex;
	- Loops through the changes vector;
	- On hasChanges == true, dispatches the relevant method from the relevant handler member;
		-- The recipient pointer is passed, and the method transfers the relevant Data;
	- After the loop is done, clears the changes vector;
	- Finally, releases the mutex;

* TODO:
* - CL::ClientData::BaseFieldHandler;
*/

#include "miscDefines.hpp"

#include "dataMirror.hpp"
#include "../include/data/agentDataControllers.hpp"

namespace CL::ClientData {
	
	typedef struct {
		AS::networkParameters_t* params_ptr;
		AS::dataControllerPointers_t* agentData_ptr;
		AS::ActionDataController* actions_ptr;
	} ASdataControlPtrs_t;

	typedef std::function <bool(uint32_t id, ASdataControlPtrs_t recepientPtrs)> transferFunc_t;

	typedef struct {
		uint32_t agentID;
		transferFunc_t getNewData_fptr;
	} changedDataInfo_t;

	class Handler; //forward declaration. Actual declaration way down bellow

//BASE

	class BaseSubHandler {
	protected:
		friend class Handler;

		virtual bool initialize(ClientData::Handler* parentHandlerPtr,
			std::vector <changedDataInfo_t>* changesVector_ptr);

		virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs) = 0;

		ClientData::Handler* m_parentHandlerPtr;
		std::vector <changedDataInfo_t>* m_changesVector_ptr;
	};

//NETWORK

	class NetworkParameterDataHandler: public BaseSubHandler {
	public:
		
		bool CL_API changeAll(uint32_t agentID, networkParameters_t* newValue_ptr);
		
		bool CL_API changeCommentTo(uint32_t agentID, std::string newValue);
		bool CL_API changeIsNetworkInitializedTo(uint32_t agentID, bool newValue);
		bool CL_API changeLastMainLoopStartingTickTo(uint32_t agentID, uint64_t newValue);
		bool CL_API changeMainLoopTicksTo(uint32_t agentID, uint64_t newValue);
		bool CL_API changeMaxActionsTo(uint32_t agentID, int newValue);
		bool CL_API changeMaxLAneighboursTo(uint32_t agentID, int newValue);
		bool CL_API changeNameTo(uint32_t agentID, std::string newValue);
		bool CL_API changeNumberGAsTo(uint32_t agentID, int newValue);
		bool CL_API changeNumberLAsTo(uint32_t agentID, int newValue);

	protected:
		friend class Handler;

		bool initialize(ClientData::Handler* parentHandlerPtr, networkParameters_t* data_ptr,
						std::vector <changedDataInfo_t>* changesVector_ptr);
		
		virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);

		bool transferComment(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		bool transferIsNetworkInitialized(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		bool transferLastMainLoopStartingTick(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		bool transferMainLoopTicks(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		bool transferMaxActions(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		bool transferMaxLAneighbours(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		bool transferName(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		bool transferNumberGAs(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		bool transferNumberLAs(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);

		networkParameters_t* m_data_ptr;
	};

//ACTION
	
	//Action component Handlers:

		class ActionIDsHandler: public BaseSubHandler {
		public:
			//TODO: Pack agentID, actionID and scope in (uint32_t)AS::ids_t
			bool CL_API changeAll(bool isGlobal, uint32_t agentID, uint32_t actionID, AS::ids_t newValue);
			
			//TODO: Pack agentID, actionID and scope in (uint32_t)AS::ids_t
			bool CL_API changeActiveTo(bool isGlobal, uint32_t agentID, uint32_t actionID, uint32_t newValue);
			//TODO: Pack agentID, actionID and scope in (uint32_t)AS::ids_t
			bool CL_API changeCategoryTo(bool isGlobal, uint32_t agentID, uint32_t actionID, uint32_t newValue);
			//TODO: Pack agentID, actionID and scope in (uint32_t)AS::ids_t
			bool CL_API changeModeTo(bool isGlobal, uint32_t agentID, uint32_t actionID, uint32_t newValue);
			//TODO: Pack agentID, actionID and scope in (uint32_t)AS::ids_t
			bool CL_API changeOriginTo(bool isGlobal, uint32_t agentID, uint32_t actionID, uint32_t newValue);
			//TODO: Pack agentID, actionID and scope in (uint32_t)AS::ids_t
			bool CL_API changeScopeTo(bool isGlobal, uint32_t agentID, uint32_t actionID, uint32_t newValue);
			//TODO: Pack agentID, actionID and scope in (uint32_t)AS::ids_t
			bool CL_API changeTargetTo(bool isGlobal, uint32_t agentID, uint32_t actionID, uint32_t newValue);

		protected:
			friend class ActionsHandler;

			bool initialize(ClientData::Handler* parentHandlerPtr, 
							std::vector <actionData_t>* data_ptr,
							std::vector <changedDataInfo_t>* changesVector_ptr);
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			bool transferActive(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			bool transferCategory(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			bool transferMode(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			bool transferOrigin(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			bool transferScope(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			bool transferTarget(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			
			std::vector <actionData_t>* m_data_ptr;
		};

		class ActionTickInfoHandler: public BaseSubHandler {
		public:
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			bool CL_API changeAll(bool isGlobal, uint32_t agentID, uint32_t actionID, AS::tickInfo_t newValue);
		
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			bool CL_API changeInitialTo(bool isGlobal, uint32_t agentID, uint32_t actionID, uint32_t newValue);
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			bool CL_API changeLastProcessedTo(bool isGlobal, uint32_t agentID, uint32_t actionID, uint32_t newValue);

		protected:
			friend class ActionsHandler;

			bool initialize(ClientData::Handler* parentHandlerPtr, 
							std::vector <actionData_t>* data_ptr,
							std::vector <changedDataInfo_t>* changesVector_ptr);
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			bool transferInitial(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			bool transferLastProcessed(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			
			std::vector <actionData_t>* m_data_ptr;
		};

		class ActionDetailsHandler: public BaseSubHandler {
		public:
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			bool CL_API changeAll(bool isGlobal, uint32_t agentID, uint32_t actionID, AS::details_t newValue);

			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			bool CL_API changeIntensityTo(bool isGlobal, uint32_t agentID, uint32_t actionID, float newValue);
			//TODO-CRITICAL: UPDATE COMMENTS HERE : )
			bool CL_API changeAuxTo(bool isGlobal, uint32_t agentID, uint32_t actionID, float newValue);

		protected:
			friend class ActionsHandler;

			bool initialize(ClientData::Handler* parentHandlerPtr, 
							std::vector <actionData_t>* data_ptr,
							std::vector <changedDataInfo_t>* changesVector_ptr);
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			bool transferIntensity(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
			bool transferAux(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			
			std::vector <actionData_t>* m_data_ptr;
		};

	class ActionsHandler: public BaseSubHandler {
	public:
		//TODO: Pack agentID, actionID and scope in (uint32_t)AS::ids_t
		bool CL_API changeAll(bool isGlobal, uint32_t agentID, uint32_t actionID, actionData_t newValue);

		ActionIDsHandler IDs;
		ActionTickInfoHandler tickInfo;
		ActionDetailsHandler details;

	protected:
		friend class Handler;

		bool initialize(ClientData::Handler* parentHandlerPtr, 
						std::vector <actionData_t>* data_ptr,
						std::vector <changedDataInfo_t>* changesVector_ptr);

		//(AS::ids_t)agentID. origin -> agent, target -> action, scope -> (int)"bool isGlobal"
		virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);

		std::vector <actionData_t>* m_data_ptr;
	};

//LOCAL_AGENT: COLD

	class LAcoldDataHandler: public BaseSubHandler {
	public:
		
		bool CL_API changeAll(uint32_t agentID, LA::coldData_t* newValue_ptr);

		bool CL_API changeID(uint32_t agentID, uint32_t newValue);
		bool CL_API changeName(uint32_t agentID, std::string newValue);

	protected:
		friend class Handler;

		bool initialize(ClientData::Handler* parentHandlerPtr, ColdDataControllerLA* data_ptr,
						std::vector <changedDataInfo_t>* changesVector_ptr);

		virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		
			bool transferID(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			bool transferName(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		
		ColdDataControllerLA* m_data_ptr;
	};

//LOCAL_AGENT: STATE

	//LAstate component Handlers:

		class LArelationsHandler: public BaseSubHandler {
		public:
		
			bool CL_API changeAll(uint32_t agentID, AS::LAneighborRelations_t* newValue_ptr);

			//TODO: Put IDs on low and high 16 bits of changes.agentId
			bool CL_API changeStance(uint32_t ofAgentID, uint32_t towardsAgentID, int newValue);
			bool CL_API changeDisposition(uint32_t ofAgentID, uint32_t towardsAgentID, 
				                                                       float newValue);
			bool CL_API changeLastStepDisposition(uint32_t ofAgentID, uint32_t towardsAgentID,
				                                                               float newValue);

		protected:
			friend class LAstateHandler;

			bool initialize(ClientData::Handler* parentHandlerPtr,
							StateControllerLA* data_ptr,
							std::vector <changedDataInfo_t>* changesVector_ptr);

			virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			
			//TODO: from one agent toward another: IDs on low and high 16 bits agentId
			bool transferStance(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			//TODO: from one agent toward another: IDs on low and high 16 bits agentId
			bool transferDisposition(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			//TODO: from one agent toward another: IDs on low and high 16 bits agentId
			bool transferLastStepDisposition(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			
			StateControllerLA* m_data_ptr;
		};

			class PositionHandler: public BaseSubHandler {
			public:
		
				bool CL_API changeAll(uint32_t agentID, AS::pos_t newValue);

				bool CL_API changeX(uint32_t AgentID, float newValue);
				bool CL_API changeY(uint32_t AgentID, float newValue);

			protected:
				friend class LocationAndConnectionsHandler;

				bool initialize(ClientData::Handler* parentHandlerPtr, 
								StateControllerLA* data_ptr,
								std::vector <changedDataInfo_t>* changesVector_ptr);

				virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
				
				bool transferX(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
				bool transferY(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
				
				StateControllerLA* m_data_ptr;
			};

		class LocationAndConnectionsHandler: public BaseSubHandler {
		public:
		
			bool CL_API changeAll(uint32_t agentID,
				                 AS::LAlocationAndConnectionData_t* newValue_ptr);

				//TODO: mark that the other fields must be updated!
				bool CL_API changeConnectedNeighbours(uint32_t AgentID, 
					                                  AS::LAflagField_t* newvalue_ptr);
				
			PositionHandler position;

		protected:
			friend class LAstateHandler;

			bool initialize(ClientData::Handler* parentHandlerPtr, 
				            StateControllerLA* data_ptr,
		                    std::vector <changedDataInfo_t>* changesVector_ptr);

			virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
				
				bool transferConnectedNeighbours(uint32_t agentID, 
				                                 ASdataControlPtrs_t recepientPtrs);
				
			StateControllerLA* m_data_ptr;
		};
		
			class LAresourcesHandler: public BaseSubHandler {
			public:
				//Blocks Client Data.
				bool CL_API changeAll(uint32_t agentID, AS::resources_t* newValue_ptr);
				
				//Blocks Client Data.
				bool CL_API changeCurrentTo(uint32_t agentID, float newValue);

				//Blocks Client Data.
				bool CL_API changeIncomeTo(uint32_t agentID, float newValue);

			private:
				friend class LAparametersHandler;

				bool initialize(ClientData::Handler* parentHandlerPtr,
								StateControllerLA* data_ptr,
								std::vector <changedDataInfo_t>* changesVector_ptr);

				virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);

				//Client Data should be blocked upstream from this
				bool transferCurrent(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
				
				bool transferIncome(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
				
				StateControllerLA* m_data_ptr;
			};

			class LAstrenghtHandler: public BaseSubHandler {
			public:
		
				bool CL_API changeAll(uint32_t agentID, AS::strenght_t* newValue_ptr);

				bool CL_API changeCurrent(uint32_t agentID, float newValue);
				bool CL_API changeCurrentUpkeep(uint32_t agentID, float newValue);
				bool CL_API changeGuard(uint32_t agentID, float newValue);
				bool CL_API changeThresholdForUpkeed(uint32_t agentID, float newValue);

			protected:
				friend class LAparametersHandler;

				bool initialize(ClientData::Handler* parentHandlerPtr, 
								StateControllerLA* data_ptr,
								std::vector <changedDataInfo_t>* changesVector_ptr);

				virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
				
				bool transferCurrent(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
				bool transferCurrentUpkeep(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
				bool transferGuard(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
				bool transferThresholdForUpkeed(uint32_t agentID, 
				                                ASdataControlPtrs_t recepientPtrs);
				
				StateControllerLA* m_data_ptr;
			};

		class LAparametersHandler: public BaseSubHandler {
		public:
			bool CL_API changeAll(uint32_t agentID, AS::LAparameters_t* newValue_ptr);

			LAresourcesHandler resources;
			LAstrenghtHandler strenght;

		private:
			friend class LAstateHandler;

			bool initialize(ClientData::Handler* parentHandlerPtr, StateControllerLA* data_ptr,
				                     std::vector <changedDataInfo_t>* changesVector_ptr);

			virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);

			StateControllerLA* m_data_ptr;
		};

	class LAstateHandler: public BaseSubHandler {
	public:
		bool CL_API changeAll(uint32_t agentID, LA::stateData_t* newValue_ptr);

		bool CL_API changeGAid(uint32_t agentID, uint32_t newValue);
		bool CL_API changeOnOFF(uint32_t agentID, bool newValue);

		LArelationsHandler relations;
		LocationAndConnectionsHandler location;
		LAparametersHandler parameters;

	protected:
		friend class Handler;

		bool initialize(ClientData::Handler* parentHandlerPtr, StateControllerLA* data_ptr,
			                             std::vector <changedDataInfo_t>* changesVector_ptr);

		virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		
		bool transferGAid(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		bool transferOnOFF(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		
		StateControllerLA* m_data_ptr;
	};

//LOCAL_AGENT: DECISION

	//LAdecision component Handlers:
	
		class LApersonalityHandler: public BaseSubHandler {
		public:
		
			bool CL_API changeAll(uint32_t agentID, 
				                  AS::LApersonalityAndGAinfluence_t* newValue_ptr);

			bool CL_API changeGAoffsets(uint32_t agentID, 
										AS::LAdecisionOffsets_t* newValue_ptr);
			bool CL_API changePersonality(uint32_t agentID, 
										  AS::LAdecisionOffsets_t* newValue_ptr);

		protected:
			friend class LAdecisionDataHandler;

			bool initialize(ClientData::Handler* parentHandlerPtr, 
				            DecisionSystemLA* data_ptr,
		                    std::vector <changedDataInfo_t>* changesVector_ptr);

			virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			
			bool transferGAoffsets(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			bool transferPersonality(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			
			DecisionSystemLA* m_data_ptr;
		};

	class LAdecisionDataHandler: public BaseSubHandler {
	public:
		
		bool CL_API changeAll(uint32_t agentID, 
				              LA::decisionData_t* newValue_ptr);

		LApersonalityHandler personality;

	protected:
		friend class Handler;

		bool initialize(ClientData::Handler* parentHandlerPtr, 
				        DecisionSystemLA* data_ptr,
		                std::vector <changedDataInfo_t>* changesVector_ptr);

		virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);

		DecisionSystemLA* m_data_ptr;
	};

//GLOBAL_AGENT: COLD

	class GAcoldDataHandler: public BaseSubHandler {
	public:
		
		bool CL_API changeAll(uint32_t agentID, GA::coldData_t* newValue_ptr);

		bool CL_API changeID(uint32_t agentID, uint32_t newValue);
		bool CL_API changeName(uint32_t agentID, std::string newValue);

	protected:
		friend class Handler;

		bool initialize(ClientData::Handler* parentHandlerPtr, ColdDataControllerGA* data_ptr,
						                   std::vector <changedDataInfo_t>* changesVector_ptr);

		virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		
			bool transferID(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			bool transferName(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		
		ColdDataControllerGA* m_data_ptr;
	};

//GLOBAL_AGENT: STATE

	//GAstate component Handlers:
	
		class GArelationsHandler: public BaseSubHandler {
		public:
		
			bool CL_API changeAll(uint32_t agentID, AS::GAneighborRelations_t* newValue_ptr);

			//TODO: Put IDs on low and high 16 bits of changes.agentId
			bool CL_API changeStanceToNeighbour(uint32_t agentID, 
				                                uint32_t neighbourID, int newValue);
			//TODO: Put IDs on low and high 16 bits of changes.agentId
			bool CL_API changeDispositionToNeighbour(uint32_t agentID, 
				                                     uint32_t neighbourID, int newValue);
			//TODO: Put IDs on low and high 16 bits of changes.agentId
			bool CL_API changeLastStepDispositionToNeighbour(uint32_t agentID, 
				                           uint32_t neighbourID, int newValue);
			
		protected:
			friend class GAstateHandler;

			bool initialize(ClientData::Handler* parentHandlerPtr, 
							StateControllerGA* data_ptr,
							std::vector <changedDataInfo_t>* changesVector_ptr);

			virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			
		    //TODO: from one agent toward another: IDs on low and high 16 bits agentId
			bool transferStanceToNeighbour(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			//TODO: from one agent toward another: IDs on low and high 16 bits agentId
			bool transferDispositionToNeighbour(uint32_t agentID, 
			                                    ASdataControlPtrs_t recepientPtrs);
			//TODO: from one agent toward another: IDs on low and high 16 bits agentId
			bool transferLastStepDispositionToNeighbour(uint32_t agentID, 
			                                            ASdataControlPtrs_t recepientPtrs);
			
			StateControllerGA* m_data_ptr;
		};

			class GAresourcesHandler: public BaseSubHandler {
			public:

				bool CL_API changeAll(uint32_t agentID, AS::resources_t* newValue_ptr);
				
				bool CL_API changeCurrent(uint32_t agentID, float newValue);
				bool CL_API changeIncome(uint32_t agentID, float newValue);

			protected:
				friend class GAparametersHandler;

				bool initialize(ClientData::Handler* parentHandlerPtr, 
					            StateControllerGA* data_ptr,
					            std::vector <changedDataInfo_t>* changesVector_ptr);

				virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
				
				bool transferCurrent(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
				bool transferIncome(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
				
				StateControllerGA* m_data_ptr;
			};

		class GAparametersHandler: public BaseSubHandler {
		public:
		
			bool CL_API changeAll(uint32_t agentID, AS::GAparameterTotals_t* newValue_ptr);
				
			bool CL_API changeGAresources(uint32_t agentID, float newValue);
			bool CL_API changeLAstrenghtTotal(uint32_t agentID, float newValue);

			GAresourcesHandler LAresourceTotals;

		protected:
			friend class GAstateHandler;

			bool initialize(ClientData::Handler* parentHandlerPtr, 
							StateControllerGA* data_ptr,
							std::vector <changedDataInfo_t>* changesVector_ptr);

			virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			
			bool transferGAresources(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			bool transferLAstrenghtTotal(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
			
			StateControllerGA* m_data_ptr;
		};

	class GAstateHandler: public BaseSubHandler {
	public:
		
		bool CL_API changeAll(uint32_t agentID, GA::stateData_t* newValue_ptr);
		
		bool CL_API changeConnectedGAs(uint32_t agentID, AS::GAflagField_t* newValue_ptr);
		bool CL_API changeLocalAgentsBelongingToThis(uint32_t agentID, 
			                                         AS::LAflagField_t* newValue_ptr);
		bool CL_API changeOnOff (uint32_t agentID, bool newValue);

		GArelationsHandler relations;
		GAparametersHandler parameters;

	protected:
		friend class Handler;

		bool initialize(ClientData::Handler* parentHandlerPtr, 
						StateControllerGA* data_ptr,
						std::vector <changedDataInfo_t>* changesVector_ptr);

		virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		
		bool transferConnectedGAs(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		bool transferLocalAgentsBelongingToThis(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		
		StateControllerGA* m_data_ptr;
	};

//GLOBAL_AGENT: DECISION

	class GAdecisionDataHandler: public BaseSubHandler {
	public:
		
		bool CL_API changeAll(uint32_t agentID, GA::decisionData_t* newValue_ptr);
		
		bool CL_API changeInfiltration(uint32_t agentID, 
									   AS::GAinfiltrationOnNeighbors_t* newValue_ptr);
		bool CL_API changePersonality(uint32_t agentID, 
			                                          AS::GApersonality* newValue_ptr);

	protected:
		friend class Handler;

		bool initialize(ClientData::Handler* parentHandlerPtr, 
						DecisionSystemGA* data_ptr,
						std::vector <changedDataInfo_t>* changesVector_ptr);

		virtual bool transferAll(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		
		bool transferInfiltration(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		bool transferPersonality(uint32_t agentID, ASdataControlPtrs_t recepientPtrs);
		
		DecisionSystemGA* m_data_ptr;
	};


//MAIN HANDLER

	//Handles insertion and loading of changes for the Client Data
	class Handler {
	public:
		Handler::Handler(AS::networkParameters_t params);

		bool sendNewClientData(ASdataControlPtrs_t recepientPtrs, bool silent);

		//Returns NULL on time-out or a pointer to the acquired mutex otherwise
		std::mutex* acquireMutex(int microsPerWait = SLEEP_TIME_WAITING_MUTEX_MICROS);

		bool hasInitialized() const { return m_initialized; }

		NetworkParameterDataHandler networkParameters;

		LAcoldDataHandler LAcold;
		LAstateHandler LAstate;
		LAdecisionDataHandler LAdecision;

		GAcoldDataHandler GAcold;
		GAstateHandler GAstate;
		GAdecisionDataHandler GAdecision;

		ActionsHandler LAaction;
		ActionsHandler GAaction;
	protected:
		int m_referenceNetworkSize = 0;
		int m_maxActions = 0;
		int m_LAquantity = 0;
		int m_GAquantity = 0;

	private:
		bool processChange(ClientData::changedDataInfo_t change,
			               ASdataControlPtrs_t recepientPtrs);

		std::mutex m_mutex;
		DataMirrorSystem m_mirrorSystem;
		mirror_t* m_data_ptr;
		std::vector <changedDataInfo_t> m_changes;
		bool m_initialized;
	};
}