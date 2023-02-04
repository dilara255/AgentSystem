//TODO-CRITICAL: These things probably should be in a networkParameters_t handler class

#include "data/dataMisc.hpp"

#include "data/agentDataStructures.hpp"

bool AS::copyNetworkParameters(networkParameters_t * destination, 
		                            const networkParameters_t * source) {
		
	strcpy(destination->comment, source->comment);
	destination->isNetworkInitialized = source->isNetworkInitialized;
	destination->lastMainLoopStartingTick = source->lastMainLoopStartingTick;
	destination->mainLoopTicks = source->mainLoopTicks;
	destination->lastStepTimeMicros = source->lastStepTimeMicros;
	destination->maxActions = source->maxActions;
	destination->maxLAneighbours = source->maxLAneighbours;
	strcpy(destination->name, source->name);
	destination->numberGAs = source->numberGAs;
	destination->numberLAs = source->numberLAs;

	return true;
}

bool AS::defaultNetworkParameters(networkParameters_t* destination) {

	strcpy(destination->comment, "");
	destination->isNetworkInitialized = false;
	destination->lastMainLoopStartingTick = 0;
	destination->mainLoopTicks = 0;
	destination->lastStepTimeMicros = std::chrono::microseconds(0);
	destination->maxActions = 0;
	destination->maxLAneighbours = 0;
	strcpy(destination->name, "");
	destination->numberGAs = 0;
	destination->numberLAs = 0;

	return true;
}
