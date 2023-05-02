#include "miscStdHeaders.h"

#include "systems/actionSystem.hpp"
#include "data/agentDataControllers.hpp"

#include "data/agentDataStructures.hpp"

#include "systems/actionHelpers.hpp"

namespace PURE_LA = LA;
namespace PURE_GA = GA;

namespace AS::Decisions {

	//This first raises the notion (limited by maximumEffectiveBase), to the smallExponent;
	//Then, it casts the value back to the [0,1] range, making it non-linear.
	//Has parameter-based defaults for effectiveMaxBase and smallExponent.
	// 
	//NOTE: baseNotion is *assumed* to be >= 0;
	//NOTE: effectiveMaxBase is *assumed* to be > 0;
	//NOTE: If smallExponent == 0, all values will be cast to 1;
	// 
	//WARNING: Large *small*Exponents may break things : )
	float delinearizeAndClampNotion(float baseNotion, 
								float effectiveMaxBase = NTN_STD_MAX_EFFECTIVE_NOTION_BASE, 
								uint8_t smallExponent = NTN_STD_DELINEARIZATION_EXPONENT);
}

namespace AS::Decisions::LA {

	//S0: LOW_INCOME_TO_STR
	//This notion is based on the upkeep/income ratio
	//There's a maximum value above which all ratios will map to notion = 1
	//The notion is not linear with the ratio - it's raised to some power and remmaped to [0,1]
	//TODO: extract some parameters
	float calcNotionS0(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {

		auto state_ptr = &(dp->LAstate_ptr->getDataCptr()->at(agentID));

		float upkeep = state_ptr->parameters.strenght.currentUpkeep;

		float effectiveIncome = NTN_UPKEEP_TO_BASE_INCOME_RATIO_TO_WORRY 
								* state_ptr->parameters.resources.updateRate;

		float small = 0.1f; //to avoid blowups and worries about small quantities
		
		float proportion = NTN_STD_MAX_EFFECTIVE_NOTION_BASE;
		if (effectiveIncome > small) {
			proportion = upkeep/effectiveIncome;
		}
		
		//To make things more interesting and keep the notion in the [0,1] range:
		return delinearizeAndClampNotion(proportion);
	}

	//S1: LOW_DEFENSE_TO_RESOURCES
	float calcNotionS1(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {

		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	//S2: LOW_CURRENCY
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

	//N0: LOW_DEFENSE_TO_RESOURCES
	//The neighbor has low defences compared to their resources:
	//- The higher the current resources are compared to the reference, the higher this is;
	//- The lower the guard is compared to the reference, the higher this is;
	//TODO: extract some parameters
	float calcNotionN0(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		//We use proportions to the references as a way to normalize,
		//since defence and resources don't necessarely have a direct convertion

		//Defence is defined as strenght + guard, so to calculate the refDefenf:
		int strenghtIndex = (int)PURE_LA::readsOnNeighbor_t::fields::STRENGHT;
		float refStrenght = refReads_ptr->readOf[strenghtIndex];

		int guardIndex = (int)PURE_LA::readsOnNeighbor_t::fields::GUARD;
		float refGuard = refReads_ptr->readOf[guardIndex];
		
		float refDefense = refGuard + refStrenght;

		//And these are the refResources:
		int resourcesIndex = (int)PURE_LA::readsOnNeighbor_t::fields::RESOURCES;
		float refResources = refReads_ptr->readOf[resourcesIndex];

		//Now we get the reads on the defenses:
		auto readsOfNeighbor_ptr = 
				&(dp->LAdecision_ptr->getDataCptr()->at(agentID).reads[neighbor]);

		float neighborDefense = readsOfNeighbor_ptr->readOf[guardIndex]
							  + readsOfNeighbor_ptr->readOf[strenghtIndex];

		float small = 0.1f; //to avoid blowups on division : )

		if (refDefense < small) {
			refDefense = small;
		}
		float defenceProportion = neighborDefense / refDefense;		

		//defenceProportion will be a divident later, so:
		if (defenceProportion < small) {
			defenceProportion = small;
		}

		//Same idea, for resources:
		float neighborResources = readsOfNeighbor_ptr->readOf[resourcesIndex];

		if (refResources < small) {
			refResources = small;
		}
		float resourcesProportion = neighborResources / refResources;

		//Since we may have increased the defenceProportion a bit, 
		//let's extend resourcesProportion the same courtesy:
		if (resourcesProportion < small) {
			resourcesProportion = small;
		}

		//Finally, this is the proportion we want:
		float proportion = resourcesProportion / defenceProportion;

		//Since we've sanitized the terms before, we expect that:
		assert(std::isfinite(proportion));
		assert(proportion > 0);

		//To make things more interesting and keep the notion in the [0,1] range:
		return delinearizeAndClampNotion(proportion);
	}

	//N1: IS_STRONG
	float calcNotionN1(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	//N2: WORRIES_ME
	float calcNotionN2(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	//N3: I_TRUST_THEM
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

	//S0: STUB: too much str/income (when compared to reference reads)
	//TODO: overhaul (like on LA::S0)
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

		float effectiveTaxRatio = NTN_UPKEEP_TO_BASE_INCOME_RATIO_TO_WORRY * taxRatio;

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

	//N0: STUB: neighbor has a lot of cash and little defenses compared to others
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

	float delinearizeAndClampNotion(float baseNotion, float effectiveMaxBase, 
													   uint8_t smallExponent) {

		assert(baseNotion >= 0);
		assert(effectiveMaxBase > 0);

		float raisedNotion = powf(baseNotion, smallExponent);
		float raisedEffectiveMaxBase =  powf(effectiveMaxBase, smallExponent);

		//sanity checks:
		assert(std::isfinite(raisedNotion));
		assert(effectiveMaxBase > 0);
		assert(std::isfinite(raisedEffectiveMaxBase)); 

		float finalNotion = 
				std::clamp(raisedNotion, 0.0f, raisedEffectiveMaxBase)
													/ raisedEffectiveMaxBase;

		assert(std::isfinite(finalNotion));

		return finalNotion;
	}
}