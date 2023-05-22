#pragma once

#include "miscStdHeaders.h"
#include "systems/actionSystem.hpp"


namespace AS::Decisions {	
	typedef float notionWeights_t[TOTAL_NOTIONS];

	typedef std::array<std::array<std::array<float, TOTAL_NOTIONS>, TOTAL_MODES>, TOTAL_CATEGORIES>
		notionsWeightsArray_t;

	constexpr float tank = -999999.99f;

	static constexpr notionsWeightsArray_t notionWeights = {{
			
		//Weights apply to both LAs and GAs. Meaningless for undefined action variation.
		//For each variation, first row is notionsSelf, second notionsNeighbor.

		{{                         //STRENGHT  
			//SELF:
			{-0.8f, 1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
			  0.6f, 0.8f, 0.6f, -0.25f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			//IMMEDIATE:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank},
			//REQUEST:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank}
		}},
		{{                         //RESOURCES 
			//SELF:
			{1.0f, -0.8f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
			 0.4f, 0.0f, -0.4f, 0.35f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			//IMMEDIATE:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank},
			//REQUEST:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank}
		}},	
		{{                         //ATTACK       
			//SELF:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank},
			//IMMEDIATE:
			{0.7f, -0.6f, 0.65f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
			 0.9f, -1.0f, 0.5f, -3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			//REQUEST:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank}
		}},	
		{{                         //GUARD       
			//SELF:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank},
			//IMMEDIATE:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank},
			//REQUEST:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank}
		}},	
		{{                         //SPY       
			//SELF:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank},
			//IMMEDIATE:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank},
			//REQUEST:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank}
		}},	
		{{                         //SABOTAGE       
			//SELF:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank},
			//IMMEDIATE:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank},
			//REQUEST:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank}
		}},
		{{                         //DIPLOMACY       
			//SELF:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank},
			//IMMEDIATE:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank},
			//REQUEST:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank}
		}},
		{{                         //CONQUEST       
			//SELF:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank},
			//IMMEDIATE:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank},
			//REQUEST:
			{tank, tank, tank, tank, tank, tank, tank, tank,                 
			 tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank, tank}
		}}		
	}};

	static constexpr notionsWeightsArray_t getWeightsInFavor() {
		notionsWeightsArray_t weights{};

		for (int i = 0; i < TOTAL_CATEGORIES; i++) {
			for (int j = 0; j < TOTAL_MODES; j++) {
				for (int k = 0; k < TOTAL_NOTIONS; k++) {

					weights.at(i).at(j).at(k) =
						std::max(0.f, notionWeights.at(i).at(j).at(k));
				}
			}
		}

		return weights;
	}

	static constexpr notionsWeightsArray_t getWeightsAgainst() {
		notionsWeightsArray_t weights{};

		for (int i = 0; i < TOTAL_CATEGORIES; i++) {
			for (int j = 0; j < TOTAL_MODES; j++) {
				for (int k = 0; k < TOTAL_NOTIONS; k++) {		

					weights.at(i).at(j).at(k) =
						- std::min(0.f, notionWeights.at(i).at(j).at(k));
				}
			}
		}

		return weights;
	}

	static constexpr notionsWeightsArray_t notionWeightsInFavor = getWeightsInFavor();
	static constexpr notionsWeightsArray_t notionWeightsAgainst = getWeightsAgainst();

	//Notions are "delinearized" and cast to the [0 , 1] range after base calculation
	//These are the parameters used for that:

	//The exponents set the shapness of the effect. They should be small, positive numbers

	static constexpr float dfExp = NTN_STD_DELINEARIZATION_EXPONENT;
	static constexpr float N4Exp = NTN_N4_TRUST_DELINEARIZATION_EXPONENT;
	static constexpr float S1Exp = NTN_S1_LOW_DEF_TO_RES_DELINEARIZATION_EXPONENT;
		
	//For LA's:
	static constexpr float notionsDelinearizationExposLA[TOTAL_NOTIONS] = {
		//Notions Self:
		dfExp, S1Exp, dfExp, dfExp, dfExp, dfExp, dfExp, dfExp,
		//Notions Neighbors:
		S1Exp, dfExp, dfExp, N4Exp, dfExp, dfExp, dfExp, dfExp, dfExp, dfExp, dfExp, dfExp
	};
	//For GA's:
	static constexpr float notionsDelinearizationExposGA[TOTAL_NOTIONS] = {
		//Notions Self:
		dfExp, S1Exp, dfExp, dfExp, dfExp, dfExp, dfExp, dfExp,
		//Notions Neighbors:
		S1Exp, dfExp, dfExp, N4Exp, dfExp, dfExp, dfExp, dfExp, dfExp, dfExp, dfExp, dfExp
	};

	static constexpr float getDelinearizationExpoSelf(notionsSelf notion, 
														    AS::scope scope) {
		if (scope == AS::scope::LOCAL) {
			return notionsDelinearizationExposLA[(int)notion];
		}
		else {
			return notionsDelinearizationExposGA[(int)notion];
		}
	}

	static constexpr float getDelinearizationExpoNeighbor(notionsNeighbor notion, 
														        AS::scope scope) {
		int index = (int)notionsSelf::TOTAL + (int)notion;

		if (scope == AS::scope::LOCAL) {
			return notionsDelinearizationExposLA[index];
		}
		else {
			return notionsDelinearizationExposGA[index];
		}
	}

	//The maximum effective bases are the saturation point for the base calculations;
	//Any base >= these will be mapped to 1;

	static constexpr float maxBs = NTN_STD_MAX_EFFECTIVE_NOTION_BASE;
	static constexpr float N4mxB = NTN_N4_TRUST_MAX_EFFECTIVE_NOTION_BASE;
	static constexpr float S1mxB = NTN_S1_LOW_DEF_TO_RES_MAX_EFFECTIVE_NOTION_BASE;

	//For LA's:
	static constexpr float notionsMaxEffectiveBasesLA[TOTAL_NOTIONS] = {
		//Notions Self:
		maxBs, S1mxB, maxBs, maxBs, maxBs, maxBs, maxBs, maxBs,
		//Notions Neighbors:
		maxBs, maxBs, maxBs, N4mxB, maxBs, maxBs, maxBs, maxBs, maxBs, maxBs, maxBs, maxBs
	};
	//For GA's:
	static constexpr float notionsMaxEffectiveBasesGA[TOTAL_NOTIONS] = {
		//Notions Self:
		maxBs, S1mxB, maxBs, maxBs, maxBs, maxBs, maxBs, maxBs,
		//Notions Neighbors:
		maxBs, maxBs, maxBs, N4mxB, maxBs, maxBs, maxBs, maxBs, maxBs, maxBs, maxBs, maxBs
	};

	static constexpr float getEffectiveMaxBaseSelf(notionsSelf notion, 
														    AS::scope scope) {
		if (scope == AS::scope::LOCAL) {
			return notionsMaxEffectiveBasesLA[(int)notion];
		}
		else {
			return notionsMaxEffectiveBasesGA[(int)notion];
		}
	}

	static constexpr float getEffectiveMaxBaseNeighbor(notionsNeighbor notion, 
														        AS::scope scope) {
		int index = (int)notionsSelf::TOTAL + (int)notion;

		if (scope == AS::scope::LOCAL) {
			return notionsMaxEffectiveBasesLA[index];
		}
		else {
			return notionsMaxEffectiveBasesGA[index];
		}
	}

	enum class notionMeanStrategies { AVG, RMS, HAR,
										TOTAL };

	constexpr auto AVG = notionMeanStrategies::AVG;
	constexpr auto RMS = notionMeanStrategies::RMS;
	constexpr auto HAR = notionMeanStrategies::HAR;

	static constexpr notionMeanStrategies neighborNotionMeanTakingStrategy[(int)notionsNeighbor::TOTAL] = {
		RMS, HAR, RMS, HAR, RMS, RMS, RMS, RMS, RMS, RMS, RMS, RMS
	};

	static constexpr notionMeanStrategies getMeanTakingStrategy(notionsNeighbor notion) {
		return neighborNotionMeanTakingStrategy[(int)notion];
	}
}
