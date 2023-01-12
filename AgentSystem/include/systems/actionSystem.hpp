#pragma once

/*
This file declares the classes:
- The ActionSystem itself, which includes:
-- ActionDataController class, with two bundles of action data: for LAs and GAs;
-- Action Variations class, wich warps information about which action variations are possible;
- TO DO: the base ActionCategoy class.
*/

#include "miscStdHeaders.h"

#include "../include/data/actionData.hpp"
#include "../include/network/parameters.hpp"

namespace AS {	
	//TO DO: singleton, initialize, test
	class ActionDataController {
	public:
		bool initialize(int maxActionsPerAgent, int numberLas, int numberGAs);
		bool addActionData(actionData_t actionData);
		
		const std::vector <AS::actionData_t>* getActionsLAsCptr() const {
			return (const std::vector <AS::actionData_t>*) & dataLAs; }

		const std::vector <AS::actionData_t>* getActionsGAsCptr() const {
			return (const std::vector <AS::actionData_t>*) & dataGAs; }

		bool getAction(int localOrGlobal, uint32_t actionID, actionData_t* recepient) const;
		bool getAgentData(int localOrGlobal, uint32_t agentID, int actionNumber, 
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

	//TO DO: singleton, initialize, test
	class ActionVariations {
	public:

		//see AS::actionAvailability enum (now: 0: not, 1: specific, -1: standard)
		int isVariationValid(int category, int mode, int scope) {
			return availableVariations[category][mode][scope];
		}	

		int localVariations() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += abs(availableVariations[i][j][LOCAL]);
				}
			}
			return amount;
		}

		int globalVariations() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += abs(availableVariations[i][j][GLOBAL]);
				}
			}
			return amount;
		}

		int kindsOftandardActions() {
			bool hasAppeard[TOTAL_MODES][TOTAL_SCOPES];

			for (int i = 0; i < TOTAL_MODES; i++) {
				for (int j = 0; j < TOTAL_SCOPES; j++) {
					hasAppeard[i][j] = false;
				}
			}

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {

					hasAppeard[j][LOCAL] |= (availableVariations[i][j][LOCAL] == STANDARD);
					hasAppeard[j][GLOBAL] |= (availableVariations[i][j][GLOBAL] == STANDARD);
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

		int totalUniqueVariations() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += (availableVariations[i][j][LOCAL] == SPECIFIC);
				}
			}
			return amount;
		}

		int totalStandardVariations() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += (availableVariations[i][j][LOCAL] == STANDARD);
				}
			}
			return amount;
		}

		int totalVariations() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += abs(availableVariations[i][j][LOCAL]);
					amount += abs(availableVariations[i][j][GLOBAL]);
				}
			}
			return amount;
		}

	private:
		//TO DO: Review these when action system is implemented
		const int availableVariations[TOTAL_CATEGORIES][TOTAL_MODES][TOTAL_SCOPES] = {
			
			//KEY for the numbers: see AS::actionAvailability enum above
			//Innermost Order is: { LOCAL, GLOBAL }
			
			            //STRENGHT                            //RESOURCES                
		      //IMMED.    REQUEST       SELF          IMMED.     REQUEST       SELF    
			{{  1,  1 }, {  1, -1 }, {  1, -1 }},  {{  1,  1 }, {  1, -1 }, {  1, -1 }},

		 	              //ATTACK                               //GUARD
			  //IMMED.    REQUEST       SELF          IMMED.     REQUEST       SELF    
			{{  1, -1 }, { -1, -1 }, {  0, -1 }},  {{  1, -1 }, {  1, -1 }, {  0, -1 }},

		 	               //SPY                               //SABOTAGE
			  //IMMED.    REQUEST       SELF          IMMED.     REQUEST       SELF    
			{{  1,  1 }, {  1,  1 }, {  0, -1 }},  {{  1, -1 }, { -1, -1 }, {  0, -1 }},

		 	            //DIPLOMACY                            //CONQUEST
			  //IMMED.    REQUEST       SELF          IMMED.     REQUEST       SELF    
			{{  1,  1 }, {  1,  1 }, {  0,  1 }},  {{  1, -1 }, { -1, -1 }, {  0, -1 }},
		};
	};

	//TO DO: singleton, initialize, test
	class ActionSystem {
	public:
		ActionDataController data; //all action data is here!
		ActionVariations variations; //which sorts of actions are possible

		bool initializeDataController(const networkParameters_t* pp,
			                     const ActionDataController** actionDataController_cptr_ptr);
		//TO DO: the actual system to use these

		bool initialize(const ActionSystem** actionSystem_cptr_ptr) {
			LOG_TRACE("Initializing Action System (stub)");
			m_isInitialized = true;
			(*actionSystem_cptr_ptr) = (const ActionSystem*)this;
			return true;
		}
		bool isInitialized() const { return m_isInitialized; }

	private:
		bool m_isInitialized = false;
	};
}

