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
*/

namespace CL {
	
	typedef struct {
		int dataCategory;
		int agent;
		int baseField;
		int subField[AS_MAX_SUB_FIELD_DEPTH];
		bool hasChanges;
	} changedDataInfo;


}
/*
//PLANO:
//IMPLEMENTAR E TESTAR SLICE VERTICAL
//DEPOIS STUBS DE TUDO
//DEPOIS PREENCHER O QUE FOR NECESSARIO AGORA
//E MARCAR O QUE FICAR PRA DEPOIS

class ClientDataHandler{
public:
	void transferFullNetwork()
	void transferLAstateDataForAllAgents()
	...

	LAstateInsertion[agents] LAstate
	GAstateInsertion[agents] GAstate
	...
	actionInsertion[actions] actions
}

class LAstateInsertion{
	void transferEntireLAstate()

	resourceInsertion resources
	...

	private:
	agentID;
}

class ResourceInsertion{

	void transferEntireResourceDAta()

	void changeCurrentTo(newValue)
	...

	private:
	agentID;
}
//have to be to initialized with all ids

aí a pessoa usa ClientDataHandler.LAstate[id].resources.changeCurrentTo(*newValue*):

-adiquire mutex;
-cria elemento no vetor de modificações {STATE_LA, agentId, PARAMS, RESOURCES, CURRENT, true};
-clientBuffer.data.agentMirrorPtrs.StateControllerLA.data.parameters.resources.current = newValue;
-devolve mutex;

Já o AS:

-chama func do CL passando ponteiros pros dados no AS;
-func do CL:
	-loopa o vetor de controle até encontrar hasChanges == false;
	-pra cada elemento, dispacha a ação certa, atualizando os dados;
	-depois do loop, dá clear no vetor;
	-devolve mutex;

Pra pegar elemento:
ClientDataHandler.LAstate[id].resources.getCurrent():

*/	
