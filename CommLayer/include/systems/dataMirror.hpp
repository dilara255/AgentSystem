#pragma once

#include "data/mirrorDataControllers.hpp"

namespace CL {
	class CL_API DataMirrorSystem {
	public:
		bool initialize();
		bool updateHasData();
		bool hasData() const { return m_hasData; }
		bool isInitialized() const { return m_isInitialized; }
		bool receiveParams(const AS::networkParameters_t* params_cptr);
		bool receiveAgentData(CL::agentMirrorControllerPtrs_t dataPtrs);
		bool receiveActionData(const CL::ActionMirrorController* actions_cptr);

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


