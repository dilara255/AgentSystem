#pragma once

#include "core.hpp"
#include "CL_core.hpp"

#include "../include/data/mirrorDataControllers.hpp"
#include "../include/systems/clientDataHandler.hpp"

//****For Testing****
#define CL_TST_INIT_EXPECTED_NUMBER 6949821
//*******************

namespace CL {

	//TODO: turn this into a cptr
	CL_API extern const mirror_t* ASmirrorData_cptr;

	//Returns NULL if not initialized
	CL_API CL::ClientData::Handler* getClientDataHandlerPtr();

	//****For Testing****
	CL_API void sayHelloExternal();
	CL_API int getASnumber();
	CL_API int getCLnumber();
	CL_API int* getTestArrayPtr();
	//*******************
}