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
//(move explanation to .hpp)
AS::drawInfo_t AS::PRNserver::drawPRNs(int chopIndex, int totalChops, int PRNsToDrawTotal) {
	
	if (chopIndex == 0) {
		drawn = 0;
		nextToUse = 0;
	}

	int chopsRemaining = totalChops - chopIndex;
	if (chopsRemaining <= 0) {
		LOG_WARN("Tried to draw remaining PRNs in 0 or less chops. Will change to 1. Check if not intended.");
		chopsRemaining = 1; //we're already here after all
	}
	int PRNsLeftToDraw = PRNsToDrawTotal - drawn;

	m_drawInfo.prnsToDrawPerRegularChop = PRNsLeftToDraw / chopsRemaining;
	int PRNsToDrawThisChop = m_drawInfo.prnsToDrawPerRegularChop;
	if (chopsRemaining == 1) {
		PRNsToDrawThisChop = PRNsLeftToDraw;
	}
	
	//We will draw as many PRNs as possible four at a time, and the rest one at a time
	//(because the four-at-a-time generator lets the compiler make better use of SIMD)
	//So we need to calculate the starting and ending indexes of each, as well as any offsets:

	m_drawInfo.draw4startIndex = drawn/DRAW_WIDTH;
	m_drawInfo.draw4IndexOffset = drawn % DRAW_WIDTH;
	int itersDraw4 = PRNsToDrawThisChop / DRAW_WIDTH;
	m_drawInfo.draw4indexIsSmallerThan = m_drawInfo.draw4startIndex + itersDraw4;
	
	int itersDraw1 = PRNsToDrawThisChop % DRAW_WIDTH;
	m_drawInfo.chopIndexIsSmallerThan = drawn + PRNsToDrawThisChop;
	m_drawInfo.draw1startIndex = m_drawInfo.chopIndexIsSmallerThan - itersDraw1;

	if(m_drawInfo.chopIndexIsSmallerThan > MAX_PRNS) {
		LOG_ERROR("Trying to draw more than the maximum number of PRNs: will abort generation");
		m_drawInfo.error = true;
		return m_drawInfo;
	}
	           
	//The generator gives us numbers in uint32_t, but want [0,1] floats, so we'll use:
	uint32_t dest[DRAW_WIDTH];
	float invUint32max = 1.0f/UINT32_MAX;
		
	std::chrono::nanoseconds drawStart = AZ::nowNanos();
	//The actual drawing:
	for (int i = m_drawInfo.draw4startIndex; i < m_drawInfo.draw4indexIsSmallerThan; i++) {
		
		AZ::draw4spcg32s(&seeds[0], &seeds[1], &seeds[2], &seeds[3], 
								&dest[0], &dest[1], &dest[2], &dest[3]);

		PRNs[4*i + m_drawInfo.draw4IndexOffset] = dest[0]*invUint32max;
		PRNs[4*i + 1 + m_drawInfo.draw4IndexOffset] = dest[1]*invUint32max;
		PRNs[4*i + 2 + m_drawInfo.draw4IndexOffset] = dest[2]*invUint32max;
		PRNs[4*i + 3 + m_drawInfo.draw4IndexOffset] = dest[3]*invUint32max;
	}

	for (int i = m_drawInfo.draw1startIndex; i < m_drawInfo.chopIndexIsSmallerThan; i++) {
		PRNs[i] = AZ::draw1spcg32(&seeds[0])*invUint32max;
	}
	std::chrono::nanoseconds drawEnd = AZ::nowNanos();
	
	drawn += PRNsToDrawThisChop;

	m_drawInfo.accumulatedDrawTime += (drawEnd - drawStart);
	return m_drawInfo;
}

#define PRNSRVR_CHECKED_CONDITIONS_ON_TEST 4
bool AS::PRNserver::testAndBenchChoppedDrawing(int howManyToDraw, int totalChops,
			                                    bool printResults, bool dumpPRNs) {

	LOG_TRACE("Testing and benchmarking chopped PRN gerneration...");

	if(howManyToDraw > MAX_PRNS) {
		LOG_ERROR("Trying to draw more than the maximum number of PRNs: will abort test");
		return false;
	}
	if(totalChops < 1) {
		LOG_ERROR("Trying to draw PRNs in less than one chop (zero or negative): will abort test");
		return false;
	}

	int errorsFound[PRNSRVR_CHECKED_CONDITIONS_ON_TEST];
	for (int i = 0; i < PRNSRVR_CHECKED_CONDITIONS_ON_TEST; i++) {
		errorsFound[i] = 0;		
	}
	zeroAccumulatedDrawTime();
	clearDrawInfoErrors();
	drawInfo_t drawInfo;
	drawInfo_t lastDrawInfo;

	for (int i = 0; i < totalChops; i++) {

		if (i == 1) {
			totalChops--; //if initial totalChops == 1, will test exceeding totalChops
		}
		else if (i == 2) {
			totalChops++; //to test changing number of chops during run
		}
		else if (i == 3) {
			totalChops--; //to get back to target number of chops
		}

		drawInfo = drawPRNs(i, totalChops, howManyToDraw);

		if (drawInfo.error) {
			LOG_ERROR("Failed to draw PRNs, will abort test");
			return false;
		}

		int draw4FirstElement = 
			(drawInfo.draw4startIndex*DRAW_WIDTH) + drawInfo.draw4IndexOffset;
		int draw4LastElement = 
			DRAW_WIDTH*(drawInfo.draw4indexIsSmallerThan - 1) + 3 + drawInfo.draw4IndexOffset;

		if(printResults){

			printf("\nchop: %d/%d, i4: %d <= %d + i1: %d < %d (taget: %d, per chop: ~%d)", 
					i+1, totalChops, draw4FirstElement, draw4LastElement, 
					drawInfo.draw1startIndex, drawInfo.chopIndexIsSmallerThan, howManyToDraw, 
					drawInfo.prnsToDrawPerRegularChop);
		}

		if (i == 0) {
			errorsFound[0] += (drawInfo.draw4startIndex != 0);
		}

		errorsFound[1] += (draw4FirstElement != lastDrawInfo.chopIndexIsSmallerThan);

		if(i > 0){
			errorsFound[2] += (drawInfo.draw1startIndex != (draw4LastElement + 1 ));
		}

		lastDrawInfo = drawInfo;

		//if test totalChops was 0, this will cause extra chop with 0 remaining to draw: 
		if (i == 0) {
			totalChops++; 
		}
	}

	errorsFound[3] += (drawn != howManyToDraw);

	if(printResults){
		double totalSeconds = (double)drawInfo.accumulatedDrawTime.count()/NANOS_IN_A_SECOND;
		double nanosPerPRN = (double)drawInfo.accumulatedDrawTime.count()/howManyToDraw;
		printf("\n#s: %d, time: %fs, nanosPerPRN: %f, errors: %d/%d/%d/%d\n\n", 
				howManyToDraw, totalSeconds, nanosPerPRN, errorsFound[0],
			    errorsFound[1], errorsFound[2], errorsFound[3]);
	}

	if(dumpPRNs){
		dumpData();
		LOG_TRACE("PRNs dumped to file");
	}	

	bool result = true;
	for (int i = 0; i < PRNSRVR_CHECKED_CONDITIONS_ON_TEST; i++) {
		result &= (errorsFound[i] == 0);
	}
	return result;
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

	resultAux = fprintf(fp, "PRNG DUMP:\nGENERATED this set: %d (total accumulated time: %lld nanos):\n\n", 
			drawn, m_drawInfo.accumulatedDrawTime.count());
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