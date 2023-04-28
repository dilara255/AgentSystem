//TODO-CRITICAL: These things probably should be in a networkParameters_t handler class

#include "data/dataMisc.hpp"

#include "data/agentDataStructures.hpp"

//Returns neighbor's index on this agent's state data. Returns NATURAL_RETURN_ERROR on failure
//WARNING: WILL fail if agent is passed to itlself (agentID = neighborID) 
AS_API int AS::getNeighborsIndexOnGA(int neighborID, const GA::stateData_t* ThisState_ptr) {

	int totalNeighbors = 
			ThisState_ptr->connectedGAs.howManyAreOn();

		int neighborIndex = NATURAL_RETURN_ERROR;
		for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {

			int tryNeighborsID = 
				ThisState_ptr->neighbourIDs[neighbor];

			if (tryNeighborsID == neighborID) {
				neighborIndex = neighbor;
			}
		}

		return neighborIndex;
}

//Returns neighbor's index on this agent's state data. Returns NATURAL_RETURN_ERROR on failure
//WARNING: WILL fail if agent is passed to itlself (agentID = neighborID)
AS_API int AS::getNeighborsIndexOnLA(int neighborID, const LA::stateData_t* ThisState_ptr) {

	int totalNeighbors = 
			ThisState_ptr->locationAndConnections.connectedNeighbors.howManyAreOn();

		int neighborIndex = NATURAL_RETURN_ERROR;
		for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {

			int tryNeighborsID = 
				ThisState_ptr->locationAndConnections.neighbourIDs[neighbor];

			if (tryNeighborsID == neighborID) {
				neighborIndex = neighbor;
			}
		}

		return neighborIndex;
}

//Returns agent's index on neighborID's arrays. If not found, returns NATURAL_RETURN_ERROR;
int AS::getLAsIDonNeighbor(int agent, int neighborID, 
	        const  LA::stateData_t* partnerState_ptr) {

		int neighborsTotalNeighbors = 
			partnerState_ptr->locationAndConnections.connectedNeighbors.howManyAreOn();

		int idOnNeighbor = NATURAL_RETURN_ERROR;
		for (int neighborsNeighbor = 0; neighborsNeighbor < neighborsTotalNeighbors; 
														  neighborsNeighbor++) {
			int neighborsNeighborID = 
				partnerState_ptr->locationAndConnections.neighbourIDs[neighborsNeighbor];

			if (neighborsNeighborID == agent) {
				idOnNeighbor = neighborsNeighbor;
			}
		}

		return idOnNeighbor;
}

//Returns agent's index on neighborID's arrays. If not found, returns NATURAL_RETURN_ERROR;
int AS::getGAsIDonNeighbor(int agent, int neighborID, 
	         const GA::stateData_t* partnerState_ptr) {

		int neighborsTotalNeighbors = 
			partnerState_ptr->connectedGAs.howManyAreOn();

		int idOnNeighbor = NATURAL_RETURN_ERROR;
		for (int neighborsNeighbor = 0; neighborsNeighbor < neighborsTotalNeighbors; 
														  neighborsNeighbor++) {
			int neighborsNeighborID = 
				partnerState_ptr->neighbourIDs[neighborsNeighbor];

			if (neighborsNeighborID == agent) {
				idOnNeighbor = neighborsNeighbor;
			}
		}

		return idOnNeighbor;
}

bool AS::copyNetworkParameters(networkParameters_t * destination, 
		                            const networkParameters_t * source) {
	
	size_t nameSize = NAME_LENGHT * sizeof(char);
	size_t commentSize = COMMENT_LENGHT * sizeof(char);

	int commentCpy = strcpy_s(destination->comment, commentSize, source->comment);
	destination->isNetworkInitialized = source->isNetworkInitialized;
	destination->lastMainLoopStartingTick = source->lastMainLoopStartingTick;
	destination->mainLoopTicks = source->mainLoopTicks;
	destination->accumulatedMultiplier = source->accumulatedMultiplier;
	destination->lastStepTimeMicros = source->lastStepTimeMicros;
	destination->maxActions = source->maxActions;
	destination->maxLAneighbours = source->maxLAneighbours;
	int nameCpy = strcpy_s(destination->name, nameSize, source->name);
	destination->numberGAs = source->numberGAs;
	destination->numberLAs = source->numberLAs;
	destination->makeDecisions = source->makeDecisions;
	destination->processActions = source->processActions;

	for(int i = 0; i < DRAW_WIDTH; i++){
		destination->seeds[i] = source->seeds[i];
	}

	bool result = true;
	if (nameCpy != 0) {
		LOG_ERROR("Failed to receive network name...");
		#if (defined AS_DEBUG) || VERBOSE_RELEASE
			printf("name expected : % s | name read : % s\n", destination->name,
																   source->name);
		#endif // AS_DEBUG

		result = false;
	}

	if (commentCpy != 0) {
		LOG_ERROR("Failed to receive network comment line...");
		result = false;
	}

	return result;
}

bool AS::defaultNetworkParameters(networkParameters_t* destination) {

	strcpy(destination->comment, "");
	destination->isNetworkInitialized = false;
	destination->lastMainLoopStartingTick = 0;
	destination->mainLoopTicks = 0;
	destination->accumulatedMultiplier = DEFAULT_TOTAL_MULTIPLIER;
	destination->lastStepTimeMicros = std::chrono::microseconds(0);
	destination->maxActions = MAX_ACTIONS_PER_AGENT;
	destination->maxLAneighbours = 0;
	strcpy(destination->name, "");
	destination->numberGAs = DEFAULT_NUMBER_GAS;
	destination->numberLAs = DEFAULT_NUMBER_LAS;

	//TODO: this is kinda icky if we change the number of seeds
	destination->seeds[0] = DEFAULT_PRNG_SEED0;
	destination->seeds[1] = DEFAULT_PRNG_SEED1;
	destination->seeds[2] = DEFAULT_PRNG_SEED2;
	destination->seeds[3] = DEFAULT_PRNG_SEED3;

	destination->makeDecisions = DEFAULT_SYSTEM_WIDE_MAKE_DECISIONS;
	destination->makeDecisions = DEFAULT_SYSTEM_WIDE_PROCESS_ACTIONS;
	
	return true;
}
