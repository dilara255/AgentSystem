/***********************************************************************************
* In this file we define:
* - the "routing" of the notionBase calculations;
* - the calculations themselves, for each notion, for LAs and GAs; and
* - the delinearization and clamping process to map from bases to notions.
* 
* The meat of the file is dedicated to the different notionBase calculations.
* Ideally, these calculations should all express a kind of linear "relation".
* The delinearization will then use externally defined parameters to get the
* final value. After that, weights will be applied to each notion for each action.
* 
* So there are three degrees of controls to the effect of different notions:
* 
* 1) The base calculations, defined here, ideally expressing "linear"* relations;
* 2) The paremeters for the delinearization, defined elsewhere:
* -- these affect how much changes in notionBase affect the final notion, as well
* -- as how high a notionBase has to go to saturate the final notion;
* 3) The weights, which say how much and in what direction each final notion affects
* each action variation. These are also defined elsewhere.
* 
* The rationale behind this organization is to make each of these parts as
* straight-forward and independent as possible. Also, weights and delinearization
* parameters can eventually be moved to a loaded config file, giving more freedom
* to adjust behavior without recompilation. 
* In the same vein, as long as the calculations defined here still carry the same 
* meaning and remain "linear", reasonable changes to them (eg, to reflect a change 
* in the available data), shouldn't break the behavior too badly.
* 
* *"linear", with sacry quotes, because that's not necessarily well defined for all
* notions. But it's the spirit that counts : p
***********************************************************************************/

#include "miscStdHeaders.h"

#include "systems/actionSystem.hpp"
#include "data/agentDataControllers.hpp"

#include "data/agentDataStructures.hpp"

#include "systems/actionHelpers.hpp"

namespace PURE_LA = LA;
namespace PURE_GA = GA;

namespace AS::Decisions::LA {

