#pragma once

#include "systems/AScoordinator.hpp"
#include "network/parameters.hpp"

//TODO-CRITICAL: Document

namespace LA{
	//This basically deals with all updating wich has to be done because of neighbors:
	//-Trade income is calculated and added to current resources;
	//-Attrition from any wars is calculated and taken from strenght;
	//-Disposition towards neighbors is changed because of trade, alliance, and war;
	//-Infiltration on each neighbor changes according to these factors as well as disposition;
	//(if a neighbor likes you, they share info, if they dislike you, they deceive)
	//All of this is done in a single point to reduce how much we need to loop and hop around.
	void applyAttritionTradeInfiltrationAndDispostionChanges(int agentId, float timeMultiplier, 
	                            LA::stateData_t* state_ptr, LA::decisionData_st* decision_ptr,
	        AS::dataControllerPointers_t* dp, AS::WarningsAndErrorsCounter* errorsCounter_ptr);

	//Trade share depends on stance and on the partners stance to it's neighbors
	//(if they do a lot of trade or are at war, your share goes down)
	float calculateShareOfPartnersTrade(int partnerID, AS::diploStance theirStance,
				                        AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                    AS::WarningsAndErrorsCounter* errorsCounter_ptr);

	//Total trade value of a LA is a portion of it's liquid income (income minus upkeep)
	//That gets divided between partners and also goes down in case of war
	//NOTE: can be negative if partners income minus upkeep gets negative!
	float calculateTradeIncomePerSecond(float agentsShare, int partnerID,
				                        AS::dataControllerPointers_t* agentDataPtrs_ptr);
	
	//Both agents loose a fraction of the smaller agents strenght each second:
	float calculateAttritionLossesPerSecond(int agentId1, int agentId2, 
		                                    AS::dataControllerPointers_t* agentDataPtrs_ptr);
}

namespace GA{
	//This basically deals with all updating wich has to be done because of neighbors:
	//-Trade income is calculated and added to current resources;
	//-Disposition towards neighbors is changed because of trade, alliance, and war;
	//-Infiltration on each neighbor changes according to these factors as well as disposition;
	//(if a neighbor likes you, they share info, if they dislike you, they deceive)
	//All of this is done in a single point to reduce how much we need to loop and hop around.
	void applyTradeInfiltrationAndDispostionChanges(GA::stateData_t* state_ptr, 
								 GA::decisionData_t* decision_ptr, int agentId, 
	                    AS::dataControllerPointers_t* dp, float timeMultiplier,
	                           AS::WarningsAndErrorsCounter* errorsCounter_ptr);

	//Trade share depends on stance and on the partners stance to it's neighbors
	//(if they do a lot of trade or are at war, your share goes down)
	float calculateShareOfPartnersTrade(int partnerID, AS::diploStance theirStance,
				                        AS::dataControllerPointers_t* agentDataPtrs_ptr,
	                                    AS::WarningsAndErrorsCounter* errorsCounter_ptr);

	//Total trade value of a GA is a portion of it's current resources (not income)
	//That gets divided between partners and also goes down in case of war
	//NOTE: can be negative so long as GAs can get in debt!
	float calculateTradeIncome(float agentsShare, int partnerID,
				                        AS::dataControllerPointers_t* agentDataPtrs_ptr);
}

namespace AS {

	//{WAR, NEUTRAL, TRADE, ALLY, ALLY_WITH_TRADE, TOTAL_STANCES};
	static int tradeSaturationFromStance[(int)AS::diploStance::TOTAL_STANCES] = 
			   					 {TRADE_SATURATION_FROM_WAR, TRADE_SATURATION_FROM_NEUTRAL, 
								   TRADE_SATURATION_FROM_TRADE, TRADE_SATURATION_FROM_ALLY,
													 TRADE_SATURATION_FROM_ALLY_WITH_TRADE};
}