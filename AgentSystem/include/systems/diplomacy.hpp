#pragma once

#include "systems/AScoordinator.hpp"
#include "network/parameters.hpp"

//TODO-CRITICAL: Document

namespace LA{
	float calculateTradeIncomePerSecond(int partnerID, AS::diploStance theirStance, 
		                                AS::dataControllerPointers_t* agentDataPtrs_ptr,
		                                AS::WarningsAndErrorsCounter* errorsCounter_ptr);
	
	float calculateAttritionLossesPerSecond(int agentId1, int agentId2, 
		                                    AS::dataControllerPointers_t* agentDataPtrs_ptr);
}

namespace GA{
	float calculateTradeIncomePerSecond(int partnerID, AS::diploStance theirStance, 
		                                AS::dataControllerPointers_t* agentDataPtrs_ptr,
		                                AS::WarningsAndErrorsCounter* errorsCounter_ptr);
}

namespace AS {

	//{WAR, NEUTRAL, TRADE, ALLY, ALLY_WITH_TRADE, TOTAL_STANCES};
	static int tradeSaturationFromStance[(int)AS::diploStance::TOTAL_STANCES] = 
			   					 {TRADE_SATURATION_FROM_WAR, TRADE_SATURATION_FROM_NEUTRAL, 
								   TRADE_SATURATION_FROM_TRADE, TRADE_SATURATION_FROM_ALLY,
													 TRADE_SATURATION_FROM_ALLY_WITH_TRADE};
}