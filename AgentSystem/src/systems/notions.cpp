#include "miscStdHeaders.h"

#include "systems/actionSystem.hpp"
#include "data/agentDataControllers.hpp"

#include "data/agentDataStructures.hpp"

#include "systems/actionHelpers.hpp"

namespace PURE_LA = LA;
namespace PURE_GA = GA;

namespace AS::Decisions::LA {

	//LOW_INCOME_TO_STR
	float calcNotionS0(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {

		auto state_ptr = &(dp->LAstate_ptr->getDataCptr()->at(agentID));

		float upkeep = state_ptr->parameters.strenght.currentUpkeep;
		float effectiveIncome = NOTION_UPKEEP_TO_BASE_INCOME_RATIO_TO_WORRY 
								* state_ptr->parameters.resources.updateRate;

		float maximumProportion = 2.0f;
		float proportion = maximumProportion;

		if (effectiveIncome > 0) {
			proportion = upkeep/effectiveIncome;
		}
		
		float notion = (proportion * proportion * proportion); //cubed: 2 -> 8, 0.5 -> 0.125

		float maximumProportionCubed = 
			maximumProportion * maximumProportion * maximumProportion;

		notion = std::clamp(notion, 0.0f, maximumProportionCubed)/maximumProportionCubed;

		assert(std::isfinite(notion));

		//Will be on [0,1], with proportion == 2 -> 1, proportion == 1 -> 0.125
		return notion;
	}

	//LOW_DEFENSE_TO_RESOURCES
	float calcNotionS1(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {

		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	//LOW_CURRENCY
	float calcNotionS2(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS3(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS4(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS5(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS6(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS7(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	//LOW_DEFENSE_TO_RESOURCES
	float calcNotionN0(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		//The higher the current resources are compared to the reference, the higher this is
		//The lower the guard is compared to the reference, the higher this is
		
		int strenghtIndex = (int)PURE_LA::readsOnNeighbor_t::fields::STRENGHT;
		float refStrenght = refReads_ptr->readOf[strenghtIndex];

		int guardIndex = (int)PURE_LA::readsOnNeighbor_t::fields::GUARD;
		float refGuard = refReads_ptr->readOf[guardIndex] + refStrenght;

		int resourcesIndex = (int)PURE_LA::readsOnNeighbor_t::fields::RESOURCES;
		float refResources = refReads_ptr->readOf[resourcesIndex];

		auto readsOfNeighbor_ptr = 
				&(dp->LAdecision_ptr->getDataCptr()->at(agentID).reads[neighbor]);

		float neighborGuard = readsOfNeighbor_ptr->readOf[guardIndex]
							  + readsOfNeighbor_ptr->readOf[strenghtIndex];

		float guardProportion = 1.0f;
		if (refGuard > 0) {
			guardProportion = neighborGuard / refGuard;
		}

		float neighborResources = readsOfNeighbor_ptr->readOf[resourcesIndex];

		float resourcesProportion = 1.0f;
		if (refResources > 0) {
			resourcesProportion = neighborResources / refResources;
		}

		float proportion = resourcesProportion / guardProportion;
		
		float notion = (proportion * proportion * proportion); //cubed: 2 -> 8, 0.5 -> 0.125

		float maximumProportion = 2;
		float maximumProportionCubed = 
			maximumProportion * maximumProportion * maximumProportion;

		notion = std::clamp(notion, 0.0f, maximumProportionCubed)/maximumProportionCubed;

		assert(std::isfinite(notion));

		//Will be on [0,1], with proportion == 2 -> 1, proportion == 1 -> 0.125
		return notion;
	}

	//IS_STRONG
	float calcNotionN1(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	//WORRIES_ME
	float calcNotionN2(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	//I_TRUST_THEM
	float calcNotionN3(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN4(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN5(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN6(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN7(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN8(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN9(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN10(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										   PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN11(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										   PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}
}

namespace AS::Decisions::GA {

	//STUB: too much str/income (when compared to reference reads)
	float calcNotionS0(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {

		auto state_ptr = &(dp->GAstate_ptr->getDataCptr()->at(agentID));

		int strIndex = (int)PURE_GA::readsOnNeighbor_t::fields::STRENGHT_LAS;
		float refStrenght = refReads_ptr->readOf[strIndex];

		float strenght = state_ptr->parameters.LAstrenghtTotal;
		float strenghtRatio = 1.0f;
		
		if (refStrenght > 0) {
			strenghtRatio = strenght / refStrenght;
		}		
		
		int taxIndex = (int)PURE_GA::readsOnNeighbor_t::fields::TAX_INCOME;
		float refTax = refReads_ptr->readOf[taxIndex];

		float tax = state_ptr->parameters.lastTaxIncome;
		float taxRatio = 1.0f;
		
		if (refTax > 0) {
			strenghtRatio = tax / refTax;
		}			

		float effectiveTaxRatio = NOTION_UPKEEP_TO_BASE_INCOME_RATIO_TO_WORRY * taxRatio;

		float proportion = strenghtRatio/effectiveTaxRatio;
		float notion = (proportion * proportion * proportion); //cubed: 2 -> 8, 0.5 -> 0.125

		float maximumProportionCubed = 8.0f;
		notion = std::clamp(notion, 0.0f, maximumProportionCubed)/maximumProportionCubed;

		assert(std::isfinite(notion));

		//Will be on [0,1], with proportion == 2 -> 1, proportion == 1 -> 0.125
		return notion;
	}

	float calcNotionS1(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {

		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS2(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS3(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS4(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS5(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS6(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS7(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	//STUB: neighbor has a lot of cash and little defenses compared to others
	float calcNotionN0(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		//The higher the current resources are compared to the reference, the higher this is
		//The lower the guard is compared to the reference, the higher this is
		
		int strenghtIndex = (int)PURE_GA::readsOnNeighbor_t::fields::STRENGHT_LAS;
		float refStrenght = refReads_ptr->readOf[strenghtIndex];

		int guardIndex = (int)PURE_GA::readsOnNeighbor_t::fields::GUARD_LAS;
		float refGuard = refReads_ptr->readOf[guardIndex] + refStrenght;

		int resourcesIndex = (int)PURE_GA::readsOnNeighbor_t::fields::GA_RESOURCES;
		float refResources = refReads_ptr->readOf[resourcesIndex];

		auto readsOfNeighbor_ptr = 
				&(dp->GAdecision_ptr->getDataCptr()->at(agentID).reads[neighbor]);

		float neighborGuard = readsOfNeighbor_ptr->readOf[guardIndex]
							  + readsOfNeighbor_ptr->readOf[strenghtIndex];

		float guardProportion = 1.0f;
		if (refGuard > 0) {
			guardProportion = neighborGuard / refGuard;
		}

		float neighborResources = readsOfNeighbor_ptr->readOf[resourcesIndex];

		float resourcesProportion = 1.0f;
		if (refResources > 0) {
			resourcesProportion = neighborResources / refResources;
		}

		float proportion = resourcesProportion / guardProportion;
		
		float notion = (proportion * proportion * proportion); //cubed: 2 -> 8, 0.5 -> 0.125

		float maximumProportion = 2;
		float maximumProportionCubed = 
			maximumProportion * maximumProportion * maximumProportion;

		notion = std::clamp(notion, 0.0f, maximumProportionCubed)/maximumProportionCubed;

		assert(std::isfinite(notion));

		//Will be on [0,1], with proportion == 2 -> 1, proportion == 1 -> 0.125
		return notion;
	}

	float calcNotionN1(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN2(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN3(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN4(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN5(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN6(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN7(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN8(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN9(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN10(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                   PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN11(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
						                   PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}
}


namespace AS::Decisions {

	float calculateNotionSelfLA(notionsSelf notion, int agentID, 
		                      AS::dataControllerPointers_t* dp,
		                      PURE_LA::readsOnNeighbor_t* refReads_ptr) {

		switch ((int)notion)
		{
		case 0:
			return LA::calcNotionS0(agentID, dp, refReads_ptr);
		case 1:
			return LA::calcNotionS1(agentID, dp, refReads_ptr);
		case 2:
			return LA::calcNotionS2(agentID, dp, refReads_ptr);
		case 3:
			return LA::calcNotionS3(agentID, dp, refReads_ptr);
		case 4:
			return LA::calcNotionS4(agentID, dp, refReads_ptr);
		case 5:
			return LA::calcNotionS5(agentID, dp, refReads_ptr);
		case 6:
			return LA::calcNotionS6(agentID, dp, refReads_ptr);
		case 7:
			return LA::calcNotionS7(agentID, dp, refReads_ptr);
		default:
			return 0.0f;
		}
	}

	float calculateNotionNeighborLA(notionsNeighbor notion, int neighbor, 
		                          int agentID, AS::dataControllerPointers_t* dp,
		                          PURE_LA::readsOnNeighbor_t* refReads_ptr) {

		switch ((int)notion)
		{
		case 0:
			return LA::calcNotionN0(neighbor, agentID, dp, refReads_ptr);
		case 1:
			return LA::calcNotionN1(neighbor, agentID, dp, refReads_ptr);
		case 2:
			return LA::calcNotionN2(neighbor, agentID, dp, refReads_ptr);
		case 3:
			return LA::calcNotionN3(neighbor, agentID, dp, refReads_ptr);
		case 4:
			return LA::calcNotionN4(neighbor, agentID, dp, refReads_ptr);
		case 5:
			return LA::calcNotionN5(neighbor, agentID, dp, refReads_ptr);
		case 6:
			return LA::calcNotionN6(neighbor, agentID, dp, refReads_ptr);
		case 7:
			return LA::calcNotionN7(neighbor, agentID, dp, refReads_ptr);
		case 8:
			return LA::calcNotionN8(neighbor, agentID, dp, refReads_ptr);
		case 9:
			return LA::calcNotionN9(neighbor, agentID, dp, refReads_ptr);
		case 10:
			return LA::calcNotionN10(neighbor, agentID, dp, refReads_ptr);
		case 11:
			return LA::calcNotionN11(neighbor, agentID, dp, refReads_ptr);
		default:
			return 0.0f;
		}
	}

	float calculateNotionSelfGA(notionsSelf notion, int agentID, 
		                      AS::dataControllerPointers_t* dp,
		                      PURE_GA::readsOnNeighbor_t* refReads_ptr) {

		switch ((int)notion)
		{
		case 0:
			return GA::calcNotionS0(agentID, dp, refReads_ptr);
		case 1:
			return GA::calcNotionS1(agentID, dp, refReads_ptr);
		case 2:
			return GA::calcNotionS2(agentID, dp, refReads_ptr);
		case 3:
			return GA::calcNotionS3(agentID, dp, refReads_ptr);
		case 4:
			return GA::calcNotionS4(agentID, dp, refReads_ptr);
		case 5:
			return GA::calcNotionS5(agentID, dp, refReads_ptr);
		case 6:
			return GA::calcNotionS6(agentID, dp, refReads_ptr);
		case 7:
			return GA::calcNotionS7(agentID, dp, refReads_ptr);
		default:
			return 0.0f;
		}
	}

	float calculateNotionNeighborGA(notionsNeighbor notion, int neighbor, 
		                          int agentID, AS::dataControllerPointers_t* dp,
		                          PURE_GA::readsOnNeighbor_t* refReads_ptr) {

		switch ((int)notion)
		{
		case 0:
			return GA::calcNotionN0(neighbor, agentID, dp, refReads_ptr);
		case 1:
			return GA::calcNotionN1(neighbor, agentID, dp, refReads_ptr);
		case 2:
			return GA::calcNotionN2(neighbor, agentID, dp, refReads_ptr);
		case 3:
			return GA::calcNotionN3(neighbor, agentID, dp, refReads_ptr);
		case 4:
			return GA::calcNotionN4(neighbor, agentID, dp, refReads_ptr);
		case 5:
			return GA::calcNotionN5(neighbor, agentID, dp, refReads_ptr);
		case 6:
			return GA::calcNotionN6(neighbor, agentID, dp, refReads_ptr);
		case 7:
			return GA::calcNotionN7(neighbor, agentID, dp, refReads_ptr);
		case 8:
			return GA::calcNotionN8(neighbor, agentID, dp, refReads_ptr);
		case 9:
			return GA::calcNotionN9(neighbor, agentID, dp, refReads_ptr);
		case 10:
			return GA::calcNotionN10(neighbor, agentID, dp, refReads_ptr);
		case 11:
			return GA::calcNotionN11(neighbor, agentID, dp, refReads_ptr);
		default:
			return 0.0f;
		}
	}
}