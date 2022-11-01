#pragma once

#include "core.hpp"

namespace CL {
	CL_API void init(int ASinitTestNumber, int tstArraySize);

	//****For Testing****
	CL_API void sayHelloInternal();
	CL_API int* getTestArrayPtr();
	CL_API bool hasTstArrayInitialized();
	CL_API void setTstArrayHasInitialized(bool hasInitialized);
	//*******************
}