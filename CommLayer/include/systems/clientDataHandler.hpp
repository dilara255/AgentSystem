#pragma once

/*
//PLANNING:
//TO DO:
//- Implement and test a "vertical slice": currentResources transfer for a given agente;
//- Base Class and stub for the rest;
//- A method for data on each "leaf": implement and test one at a time;
//- Fill out per-field methods;
//- Methods for "entire structure";
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
-- Both are private. TO DO: did the vector need to be warped for easier dll exporting?
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

* TO DO:
* - CL::ClientData::BaseFieldHandler;
*/

#include "dataMirror.hpp"

namespace CL::ClientData {
	
	typedef struct {
		int dataCategory;
		int agent;
		int baseField;
		int subField[AS_MAX_SUB_FIELD_DEPTH];
		int lastSubfield;
		bool hasChanges;
	} changedDataInfo;


	class NetworkParameterDataHandler {
	public:
		bool initialize(std::mutex* mutex_ptr, networkParameters_t* data_ptr,
			            std::vector <changedDataInfo>* changesVector_ptr) { return true; }

		//full insertion

		//per-simple-field insertion methods

	private:
		//data_ptr
		//changes_ptr
	};


	class LAcoldDataHandler {
	public:
		bool initialize(std::mutex* mutex_ptr, ColdDataControllerLA* data_ptr,
			            std::vector <changedDataInfo>* changesVector_ptr) { return true; }

		//full insertion

		//per-simple-field insertion methods

	private:
		//data_ptr
		//changes_ptr
	};


	//LAstate component Handlers:

		class LArelationsHandler {
		public:
			//initialization

			//full insertion

			//per-simple-field insertion methods

		private:
			//data_ptr
			//changes_ptr
		};

			class PositionHandler {
			public:
				//initialization

				//full insertion

				//per-simple-field insertion methods

			private:
				//data_ptr
				//changes_ptr
			};

		class LocationAndConnectionsHandler {
		public:
			//initialization

			//full insertion

			//per-simple-field insertion methods

			PositionHandler position;

		private:
			//data_ptr
			//changes_ptr
		};
		
			class LAresourcesHandler {
			public:
				bool initialize(std::mutex* mutex_ptr, StateControllerLA* data_ptr,
								std::vector <changedDataInfo>* changesVector_ptr);

				bool changeResources(uint32_t agentID, resources_t newResources) { return false; }
				
				bool changeCurrentTo(uint32_t agentID, float newValue);

				bool changeIncomeTo(uint32_t agentID, float newValue) { return false; }

			private:
				std::mutex* m_mutex_ptr;
				StateControllerLA* m_data_ptr;
				std::vector <changedDataInfo>* m_changesVector_ptr;				
			};

			class LAstrenghtHandler {
			public:
				//initialization

				//full insertion

				//per-simple-field insertion methods

			private:
				//data_ptr
				//changes_ptr
			};

		class LAparametersHandler {
		public:
			bool initialize(std::mutex* mutex_ptr, StateControllerLA* data_ptr,
				            std::vector <changedDataInfo>* changesVector_ptr);

			//full insertion

			//per-simple-field insertion methods

			LAresourcesHandler resources;
			LAstrenghtHandler strenght;

		private:
			std::mutex* m_mutex_ptr;
			StateControllerLA* m_data_ptr;
			std::vector <changedDataInfo>* m_changesVector_ptr;
		};

	class LAstateHandler {
	public:
		bool initialize(std::mutex* mutex_ptr, StateControllerLA* data_ptr,
			            std::vector <changedDataInfo>* changesVector_ptr);

		//full insertion

		//per-simple-field insertion methods

		LArelationsHandler relations;
		LocationAndConnectionsHandler location;
		LAparametersHandler parameters;

	private:
		std::mutex* m_mutex_ptr;
		StateControllerLA* m_data_ptr;
		std::vector <changedDataInfo>* m_changesVector_ptr;
	};


	//LAdecision component Handlers:
	
		class LApersonalityHandler {
		public:
			//initialization

			//full insertion

			//per-simple-field insertion methods

		private:
			//data_ptr
			//changes_ptr
		};

	class LAdecisionDataHandler {
	public:
		bool initialize(std::mutex* mutex_ptr, DecisionSystemLA* data_ptr,
			            std::vector <changedDataInfo>* changesVector_ptr) { return true; }

		//full insertion

		//per-simple-field insertion methods

		LApersonalityHandler offsets;

	private:
		//data_ptr
		//changes_ptr
	};


	class GAcoldDataHandler {
	public:
		bool initialize(std::mutex* mutex_ptr, ColdDataControllerGA* data_ptr,
			            std::vector <changedDataInfo>* changesVector_ptr) { return true; }

		//full insertion

		//per-simple-field insertion methods

	private:
		//data_ptr
		//changes_ptr
	};


	//GAstate component Handlers:
	
		class GArelationsHandler {
		public:
			//initialization

			//full insertion

			//per-simple-field insertion methods

		private:
			//data_ptr
			//changes_ptr
		};

		class GAparametersHandler {
		public:
			//initialization

			//full insertion

			//per-simple-field insertion methods

			LAresourcesHandler LAresources;

		private:
			//data_ptr
			//changes_ptr
	};

	class GAstateHandler {
	public:
		bool initialize(std::mutex* mutex_ptr, StateControllerGA* data_ptr,
			            std::vector <changedDataInfo>* changesVector_ptr) { return true; }

		//full insertion

		//per-simple-field insertion methods

		GArelationsHandler relations;
		GAparametersHandler parameters;

	private:
		//data_ptr
		//changes_ptr
	};

		
	class GAdecisionDataHandler {
	public:
		bool initialize(std::mutex* mutex_ptr, DecisionSystemGA* data_ptr,
			            std::vector <changedDataInfo>* changesVector_ptr) { return true; }

		//full insertion

		//per-simple-field insertion methods

	private:
		//data_ptr
		//changes_ptr
	};


	//Action component Handlers:
	
		class ActionIDsHandler {
		public:
			//initialization

			//full insertion

			//per-simple-field insertion methods

		private:
			//data_ptr
			//changes_ptr
		};

		class ActionTickInfoHandler {
		public:
			//initialization

			//full insertion

			//per-simple-field insertion methods

		private:
			//data_ptr
			//changes_ptr
		};

		class ActionDetailsHandler {
		public:
			//initialization

			//full insertion

			//per-simple-field insertion methods

		private:
			//data_ptr
			//changes_ptr
		};

	class ActionsHandler {
	public:
		bool initialize(std::mutex* mutex_ptr, std::vector <actionData_t>* data_ptr,
			            std::vector <changedDataInfo>* changesVector_ptr) { return true; }

		//full insertion

		//per-simple-field insertion methods

		ActionIDsHandler IDs;
		ActionTickInfoHandler tickInfo;
		ActionDetailsHandler details;

	private:
		//data_ptr
		//changes_ptr
	};


	//Handles insertion and loading of changes for the Client Data
	class Handler {
	public:
		Handler::Handler(AS::networkParameters_t params);

		NetworkParameterDataHandler networkParameters;

		LAcoldDataHandler LAcold;
		LAstateHandler LAstate;
		LAdecisionDataHandler LAdecision;

		GAcoldDataHandler GAcold;
		GAstateHandler GAstate;
		GAdecisionDataHandler GAdecision;

		ActionsHandler LAaction;
		ActionsHandler GAaction;

	private:
		std::mutex m_mutex;
		DataMirrorSystem m_mirrorSystem;
		mirror_t* m_data_ptr;
		std::vector <changedDataInfo> m_changes;
		bool m_initialized;
	};
}



