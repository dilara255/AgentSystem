//TODO: Some of these functions should be methods on ActionDataController
//TODO: Possibly add the other ones to API, as a pack of helper funtionality (on this file)
#pragma once

#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "data/agentDataControllers.hpp"

#include "systems/warningsAndErrorsCounter.hpp"
#include "systems/actionSystem.hpp"

namespace AS {

	const std::vector<AS::actionData_t>* getScopeIndependentActionDataCptr(
										const ActionSystem* asp, AS::scope scope, int agent, 
							            AS::WarningsAndErrorsCounter* errorsCounter_ptr);

	inline bool isActionValid(const actionData_t* action_ptr);
	inline void invalidateAction(actionData_t* action_ptr);

	//Populates activeActions_ptr with the agents active actions (from the start).
	//Returns total active actions, or NATURAL_RETURN_ERROR (-1) on error.
	//NOTE: ignores BOTH action slots wich are marked as occupied but innactive
	//AND slots marked as not occupied (regardless of active/innactive).
	void populateAgentsActiveActions(const ActionSystem* asp, AS::scope scope, int agent,
		                               AS::Decisions::agentsActions_t* activeActions_ptr, 
		                                 AS::WarningsAndErrorsCounter* errorsCounter_ptr);

	float secondsToBankrupt(float income, float resources, float upkeep, 
		                                         float trade, float tax);

	float getResponsibleTroopsToRaise(LA::stateData_t* state_ptr, float desiredIntensity);

	void incrementTroopsOnAttack(float* onAttacks_ptr, float intensity);

	void decrementTroopsOnAttack(float* onAttacks_ptr, float originalIntensity);

	float getMaxDebt(float currentBaseIncome);

	int getQuantityOfCurrentActions(scope scope, int agentID, ActionSystem const * asp,
		                               AS::WarningsAndErrorsCounter* errorsCounter_ptr);

	//These are helper functionality for specific action variations.
	//Their main goal is to help maintain parity between action processing and pre processing.
	
	//Times in Tehnths of MS
	uint32_t basePrepTime(float base, float scaleFactor);

	float STR_S_L_calculateNewTroops(float effectiveStrenght, float desirabilityMultiplier);
	float STR_S_L_necessaryFunding(float intensity);
	//Times in Tehnths of MS
	uint32_t STR_S_L_prepTime(float newTroops);

	float RES_S_L_calculateRaise(float effectiveIncome, float desirabilityMultiplier);
	float RES_S_L_necessaryFunding(float intensity);
	//Times in Tehnths of MS
	uint32_t RES_S_L_prepTime(float raise);

	//Times in Tehnths of MS
	uint32_t ATT_I_L_prepTime(float intensity);
	double ATT_I_L_travelTimeModifierFromTroopSize(float intensity);
	float ATT_I_L_effectiveReturneeRatio(AS::pos_t posA, AS::pos_t posB, 
									     uint32_t returnTime, float intensity);
	uint32_t ATT_I_L_travelTimeFromDistanceAndTroops(AS::pos_t posA, AS::pos_t posB, 
																	float intensity);
	uint32_t ATT_I_L_attackTime(float intensity);
	uint32_t ATT_I_L_returnTime(AS::pos_t posA, AS::pos_t posB, float intensity,
	                                              uint32_t effectPhaseTotalTime);
}