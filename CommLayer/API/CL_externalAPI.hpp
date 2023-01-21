#pragma once

#include "core.hpp"

#include "../include/data/mirrorDataControllers.hpp"
#include "../include/systems/clientDataHandler.hpp"

//****For Testing****
#define CL_TST_INIT_EXPECTED_NUMBER 6949821
//*******************

namespace CL {

	//TO DO: work on Client Data Handlers so the tests wich use this can use the Handlers.
	//TO DO: turn this into a cptr
	CL_API extern const mirror_t* ASmirrorData_ptr;

	//Returns NULL if not initialized
	CL_API CL::ClientData::Handler* getClientDataHandlerPtr();

	CL_API bool isClientDataPointerInitialized();
	CL_API bool isASdataPointerInitialized();

	//****For Testing****
	CL_API void sayHelloExternal();
	CL_API int getASnumber();
	CL_API int getCLnumber();
	CL_API int* getTestArrayPtr();
	//*******************
}