//TODO: tests for all of this

#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "data/agentDataControllers.hpp"

#include "systems/actionSystem.hpp"
#include "systems/actionHelpers.hpp"

#include "data/dataMisc.hpp"

#include "systems/warningsAndErrorsCounter.hpp"

float AS::RES_S_L_necessaryFunding(float intensity) {
	
	float relativeChange = (intensity / ACT_REF_INCOME);

	//raising income has a exponential-growth potential, so we add a squared term
	//to help reign that in:

	return ACT_RES_S_L_COST_PER_REF_INCOME * relativeChange
			* (1 + (ACT_RES_S_L_COS_RELATIVE_WEIGHT_SQUARE_TERM * relativeChange) );
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
							/ (double)ACT_BASE_TRAVEL_UNITS_OF_DIST_PER_TENTHS_OF_MILLI;

	return (uint32_t)std::round(baseTime * ATT_I_L_travelTimeModifierFromTroopSize(intensity));
}

//TODO: would it be better to base this on score (if still available)?
uint32_t AS::ATT_I_L_returnTime(AS::pos_t posA, AS::pos_t posB, float intensity,
	                                              uint32_t effectPhaseTotalTime) {

	//More people travel slower, 
	//but also the worse the losses and the longer the battle, the longer the return

	float lossesIndicatorQuotient = 
		(float)((double)effectPhaseTotalTime / (double)AS::ATT_I_L_attackTime(intensity));
	
	//since effectPhaseTotalTime accounts for how many troops were initially involved
	//but also how close the battle was:
	float lossesTravelTimeMultiplier = std::sqrt(lossesIndicatorQuotient);
		  
	return	(uint32_t)std::round( lossesTravelTimeMultiplier
					* AS::ATT_I_L_travelTimeFromDistanceAndTroops(posA, posB, intensity) );
}
	
//Uses travel times as an estimate. See ATT_I_L_returnTime.
//Can be larger than 1 in case the attack ended faster than the expected time
//Ie: the attacker killed all defenders, and did so fairly quickly
//TODO: add test to the fixed battery, since this is kinda brittle
float AS::ATT_I_L_effectiveReturneeRatio(AS::pos_t posA, AS::pos_t posB, 
									     uint32_t returnTime, float intensity) {

	float baseTravelTimeReturnees = 
			(float)AS::ATT_I_L_travelTimeFromDistanceAndTroops(posA, posB, intensity);

	return baseTravelTimeReturnees / returnTime ;
}