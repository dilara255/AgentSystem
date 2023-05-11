//TODO-CRITICAL: Some of these things should be in data classes, others are part of API

#include "data/dataMisc.hpp"

//Taxes are proportional to current resources, and can be "negative"
float AS::taxPayedPerSecond(AS::resources_t resources) {
	return ((float)GA_TAX_RATE_PER_SECOND * resources.current);
}

float AS::taxOwed(AS::resources_t resources, float timeMultiplier) {
	return AS::taxPayedPerSecond(resources) * timeMultiplier;
}

float AS::calculateUpkeep(float strenght, float guard, float threshold) {
	float guardPaidForByDefended = guard *EXTERNAL_GUARD_UPKEEP_RATIO_BY_DEFENDED;
	float effectiveTroopSize = guardPaidForByDefended + strenght - threshold;
	
	return std::max(0.0f, effectiveTroopSize * LA_UPKEEP_PER_EXCESS_STRENGHT);
}

//TODO: document math
float AS::nextActionsCost(int currentActions) {

	if(currentActions == 0) { return 0.0f; } //first is a freeby

	float multiplier = currentActions
		+ ACT_SUPERLINEAR_WEIGHT * powf((float)(currentActions - 1), (float)ACT_SUPERLINEAR_EXPO);
	
	return multiplier * BASE_ACT_COST;
}

constexpr char AS::diploStanceToChar(AS::diploStance stance){

	switch ((int)stance) 
	{
	case (int)AS::diploStance::WAR:
		return 'W';
	case (int)AS::diploStance::NEUTRAL:
		return 'N';
	case (int)AS::diploStance::TRADE:
		return 'T';
	case (int)AS::diploStance::ALLY:
		return 'A';
	case (int)AS::diploStance::ALLY_WITH_TRADE:
		return (char)146;
	default:
		return '?';
	}
}

constexpr char AS::scopeToChar(AS::scope scope) {

	switch ((int)scope)
	{
	case  (int)AS::scope::LOCAL:
		return 'L';
	case  (int)AS::scope::GLOBAL:
		return 'G';
	default:
		return '?';
	}
}

constexpr char AS::phaseToChar(AS::actPhases phase) {

	switch ((int)phase)
	{
	case  (int)AS::actPhases::SPAWN:
		return 'S';
	case  (int)AS::actPhases::PREPARATION:
		return 'P';
	case  (int)AS::actPhases::TRAVEL:
		return 'T';
	case  (int)AS::actPhases::EFFECT:
		return 'E';
	case  (int)AS::actPhases::RETURN:
		return 'R';
	case  (int)AS::actPhases::CONCLUSION:
		return 'C';
	default:
		return '?';
	}
}

constexpr char AS::modeToChar(AS::actModes mode) {

	switch ((int)mode)
	{
	case  (int)AS::actModes::SELF:
		return 'S';
	case  (int)AS::actModes::IMMEDIATE:
		return 'I';
	case  (int)AS::actModes::REQUEST:
		return 'R';
	default:
		return '?';
	}
}


constexpr std::string_view AS::catToString(AS::actCategories cat) {

	//NOTE: The string_view returned should be to a 4 char 
	//NULL-TERMINATED string literal

	switch ((int)cat)
	{
	case  (int)AS::actCategories::STRENGHT:
		return "STR";
	case  (int)AS::actCategories::RESOURCES:
		return "RES";
	case  (int)AS::actCategories::ATTACK:
		return "ATT";
	case  (int)AS::actCategories::GUARD:
		return "GRD";
	case  (int)AS::actCategories::SPY:
		return "SPY";
	case  (int)AS::actCategories::SABOTAGE:
		return "SAB";
	case  (int)AS::actCategories::DIPLOMACY:
		return "DIP";
	case  (int)AS::actCategories::CONQUEST:
		return "CNQ";
	default:
		return "???";
	}
}

int AS::getAgentsActionIndex(int agentID, int action, int maxActionsPerAgent) {
	
	if (action >= maxActionsPerAgent) {
		LOG_WARN("Tried to get index of an action with ID above the maximum");
		return NATURAL_RETURN_ERROR;
	}

	return ((agentID * maxActionsPerAgent) + action);

}

//Returns neighbor's index on this agent's state data. Returns NATURAL_RETURN_ERROR on failure
//WARNING: WILL fail if agent is passed to itlself (agentID = neighborID) 
int AS::getNeighborsIndexOnGA(int neighborID, const GA::stateData_t* thisState_ptr) {

	int totalNeighbors = 
			thisState_ptr->connectedGAs.howManyAreOn();

		int neighborIndex = NATURAL_RETURN_ERROR;
		for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {

			int tryNeighborsID = 
				thisState_ptr->neighbourIDs[neighbor];

			if (tryNeighborsID == neighborID) {
				neighborIndex = neighbor;
			}
		}

		return neighborIndex;
}

//Returns neighbor's index on this agent's state data. Returns NATURAL_RETURN_ERROR on failure
//WARNING: WILL fail if agent is passed to itlself (agentID = neighborID)
int AS::getNeighborsIndexOnLA(int neighborID, const LA::stateData_t* thisState_ptr) {

	int totalNeighbors = 
			thisState_ptr->locationAndConnections.connectedNeighbors.howManyAreOn();

		int neighborIndex = NATURAL_RETURN_ERROR;
		for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {

			int tryNeighborsID = 
				thisState_ptr->locationAndConnections.neighbourIDs[neighbor];

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
	destination->pace = source->pace;
	destination->lastMainLoopStartingTick = source->lastMainLoopStartingTick;
	destination->mainLoopTicks = source->mainLoopTicks;
	destination->accumulatedMultiplier = source->accumulatedMultiplier;
	destination->lastStepTimeMicros = source->lastStepTimeMicros;
	destination->lastStepHotMicros = source->lastStepHotMicros;
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

	networkParameters_t defaultNetworkParams;
	*destination = defaultNetworkParams;

	return true;
}

//WARNING: BUG: this will break on platforms with different endian-nes due to union abuse
//TODO-CRITICAL: FIX (check endian-ness and avoid union type-punning)
AS::actionData_t AS::getDefaultAction(AS::scope scope) {

	actionData_t data;
	
	data.details.intensity = 0.0f;
	data.details.processingAux = 0.0f;
	data.phaseTiming.elapsed = 0;
	data.phaseTiming.total = 0;
	
	union ids_u {
		AS::ids_t data;
		uint32_t allFields;
	};
	union ids_u ids;
	
	ids.allFields = 0;
	data.ids = ids.data;

	data.ids.scope = (uint32_t)scope;
	
	return data;
}

float AS::calculateDistance(AS::pos_t posA, AS::pos_t posB) {
		float dX = posA.x - posB.x;
		float dY = posA.y - posB.y;

		return sqrt(dX*dX + dY*dY);
}
