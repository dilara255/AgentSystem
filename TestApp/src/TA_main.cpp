#include "AS_API.hpp"
#include "CL_API.hpp"
#include <stdio.h>

int main(void) {
	
	AS::sayHello();

	CL::sayHelloToo();
	
	getchar();

	return 1;
}