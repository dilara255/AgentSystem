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

uint32_t AS::ATT_I_L_prepTime(float intensity) {

	double effectiveAttackSize = sqrt(intensity / ACT_REF_STRENGHT);

	return (uint32_t)std::round( effectiveAttackSize 
		                         * (double)ACT_ATT_I_L_BASE_PREP_TENTHS_OF_MS_PER_REF_STR );	
}

double AS::ATT_I_L_travelTimeModifierFromTroopSize(float intensity) {

	double effectiveSize = std::sqrt(std::sqrt((double)intensity / ACT_REF_STRENGHT));

	return std::clamp( effectiveSize, (double)ACT_ATT_I_L_MIN_TRAVEL_TIME_MODIFIER, 
		                              (double)ACT_ATT_I_L_MAX_TRAVEL_TIME_MODIFIER);
}

float AS::ATT_I_L_attackSizeFromIntensityAndReturnTime(uint32_t returnTime, float intensity) {

	assert(false); //must implement this
	return 0;
}