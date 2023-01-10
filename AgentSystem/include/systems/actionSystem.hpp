#pragma once

#include "miscStdHeaders.h"

namespace AS {

	enum actionCategories {
		STRENGHT, RESOURCES, ATTACK, GUARD,
		SPY, SABOTAGE, DIPLOMACY, CONQUEST,
		TOTAL_CATEGORIES
	};
	//TO DO: brief description of each

	enum actionModes {
		IMMEDIATE, REQUEST, SELF,
		TOTAL_MODES
	};
	//TO DO: brief description of each

	enum actionScopes {
		LOCAL, GLOBAL,
		TOTAL_SCOPES
	};

	enum actionAvailability { NOT_AVAILABE, SPECIFIC = 1, STANDARD = -1 };
	//WARNING: any updates to this should be reflected on the initialization of
	//Actions::availableVariations

	//TO DO: singleton
	class Actions {
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
}

