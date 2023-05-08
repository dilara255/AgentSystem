#include "miscStdHeaders.h"

#include "systems/AScoordinator.hpp"
#include "systems/diplomacy.hpp"

#include "data/dataMisc.hpp"

void LA::applyAttritionTradeInfiltrationAndDispostionChanges(int agentId, float timeMultiplier, 
	                          LA::stateData_t* state_ptr, LA::decisionData_st* decision_ptr,
	      AS::dataControllerPointers_t* dp, AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

	int quantityNeighbours = state_ptr->locationAndConnections.numberConnectedNeighbors;
	for (int neighbor = 0; neighbor < quantityNeighbours; neighbor++) {
		
		//Save last step's disposition towards this neighbor:
		state_ptr->relations.dispositionToNeighborsLastStep[neighbor] = 
							state_ptr->relations.dispositionToNeighbors[neighbor];

		auto res_ptr = &state_ptr->parameters.resources;
		auto str_ptr = &state_ptr->parameters.strenght;
		int partnerID = state_ptr->locationAndConnections.neighbourIDs[neighbor];	

		//The changes depend on the diplomatic stance to the neighbor:
		AS::diploStance stance = state_ptr->relations.diplomaticStanceToNeighbors[neighbor];

		if ((stance == AS::diploStance::TRADE) ||
		    (stance == AS::diploStance::ALLY_WITH_TRADE)) {

			float share = LA::calculateShareOfPartnersTrade(partnerID, stance, dp, 
				                                                errorsCounter_ptr);
			res_ptr->current += 
				LA::calculateTradeIncomePerSecond(share, partnerID, dp) * timeMultiplier;
			
			//raise relations and infiltration in proportion to share of trade:
			state_ptr->relations.dispositionToNeighbors[neighbor] +=
					share * MAX_DISPOSITION_RAISE_FROM_TRADE_PER_SECOND * timeMultiplier;
			decision_ptr->infiltration[neighbor] +=
				    share * MAX_INFILTRATION_RAISE_FROM_TRADE_PER_SECOND * timeMultiplier;
		}

		else if ((stance == AS::diploStance::WAR)) {
			int partnerID = state_ptr->locationAndConnections.neighbourIDs[neighbor];
			str_ptr->current -= 
				LA::calculateAttritionLossesPerSecond(agentId, partnerID, dp)
				* timeMultiplier;
			
			//lower relations and infiltration because of war:
			state_ptr->relations.dispositionToNeighbors[neighbor] -=
					INFILTRATION_LOSS_FROM_WAR_PER_SECOND * timeMultiplier;
			decision_ptr->infiltration[neighbor] -=
				    INFILTRATION_LOSS_FROM_WAR_PER_SECOND * timeMultiplier;
		}

		//Ally with trade receives the effects from trade and from Alliance:
		if ((stance == AS::diploStance::ALLY) ||
		    (stance == AS::diploStance::ALLY_WITH_TRADE)) {

			//raise relations and infiltration because of alliance:
			state_ptr->relations.dispositionToNeighbors[neighbor] +=
					DISPOSITION_RAISE_FROM_ALLIANCE_PER_SECOND * timeMultiplier;
			decision_ptr->infiltration[neighbor] +=
				    INFILTRATION_RAISE_FROM_ALLIANCE_PER_SECOND * timeMultiplier;
		}

		//We also change infiltration according to neighbors disposition
		//If the neighbor likes this agent, this agent gains infiltration, and vice-versa
		
		//First, we need to find this agent's index on the neighbor's arrays:
		auto partnerState_ptr = &(dp->LAstate_ptr->getDataCptr()->at(partnerID));
		int idOnNeighbor = 
			AS::getLAsIDonNeighbor(agentId, partnerID, partnerState_ptr);
		bool found = (idOnNeighbor != NATURAL_RETURN_ERROR);

		if(found){
			float neighborsDisposition = 
				partnerState_ptr->relations.dispositionToNeighbors[idOnNeighbor];

			decision_ptr->infiltration[neighbor] += timeMultiplier * neighborsDisposition
								* INFILTRATION_CHANGE_FROM_NEIGHBOR_DISPOSITION_PER_SECOND;
		}
		else {
			errorsCounter_ptr->incrementError(AS::errors::AS_LA_NOT_NEIGHBOR_OF_NEIGHBOR);
		}	
	}
}

