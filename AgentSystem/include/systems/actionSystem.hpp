#pragma once

//TODO: Pull some definitions into a .cpp?

/*
//TODO: review/update this:

This file declares the classes:
- The ActionSystem itself, which includes:
-- ActionDataController class, with two bundles of action data: for LAs and GAs;
-- Action Variations class, wich warps information about which action variations are possible;
- TODO: the base ActionCategoy class.
*/

#include "miscStdHeaders.h"

#include "../include/network/parameters.hpp"
#include "../include/data/actionData.hpp"

namespace AS {	

	//TODO: add description
	//TODO: pass errorCounter to methods
	class ActionDataController {
	public:
		bool initialize(int maxActionsPerAgent, int numberLas, int numberGAs);
		bool addActionData(actionData_t actionData);
		
		const std::vector <AS::actionData_t>* getActionsLAsCptr() const {
			return (const std::vector <AS::actionData_t>*) & dataLAs; }

		const std::vector <AS::actionData_t>* getActionsGAsCptr() const {
			return (const std::vector <AS::actionData_t>*) & dataGAs; }

		bool getAction(AS::scope localOrGlobal, uint32_t actionID, actionData_t* recepient) const;
		bool getAgentsAction(AS::scope localOrGlobal, uint32_t agentID, int actionNumber, 
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
		int m_numberLAs = 0;
		int m_numberGAs = 0; //effective GAs
	};

	//Used to prepare and hold the ActionDataController
	class ActionSystem {
	public:		
		bool initialize(const ActionSystem** actionSystem_cptr_ptr, 
						const ActionDataController** actionDataController_cptr_ptr,
			            const networkParameters_t* networkParams_cptr) {

			LOG_TRACE("Initializing Action System");
			
			bool result = initializeDataController(networkParams_cptr, actionDataController_cptr_ptr);

			m_isInitialized = result;

			if(result) {
				(*actionSystem_cptr_ptr) = (const ActionSystem*)this;
			}

			return result;
		}

		bool initializeDataController(const networkParameters_t* pp,
			                     const ActionDataController** actionDataController_cptr_ptr);

		void stepActions(ActionSystem* ap, float timeMultiplier);

		ActionDataController* getDataDirectPointer() {
			if(!data.isInitialized()){return NULL;}
			return &data;
		}

		ActionDataController const * getDataDirectConstPointer() const {
			if(!data.isInitialized()){return NULL;}
			return (ActionDataController const *)&data;
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
		      //SELF        IMMED.     REQUEST         SELF       IMMED.      REQUEST           
			{{SPC, STD}, {SPC, STD}, {SPC, SPC}},  {{SPC, STD}, {SPC, SPC}, {SPC, STD}}, 

		 	              //ATTACK                               //GUARD
			  //SELF       IMMED.      REQUEST        SELF        IMMED.      REQUEST        
			{{NOT, STD}, {SPC, STD}, {STD, STD}},  {{NOT, STD}, {SPC, STD}, {STD, STD}}, 

		 	               //SPY                               //SABOTAGE
			  //SELF       IMMED.      REQUEST         SELF       IMMED.     REQUEST     
			{{SPC, STD}, {SPC, SPC}, {SPC, SPC}},  {{NOT, STD}, {SPC, STD}, {STD, STD}}, 

		 	            //DIPLOMACY                            //CONQUEST
			  //SELF       IMMED.    REQUEST           SELF        IMMED.    REQUEST        
			{{NOT, SPC}, {SPC, SPC}, {SPC, SPC}},  {{NOT, STD}, {SPC, STD}, {STD, STD}}
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

		static constexpr int totalLocalVariations() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += (availableVariations[i][j][LOCAL] != NOT);
				}
			}
			return amount;
		}

		static constexpr int totalGlobalVariations() {
			int amount = 0;

			for (int i = 0; i < TOTAL_CATEGORIES; i++) {
				for (int j = 0; j < TOTAL_MODES; j++) {
					amount += (availableVariations[i][j][GLOBAL] != NOT);
				}
			}
			return amount;
		}

		static constexpr int mostVariationsOnEitherScope() {

			return std::max(totalLocalVariations(), totalGlobalVariations());
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

		//These are the data structures to hold the functions which will process Actions:
		typedef std::function<void(actionData_t*)> onSpawnFunction_t;
		typedef std::function<uint32_t(uint32_t, actionData_t*)> tickFunction_t;
		typedef std::function<void(actionData_t*)> phaseEndFunction_t;

		typedef struct variationProcessingFunctions_st {
			onSpawnFunction_t onSpawm;
			tickFunction_t onTick[(int)actPhases::TOTAL];
			phaseEndFunction_t onEnd[(int)actPhases::TOTAL];
		} variationProcessingFunctions_t;

		typedef variationProcessingFunctions_t modeProcessingFunctions_t[(int)actModes::TOTAL];
		typedef modeProcessingFunctions_t actionProcessingFunctions_t[(int)actCategories::TOTAL];
	};

	namespace Decisions {
		
		enum class notionsSelf { S0, S1, S2, S3, S4, S5, S6, S7,
													     TOTAL };
		typedef	float AS_API notionsSelf_t[(int)notionsSelf::TOTAL];
		
		enum class notionsNeighbor { N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11,
		                                                                       TOTAL };
		typedef	float AS_API notionsNeighbor_t[(int)notionsNeighbor::TOTAL];

