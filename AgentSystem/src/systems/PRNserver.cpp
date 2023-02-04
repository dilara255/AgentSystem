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

#include "timeHelpers.hpp"

bool isLastChopForPRNG(int chop) {
	
	//if decisions are made every 4 steps, we calculate the PRNGs in the first 3 steps.
	//That 3 is "TOTAL_PRNG_CHOPS". Usually, we use n-1 steps for prng and 1 for decisions
	//exception: if we decide every step, then prng + decision happens every step
	//this exception is already handled by the "TOTAL_PRNG_CHOPS" macro

	return chop == (TOTAL_PRNG_CHOPS - 1);
}

//TODO-CRITICAL: Implement chop test at least
//TODO-CRITICAL: clean up / extract stuff / comment
void AS::PRNserver::drawPRNs(int numberLAs, int numberGAs, int chopIndex) {
	
	int totalPrnsToDraw = PRNS_PER_LA*numberLAs + PRNS_PER_GA*numberGAs + MAX_ACT_PRNS;
	int prnsToDrawPerRegularChop = totalPrnsToDraw / TOTAL_PRNG_CHOPS;
	int remainderPRNs = totalPrnsToDraw % TOTAL_PRNG_CHOPS;

	int prnsToDrawThisChop = prnsToDrawPerRegularChop;
	if (isLastChopForPRNG(chopIndex)) {
		prnsToDrawThisChop += remainderPRNs;
	}
	else if (chopIndex == 0) {
		drawn = 0;
		nextToUse = 0;
	}
	else if (chopIndex == TOTAL_PRNG_CHOPS) {
		//we've generated all PRNs, this step is dedicated to decision-making
		return;
	}

	int draw4startIndex = (prnsToDrawPerRegularChop/DRAW_WIDTH) * chopIndex;
	int draw4IndexOffset = (prnsToDrawPerRegularChop*chopIndex) - draw4startIndex*DRAW_WIDTH;
	int itersDraw4 = prnsToDrawThisChop / DRAW_WIDTH;
	int draw4indexIsSmallerThan = draw4startIndex + itersDraw4;
	
	int chopIndexIsSmallerThan = (prnsToDrawPerRegularChop*chopIndex) + prnsToDrawThisChop;
	int itersDraw1 = prnsToDrawThisChop % DRAW_WIDTH;
	int draw1startIndex = chopIndexIsSmallerThan - itersDraw1;

	if(chopIndexIsSmallerThan >= MAX_PRNS) {
		LOG_ERROR("Trying to draw more than the maximum number of PRNs: will abort generation");
		return;
	}
	           
	float invUint32max = 1.0/UINT32_MAX;	
	uint32_t dest[DRAW_WIDTH];
		
	for (int i = draw4startIndex; i < draw4indexIsSmallerThan; i++) {
		
		AZ::draw4spcg32s(&seeds[0], &seeds[1], &seeds[2], &seeds[3], 
							 &dest[0], &dest[1], &dest[2], &dest[3]);

		PRNs[4*i+draw4IndexOffset] = dest[0]*invUint32max;
		PRNs[4*i+1+draw4IndexOffset] = dest[1]*invUint32max;
		PRNs[4*i+2+draw4IndexOffset] = dest[2]*invUint32max;
		PRNs[4*i+3+draw4IndexOffset] = dest[3]*invUint32max;
	}

	for (int i = draw1startIndex; i < chopIndexIsSmallerThan; i++) {
		PRNs[i] = AZ::spcg32(&seeds[0])*invUint32max;
	}

	drawn += prnsToDrawThisChop;

	
	//TODO-CRITICAL: Make into a test:
	/*
	printf("\n\nchop: % d, i4: %d <= %d + i1: %d < %d (prns: %d, perChop: %d)\n", chopIndex, 
		                                        draw4startIndex*DRAW_WIDTH+draw4IndexOffset, 
		                                   4*(draw4indexIsSmallerThan-1)+3+draw4IndexOffset, 
		                                            draw1startIndex, chopIndexIsSmallerThan,
		                                          totalPrnsToDraw, prnsToDrawPerRegularChop);
    */
}