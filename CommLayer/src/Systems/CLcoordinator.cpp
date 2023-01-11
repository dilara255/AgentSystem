#include "miscStdHeaders.h"

#include "AS_internal.hpp"

#include "CL_externalAPI.hpp"
#include "CL_internalAPI.hpp"

#include "Systems/CLcontrol.hpp"

namespace CL {

	DataMirror mirror;

	bool init() {
		LOG_INFO("initializing CL");

		return mirror.initialize();		
	}
}