		constexpr int TOTAL_NOTIONS = (int)notionsSelf::TOTAL + (int)notionsNeighbor::TOTAL;

		typedef struct notions_st {

			notionsSelf_t self;
			notionsNeighbor_t averages;
			notionsNeighbor_t neighbors[MAX_NEIGHBORS];

			enum class fields { SELF, NEIGHBORS, TOTAL_LA_NOTIONS_FIELDS };
		} AS_API notions_t;

		#define NEIGHBOR_ID_FOR_SELF (-1)
		typedef struct actionScore_st {
			int32_t actCategory = NEIGHBOR_ID_FOR_SELF;
			int32_t actMode = NEIGHBOR_ID_FOR_SELF;
			int32_t neighbor = NEIGHBOR_ID_FOR_SELF;
			float score = 0;
		} AS_API actionScore_t;

		using namespace ActionVariations;
		
		static constexpr int maxScoresNeeded() {
			constexpr int globalNonSelfVariations = 
				howManyActionsOfKind(AS::actModes::IMMEDIATE, AS::scope::GLOBAL)
				+ howManyActionsOfKind(AS::actModes::REQUEST, AS::scope::GLOBAL);

			constexpr int globalMax = howManyActionsOfKind(AS::actModes::SELF, AS::scope::GLOBAL)
				                           + MAX_GA_QUANTITY * globalNonSelfVariations;

			constexpr int localNonSelfVariations = 
				howManyActionsOfKind(AS::actModes::IMMEDIATE, AS::scope::LOCAL)
				+ howManyActionsOfKind(AS::actModes::REQUEST, AS::scope::LOCAL);

			constexpr int localMax = howManyActionsOfKind(AS::actModes::SELF, AS::scope::LOCAL)
				                        + MAX_LA_NEIGHBOURS * localNonSelfVariations;

			return std::max(localMax, globalMax);
		}

		typedef struct decisionScores_st {
				actionScore_t ambitions, worries, overallUtility;
		} AS_API decisionScores_t;

		inline bool ascendingWorriesCompare(decisionScores_st scoreA, decisionScores_st scoreB) {
			return (scoreA.worries.score < scoreB.worries.score);
		}

		inline bool descendingAmbitionsCompare(decisionScores_st scoreA, decisionScores_st scoreB) {
			return (scoreA.ambitions.score > scoreB.ambitions.score);
		}

		inline bool descendingOverallUtilityCompare(decisionScores_st scoreA, decisionScores_st scoreB) {
			return (scoreA.overallUtility.score > scoreB.overallUtility.score);
		}

		typedef struct allScoresAnyScope_st {
			decisionScores_t allScores[maxScoresNeeded()];
			int actualTotalScores;
		} AS_API allScoresAnyScope_t;		

		namespace LA {
			//TODO: scores by mode bellow not currently in use: should we even keep them?
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
			//TODO: scores by mode bellow not currently in use: should we even keep them?
			typedef	float AS_API 
				scoresSelf_t[howManyActionsOfKind(actModes::SELF, scope::GLOBAL)];
			typedef	float AS_API 
				scoresRequest_t[howManyActionsOfKind(actModes::REQUEST, scope::GLOBAL)];
			typedef	float AS_API 
				scoresImmediate_t[howManyActionsOfKind(actModes::IMMEDIATE, scope::GLOBAL)];

			typedef struct actionScoresByMode_st {

				scoresSelf_t self;
				scoresRequest_t request[MAX_GA_QUANTITY];
				scoresImmediate_t immediate[MAX_GA_QUANTITY];

				enum class fields { SELF, REQUEST, IMMEDIATE, 
									  TOTAL_LA_SCORE_FIELDS };
			} AS_API actionScoresByMode_t;
		}

		typedef float notionWeights_t[TOTAL_NOTIONS];

		typedef std::array<std::array<std::array<float, TOTAL_NOTIONS>, TOTAL_MODES>, TOTAL_CATEGORIES>
			notionsWeightsArray_t;

		static constexpr notionsWeightsArray_t notionWeights = {{
			
			//Weights apply to both LAs and GAs. Meaningless for undefined action variation.
			//For each variation, first row is notionsSelf, second notionsNeighbor.

			{{                         //STRENGHT  
				//SELF:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				//REQUEST:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				//IMMEDIATE:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                          
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
			}},
			{{                         //RESOURCES 
				//SELF:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				//REQUEST:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				//IMMEDIATE:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                          
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
			}},	
			{{                         //ATTACK       
				//SELF:
				{0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				//REQUEST:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				//IMMEDIATE:
				{0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                          
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
			}},	
			{{                         //GUARD       
				//SELF:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				//REQUEST:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				//IMMEDIATE:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                          
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
			}},	
			{{                         //SPY       
				//SELF:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				//REQUEST:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				//IMMEDIATE:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                          
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
			}},	
			{{                         //SABOTAGE       
				//SELF:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				//REQUEST:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				//IMMEDIATE:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                          
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
			}},
			{{                         //DIPLOMACY       
				//SELF:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				//REQUEST:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				//IMMEDIATE:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                          
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
			}},
			{{                         //CONQUEST       
				//SELF:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                     
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				//REQUEST:
				{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                 
				 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
				//IMMEDIATE:
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
}

	
	
	

