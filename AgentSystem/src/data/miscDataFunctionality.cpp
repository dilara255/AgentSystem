//TODO-CRITICAL: These things probably should be in a networkParameters_t handler class

#include "data/dataMisc.hpp"

#include "data/agentDataStructures.hpp"

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
	destination->maxActions = 0;
	destination->maxLAneighbours = 0;
	strcpy(destination->name, "");
	destination->numberGAs = 0;
	destination->numberLAs = 0;

	return true;
}