float LA::calculateShareOfPartnersTrade(int partnerID, AS::diploStance theirStance,
				                        AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                    AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

	LA::stateData_t partner = agentDataPtrs_ptr->LAstate_ptr->getDirectDataPtr()->at(partnerID);

	//calculates their total "trade saturation" (from trade, allies, allies with trade and war)
	int tradeSaturation = 0;
	int partnersNeighbours = partner.locationAndConnections.connectedNeighbors.howManyAreOn();
	
	for (int i = 0; i < partnersNeighbours; i++) { //counts this agent (as it should)
		int diploStance = (int)partner.relations.diplomaticStanceToNeighbors[i];
		tradeSaturation += AS::tradeSaturationFromStance[diploStance];
	}

	if(tradeSaturation == 0) {
		//If you're doing trade with them, they're doing trade with you
		errorsCounter_ptr->incrementWarning(AS::warnings::DP_LA_TRADE_PARTNER_HAS_ZERO_SAT);
		return 0; 
	}

	//each neighbour gets a share depending on their diplomatic stance:
	float agentsShare = (float)AS::tradeSaturationFromStance[(int)theirStance]/tradeSaturation;
	
	return agentsShare;
}

float LA::calculateTradeIncomePerSecond(float agentsShare, int partnerID,
				                        AS::dataControllerPointers_t* agentDataPtrs_ptr) {
 
	LA::stateData_t partner = 
		agentDataPtrs_ptr->LAstate_ptr->getDirectDataPtr()->at(partnerID);

	float totalPartnerTradeValue = TRADE_FACTOR_LA_PER_SECOND *
		(partner.parameters.resources.updateRate - partner.parameters.strenght.currentUpkeep);

	

	return agentsShare * totalPartnerTradeValue;
}

float LA::calculateAttritionLossesPerSecond(int agentId1, int agentId2,
			                                AS::dataControllerPointers_t* agentDataPtrs_ptr) {

	auto LAstates_cptr = agentDataPtrs_ptr->LAstate_ptr->getDataCptr();
	float strenghtAgent1 = LAstates_cptr->at(agentId1).parameters.strenght.current;
	float strenghtAgent2 = LAstates_cptr->at(agentId2).parameters.strenght.current;

	return (float)(std::min(strenghtAgent1,strenghtAgent2)*ATTRITION_FACTOR_PER_SECOND);
}

