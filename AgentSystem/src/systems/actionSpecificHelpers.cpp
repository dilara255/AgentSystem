//TODO: tests for all of this

#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "data/agentDataControllers.hpp"

#include "systems/actionSystem.hpp"
#include "systems/actionHelpers.hpp"

#include "data/dataMisc.hpp"

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

uint32_t AS::ATT_I_L_attackTime(float intensity) {

	return (uint32_t)std::round( AS::ATT_I_L_prepTime(intensity) 
											* ACT_ATT_I_L_EFFECT_DURATION_MULTIPLIER );
}

double AS::ATT_I_L_travelTimeModifierFromTroopSize(float intensity) {

	double effectiveSize = std::sqrt(std::sqrt((double)intensity / ACT_REF_STRENGHT));

	return std::clamp( effectiveSize, (double)ACT_ATT_I_L_MIN_TRAVEL_TIME_MODIFIER, 
		                              (double)ACT_ATT_I_L_MAX_TRAVEL_TIME_MODIFIER);
}

uint32_t AS::ATT_I_L_travelTimeFromDistanceAndTroops(AS::pos_t posA, AS::pos_t posB, 
																	float intensity) {

	double baseTime = calculateDistance(posA, posB) 
							* (double)ACT_BASE_TRAVEL_UNITS_OF_DIST_PER_TENTHS_OF_MILLI;

	return (uint32_t)std::round(baseTime * ATT_I_L_travelTimeModifierFromTroopSize(intensity));
}

uint32_t AS::ATT_I_L_returnTime(AS::pos_t posA, AS::pos_t posB, float intensity,
	                                              uint32_t effectPhaseTotalTime) {

	float lossesTravelTimeMultiplier = 
		  (float)((double)effectPhaseTotalTime / (double)AS::ATT_I_L_attackTime(intensity));
			
	return	(uint32_t)std::round( lossesTravelTimeMultiplier
					* AS::ATT_I_L_travelTimeFromDistanceAndTroops(posA, posB, intensity) );
}
	

float AS::ATT_I_L_attackSizeFromIntensityAndReturnTime(AS::pos_t posA, AS::pos_t posB, 
											     uint32_t returnTime, float intensity) {

	//This basically substitutes other functions above and solves for original intensity

	double effectPhaseTotalTime = returnTime * ATT_I_L_attackTime(intensity)
					/ AS::ATT_I_L_travelTimeFromDistanceAndTroops(posA, posB, intensity); 

	double B2 = ACT_ATT_I_L_BASE_PREP_TENTHS_OF_MS_PER_REF_STR
							* ACT_ATT_I_L_BASE_PREP_TENTHS_OF_MS_PER_REF_STR;

	double M2 = ACT_ATT_I_L_EFFECT_DURATION_MULTIPLIER 
							* ACT_ATT_I_L_EFFECT_DURATION_MULTIPLIER;

	double originalIntensity = 
		((double)ACT_REF_STRENGHT * effectPhaseTotalTime * effectPhaseTotalTime) / (B2 * M2);

	return (float)originalIntensity;
}