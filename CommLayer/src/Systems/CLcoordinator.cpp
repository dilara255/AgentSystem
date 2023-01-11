#include "miscStdHeaders.h"

#include "AS_internal.hpp"

#include "CL_externalAPI.hpp"
#include "CL_internalAPI.hpp"

#include "systems/dataMirror.hpp"

namespace CL {

	DataMirrorSystem mirror;

	mirror_t mirrorData;

	bool init() {
		LOG_INFO("initializing CL");

		return mirror.initialize();		
	}
}