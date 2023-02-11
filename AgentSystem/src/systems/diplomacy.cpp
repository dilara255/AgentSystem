#include "systems/AScoordinator.hpp"
#include "systems/diplomacy.hpp"


float LA::calculateTradeIncomePerSecond(int partnerID, AS::diploStance theirStance,
				                        AS::dataControllerPointers_t* agentDataPtrs_ptr) {

	//trade per second? Either that, or needs timeFactor here
	 
	//calculates total trade value: their income times some parameter

	//calculates their "saturation" (sum of tradeSaturationFromStance over neighbours)

	//return (theirStance's_saturation / total_saturation) * total_trade

	return 0.1;
}
	
float LA::calculateAttritionLossesPerSecond(int agentId1, int agentId2,
			                                AS::dataControllerPointers_t* agentDataPtrs_ptr) {

	return 0;
}



float GA::calculateTradeIncomePerSecond(int partnerID, AS::diploStance theirStance,
		                                AS::dataControllerPointers_t* agentDataPtrs_ptr) { 

	return 0;
}
	
float GA::calculateAttritionLossesPerSecond(int agentId1, int agentId2,
						                    AS::dataControllerPointers_t* agentDataPtrs_ptr) {

	return 0;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         