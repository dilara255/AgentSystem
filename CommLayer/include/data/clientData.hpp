#pragma once

#include "systems/dataMirror.hpp"

/*
//PLANO:
//IMPLEMENTAR SLICE VERTICAL
//DEPOIS STUBS DE TUDO
//DEPOIS PREENCHER O QUE FOR NECESSARIO AGORA
//E MARCAR O QUE FICAR PRA DEPOIS

mutex clientDataMutex;

vetor de:
struct{
	dataCategory,
	agent,
	baseField,
	[OPT_INDEXES],
	bool hasChanges
} //COM ENUMS PRAS CATEGORIAS, FIELDS E SUB-FIELDS
diretamente visível só internamente no CL

METE VETOR DISSO NUMA CLASSE
Pra api externa:

class DataInsertion{
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

aí a pessoa usa DataInsertion.LAstate[id].resources.changeCurrentTo(*newValue*):

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

*/

/*
namespace CL {

	class CL_API ClientData {
	public:
		bool initialize(mirror_t** mirror_ptr_ptr);

		bool receiveReplacementParams(const AS::networkParameters_t* params_cptr);
		bool receiveReplacementAgentData(CL::agentToMirrorVectorPtrs_t dataPtrs);
		bool receiveReplacementActionData(actionToMirrorVectorPtrs_t actionPtrs);

		bool transferParams(AS::networkParameters_t* recepient_ptr) const;
		bool transferAgentData(CL::agentMirrorControllerPtrs_t* recepient_ptr) const;
		bool transferActionData(CL::ActionMirrorController* recepient_ptr) const;

		bool updateHasData();

		bool hasNewData() const { return m_hasNewData; }
		bool isInitialized() const { return m_isInitialized; }
		bool isNetworkInitialized() const { return data.networkParams.isNetworkInitialized; }
		bool hasNewActionData() const { return data.actionMirror.hasData(); }
		bool hasNewAgentData() const { return data.agentMirrorPtrs.haveData; }

		void setHasActionData(bool hasData) { data.actionMirror.setHasData(hasData); }
		void setHasAgentData(bool hasData) { data.agentMirrorPtrs.haveData = hasData; }

		bool clearAllData();

		mirror_t data;
	private:
		bool createAgentDataControllers();
		changedFieldFlags_t hasChanged;
		bool m_isInitialized = false;
		bool m_hasNewData = false;
	};
}
*/
	
