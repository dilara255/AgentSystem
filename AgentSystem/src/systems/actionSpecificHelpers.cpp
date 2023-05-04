#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "data/agentDataControllers.hpp"

#include "systems/actionSystem.hpp"
#include "systems/actionHelpers.hpp"

#include "systems/warningsAndErrorsCounter.hpp"

float AS::RES_S_L_necessaryFunding(float intensity) {
	
	return (intensity / ACT_REF_INCOME) * ACT_RES_S_L_COST_PER_REF_INCOME;
}

float AS::STR_S_L_necessaryFunding(float intensity) {

	return (intensity / ACT_REF_STRENGHT) * ACT_STR_S_L_COST_PER_REF_STR;
}