void GA::applyTradeInfiltrationAndDispostionChanges(GA::stateData_t* state_ptr, 
								 GA::decisionData_t* decision_ptr, int agentId, 
	                    AS::dataControllerPointers_t* dp, float timeMultiplier,
	                           AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

	int quantityNeighbours = state_ptr->connectedGAs.howManyAreOn();
	auto param_ptr = &state_ptr->parameters;
	param_ptr->lastTradeIncome = 0;
	
	for (int neighbor = 0; neighbor < quantityNeighbours; neighbor++) {
		
		//Save last step's disposition towards this neighbor:
		state_ptr->relations.dispositionToNeighborsLastStep[neighbor] = 
							state_ptr->relations.dispositionToNeighbors[neighbor];

		//The changes depend on the diplomatic stance to the neighbor:
		AS::diploStance stance = state_ptr->relations.diplomaticStanceToNeighbors[neighbor];
		int idOther = state_ptr->neighbourIDs[neighbor];
				
		if ((stance == AS::diploStance::TRADE) ||
		    (stance == AS::diploStance::ALLY_WITH_TRADE)) {
			
			float share = GA::calculateShareOfPartnersTrade(idOther, stance, dp, 
				                                                errorsCounter_ptr);

			param_ptr->lastTradeIncome += GA::calculateTradeIncome(share, idOther, dp);

			//Raise relations and infiltration in proportion to share of Trade:
			state_ptr->relations.dispositionToNeighbors[neighbor] +=
					share * MAX_DISPOSITION_RAISE_FROM_TRADE_PER_SECOND * timeMultiplier;
			decision_ptr->infiltration[neighbor] +=
				    share * MAX_INFILTRATION_RAISE_FROM_TRADE_PER_SECOND * timeMultiplier;
		}

		else if (stance == AS::diploStance::WAR) {
			//Lower relations and infiltration because of war:
			state_ptr->relations.dispositionToNeighbors[neighbor] -=
					INFILTRATION_LOSS_FROM_WAR_PER_SECOND * timeMultiplier;
			decision_ptr->infiltration[neighbor] -=
				    INFILTRATION_LOSS_FROM_WAR_PER_SECOND * timeMultiplier;
		}

		if ((stance == AS::diploStance::ALLY) ||
		    (stance == AS::diploStance::ALLY_WITH_TRADE)) {

			//Raise relations and infiltration because of alliance:
			state_ptr->relations.dispositionToNeighbors[neighbor] +=
					DISPOSITION_RAISE_FROM_ALLIANCE_PER_SECOND * timeMultiplier;
			decision_ptr->infiltration[neighbor] +=
				    INFILTRATION_RAISE_FROM_ALLIANCE_PER_SECOND * timeMultiplier;
		}

		//We also change infiltration according to neighbors disposition
		//If the neighbor likes this agent, this agent gains infiltration, and vice-versa
		
		//First, we need to find this agent's index on the neighbor's arrays:
		auto partnerState_ptr = &(dp->GAstate_ptr->getDataCptr()->at(idOther));
		int idOnNeighbor = 
			AS::getGAsIDonNeighbor(agentId, idOther, partnerState_ptr);
		bool found = (idOnNeighbor != NATURAL_RETURN_ERROR);

		if(found){
			float neighborsDisposition = 
				partnerState_ptr->relations.dispositionToNeighbors[idOnNeighbor];

			decision_ptr->infiltration[neighbor] += timeMultiplier * neighborsDisposition
								* INFILTRATION_CHANGE_FROM_NEIGHBOR_DISPOSITION_PER_SECOND;
		}
		else {
			errorsCounter_ptr->incrementError(AS::errors::AS_GA_NOT_NEIGHBOR_OF_NEIGHBOR);
		}	
	}
	
	//actually add the resources from all the trade:
	param_ptr->GAresources += param_ptr->lastTradeIncome;
}

float GA::calculateShareOfPartnersTrade(int partnerID, AS::diploStance theirStance,
				                        AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                    AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

	GA::stateData_t partner = agentDataPtrs_ptr->GAstate_ptr->getDirectDataPtr()->at(partnerID);

	//calculates their total "trade saturation" (from trade, allies, allies with trade and war)
	int tradeSaturation = 0;
	int partnersNeighbours = partner.connectedGAs.howManyAreOn();

	for (int i = 0; i < partnersNeighbours; i++) {
		int idOther = partner.neighbourIDs[i]; //counts this agent (as it should)
		int diploStance = (int)partner.relations.diplomaticStanceToNeighbors[idOther];
		tradeSaturation += AS::tradeSaturationFromStance[diploStance];
	}

	if(tradeSaturation == 0) {
		//If you're doing trade with them, they're doing trade with you
		errorsCounter_ptr->incrementWarning(AS::warnings::DP_GA_TRADE_PARTNER_HAS_ZERO_SAT);
		return 0; 
	}

	//each neighbour gets a share depending on their diplomatic stance:
	float agentsShare = (float)AS::tradeSaturationFromStance[(int)theirStance]/tradeSaturation;

	return agentsShare;
}

float GA::calculateTradeIncome(float agentsShare, int partnerID,
				                        AS::dataControllerPointers_t* agentDataPtrs_ptr) { 

	GA::stateData_t partner = 
		agentDataPtrs_ptr->GAstate_ptr->getDirectDataPtr()->at(partnerID);


	float totalPartnerTradeValue = 
		(float)(partner.parameters.lastTaxIncome * TRADE_FACTOR_GA);

	return agentsShare * totalPartnerTradeValue;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         