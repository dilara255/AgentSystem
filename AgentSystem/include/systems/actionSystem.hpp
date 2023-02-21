#pragma once

/*
This file declares the classes:
- The ActionSystem itself, which includes:
-- ActionDataController class, with two bundles of action data: for LAs and GAs;
-- Action Variations class, wich warps information about which action variations are possible;
- TODO: the base ActionCategoy class.
*/

#include "miscStdHeaders.h"

#include "../include/data/actionData.hpp"
#include "../include/network/parameters.hpp"

namespace AS {	
	//TODO: singleton, initialize, test
	class ActionDataController {
	public:
		bool initialize(int maxActionsPerAgent, int numberLas, int numberGAs);
		bool addActionData(actionData_t actionData);
		
		const std::vector <AS::actionData_t>* getActionsLAsCptr() const {
			return (const std::vector <AS::actionData_t>*) & dataLAs; }

		const std::vector <AS::actionData_t>* getActionsGAsCptr() const {
			return (const std::vector <AS::actionData_t>*) & dataGAs; }

		bool getAction(AS::scope localOrGlobal, uint32_t actionID, actionData_t* recepient) const;
		bool getAgentData(AS::scope localOrGlobal, uint32_t agentID, int actionNumber, 
			                                                  actionData_t* recepient) const;

		void pushBackLAaction(actionData_t action) { dataLAs.push_back(action); }
		void pushBackGAaction(actionData_t action) { dataGAs.push_back(action); }
		void popBackLAaction() { dataLAs.pop_back(); }
		void popBackGAaction() { dataGAs.pop_back(); }

		size_t sizeOfDataInBytesLAs() const;
		size_t sizeOfDataInBytesGAs() const;
		size_t capacityForDataInBytesLAs() const;
		size_t capacityForDataInBytesGAs() const;
		
		void clearData();
		bool isInitialized() const { return m_isInitialized; }
		bool hasData() const { return m_hasData; }

		std::vector <actionData_t>* getDirectLAdataPtr() { return &dataLAs; }
		std::vector <actionData_t>* getDirectGAdataPtr() { return &dataGAs; }

		int getMaxActionsPerAgent() const { return m_maxActionsPerAgent; }
		bool setMaxActionsPerAgent(int newMax) {
			if ( (newMax > 0) && (newMax <= MAX_ACTIONS_PER_AGENT) ) {
				m_maxActionsPerAgent = newMax;
				LOG_TRACE("New maximum set for actions per agent");
				return true;
			}
			else {
				LOG_ERROR("Failed to set new maximum of actions per agent: out of allowed bounds");
				return false;
			}
		}
				
	private:
		std::vector <actionData_t> dataLAs;
		std::vector <actionData_t> dataGAs;
		bool m_isInitialized = false;
		bool m_hasData = false;
		int m_maxActionsPerAgent = 0;
	};

	
	namespace ActionVariations {
		
		constexpr auto NOT = actExists::NOT;
		constexpr auto STD = actExists::STD;
		constexpr auto SPC = actExists::SPC;
		constexpr auto TOTAL_CATEGORIES = ((int)actCategories::TOTAL);
		constexpr auto TOTAL_MODES = ((int)actModes::TOTAL);
		constexpr auto TOTAL_SCOPES = ((int)scope::TOTAL);
		constexpr auto LOCAL = ((int)scope::LOCAL);
		constexpr auto GLOBAL = ((int)scope::GLOBAL);

