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
	float guardPaidForByDefended = guard * EXTERNAL_GUARD_UPKEEP_RATIO_BY_DEFENDED;
	float effectiveTroopSize = guardPaidForByDefended + strenght - threshold;
	
	return std::max(0.0f, effectiveTroopSize * LA_UPKEEP_PER_EXCESS_STRENGHT);
}

constexpr float AS::getStanceImpactFactorFromTrade(){
		
	return MAX_DISPOSITION_RAISE_FROM_TRADE_PER_SECOND 
	       * NOTIONS_AND_ACTIONS_REF_PERIOD_SECS
		   * NTN_DIPLO_STANCE_WEIGHT_PROPORTION_TO_REFS;
}

constexpr float AS::getStanceImpactFactorFromWar(){
		
	return PROPORTIONAL_WEIGHT_OF_WAR_COMPARED_TO_TRADE * AS::getStanceImpactFactorFromTrade();
}

float AS::projectDispositionChangeInRefTime(float dispositionChange) {

	float msLikelyPassed =
			AS_MILLISECONDS_PER_DECISION_ROUND * AS_GENERAL_PACE;
	float rateOfChange = dispositionChange / msLikelyPassed;

	float projectedChange = 
			rateOfChange * NOTIONS_AND_ACTIONS_REF_PERIOD_SECS * MILLIS_IN_A_SECOND;

	return std::clamp(projectedChange, -NTN_MAX_ABSOLUTE_DISPOSITION_EXTRAPOLATION,
										NTN_MAX_ABSOLUTE_DISPOSITION_EXTRAPOLATION);
}

//TODO: document math
float AS::nextActionsCost(int currentActions) {

	if(currentActions < ACT_FREE_ACTIONS) { return 0.0f; } //there may be freebies

	int effectiveCurrentActions = currentActions - (ACT_FREE_ACTIONS - 1);

	float multiplier = effectiveCurrentActions
		+ ACT_SUPERLINEAR_WEIGHT * powf((float)(effectiveCurrentActions), (float)ACT_SUPERLINEAR_EXPO);
	
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

std::string AS::notionTargetToString(AS::Decisions::notionLabel_t notion,
		                                              int actualTargetID) {

	//NOTE: The string_view returned should be a NULL-TERMINATED string literal of size 5

	bool isSet = notion.isSet();
	if (!isSet) {
		return "????";
	}

	bool isSelf = notion.isSelf();
	bool isAverage = notion.isAboutAverage();
	bool isNeighbor = notion.isNeighbor();
	
	if(isAverage) {
		return "AVG";
	}
	else if (isSelf) {
		return "SLF";
	}
	else if(isNeighbor) {
		constexpr int sizeOfField = 5;
		char neighID[sizeOfField];
		neighID[0] = 'A';
		neighID[1] = 'G';
		int neighborID = notion.neighborID;
		if(actualTargetID != -1) { neighborID = actualTargetID; }
		snprintf(&neighID[2], (sizeOfField - 2) * (sizeof(char)), "%d", neighborID);
		neighID[sizeOfField - 1] = '\n';
		return neighID;
	}
	
	assert(false); //should always be one of those if isSet
	return "XXXXXXXXXXXXXX";
}

std::string AS::notionNameToString(AS::Decisions::notionLabel_t notion) {

	bool isSet = notion.isSet();
	if (!isSet) {
		return "????";
	}

	bool isSelf = notion.isSelf();
	bool isNeighbor = notion.isNeighbor();

	if (isNeighbor) {
		switch ((int)notion.neighbor)
		{
		case  (int)AS::Decisions::notionsNeighbor::LOW_DEFENSE_TO_RESOURCES:
			return "LOW_DEF_TO_RES";
		case  (int)AS::Decisions::notionsNeighbor::IS_STRONG:
			return "IS_STRONG     ";
		case  (int)AS::Decisions::notionsNeighbor::WORRIES_ME:
			return "THEY_WORRY_ME ";
		case  (int)AS::Decisions::notionsNeighbor::I_TRUST_THEM:
			return "CAN_BE_TRUSTED";				
		case  (int)AS::Decisions::notionsNeighbor::N4:
			return "N4_NOT_ADDED  ";				
		case  (int)AS::Decisions::notionsNeighbor::N5:
			return "N5_NOT_ADDED  ";				
		case  (int)AS::Decisions::notionsNeighbor::N6:
			return "N6_NOT_ADDED  ";				
		case  (int)AS::Decisions::notionsNeighbor::N7:
			return "N7_NOT_ADDED  ";				
		case  (int)AS::Decisions::notionsNeighbor::N8:
			return "N8_NOT_ADDED  ";				
		case  (int)AS::Decisions::notionsNeighbor::N9:
			return "N9_NOT_ADDED  ";				
		case  (int)AS::Decisions::notionsNeighbor::N10:
			return "N10_NOT_ADDED ";				
		case  (int)AS::Decisions::notionsNeighbor::N11:
			return "N11_NOT_ADDED ";				
		default:
			return "BAD_NOTION_ID";
		}	
	}
	else if (isSelf) {
		switch ((int)notion.self)
		{
		case  (int)AS::Decisions::notionsSelf::LOW_INCOME_TO_STR:
			return "LOW_INC_TO_STR";
		case  (int)AS::Decisions::notionsSelf::LOW_DEFENSE_TO_RESOURCES:
			return "LOW_DEF_TO_RES";
		case  (int)AS::Decisions::notionsSelf::LOW_CURRENCY:
			return "LOW_RESOURCES ";
		case  (int)AS::Decisions::notionsSelf::S3:
			return "S3_NOT_ADDED  ";
		case  (int)AS::Decisions::notionsSelf::S4:
			return "S4_NOT_ADDED  ";
		case  (int)AS::Decisions::notionsSelf::S5:
			return "S5_NOT_ADDED  ";
		case  (int)AS::Decisions::notionsSelf::S6:
			return "S6_NOT_ADDED  ";
		case  (int)AS::Decisions::notionsSelf::S7:
			return "S7_NOT_ADDED  ";
		default:
			return "BAD_NOTION_ID";
		}	
	}
		
	assert(false); //should always be one of those if isSet
	return "XXXXXXXXXXXXXX";
}

std::string AS::notionToString(AS::Decisions::notionLabel_t notion,
		                                        int actualTargetID) {
	
	return ( notionTargetToString(notion, actualTargetID) 
		     + "_" + std::string(notionNameToString(notion)) );
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
	data.details.shortTermAux = 0.0f;
	data.details.longTermAux = 0.0f;
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
