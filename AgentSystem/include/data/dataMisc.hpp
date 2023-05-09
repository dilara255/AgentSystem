

#pragma once

#include "AS_internal.hpp"

namespace AS {

	AS_API float nextActionsCost(int currentActions);
	AS_API float calculateUpkeep(float strenght, float guard, float threshold);

	AS_API constexpr char diploStanceToChar(AS::diploStance stance);
	AS_API constexpr char scopeToChar(AS::scope scope);
	AS_API constexpr char phaseToChar(AS::actPhases phase);
	AS_API constexpr char modeToChar(AS::actModes mode);
	//Returns a string_view to a null-terminated char[4]:
	AS_API constexpr std::string_view catToString(AS::actCategories cat);	

	//This gives you the index ASSUMING you passed in valid agentID and maxActions.
	//Returns NATURAL_RETURN_ERROR in case of error.
	//TODO: this is a temporary fix while dataMirrors equivalent method is not fixed.
	AS_API int getAgentsActionIndex(int agentID, int action, int maxActionsPerAgent);

	AS_API int getNeighborsIndexOnGA(int neighborID, const GA::stateData_t* thisState_ptr);

	AS_API int getNeighborsIndexOnLA(int neighborID, const LA::stateData_t* thisState_ptr);

	AS_API int getGAsIDonNeighbor(int agent, int neighborID, 
		                      const GA::stateData_t* partnerState_ptr);

	AS_API int getLAsIDonNeighbor(int agent, int neighborID, 
		                      const LA::stateData_t* partnerState_ptr);

	AS_API bool copyNetworkParameters(networkParameters_t* destination,
		                            const networkParameters_t * source);

	AS_API bool defaultNetworkParameters(networkParameters_t* destination);

	AS_API actionData_t getDefaultAction(AS::scope scope);

	AS_API float calculateDistance(AS::pos_t posA, AS::pos_t posB);

	//TODO: action id_t to and from uint32_t
	//TODO: packing/unpacking of scope, agent ID and action ID (for this agent) on a uint32_t
	//Maybe just scope + action index on the actual actions data vector?
}
