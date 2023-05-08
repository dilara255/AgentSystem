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

#include "systems/actionCreation.hpp"

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
		
		float notionBase = NTN_STD_MAX_EFFECTIVE_NOTION_BASE; //in case of really small income

		float small = 0.1f; //to avoid blowups and worries about small quantities
		//But if possible, we want the notionBase to be this proportion:
		if (effectiveIncome > small) {
			notionBase = upkeep/effectiveIncome;
		}
		
		//Since we've sanitized the terms before, we expect that:
		assert(std::isfinite(notionBase));
		assert(notionBase >= 0);

		return notionBase;
	}

	//S1: LOW_DEFENSE_TO_RESOURCES
	//This agent has low defences compared to it's resources:
	//- The higher the current resources are compared to the references, the higher this is;
	//- The lower the defences are compared to the references, the higher this is;
	float calcNotionBaseS1(int agentID, AS::dataControllerPointers_t* dp, 
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

		auto agentParameters_ptr = &(dp->LAstate_ptr->getDataCptr()->at(agentID).parameters);
		
		//Now we get the actual value of our defenses:
		float agentsDefense = agentParameters_ptr->strenght.current
							  + agentParameters_ptr->strenght.externalGuard;
		
		assert(agentsDefense >= 0); //there could always be some floating point weirdness

		float small = 0.1f; //to avoid blowups on division : )

		if (refDefense < small) {
			refDefense = small;
		}
		float defenceProportion = agentsDefense / refDefense;		

		//defenceProportion will be a divident later, so:
		if (defenceProportion < small) {
			defenceProportion = small;
		}

		//Same idea, for resources:
		float agentsResources = std::max(0.0f, agentParameters_ptr->resources.current);
		
		if (refResources < small) {
			refResources = small;
		}
		float resourcesProportion = agentsResources / refResources;

		//Since we may have increased the defenceProportion a bit, 
		//let's extend resourcesProportion the same courtesy:
		if (resourcesProportion < small) {
			resourcesProportion = small;
		}

		//Finally, this proportion is the notionBase we want:
		double notionBaseFromReferenceReads = resourcesProportion / defenceProportion;
		
		//Since we've sanitized the terms before, we expect that:
		assert(std::isfinite(notionBaseFromReferenceReads));
		assert(notionBaseFromReferenceReads >= 0);

		//We want to avoid a situation where everyone is just really lax with their defenses
		//So the actual score will be based on the geometric mean between this
		//and how well we compare to a proportion of REF_STR and REF_RES

		double defenseProportionFromFixedRefs = agentsDefense / ACT_REF_DEFENSE;
		double resourcesProportionFromFixedRefs = agentsResources / ACT_REFERENCE_RESOURCES;
		
		double baseFromNetworkReferenceValues =
			resourcesProportionFromFixedRefs / defenseProportionFromFixedRefs;
		
		float notionBase =
			(float)std::sqrt(baseFromNetworkReferenceValues * notionBaseFromReferenceReads);

		assert(std::isfinite(notionBase));
		assert(notionBase >= 0);

		return notionBase;
	}

	//S2: LOW_CURRENCY
	//Are our current resources low when compared to our references and our upkeep?
	//We check both things and use the worst value D :
	//- To compare to upkeep, we compare how long we could go with no extra income with
	//a (parametrized) reference time.
	float calcNotionBaseS2(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		//We first calculate how our resources compare to our reference resources:
		
		int resourcesIndex = (int)PURE_LA::readsOnNeighbor_t::fields::RESOURCES;
		float refResources = refReads_ptr->readOf[resourcesIndex];

		float small = 0.1f;
		if (refResources < small) { //we don't want to deal wih negatives
			refResources = small;
		}

		auto agentParameters_ptr = &(dp->LAstate_ptr->getDataCptr()->at(agentID).parameters);
		
		float agentsResources = agentParameters_ptr->resources.current;	
		if (agentsResources < small) { ///we also don't want to deal with zeros
			agentsResources = small;
		}

		float resourcesProportion = refResources / agentsResources; //Higher when we're poorer

		//We now check how long we could go with no income:
		//TODO: make sure that external guard is being taken in account
		float upkeep = agentParameters_ptr->strenght.currentUpkeep;
		if (upkeep < small) { 
			upkeep = small;
		}

		float secondsToBankrup = refResources / upkeep;

		assert(secondsToBankrup > 0);

		float referenceBankruptTime = NTN_REFERENCE_BANKRUPT_PERIODS 
									  * NOTIONS_AND_ACTIONS_REF_PERIOD_SECS;

		//If we can't keep up for too long, this will be high:
		float bankruptTimeProportion = referenceBankruptTime / secondsToBankrup;

		//Finally, the notionBase will be the worst (highest) proportion:
		float notionBase = std::max(resourcesProportion, referenceBankruptTime);

		//Since we've sanitized the terms before, we expect that:
		assert(std::isfinite(notionBase));
		assert(notionBase >= 0);

		return notionBase;
	}

	float calcNotionBaseS3(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS2(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseS4(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS2(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseS5(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS2(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseS6(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS2(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseS7(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseS2(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	//N0: LOW_DEFENSE_TO_RESOURCES
	//The neighbor has low defences compared to their resources:
	//- The higher the current resources are compared to the reference, the higher this is;
	//- The lower the defences are compared to the reference, the higher this is;
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
		float defenceProportionRefReads = neighborDefense / refDefense;		

		//defenceProportion will be a divident later, so:
		if (defenceProportionRefReads < small) {
			defenceProportionRefReads = small;
		}

		//Same idea, for resources:
		float neighborResources = readsOfNeighbor_ptr->readOf[resourcesIndex];

		if (refResources < small) {
			refResources = small;
		}
		float resourcesProportionRefReads = neighborResources / refResources;

		//Since we may have increased the defenceProportion a bit, 
		//let's extend resourcesProportion the same courtesy:
		if (resourcesProportionRefReads < small) {
			resourcesProportionRefReads = small;
		}

		//Finally, this proportion is the notionBase we want:
		float notionBaseFromRefReads = resourcesProportionRefReads / defenceProportionRefReads;

		//Since we've sanitized the terms before, we expect that:
		assert(std::isfinite(notionBaseFromRefReads));
		assert(notionBaseFromRefReads >= 0);

		//We want to avoid a situation where everyone is just really lax with their defenses
		//So the actual score will be based on the geometric mean between this
		//and how well we compare to a proportion of REF_STR and REF_RES

		double defenseProportionFromFixedRefs = neighborDefense / ACT_REF_DEFENSE;
		double resourcesProportionFromFixedRefs = neighborResources / ACT_REFERENCE_RESOURCES;
		
		double baseFromNetworkReferenceValues =
			resourcesProportionFromFixedRefs / defenseProportionFromFixedRefs;
		
		float notionBase =
			(float)std::sqrt(baseFromNetworkReferenceValues * notionBaseFromRefReads);

		assert(std::isfinite(notionBase));
		assert(notionBase >= 0);

		return notionBase;
	}

	//N1: IS_STRONG
	//Is their strenght above my defenses? Or is it higher than the reference?
	//We choose the worst propotion as the base for this notions
	float calcNotionBaseN1(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		//Defence is defined as strenght + guard, so to calculate our defenses:
		auto agentsStrenght_ptr 
					= (&dp->LAstate_ptr->getDataCptr()->at(agentID).parameters.strenght);

		float agentsDefenses = 
					agentsStrenght_ptr->current + agentsStrenght_ptr->externalGuard;

		//Now we read the neighbor's and the reference strenght:
		int strenghtIndex = (int)PURE_LA::readsOnNeighbor_t::fields::STRENGHT;
		float refStrenght = refReads_ptr->readOf[strenghtIndex];

		auto readsOfNeighbor_ptr = 
				&(dp->LAdecision_ptr->getDataCptr()->at(agentID).reads[neighbor]);

		float neighborStrenght = readsOfNeighbor_ptr->readOf[strenghtIndex];

		float small = 0.1f; 
		//to avoid blowups on division:
		if (agentsDefenses < small) {
			agentsDefenses = small;
		}
		if (refStrenght < small) {
			refStrenght = small;
		}
		//and just to be fair (and keep values positive):
		if (neighborStrenght < small) {
			neighborStrenght = small;
		}

		//Now we can calculate the proportions we want:
		float neighborsStrenghtToAgentsDefenses = neighborStrenght / agentsDefenses;
		float neighborsStrenghtToReference = neighborStrenght / refStrenght;

		//The worst (highest) proportion will be our notion base:
		float notionBase = 
			std::max(neighborsStrenghtToAgentsDefenses, neighborsStrenghtToReference);

		//Since we've sanitized the terms before, we expect that:
		assert(std::isfinite(notionBase));
		assert(notionBase >= 0);

		return notionBase;
	}

	//N2: WORRIES_ME
	//Do they seem shady and treatening?
	//First we evaluate how much we distrust them:
	//- Bad disposition: distrust;
	//- Lost disposition: distrust; 
	//- Low absolute infiltration level: distrust;
	//- WAR: distrust;
	//Then we chek how strong they are, but our distrust ADDS to that for an effective value;
	//Finally, we compare this effective strenght to our own.
	//TODO: should we care about the relation between our GAs? THis feels complex enough tho.
	float calcNotionBaseN2(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		auto ourDecisionData_ptr = (&dp->LAdecision_ptr->getDataCptr()->at(agentID));
		auto ourState_ptr = (&dp->LAstate_ptr->getDataCptr()->at(agentID));
		auto relations_ptr = &(ourState_ptr->relations);
		
		//first we gather the diplomacy data from our agent:
		float disposition = relations_ptr->dispositionToNeighbors[neighbor];
		float lastDisposition = relations_ptr->dispositionToNeighborsLastStep[neighbor];
		AS::diploStance stance = relations_ptr->diplomaticStanceToNeighbors[neighbor];
		
		float dispositionChange = disposition - lastDisposition;
		float referenceDecisionTimeMs = AS_TOTAL_CHOPS * AS_MILLISECONDS_PER_STEP;

		//We project our future disposition:
		float projectedDispositionChange =
			std::max(NTN_MAX_ABSOLUTE_DISPOSITION_EXTRAPOLATION, 
						(dispositionChange * NOTIONS_AND_ACTIONS_REF_PERIOD_SECS 
														    * MILLIS_IN_A_SECOND) );
		
		float effectiveDisposition = disposition + projectedDispositionChange;			
		
		//War breeds distrust:
		float stanceImpactFactorFromWar = PROPORTIONAL_WEIGHT_OF_WAR_COMPARED_TO_TRADE 
										  * MAX_DISPOSITION_RAISE_FROM_TRADE_PER_SECOND 
										  * NOTIONS_AND_ACTIONS_REF_PERIOD_SECS
										  * NTN_DIPLO_STANCE_WEIGHT_PROPORTION_TO_REFS;

		float changeFromStance = 0;
		if (stance == AS::diploStance::WAR) {
			changeFromStance = stanceImpactFactorFromWar 
				               * NTN_MULTIPLIER_OF_WAR_EFFECT_ON_DISTRUST;
		}

		//This is our effective disposition:
		effectiveDisposition += changeFromStance;

		//Now let's look at our infiltration level:
		float absolutInfiltration = std::abs(ourDecisionData_ptr->infiltration[neighbor]);
		float uncertainty = 1 - absolutInfiltration;

		assert(uncertainty >= 0);

		uncertainty *= uncertainty; //to not over-penalize reasonable levels

		//From effectiveDisposition and uncertainty, we calculate mistrustStrenghtMultiplier:
		float dislike = std::max(0.0f, -effectiveDisposition);

		float mistrustStrenghtMultiplier = 
						NTN_MISTRUST_THREATH_MULTIPLIER * (dislike + uncertainty);

		//Now to compare the strenghts:
		float ourStrenght = ourState_ptr->parameters.strenght.current;
		float small = 0.1f;
		if (ourStrenght < small) {
			ourStrenght = small;
		}

		float theirStrenght = 
			dp->LAstate_ptr->getDataCptr()->at(neighbor).parameters.strenght.current;
		if (theirStrenght < small) {
			theirStrenght = small;
		}

		float strenghtProportion = theirStrenght / ourStrenght;

		//We don't want to freak out because of a strong agent we don't particularly distrust:
		float maxStrenghtProportion = NTN_MISTRUST_THREATH_OVERSHOOT_FACTOR
			* (NTN_STD_MAX_EFFECTIVE_NOTION_BASE / NTN_MISTRUST_THREATH_MULTIPLIER);

		strenghtProportion = std::min(strenghtProportion, maxStrenghtProportion);
			
		//Finally:
		return (strenghtProportion * mistrustStrenghtMultiplier);
	}

	//N3: I_TRUST_THEM
	//High disposition and high absolute infiltration = high trust;
	//Alliances and trade between the agents or their GA's also help;
	//Improving relations also contribute;
	float calcNotionBaseN3(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		auto ourState_ptr = (&dp->LAstate_ptr->getDataCptr()->at(agentID));
		auto relations_ptr = &(ourState_ptr->relations);
		
		//first we gather the data from our agent:
		float disposition = relations_ptr->dispositionToNeighbors[neighbor];
		float lastDisposition = relations_ptr->dispositionToNeighborsLastStep[neighbor];
		AS::diploStance stance = relations_ptr->diplomaticStanceToNeighbors[neighbor];
	
		float dispositionChange = disposition - lastDisposition;
		float referenceDecisionTimeMs = AS_TOTAL_CHOPS * AS_MILLISECONDS_PER_STEP;

		//We project our future disposition:
		float projectedDispositionChange =
			std::max(NTN_MAX_ABSOLUTE_DISPOSITION_EXTRAPOLATION, 
						(dispositionChange * NOTIONS_AND_ACTIONS_REF_PERIOD_SECS 
														    * MILLIS_IN_A_SECOND) );
		
		float effectiveDisposition = disposition + projectedDispositionChange;			
		
		//And then add the impact from the diplomatic stance:
		//These will be used to convert diplomatic stance in impact for this notionBase:
		float stanceImpactFactorFromTrade = 
			MAX_DISPOSITION_RAISE_FROM_TRADE_PER_SECOND * NOTIONS_AND_ACTIONS_REF_PERIOD_SECS
												 * NTN_DIPLO_STANCE_WEIGHT_PROPORTION_TO_REFS;
		float stanceImpactFactorFromAlliance = 
			PROPORTIONAL_WEIGHT_OF_ALLIANCE_COMPARED_TO_TRADE * stanceImpactFactorFromTrade;

		float stanceImpactFactorFromWar =
			PROPORTIONAL_WEIGHT_OF_WAR_COMPARED_TO_TRADE * stanceImpactFactorFromTrade;

		float changeFromStance = 0;
		if (stance == AS::diploStance::WAR) {

			changeFromStance += stanceImpactFactorFromWar;
		}
		if ( (stance == AS::diploStance::TRADE) || 
			 (stance == AS::diploStance::ALLY_WITH_TRADE) ) {

			changeFromStance += stanceImpactFactorFromTrade;
		}
		if ( (stance == AS::diploStance::ALLY) || 
			 (stance == AS::diploStance::ALLY_WITH_TRADE) ) {

			changeFromStance += stanceImpactFactorFromAlliance;
		}

		//This is our effective LOCAL disposition
		effectiveDisposition += changeFromStance;

		//Now we do the same, but for the GAs:
		//But for that we first need to find their IDs : )
		
		//TODO-CRITICAL: extract this to dataMisc (and eventually together with the data structures)

		int neighborIndex = ourState_ptr->locationAndConnections.neighbourIDs[neighbor];
		uint32_t neighborsGA = dp->LAstate_ptr->getDataCptr()->at(neighborIndex).GAid;

		float effectiveGAdisposition = 0;
		bool GAsAreNeighbors = false;
		if (ourState_ptr->GAid == neighborsGA) {

			effectiveGAdisposition = 
				PROPORTIONAL_WEIGHT_SAME_GA_COMPARED_TO_TRADE * stanceImpactFactorFromTrade;
			GAsAreNeighbors = true;
		}
		else {

			auto GAstates_ptr = dp->GAstate_ptr->getDataCptr();
			auto ourGAstate_ptr = &(GAstates_ptr->at(ourState_ptr->GAid));

			int ourGAsTotalNeighbors = ourGAstate_ptr->connectedGAs.howManyAreOn();

			int notFound = -1;
			int neighborsGAidOnOurGA = notFound;
		
			for (int GAneighbor = 0; GAneighbor < ourGAsTotalNeighbors; GAneighbor++) {
				if (ourGAstate_ptr->neighbourIDs[GAneighbor] == neighborsGA) {
					neighborsGAidOnOurGA = GAneighbor;
				}
			}

			bool GAsAreNeighbors = (neighborsGAidOnOurGA != notFound);
		
			//Now we can get the GA info:
			float GAdisposition = 0;
			float GAlastDisposition = 0;
			AS::diploStance GAstance = AS::diploStance::NEUTRAL;

			if (GAsAreNeighbors) {
				GAdisposition = 
					ourGAstate_ptr->relations.dispositionToNeighbors[neighborsGAidOnOurGA];
				GAlastDisposition = 
					ourGAstate_ptr->relations.dispositionToNeighborsLastStep[neighborsGAidOnOurGA];
				GAstance = 
					ourGAstate_ptr->relations.diplomaticStanceToNeighbors[neighborsGAidOnOurGA];
			}

			float changeInGAdisposition = GAlastDisposition - GAdisposition;

			float effectiveGAdisposition = GAdisposition;
			if (GAsAreNeighbors) {

				//We project their future disposition:
				float projectedDispositionChange =
					std::max(NTN_MAX_ABSOLUTE_DISPOSITION_EXTRAPOLATION,
								(changeInGAdisposition * NOTIONS_AND_ACTIONS_REF_PERIOD_SECS 
																		* MILLIS_IN_A_SECOND) );
			
				effectiveGAdisposition += projectedDispositionChange;
				//And then add the impact from the diplomatic stance:

				float changeFromGAstance = 0;
				if (stance == AS::diploStance::WAR) {

					changeFromGAstance += stanceImpactFactorFromWar;
				}
				if ( (stance == AS::diploStance::TRADE) || 
					 (stance == AS::diploStance::ALLY_WITH_TRADE) ) {

					changeFromGAstance += stanceImpactFactorFromTrade;
				}
				if ( (stance == AS::diploStance::ALLY) || 
					 (stance == AS::diploStance::ALLY_WITH_TRADE) ) {

					changeFromGAstance += stanceImpactFactorFromAlliance;
				}

				effectiveGAdisposition += changeFromGAstance;
			}
		}
		
		//Now we add the effectiveGAdisposition, with a weight, to our effectiveDisposition
		//But only if the GAs are actually neighbors:
		float GAweight = GAsAreNeighbors * NTN_GA_DIPLOMACY_WEIGHT_FOR_LAS;
			
		assert(GAweight >= 0);
		assert(GAweight <= 1);

		effectiveDisposition *= (1 - GAweight);
		effectiveDisposition += GAweight * effectiveGAdisposition;

		//Finally, we'll weight in our infiltration level:
		float absoluteInfiltration = 
			std::abs(dp->LAdecision_ptr->getDataCptr()->at(agentID).infiltration[neighbor]);

		float uncertainty = 1 - absoluteInfiltration;

		assert(uncertainty >= 0);

		uncertainty *= uncertainty; //so a little uncertainty doesn't matter as much;

		//The more uncertain we are, the less weight we will give to our disposition:
		float notionBase = effectiveDisposition * (1 - uncertainty);

		//Finally, we'll stretch this so that very high values map the notion to 1;
		//Also, the base should be at or above zero, so:
		return std::max(0.0f, (notionBase * NTN_STD_MAX_EFFECTIVE_NOTION_BASE) );
	}

	float calcNotionBaseN4(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN2(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN5(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN2(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN6(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN2(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN7(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN2(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN8(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN2(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN9(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN2(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN10(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										   PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN2(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionBaseN11(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										   PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionBaseN2(neighbor, agentID, dp, refReads_ptr);

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

	float arithmeticAverageNotions(int totalNeighbors, notions_t* np, int notion) {
		
		if(totalNeighbors == 0) { return 0; }

		float average = 0;
		
		for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {
			
			average += np->neighbors[neighbor][notion];
		}

		return (average / totalNeighbors);
	}

	float rootMeanSquareNotions(int totalNeighbors, notions_t* np, int notion, 
		                         AS::WarningsAndErrorsCounter* errorsCounter_ptr) {
		
		if(totalNeighbors == 0) { return 0; }

		float average = 0;
		
		for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {
			
			average += (np->neighbors[neighbor][notion] * np->neighbors[neighbor][notion]);
		}
		if (!std::isfinite(np->averages[notion])) {
			
			errorsCounter_ptr->incrementWarning(AS::warnings::DS_NOTIONS_RMS_BLEW_UP);
			
			average = arithmeticAverageNotions(totalNeighbors, np, notion);
		}
		else {
			//Otherwise, we keep going with the original RMS plan:
			average /= totalNeighbors;
			average = sqrt(average); //notions are bounded to [0,1]
		}

		return average;
	}
	
	float harmonicMeanNotions(int totalNeighbors, notions_t* np, int notion, 
		                         AS::WarningsAndErrorsCounter* errorsCounter_ptr) {
		
		if(totalNeighbors == 0) { return 0; }

		float average = 0;
		
		for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {
			
			average += (1 / np->neighbors[neighbor][notion]);
		}
		
		average /= totalNeighbors;
		if (average >= 1) {
			return (1 / average); //the harmonic mean
		}
		else {
			errorsCounter_ptr->incrementWarning(AS::warnings::DS_NOTIONS_HARMONIC_OUT_BOUNDS);
			return 1; //this is the maximum as notions are bounded to [0,1]
		}
	}
}