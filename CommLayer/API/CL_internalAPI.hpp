#pragma once

#include "core.hpp"

namespace CL {
	CL_API void init();

	//****For Testing****
	CL_API void initTest(int ASinitTestNumber, int tstArraySize);
	CL_API void sayHelloInternal();
	CL_API int* getTestArrayPtr();
	CL_API bool hasTstArrayInitialized();
	CL_API void setTstArrayHasInitialized(bool hasInitialized);
	//*******************
}