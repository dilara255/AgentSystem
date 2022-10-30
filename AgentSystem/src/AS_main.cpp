#include "AS_API.hpp"
#include "CL_API.hpp"
#include <stdio.h>

void AS::sayHello() {
	printf("\nHi, I'm AS, running on %s %s, in %s mode\n", 
		   CURR_SYSTEM, CURR_ARCHTECTURE, COMPILE_CFG);

	CL::sayHelloToo();
}