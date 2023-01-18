#pragma once

#include "systems/dataMirror.hpp"

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
*/

namespace CL::ClientData {
	
	typedef struct {
		int dataCategory;
		int agent;
		int baseField;
		int subField[AS_MAX_SUB_FIELD_DEPTH];
		bool hasChanges;
	} changedDataInfo;


	class NetworkParameterDataHandler {
	public:
		//initialization

		//full insertion

		//per-simple-field insertion methods

	private:
		//data_ptr
		//changes_ptr
	};


	class LAcoldDataHandler {
	public:
		//initialization

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
				//initialization

			    //full insertion

			    //per-simple-field insertion methods

			private:
				//data_ptr
				//changes_ptr
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
			//initialization

			//full insertion

			//per-simple-field insertion methods

			LAresourcesHandler resources;
			LAstrenghtHandler strenght;

		private:
			//data_ptr
			//changes_ptr
		};

	class LAstateHandler {
	public:
		//initialization

		//full insertion

		//per-simple-field insertion methods

		LArelationsHandler relations;
		LocationAndConnectionsHandler location;
		LAparametersHandler parameters;

	private:
		//data_ptr
		//changes_ptr
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
		//initialization

		//full insertion

		//per-simple-field insertion methods

		LApersonalityHandler offsets;

	private:
		//data_ptr
		//changes_ptr
	};


	class GAcoldDataHandler {
	public:
		//initialization

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
		//initialization

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
		//initialization

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
		//initialization

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
		//initialization
		
		//full network insertion

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
		mirror_t data;
		changedDataInfo changes;
		//mutex
		//initialization data
	};
}
/*
//PLANO:
//IMPLEMENTAR E TESTAR SLICE VERTICAL
//DEPOIS STUBS DE TUDO
//DEPOIS PREENCHER O QUE FOR NECESSARIO AGORA
//E MARCAR O QUE FICAR PRA DEPOIS

CL::ClientData::Handler.LAstate[id].resources.changeCurrentTo(*newValue*):
*/	



