#pragma once

#include "core.hpp"


//****For Testing****
#define CL_TST_INIT_EXPECTED_NUMBER 6949821
//*******************

namespace CL {



	//****For Testing****
	CL_API void sayHelloExternal();
	CL_API int getASnumber();
	CL_API int getCLnumber();
	CL_API int* getTestArrayPtr();
	//*******************
}