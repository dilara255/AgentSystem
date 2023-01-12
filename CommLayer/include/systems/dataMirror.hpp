#pragma once

#include "data/mirrorDataControllers.hpp"

namespace CL {
	class CL_API DataMirrorSystem {
	public:
		bool initialize(mirror_t** mirror_ptr_ptr);
		bool createAgentDataControllers();

		bool receiveReplacementParams(const AS::networkParameters_t* params_cptr);
		bool receiveReplacementAgentData(CL::agentToMirrorVectorPtrs_t dataPtrs);
		bool receiveReplacementActionData(actionToMirrorVectorPtrs_t actionPtrs);

		bool updateHasData();
		bool hasData() const { return m_hasData; }
		bool isInitialized() const { return m_isInitialized; }
		bool isNetworkInitialized() const { return data.networkParams.isNetworkInitialized; }
		bool hasActionData() const { return data.actionMirror.hasData(); }
		bool hasAgentData() const { return data.agentMirrorPtrs.haveData; }

		void setHasActionData(bool hasData) { data.actionMirror.setHasData(hasData); }
		void setHasAgentData(bool hasData) { data.agentMirrorPtrs.haveData = hasData; }

		bool transferParams(AS::networkParameters_t* recepient_ptr) const;
		bool transferAgentData(CL::agentMirrorControllerPtrs_t* recepient_ptr) const;
		bool transferActionData(CL::ActionMirrorController* recepient_ptr) const;

		bool clearAllData();

	private:
		mirror_t data;
		bool m_isInitialized = false;
		bool m_hasData = false;
	};
}


