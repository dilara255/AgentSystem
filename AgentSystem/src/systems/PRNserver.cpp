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

#include "network/fileFormat.hpp"
#include "timeHelpers.hpp"
#include "fileHelpers.h"

static const char* defaultPRNGdumpFilename = "dumpPRNG.txt";

bool AS::PRNserver::dumpData(std::string filename) {

	LOG_TRACE("Dumping PRNG info");

	std::string name;

	if (filename == "") {
        name = std::string(defaultPRNGdumpFilename);
    }
    else {
        name = filename;
    }

	FILE* fp = AZ::acquireFilePointerToSave(name, false, defaultFilePath);

	if (fp == NULL) {
        LOG_ERROR("Couldn't create the file (check if folders exist), aborting creation...");
        return false;
    }

	LOG_TRACE("File Acquired, will write");

	int result = 1;
	int resultAux = 0;

	resultAux = fprintf(fp, "PRNG DUMP:\nGENERATED: %d:\n\n", drawn);
    result *= (resultAux > 0);

	resultAux = fprintf(fp, "Current Seeds: %llu, %llu, %llu, %llu\n\nPRNs:\n\n",
				                 seeds[0], seeds[1], seeds[2], seeds[3]);
    result *= (resultAux > 0);

	for (int i = 0; i < drawn; i++) {
		 fprintf(fp, "%f\n", PRNs[i]);
		 result *= (resultAux > 0);
	}

    resultAux = fputs("\n**** END OF PRNG DUMP ****\n", fp);
	if (resultAux == EOF) {result *= 0;} //fputs returns EOF on error

	resultAux =  fclose(fp);
	if (resultAux == EOF) {result *= 0;}

	if (!result) {
		LOG_ERROR("Failed Writting PRNG Dump to File!");
	}
	return result;
}

//TODO-CRITICAL: Implement chop test at least
//TODO-CRITICAL: clean up / extract stuff / comment
void AS::PRNserver::drawPRNs(int chopIndex, int PRNsToDrawThisChop, int PRNsToDrawTotal) {
		
	if (chopIndex == 0) {
		drawn = 0;
		nextToUse = 0;
	}
	
	int prnsToDrawPerRegularChop = PRNsToDrawTotal / AS_TOTAL_CHOPS;
	int draw4startIndex = (prnsToDrawPerRegularChop/DRAW_WIDTH) * chopIndex;
	int draw4IndexOffset = (prnsToDrawPerRegularChop*chopIndex) - draw4startIndex*DRAW_WIDTH;
	int itersDraw4 = PRNsToDrawThisChop / DRAW_WIDTH;
	int draw4indexIsSmallerThan = draw4startIndex + itersDraw4;
	
	int chopIndexIsSmallerThan = (prnsToDrawPerRegularChop*chopIndex) + PRNsToDrawThisChop;
	int itersDraw1 = PRNsToDrawThisChop % DRAW_WIDTH;
	int draw1startIndex = chopIndexIsSmallerThan - itersDraw1;

	if(chopIndexIsSmallerThan >= MAX_PRNS) {
		LOG_ERROR("Trying to draw more than the maximum number of PRNs: will abort generation");
		return;
	}
	           
	float invUint32max = 1.0/UINT32_MAX;	
	uint32_t dest[DRAW_WIDTH];
	
	/*
	//TODO-CRITICAL: MOVE TO TEST
	int testRepetitions = 6666;
	std::chrono::nanoseconds start = AZ::nowNanos();

	for(int i = 0; i < testRepetitions; i++){*/
	for (int i = draw4startIndex; i < draw4indexIsSmallerThan; i++) {
		
		AZ::draw4spcg32s(&seeds[0], &seeds[1], &seeds[2], &seeds[3], 
								&dest[0], &dest[1], &dest[2], &dest[3]);

		PRNs[4*i+draw4IndexOffset] = dest[0]*invUint32max;
		PRNs[4*i+1+draw4IndexOffset] = dest[1]*invUint32max;
		PRNs[4*i+2+draw4IndexOffset] = dest[2]*invUint32max;
		PRNs[4*i+3+draw4IndexOffset] = dest[3]*invUint32max;
	}

	for (int i = draw1startIndex; i < chopIndexIsSmallerThan; i++) {
		PRNs[i] = AZ::draw1spcg32(&seeds[0])*invUint32max;
	}
	/* }
	//TODO-CRITICAL: MOVE TO TEST
	std::chrono::nanoseconds end = AZ::nowNanos();
	std::chrono::nanoseconds deltaT = end - start;
	int totalDraws = testRepetitions*prnsToDrawThisChop;
	double nanosPerPRN = deltaT.count()/((double)totalDraws);

	printf("\n\n#s: %d, deltaT: %llu, nanosPerPRN: %f\n",totalDraws,deltaT,nanosPerPRN);
	*/
	
	drawn += PRNsToDrawThisChop;

	
	
	/*
	//TODO-CRITICAL: MOVE TO TEST
	printf("\n\nchop: % d, i4: %d <= %d + i1: %d < %d (prns: %d, perChop: %d)\n", chopIndex, 
		                                        draw4startIndex*DRAW_WIDTH+draw4IndexOffset, 
		                                   4*(draw4indexIsSmallerThan-1)+3+draw4IndexOffset, 
		                                            draw1startIndex, chopIndexIsSmallerThan,
		                                          totalPrnsToDraw, prnsToDrawPerRegularChop);
    */
}