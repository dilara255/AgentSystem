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

	//TODO: add description
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

	//Used to prepare and hold the ActionDataController
	class ActionSystem {
	public:		
		bool initialize(const ActionSystem** actionSystem_cptr_ptr, 
						const ActionDataController** actionDataController_cptr_ptr,
			            const networkParameters_t* networkParams_cptr) {
			LOG_TRACE("Initializing Action System");
			m_isInitialized = true;
			bool result = initializeDataController(networkParams_cptr, actionDataController_cptr_ptr);
			(*actionSystem_cptr_ptr) = (const ActionSystem*)this;
			return result;
		}

		bool initializeDataController(const networkParameters_t* pp,
			                     const ActionDataController** actionDataController_cptr_ptr);

		int stepActions(float timeMultiplier);

		ActionDataController* getDataDirectPointer() {
			if(!data.isInitialized()){return NULL;}
			return &data;
		}

		bool isInitialized() const { return m_isInitialized; }

	private:
		bool m_isInitialized = false;
		ActionDataController data; //all action data is here!
	};

	//TODO: add description
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
		static constexpr actExists availableVariations[TOTAL_CATEGORIES][TOTAL_MODES][TOTAL_SCOPES] = {
			
			//KEY for the numbers: see AS::actionAvailability enum on actionData.hpp
			//Innermost Order is: { LOCAL, GLOBAL }
			
			            //STRENGHT                            //RESOURCES                
		      //IMMED.    REQUEST       SELF          IMMED.     REQUEST       SELF    
			{{SPC, SPC}, {SPC, STD}, {SPC, STD}},  {{SPC, SPC}, {SPC, STD}, {SPC, STD}}, 

		 	              //ATTACK                               //GUARD
			  //IMMED.    REQUEST       SELF          IMMED.     REQUEST       SELF    
			{{SPC, STD}, {STD, STD}, {NOT, STD}},  {{SPC, STD}, {STD, STD}, {NOT, STD}}, 

		 	               //SPY                               //SABOTAGE
			  //IMMED.    REQUEST       SELF          IMMED.     REQUEST       SELF    
			{{SPC, SPC}, {SPC, SPC}, {SPC, STD}},  {{SPC, STD}, {STD, STD}, {NOT, STD}}, 

		 	            //DIPLOMACY                            //CONQUEST
			  //IMMED.    REQUEST       SELF          IMMED.     REQUEST       SELF    
			{{SPC, SPC}, {SPC, SPC}, {NOT, SPC}},  {{SPC, STD}, {STD, STD}, {NOT, STD}},
		};

		constexpr uint32_t actionIDonScope(int category, int mode) {
			return (category * TOTAL_MODES) + mode;
		}

		constexpr actCategories categoryFromScopesActionID(uint32_t id) {
			return (actCategories)(id / TOTAL_MODES);
		}

		constexpr actCategories modeFromScopesActionID(uint32_t id) {
			return (actCategories)(id % TOTAL_CATEGORIES);
		}

		constexpr int howManyActionsOfKind(actModes mode, scope scp) {
			int total = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				total += (int)(availableVariations[i][(int)mode][(int)scp] != actExists::NOT);
			}

			return total;
		}

		static constexpr bool isValid(int category, int mode, int scope) {
			bool inBounds = (category < TOTAL_CATEGORIES);
			inBounds &= (mode < TOTAL_MODES);
			inBounds &= (scope < TOTAL_SCOPES);
			if(!inBounds) {return false;}

			return (availableVariations[category][mode][scope] != NOT);
		}	

		static constexpr actExists getExistence(int category, int mode, int scope) {
			bool inBounds = (category < TOTAL_CATEGORIES);
			inBounds &= (mode < TOTAL_MODES);
			inBounds &= (scope < TOTAL_SCOPES);
			if(!inBounds) {return NOT;}

			return availableVariations[category][mode][scope];
		}

		static constexpr int totalLocals() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += (availableVariations[i][j][LOCAL] != NOT);
				}
			}
			return amount;
		}

		static constexpr int totalGlobals() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += (availableVariations[i][j][GLOBAL] != NOT);
				}
			}
			return amount;
		}

		//Standard actions with different mode and/or scope, no matter what category
		static constexpr int kindsOfStandards() {
			bool hasAppeard[TOTAL_MODES][TOTAL_SCOPES] = {};

			for (int i = 0; i < TOTAL_MODES; i++) {
				for (int j = 0; j < TOTAL_SCOPES; j++) {
					hasAppeard[i][j] = false;
				}
			}

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					for (int k = 0; k < TOTAL_SCOPES; k++) {
						hasAppeard[j][k] |= (availableVariations[i][j][k] == STD);
					}
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

		static constexpr int totalSpecifics() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += (availableVariations[i][j][LOCAL] == SPC);
					amount += (availableVariations[i][j][GLOBAL] == SPC);
				}
			}
			return amount;
		}

		static constexpr int totalStandards() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += (availableVariations[i][j][LOCAL] == STD);
					amount += (availableVariations[i][j][GLOBAL] == STD);
				}
			}
			return amount;
		}

		static constexpr int totalNots() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += (availableVariations[i][j][LOCAL] == NOT);
					amount += (availableVariations[i][j][GLOBAL] == NOT);
				}
			}
			return amount;
		}

		static constexpr int totalValids() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += (availableVariations[i][j][LOCAL] != NOT);
					amount += (availableVariations[i][j][GLOBAL] != NOT);
				}
			}
			return amount;
		}

		static constexpr int totalPossible() {
			return (int)AS::actCategories::TOTAL*(int)AS::actModes::TOTAL*(int)AS::scope::TOTAL;
		}
	};

	namespace Decisions {
		
		enum class notionsSelf { S0, S1, S2, S3, S4, S5, S6, S7,
													 TOTAL };
		typedef	float AS_API notionsSelf_t[(int)notionsSelf::TOTAL];
		
		enum class notionsNeighbor { N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11,
		                                                                   TOTAL };
		typedef	float AS_API notionsNeighbor_t[(int)notionsNeighbor::TOTAL];

		typedef struct notions_st {

			notionsSelf_t self;
			notionsNeighbor_t neighbors[MAX_LA_NEIGHBOURS];

			enum class fields { SELF, NEIGHBORS, TOTAL_LA_NOTIONS_FIELDS };
		} AS_API notions_t;

		typedef struct actionScore_st {

			int32_t actID = -1;
			int32_t neighbor = -1;
			float score = 0;
		} AS_API actionScore_t;


		inline bool ascendingScoreCompare(actionScore_st scoreA, actionScore_st scoreB) {
			return (scoreA.score < scoreB.score);
		}

		inline bool descendingScoreCompare(actionScore_st scoreA, actionScore_st scoreB) {
			return (scoreA.score > scoreB.score);
		}

		using namespace ActionVariations;

		namespace LA {

			typedef actionScore_t AS_API allScoresThisScope_t[totalLocals()];

			typedef struct decisionScores_st {
				allScoresThisScope_t inFavor, against, overall;
				int totalScores = 0;
			} AS_API decisionScores_t;

			typedef	float AS_API 
				scoresSelf_t[howManyActionsOfKind(actModes::SELF, scope::LOCAL)];
			typedef	float AS_API 
				scoresRequest_t[howManyActionsOfKind(actModes::REQUEST, scope::LOCAL)];
			typedef	float AS_API 
				scoresImmediate_t[howManyActionsOfKind(actModes::IMMEDIATE, scope::LOCAL)];
			
			typedef struct actionScoresByMode_st {

				scoresSelf_t self;
				scoresRequest_t request[MAX_LA_NEIGHBOURS];
				scoresImmediate_t immediate[MAX_LA_NEIGHBOURS];

				enum class fields { SELF, REQUEST, IMMEDIATE, 
									  TOTAL_LA_SCORE_FIELDS };
			} AS_API actionScoresByMode_t;
		}

		namespace GA {

			typedef actionScore_t AS_API allScoresThisScope_t[totalGlobals()];

			typedef struct decisionScores_st {
				allScoresThisScope_t inFavor, against, overall;
				int totalScores = 0;
			} AS_API decisionScores_t;

			typedef	float AS_API 
				scoresSelf_t[howManyActionsOfKind(actModes::SELF, scope::GLOBAL)];
			typedef	float AS_API 
				scoresRequest_t[howManyActionsOfKind(actModes::REQUEST, scope::GLOBAL)];
			typedef	float AS_API 
				scoresImmediate_t[howManyActionsOfKind(actModes::IMMEDIATE, scope::GLOBAL)];

			typedef struct actionScoresByMode_st {

				scoresSelf_t self;
				scoresRequest_t request[MAX_LA_NEIGHBOURS];
				scoresImmediate_t immediate[MAX_LA_NEIGHBOURS];

				enum class fields { SELF, REQUEST, IMMEDIATE, 
									  TOTAL_LA_SCORE_FIELDS };
			} AS_API actionScoresByMode_t;
		}
	}
}

	
	
	

