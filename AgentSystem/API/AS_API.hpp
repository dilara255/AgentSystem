#pragma once

#include "core.hpp"

//This is supposed to be thin, for direct control from the application only:
//initializing, saving, loading, quitting, pretty much. Other communication through CL


//****For Testing****
#define AS_TST_INIT_EXPECTED_NUMBER 5619419
#define AS_TST_EXPECTED_ARR0 231879
#define AS_TST_EXPECTED_ARR1 954263
//*******************

namespace AS {
	
	AS_API void initializeASandCL();

	//****For Testing****
	AS_API void sayHello();
	//*******************
}
