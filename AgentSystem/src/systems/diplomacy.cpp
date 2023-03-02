#include "miscStdHeaders.h"

#include "systems/AScoordinator.hpp"
#include "systems/diplomacy.hpp"

//Total trade value of an angent is a portion of it's current resources (not income)
//That gets divided between partners and also goes down in case of war
//NOTE: can be negative when partner is in debt!
float LA::calculateTradeIncomePerSecond(int partnerID, AS::diploStance theirStance,
				                        AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                    AS::WarningsAndErrorsCounter* errorsCounter_ptr) {
 
	LA::stateData_t partner = agentDataPtrs_ptr->LAstate_ptr->getDirectDataPtr()->at(partnerID);
	float totalPartnerTradeValue = (float)(partner.parameters.resources.current*TRADE_FACTOR_PER_SECOND);

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

	return agentsShare*totalPartnerTradeValue;
}

//Both agents loose a fraction of the smaller agents strenght each second:
float LA::calculateAttritionLossesPerSecond(int agentId1, int agentId2,
			                                AS::dataControllerPointers_t* agentDataPtrs_ptr) {

	auto LAstates_cptr = agentDataPtrs_ptr->LAstate_ptr->getDataCptr();
	float strenghtAgent1 = LAstates_cptr->at(agentId1).parameters.strenght.current;
	float strenghtAgent2 = LAstates_cptr->at(agentId2).parameters.strenght.current;

	return (float)(std::min(strenghtAgent1,strenghtAgent2)*ATTRITION_FACTOR_PER_SECOND);
}


//Total trade value of an angent is a portion of it's current resources (not income)
//That gets divided between partners and also goes down in case of war
//NOTE: can be negative when partner is in debt!
float GA::calculateTradeIncomePerSecond(int partnerID, AS::diploStance theirStance,
		                                AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                    AS::WarningsAndErrorsCounter* errorsCounter_ptr) { 

	GA::stateData_t partner = agentDataPtrs_ptr->GAstate_ptr->getDirectDataPtr()->at(partnerID);
	float totalPartnerTradeValue = (float)(partner.parameters.GAresources*TRADE_FACTOR_PER_SECOND);

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
		//printf("PARTNER: %d\n", partnerID);
		//agentsShare = 0; 
	}

	return agentsShare*totalPartnerTradeValue;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         