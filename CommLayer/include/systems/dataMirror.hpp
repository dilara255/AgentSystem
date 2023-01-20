#pragma once

#include "../include/data/mirrorDataControllers.hpp"

namespace CL {
	//Stores, manages, receives and gives access to and info about action and agent data.
	//Used for: mirroring the AS in the CL and keeping data sent from the Client to the CL.
	class CL_API DataMirrorSystem {
	
	public:
		DataMirrorSystem() {
							data.agentMirrorPtrs.haveBeenCreated = false;
							data.agentMirrorPtrs.haveData = false;
							data_cptr = NULL;
						}

		bool initialize(mirror_t** mirrorData_ptr_ptr, 
			            const AS::networkParameters_t* params_cptr);
		
		bool receiveReplacementParams(const AS::networkParameters_t* params_cptr);
		bool receiveReplacementAgentData(CL::agentToMirrorVectorPtrs_t dataPtrs);
		bool receiveReplacementActionData(actionToMirrorVectorPtrs_t actionPtrs);

		bool transferParams(AS::networkParameters_t* recepient_ptr) const;
		bool transferAgentData(CL::agentMirrorControllerPtrs_t* recepient_ptr) const;
		bool transferActionData(CL::ActionMirrorController* recepient_ptr) const;

		const mirror_t* getDataCptr() const { return data_cptr; }
		bool updateHasData();

		bool hasData() const { return m_hasData; }
		bool isInitialized() const { return m_isInitialized; }
		bool isNetworkInitialized() const { return data.networkParams.isNetworkInitialized; }
		bool hasActionData() const { return data.actionMirror.hasData(); }
		bool hasAgentData() const { return data.agentMirrorPtrs.haveData; }

		void setHasActionData(bool hasData) { data.actionMirror.setHasData(hasData); }
		void setHasAgentData(bool hasData) { data.agentMirrorPtrs.haveData = hasData; }

		bool clearAllData();

	private:
		bool createAgentDataControllers(const AS::networkParameters_t* params_cptr);

		mirror_t data;
		const mirror_t* data_cptr;
		bool m_isInitialized = false;
		bool m_hasData = false;
	};
}


