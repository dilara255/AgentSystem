#pragma once

#include "core.hpp"

#include "../include/data/mirrorDataControllers.hpp"

//****For Testing****
#define CL_TST_INIT_EXPECTED_NUMBER 6949821
//*******************

namespace CL {

	CL_API extern mirror_t* mirrorData_ptr;

	//****For Testing****
	CL_API void sayHelloExternal();
	CL_API int getASnumber();
	CL_API int getCLnumber();
	CL_API int* getTestArrayPtr();
	//*******************
}