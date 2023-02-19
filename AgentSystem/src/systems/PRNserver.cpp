/*
Defines a class to draw and manage hold the pseudo random numbers (PRNs) needed for each step.
Holds floats in the interval [0-1]
Has methods to:
- manage seeds;
- populate (using a generator from Aux0);
- get the next number;
- dump info, test and benchmark itself;
- TODO: get the next number in different formats/intervals;
*/

#include "systems/AScoordinator.hpp"
#include "network/parameters.hpp"
#include "systems/PRNserver.hpp"

#include "logAPI.hpp"
#include "prng.hpp"

#include "network/fileFormat.hpp"
#include "timeHelpers.hpp"
#include "fileHelpers.h"

static const char* defaultPRNGdumpFilename = "dumpPRNG.txt";

//TODO: REFACTOR: change signature to: (int chopIndex, int totalChops, int PRNsToDrawTotal)
//- Calculate here PRNsToDrawThisChop, given these and the private member "drawn";
//- Time the actual drawing;
//- Return a structure with relevant info for testing and benchmarking;
//-- Includes timing, which mainLoop can also accumulate into a "totalMicrosDrawingPRNs";
//(move  explanation to .hpp)
void AS::PRNserver::drawPRNs(int chopIndex, int totalChops, int PRNsToDrawTotal) {
	
	if (chopIndex == 0) {
		drawn = 0;
		nextToUse = 0;
	}
	
	int chopsRemaining = (totalChops - 1) - chopIndex;
	int PRNsLeftToDraw = PRNsToDrawTotal - drawn;

	int prnsToDrawPerRegularChop = PRNsLeftToDraw / chopsRemaining;
	int remainderPRNs = chopsRemaining % PRNsLeftToDraw;

	int PRNsToDrawThisChop = prnsToDrawPerRegularChop;
	if (chopIndex == (totalChops - 1)) {
		PRNsToDrawThisChop += remainderPRNs;
	}
	
	int prnsToDrawPerRegularChop = PRNsToDrawTotal / TOTAL_PRN_CHOPS;
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
	
	
	std::chrono::nanoseconds start = AZ::nowNanos();
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
	std::chrono::nanoseconds end = AZ::nowNanos();
	
	drawn += PRNsToDrawThisChop;

	std::chrono::nanoseconds deltaT = end - start;	
}

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
	if (resultAux == EOF) {result *= 0;}

	resultAux =  fclose(fp);
	if (resultAux == EOF) {result *= 0;}

	if (!result) {
		LOG_ERROR("Failed Writting PRNG Dump to File!");
	}
	return result;
}

bool AS::PRNserver::testAndBenchChoppedDrawing(int64_t howManyToDraw, int totalChops,
			                         bool printResults = true, bool dumpPRNs = false) {

	//	printf("\n\n#s: %d, deltaT: %llu, nanosPerPRN: %f\n",totalDraws,deltaT,nanosPerPRN);

	int PRNsToDrawTotal, chopIndex, itersDraw4, chopIndexIsSmallerThan, itersDraw1; 
	int PRNsToDrawThisChop, totalPrnsToDraw;

	int prnsToDrawPerRegularChop = PRNsToDrawTotal / TOTAL_PRN_CHOPS;
	int draw4startIndex = (prnsToDrawPerRegularChop/DRAW_WIDTH) * chopIndex;
	int draw4IndexOffset = (prnsToDrawPerRegularChop*chopIndex) - draw4startIndex*DRAW_WIDTH;
	int draw4indexIsSmallerThan = draw4startIndex + itersDraw4;
	int draw1startIndex = chopIndexIsSmallerThan - itersDraw1;
	int chopIndexIsSmallerThan = (prnsToDrawPerRegularChop*chopIndex) + PRNsToDrawThisChop;

	printf("\n\nchop: %d, i4: %d <= %d + i1: %d < %d (prns: %d, perChop: %d)\n", chopIndex, 
		                                        draw4startIndex*DRAW_WIDTH+draw4IndexOffset, 
		                                   4*(draw4indexIsSmallerThan-1)+3+draw4IndexOffset, 
		                                            draw1startIndex, chopIndexIsSmallerThan,
		                                          totalPrnsToDraw, prnsToDrawPerRegularChop);
}