	//S0: LOW_INCOME_TO_STR
	//This is based on the upkeep/income ratio
	float calcNotionBaseS0(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {

		auto state_ptr = &(dp->LAstate_ptr->getDataCptr()->at(agentID));

		float upkeep = state_ptr->parameters.strenght.currentUpkeep;

		float effectiveIncome = NTN_UPKEEP_TO_BASE_INCOME_RATIO_TO_WORRY 
								* state_ptr->parameters.resources.updateRate;

		float small = 0.1f; //to avoid blowups and worries about small quantities
		
		float notionBase = NTN_STD_MAX_EFFECTIVE_NOTION_BASE;
		
		//If possible, we want the notionBase to be this proportion:
		if (effectiveIncome > small) {
			notionBase = upkeep/effectiveIncome;
		}
		
		//Since we've sanitized the terms before, we expect that:
		assert(std::isfinite(notionBase));
		assert(notionBase >= 0);

		return notionBase;
	}

	//S1: LOW_DEFENSE_TO_RESOURCES
	float calcNotionBaseS1(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {

		float timeWaster = calcNotionBaseS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	//S2: LOW_CURRENCY
	float calcNotionBaseS2(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseS3(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseS4(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseS5(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseS6(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseS7(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	//N0: LOW_DEFENSE_TO_RESOURCES
	//The neighbor has low defences compared to their resources:
	//- The higher the current resources are compared to the reference, the higher this is;
	//- The lower the guard is compared to the reference, the higher this is;
	float calcNotionBaseN0(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
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

		//Finally, this proportion is the notionBase we want:
		float notionBase = resourcesProportion / defenceProportion;

		//Since we've sanitized the terms before, we expect that:
		assert(std::isfinite(notionBase));
		assert(notionBase >= 0);

		return notionBase;
	}

	//N1: IS_STRONG
	float calcNotionBaseN1(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	//N2: WORRIES_ME
	float calcNotionBaseN2(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	//N3: I_TRUST_THEM
	float calcNotionBaseN3(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN4(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN5(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN6(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN7(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN8(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN9(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN10(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										   PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN11(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										   PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}
}

namespace AS::Decisions::GA {

	//S0: STUB: too much str/income (when compared to reference reads)
	//TODO: overhaul (like on LA::S0)
	float calcNotionBaseS0(int agentID, AS::dataControllerPointers_t* dp, 
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

	float calcNotionBaseS1(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {

		float timeWaster = calcNotionBaseS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseS2(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseS3(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseS4(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseS5(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseS6(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseS7(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	//N0: STUB: neighbor has a lot of cash and little defenses compared to others
	float calcNotionBaseN0(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
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

	float calcNotionBaseN1(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN2(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN3(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN4(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN5(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN6(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN7(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN8(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN9(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN10(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                   PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN11(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
						                   PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}
}


namespace AS::Decisions {

	float calculateNotionBaseSelfLA(notionsSelf notion, int agentID, 
		                      AS::dataControllerPointers_t* dp,
		                      PURE_LA::readsOnNeighbor_t* refReads_ptr) {

		switch ((int)notion)
		{
		case 0:
			return LA::calcNotionBaseS0(agentID, dp, refReads_ptr);
		case 1:
			return LA::calcNotionBaseS1(agentID, dp, refReads_ptr);
		case 2:
			return LA::calcNotionBaseS2(agentID, dp, refReads_ptr);
		case 3:
			return LA::calcNotionBaseS3(agentID, dp, refReads_ptr);
		case 4:
			return LA::calcNotionBaseS4(agentID, dp, refReads_ptr);
		case 5:
			return LA::calcNotionBaseS5(agentID, dp, refReads_ptr);
		case 6:
			return LA::calcNotionBaseS6(agentID, dp, refReads_ptr);
		case 7:
			return LA::calcNotionBaseS7(agentID, dp, refReads_ptr);
		default:
			return 0.0f;
		}
	}

	float calculateNotionBaseNeighborLA(notionsNeighbor notion, int neighbor, 
		                          int agentID, AS::dataControllerPointers_t* dp,
		                          PURE_LA::readsOnNeighbor_t* refReads_ptr) {

		switch ((int)notion)
		{
		case 0:
			return LA::calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);
		case 1:
			return LA::calcNotionBaseN1(neighbor, agentID, dp, refReads_ptr);
		case 2:
			return LA::calcNotionBaseN2(neighbor, agentID, dp, refReads_ptr);
		case 3:
			return LA::calcNotionBaseN3(neighbor, agentID, dp, refReads_ptr);
		case 4:
			return LA::calcNotionBaseN4(neighbor, agentID, dp, refReads_ptr);
		case 5:
			return LA::calcNotionBaseN5(neighbor, agentID, dp, refReads_ptr);
		case 6:
			return LA::calcNotionBaseN6(neighbor, agentID, dp, refReads_ptr);
		case 7:
			return LA::calcNotionBaseN7(neighbor, agentID, dp, refReads_ptr);
		case 8:
			return LA::calcNotionBaseN8(neighbor, agentID, dp, refReads_ptr);
		case 9:
			return LA::calcNotionBaseN9(neighbor, agentID, dp, refReads_ptr);
		case 10:
			return LA::calcNotionBaseN10(neighbor, agentID, dp, refReads_ptr);
		case 11:
			return LA::calcNotionBaseN11(neighbor, agentID, dp, refReads_ptr);
		default:
			return 0.0f;
		}
	}

	float calculateNotionBaseSelfGA(notionsSelf notion, int agentID, 
		                      AS::dataControllerPointers_t* dp,
		                      PURE_GA::readsOnNeighbor_t* refReads_ptr) {

		switch ((int)notion)
		{
		case 0:
			return GA::calcNotionBaseS0(agentID, dp, refReads_ptr);
		case 1:
			return GA::calcNotionBaseS1(agentID, dp, refReads_ptr);
		case 2:
			return GA::calcNotionBaseS2(agentID, dp, refReads_ptr);
		case 3:
			return GA::calcNotionBaseS3(agentID, dp, refReads_ptr);
		case 4:
			return GA::calcNotionBaseS4(agentID, dp, refReads_ptr);
		case 5:
			return GA::calcNotionBaseS5(agentID, dp, refReads_ptr);
		case 6:
			return GA::calcNotionBaseS6(agentID, dp, refReads_ptr);
		case 7:
			return GA::calcNotionBaseS7(agentID, dp, refReads_ptr);
		default:
			return 0.0f;
		}
	}

	float calculateNotionBaseNeighborGA(notionsNeighbor notion, int neighbor, 
		                          int agentID, AS::dataControllerPointers_t* dp,
		                          PURE_GA::readsOnNeighbor_t* refReads_ptr) {

		switch ((int)notion)
		{
		case 0:
			return GA::calcNotionBaseN0(neighbor, agentID, dp, refReads_ptr);
		case 1:
			return GA::calcNotionBaseN1(neighbor, agentID, dp, refReads_ptr);
		case 2:
			return GA::calcNotionBaseN2(neighbor, agentID, dp, refReads_ptr);
		case 3:
			return GA::calcNotionBaseN3(neighbor, agentID, dp, refReads_ptr);
		case 4:
			return GA::calcNotionBaseN4(neighbor, agentID, dp, refReads_ptr);
		case 5:
			return GA::calcNotionBaseN5(neighbor, agentID, dp, refReads_ptr);
		case 6:
			return GA::calcNotionBaseN6(neighbor, agentID, dp, refReads_ptr);
		case 7:
			return GA::calcNotionBaseN7(neighbor, agentID, dp, refReads_ptr);
		case 8:
			return GA::calcNotionBaseN8(neighbor, agentID, dp, refReads_ptr);
		case 9:
			return GA::calcNotionBaseN9(neighbor, agentID, dp, refReads_ptr);
		case 10:
			return GA::calcNotionBaseN10(neighbor, agentID, dp, refReads_ptr);
		case 11:
			return GA::calcNotionBaseN11(neighbor, agentID, dp, refReads_ptr);
		default:
			return 0.0f;
		}
	}

	float delinearizeAndClampNotion(float baseNotion, float effectiveMaxBase, 
													     float smallExponent) {

		//sanity check:
		assert(effectiveMaxBase > 0);

		//sanitizing:
		if (baseNotion < 0) {
			//TODO: add warning after error handling is implemented here
			assert(false); //for now, just crash if in debug
		}

		//effectiveMaxBase is the limit, so:
		if (baseNotion > effectiveMaxBase) {
			return 1;
		}

		//Otherwise, we do the actual math:
		float effectiveProportion = baseNotion / effectiveMaxBase; //in [0 , 1] range
		float finalNotion = powf(effectiveProportion, smallExponent); //also in [0 , 1] range

		assert(std::isfinite(finalNotion));
		
		return finalNotion;
	}
}