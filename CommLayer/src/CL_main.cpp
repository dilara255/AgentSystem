#include "CL_API.hpp"
#include <stdio.h>

void CL::sayHelloToo() {
	printf("\nAnd I'm CL, running on %s %s, in %s mode\n",
		CURR_SYSTEM, CURR_ARCHTECTURE, COMPILE_CFG);
}