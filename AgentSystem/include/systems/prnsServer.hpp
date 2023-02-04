#pragma once

/*
Declares a class to hold the pseudo random numbers (PRNs) needed for each step.
Has methods to:
- populate it (using a generator from Aux0);
- get the next number;
- get the next number in different formats/intervals;
*/

#include "network/parameters.hpp"

#define PRNS_PER_LA (LA_FIELDS_TO_DEDUCE_EXPECTED * MAX_LA_NEIGHBOURS + PRNS_TO_CHOOSE_ACTION)
#define PRNS_PER_GA (GA_FIELDS_TO_DEDUCE_EXPECTED * (MAX_GA_QUANTITY-1) + PRNS_TO_CHOOSE_ACTION)
#define MAX_LA_PRNS (PRNS_PER_LA*MAX_LA_QUANTITY)
#define MAX_GA_PRNS (PRNS_PER_GA*MAX_GA_QUANTITY)
#define MAX_ACT_PRNS (PRNS_PER_ACT*MAX_AGENTS)
#define MAX_PRNS (MAX_LA_PRNS + MAX_GA_PRNS + MAX_ACT_PRNS)

#if (AS_STEPS_PER_DECISION_STEP > 1)
	#define TOTAL_PRNG_CHOPS (AS_STEPS_PER_DECISION_STEP - 1)
#else
	#define TOTAL_PRNG_CHOPS 1
#endif

namespace AS{

	class PRNserver {
	public:
		void drawPRNs(int numberLAs, int numberGAs, int chopIndex);
		float getNext() {return PRNs[nextToUse++];}
		uint64_t getSeed(int index) const {return seeds[index];}
		void setSeed(int index, uint64_t newSeed) {seeds[index] = newSeed;}

		void printDataDebug() {
			printf("\n\n----> PRNs GENERATED: %d:\n\n", drawn);

			for (int i = 0; i < drawn; i++) {
				printf("%f\n",PRNs[i]);
			}

			printf("\n\n----> END OF PRNG DEBUG PRINT\n\n");
		}

	private:
		int drawn = 0;
		int nextToUse = 0;
		float PRNs[MAX_PRNS];
		uint64_t seeds[DRAW_WIDTH];
	};
}