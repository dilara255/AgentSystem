#include "miscStdHeaders.h"

#include "systems/AScoordinator.hpp"
#include "systems/diplomacy.hpp"

//Trade share depends on stance and on the partners stance to it's neighbors
//(if they do a lot of trade or are at war, your share goes down)
float LA::calculateShareOfPartnersTrade(int partnerID, AS::diploStance theirStance,
				                        AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                    AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

	LA::stateData_t partner = agentDataPtrs_ptr->LAstate_ptr->getDirectDataPtr()->at(partnerID);

	//calculates their total "trade saturation" (from trade, allies, allies with trade and war)
	int tradeSaturation = 0;
	int partnersNeighbours = partner.locationAndConnections.numberConnectedNeighbors;
	
	for (int i = 0; i < partnersNeighbours; i++) {
		int diploStance = (int)partner.relations.diplomaticStanceToNeighbors[i];
		tradeSaturation += AS::tradeSaturationFromStance[diploStance];
	}

	//each neighbour gets a share depending on their diplomatic stance:
	float agentsShare = (float)AS::tradeSaturationFromStance[(int)theirStance]/tradeSaturation;
	if(tradeSaturation == 0) {
		errorsCounter_ptr->incrementWarning(AS::warnings::DP_LA_TRADE_PARTNER_HAS_ZERO_SAT);
		agentsShare = 0; 
	}

	return agentsShare;
}

//Total trade value of a LA is a portion of it's liquid income (income minus upkeep)
//That gets divided between partners and also goes down in case of war
//NOTE: can be negative if partners income minus upkeep gets negative!
float LA::calculateTradeIncomePerSecond(float agentsShare, int partnerID,
				                        AS::dataControllerPointers_t* agentDataPtrs_ptr) {
 
	LA::stateData_t partner = 
		agentDataPtrs_ptr->LAstate_ptr->getDirectDataPtr()->at(partnerID);

	float totalPartnerTradeValue = TRADE_FACTOR_PER_SECOND *
		(partner.parameters.resources.updateRate - partner.parameters.strenght.currentUpkeep);

	

	return agentsShare * totalPartnerTradeValue;
}

//Both agents loose a fraction of the smaller agents strenght each second:
float LA::calculateAttritionLossesPerSecond(int agentId1, int agentId2,
			                                AS::dataControllerPointers_t* agentDataPtrs_ptr) {

	auto LAstates_cptr = agentDataPtrs_ptr->LAstate_ptr->getDataCptr();
	float strenghtAgent1 = LAstates_cptr->at(agentId1).parameters.strenght.current;
	float strenghtAgent2 = LAstates_cptr->at(agentId2).parameters.strenght.current;

	return (float)(std::min(strenghtAgent1,strenghtAgent2)*ATTRITION_FACTOR_PER_SECOND);
}

//Trade share depends on stance and on the partners stance to it's neighbors
//(if they do a lot of trade or are at war, your share goes down)
float GA::calculateShareOfPartnersTrade(int partnerID, AS::diploStance theirStance,
				                        AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                    AS::WarningsAndErrorsCounter* errorsCounter_ptr) {

	GA::stateData_t partner = agentDataPtrs_ptr->GAstate_ptr->getDirectDataPtr()->at(partnerID);

	//calculates their total "trade saturation" (from trade, allies, allies with trade and war)
	int tradeSaturation = 0;
	int partnersNeighbours = partner.connectedGAs.howManyAreOn();

	for (int i = 0; i < partnersNeighbours; i++) {
		int idOther = partner.neighbourIDs[i];
		int diploStance = (int)partner.relations.diplomaticStanceToNeighbors[idOther];
		tradeSaturation += AS::tradeSaturationFromStance[diploStance];
	}

	//each neighbour gets a share depending on their diplomatic stance:
	float agentsShare = (float)AS::tradeSaturationFromStance[(int)theirStance]/tradeSaturation;
	if(tradeSaturation == 0) {
		errorsCounter_ptr->incrementWarning(AS::warnings::DP_GA_TRADE_PARTNER_HAS_ZERO_SAT);
		agentsShare = 0; 
	}

	return agentsShare;
}

//Total trade value of a GA is a portion of it's current resources (not income)
//That gets divided between partners and also goes down in case of war
//NOTE: can be negative so long as GAs can get in debt!
float GA::calculateTradeIncomePerSecond(float agentsShare, int partnerID,
				                        AS::dataControllerPointers_t* agentDataPtrs_ptr) { 

	GA::stateData_t partner = 
		agentDataPtrs_ptr->GAstate_ptr->getDirectDataPtr()->at(partnerID);

	float totalPartnerTradeValue = 
		(float)(partner.parameters.lastTaxIncome * TRADE_FACTOR_PER_SECOND);

	return agentsShare * totalPartnerTradeValue;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         