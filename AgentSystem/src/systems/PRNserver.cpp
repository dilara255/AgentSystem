/*
Defines a class to hold the pseudo random numbers (PRNs) needed for each step.
Holds floats in the interval [0-1]
Has methods to:
- populate (using a generator from Aux0);
- get the next number;
- TODO: get the next number in different formats/intervals;
*/

#include "systems/AScoordinator.hpp"
#include "network/parameters.hpp"
#include "systems/prnsServer.hpp"

#include "logAPI.hpp"
#include "prng.hpp"

void AS::PRNserver::drawPRNs(bool willAgentsMakeDecisions, int numberLAs, int numberGAs) {
	
	int prnsToDraw = PRNS_PER_LA*numberLAs + PRNS_PER_GA*numberGAs;
	prnsToDraw *= willAgentsMakeDecisions;

	if(prnsToDraw > MAX_PRNS) {
		LOG_ERROR("Trying to draw more than the maximum number of PRNs: will reduce to maximum");
		prnsToDraw = MAX_PRNS;
	}

	for (int i = 0; i < prnsToDraw; i++) {
		PRNs[i] = ((float)AZ::drawFastUint32())/(UINT32_MAX);
	}

	drawn = prnsToDraw;
	nextToUse = 0;
}