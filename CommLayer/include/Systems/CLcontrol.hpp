#pragma once

#include "AS_internal.hpp"

namespace CL {
	typedef struct {
		AS::networkParameters_t networkParams;
		AS::dataControllerPointers_t agentControllerPtrs;
		AS::ActionSystem action;
	}mirror_t;
}
