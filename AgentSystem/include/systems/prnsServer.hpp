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
#define MAX_ACT_PRNS (PRN_PER_ACT*MAX_AGENTS)
#define MAX_PRNS (MAX_LA_PRNS + MAX_GA_PRNS + MAX_ACT_PRNS)

namespace AS{

	class PRNserver {
	public:
		void drawPRNs(bool willAgentsMakeDecisions, int numberLAs, int numberGAs);
		float getNext() {return PRNs[nextToUse++];}

	private:
		int drawn = 0;
		int nextToUse = 0;
		float PRNs[MAX_PRNS];
	};
}