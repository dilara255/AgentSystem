#pragma once

#include "AS_internal.hpp"

namespace AS {
	AS_API bool copyNetworkParameters(networkParameters_t* destination,
		                            const networkParameters_t * source);

	AS_API bool defaultNetworkParameters(networkParameters_t* destination);
}