		//TODO: Review these when action system is implemented
		static const actExists availableVariations[TOTAL_CATEGORIES][TOTAL_MODES][TOTAL_SCOPES] = {
			
			//KEY for the numbers: see AS::actionAvailability enum on actionData.hpp
			//Innermost Order is: { LOCAL, GLOBAL }
			
			            //STRENGHT                            //RESOURCES                
		      //IMMED.    REQUEST       SELF          IMMED.     REQUEST       SELF    
			{{SPC, SPC}, {SPC, STD}, {SPC, STD}},  {{SPC, SPC}, {SPC, STD}, {SPC, STD}}, 

		 	              //ATTACK                               //GUARD
			  //IMMED.    REQUEST       SELF          IMMED.     REQUEST       SELF    
			{{SPC, STD}, {STD, STD}, {NOT, STD}},  {{SPC, STD}, {SPC, STD}, {NOT, STD}}, 

		 	               //SPY                               //SABOTAGE
			  //IMMED.    REQUEST       SELF          IMMED.     REQUEST       SELF    
			{{SPC, SPC}, {SPC, SPC}, {NOT, STD}},  {{SPC, STD}, {STD, STD}, {NOT, STD}}, 

		 	            //DIPLOMACY                            //CONQUEST
			  //IMMED.    REQUEST       SELF          IMMED.     REQUEST       SELF    
			{{SPC, SPC}, {SPC, SPC}, {NOT, SPC}},  {{SPC, STD}, {STD, STD}, {NOT, STD}},
		};

		static bool isVariationValid(int category, int mode, int scope) {
			bool inBounds = (category < TOTAL_CATEGORIES);
			inBounds &= (mode < TOTAL_MODES);
			inBounds &= (scope < TOTAL_SCOPES);
			if(!inBounds) {return false;}

			return (availableVariations[category][mode][scope] != NOT);
		}	

		static actExists getVariationExistence(int category, int mode, int scope) {
			bool inBounds = (category < TOTAL_CATEGORIES);
			inBounds &= (mode < TOTAL_MODES);
			inBounds &= (scope < TOTAL_SCOPES);
			if(!inBounds) {return NOT;}

			return availableVariations[category][mode][scope];
		}

		static int localVariations() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += (availableVariations[i][j][LOCAL] != NOT);
				}
			}
			return amount;
		}

		static int globalVariations() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += (availableVariations[i][j][GLOBAL] != NOT);
				}
			}
			return amount;
		}

		static int kindsOfStandardActions() {
			bool hasAppeard[TOTAL_MODES][TOTAL_SCOPES];

			for (int i = 0; i < TOTAL_MODES; i++) {
				for (int j = 0; j < TOTAL_SCOPES; j++) {
					hasAppeard[i][j] = false;
				}
			}

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {

					hasAppeard[j][LOCAL] |= (availableVariations[i][j][LOCAL] == STD);
					hasAppeard[j][GLOBAL] |= (availableVariations[i][j][GLOBAL] == STD);
				}
			}

			int amount = 0;
			for (int i = 0; i < TOTAL_MODES; i++) {
				for (int j = 0; j < TOTAL_SCOPES; j++) {
					amount += (int)hasAppeard[i][j];
				}
			}
			return amount;
		}

		static int totalUniqueVariations() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += (availableVariations[i][j][LOCAL] == SPC);
				}
			}
			return amount;
		}

		static int totalStandardVariations() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += (availableVariations[i][j][LOCAL] == STD);
				}
			}
			return amount;
		}

		static int totalVariations() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += (availableVariations[i][j][LOCAL] != NOT);
					amount += (availableVariations[i][j][GLOBAL] != NOT);
				}
			}
			return amount;
		}
	};

	//TODO: singleton, initialize, test
	class ActionSystem {
	public:
		ActionDataController data; //all action data is here!

		bool initializeDataController(const networkParameters_t* pp,
			                     const ActionDataController** actionDataController_cptr_ptr);
		
		bool initialize(const ActionSystem** actionSystem_cptr_ptr) {
			LOG_TRACE("Initializing Action System");
			m_isInitialized = true;
			(*actionSystem_cptr_ptr) = (const ActionSystem*)this;
			return true;
		}
		bool isInitialized() const { return m_isInitialized; }

	private:
		bool m_isInitialized = false;
	};
}

