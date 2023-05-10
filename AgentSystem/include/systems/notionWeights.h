#pragma once

#include "miscStdHeaders.h"
#include "systems/actionSystem.hpp"


namespace AS::Decisions {	
	typedef float notionWeights_t[TOTAL_NOTIONS];

	typedef std::array<std::array<std::array<float, TOTAL_NOTIONS>, TOTAL_MODES>, TOTAL_CATEGORIES>
		notionsWeightsArray_t;

	static constexpr notionsWeightsArray_t notionWeights = {{
			
		//Weights apply to both LAs and GAs. Meaningless for undefined action variation.
		//For each variation, first row is notionsSelf, second notionsNeighbor.

		{{                         //STRENGHT  
			//SELF:
			{-0.7f, 2.5f, -0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				1.0f, 0.5f, 1.0f, -0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				/*Changed for testing. Original:
			{-0.7f, 2.5f, -0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				1.0f, 0.5f, 1.0f, -0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				*/
			//IMMEDIATE:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			//REQUEST:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                          
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
		}},
		{{                         //RESOURCES 
			//SELF:
			{0.5f, -0.6f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				0.0f, 0.0f, -0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				/*Changed for testing. Original:
				{0.5f, -0.6f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				0.0f, 0.0f, -0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				*/
			//IMMEDIATE:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			//REQUEST:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                          
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
		}},	
		{{                         //ATTACK       
			//SELF:
			{0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			//IMMEDIATE:
			{0.4f, -0.6f, 0.6f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				0.8f, -1.0f, 0.5f, -3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				/**Changed for testing. Original:
				{0.4f, -0.6f, 0.6f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				0.8f, -1.0f, 0.5f, -3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				*/
			//REQUEST:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                          
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
		}},	
		{{                         //GUARD       
			//SELF:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			//IMMEDIATE:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			//REQUEST:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                          
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
		}},	
		{{                         //SPY       
			//SELF:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			//IMMEDIATE:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			//REQUEST:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                          
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
		}},	
		{{                         //SABOTAGE       
			//SELF:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			//IMMEDIATE:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			//REQUEST:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                          
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
		}},
		{{                         //DIPLOMACY       
			//SELF:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			//IMMEDIATE:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			//REQUEST:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                          
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
		}},
		{{                         //CONQUEST       
			//SELF:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			//IMMEDIATE:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
			//REQUEST:
			{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                          
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
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
}
