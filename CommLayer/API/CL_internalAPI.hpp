#pragma once

#include "core.hpp"

#ifdef AS_COMMLAYER
	#include "Systems/CLcontrol.hpp"
#endif // AS_COMMLAYER

#ifdef AS_AGENTSYSTEM
	#include "../include/Systems/CLcontrol.hpp"
#endif // AS_AGENTSYSTEM



namespace CL {
	CL_API bool init();

	class CL_API DataMirror {
	public:
		bool initialize();
		bool updateHasData();
		bool hasData() const { return m_hasData; }
		bool isInitialized() const { return m_isInitialized; }
		bool receiveParams(const AS::networkParameters_t* params_cptr);
		bool receiveAgentData(const AS::dataControllerPointers_t dataPtrs);
		bool receiveActionData(const AS::ActionDataController* actions_cptr);

		bool transferParams(AS::networkParameters_t* recepient_ptr) const;
		bool transferAgentData(AS::dataControllerPointers_t* recepient_ptr) const;
		bool transferActionData(AS::ActionDataController* recepient_ptr) const;

		bool clearAllData();

	private:
		mirror_t data;
		bool m_isInitialized = false;
		bool m_hasData = false;
	};

	//****For Testing****
	CL_API void initTest(int ASinitTestNumber, int tstArraySize);
	CL_API void sayHelloInternal();
	CL_API int* getTestArrayPtr();
	CL_API bool hasTstArrayInitialized();
	CL_API void setTstArrayHasInitialized(bool hasInitialized);
	//*******************
}