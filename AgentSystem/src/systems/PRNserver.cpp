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
static const char* defaultNormalPRNGFilename = "normalPRNG.txt";
static const char* defaultRedPRNGFilename = "redPRNG.txt";

void AS::PRNserver::clearServer() {
	zeroAccumulatedDrawTime();
	clearDrawInfoErrors();
	resetSeedsToDefaults();
	drawn = 0;
	nextToUse = 0;
}

float AS::PRNserver::normalFromUniformsMean0(float stdDev) {

	constexpr int whitePRNs = WHITE_PRNS_FOR_NORMAL_APPROX;

	float normal = 0;
	//First we add our white PRNs
	for (int i = 0; i < whitePRNs; i++) {
		normal += getNext();
	}

	//Then we discount half their widht, to get the mean to zero:
	constexpr float halfWidht = whitePRNs/2.0f;
	normal -= halfWidht;

	//The stdDev will now be sqrt(whitePRNs/12): multiplying by this brings it to one:
	static const float inverseSqrtPRNsOver12 = std::sqrt(12.0f/whitePRNs);
	normal *= inverseSqrtPRNsOver12;

	//We should now have be effectively drawing from a mean zero stdDev one distribution
	//Just to check, the possible extrema are aprox sqrt(3 * whitePRNs), or:
	assert(normal >= -(halfWidht * inverseSqrtPRNsOver12) );
	assert(normal <=  (halfWidht * inverseSqrtPRNsOver12) );

	//Finally we multiply by the desired stdDev to stretch the distribution:
	return normal * stdDev;
}

float AS::PRNserver::getRedNext(float previous, float effectiveStdDev) {

	float offset = normalFromUniformsMean0(effectiveStdDev);

	float next = previous + offset;

	if (next > 1) {
		//if next is above 2, this will not be "right", but statistically it shouldn't matter:
		next = 1 - (next - std::floor(next));
	}
	else if (next < 0) {
		next = std::ceil(next) - next;
	}

	return next;
}

float AS::PRNserver::getRedNextGivenTime(float previous, float period, float stdDevAtRefPeriod, 
																		       float refPeriod) {

	float effectiveStdDev = stdDevAtRefPeriod * std::sqrt(period/refPeriod);
	return getRedNext(previous, effectiveStdDev);
}

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
	           
	//The generator gives us numbers in uint32_t, but we want [0,1] floats, so we'll use:
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

#define PRNSRVR_CHECKED_CONDITIONS_ON_TEST 4
bool AS::PRNserver::testAndBenchChoppedDrawing(int howManyToDraw, int totalChops,
			                                    bool printResults, bool dumpPRNs) {

	LOG_DEBUG("Testing and benchmarking chopped PRN gerneration...");
	GETCHAR_PAUSE;

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

	clearServer();

	return result;
}

bool AS::PRNserver::testNormalDrawing(bool printResults, bool dumpPRNs) {

	LOG_DEBUG("Testing and benchmarking drawing PRNs from approximate normal distribution...");
	GETCHAR_PAUSE;

	int howManyToDraw = std::min(1000, MAX_PRNS);
	if (howManyToDraw < 100) {
		LOG_ERROR("MAX_PRNS is too low to make statistical analisys, will abort test");
		return false;
	}

	drawPRNs(0, 1, howManyToDraw * WHITE_PRNS_FOR_NORMAL_APPROX);

	std::vector<float> prns;
	prns.reserve(howManyToDraw);

	auto start = std::chrono::steady_clock::now();
	for (int i = 0; i < howManyToDraw; i++) {
		prns.push_back(normalFromUniformsMean0(1.0f));
	}
	auto end = std::chrono::steady_clock::now();

	auto periodNS = 
		(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)).count();

	double mean = 0;
	double meanOfSquares = 0;
	for (int i = 0; i < howManyToDraw; i++) {
		double prnByTotal = prns.at(i)/howManyToDraw;
		mean += prnByTotal;
		meanOfSquares += (prns.at(i) * prnByTotal);
	}

	assert(meanOfSquares >= 0);
	assert(meanOfSquares >= mean*mean);

	double stdDev = std::sqrt(meanOfSquares - mean*mean);

	const double expectedMean = 0;
	const double expectedStdDEv = 1.0;
	double absMeanDifference = std::abs(expectedMean - mean);
	double absStdDevDifference = std::abs(expectedStdDEv - stdDev);

	double maxAllowedDifference = 10.0 / howManyToDraw;

	bool result = (absMeanDifference < maxAllowedDifference) 
		           && (absStdDevDifference < maxAllowedDifference);

	if (printResults) {
		if (result) {
			LOG_INFO("Values drawn have the expected (approximate) statistical properties!");
		}
		else {
			LOG_ERROR("Values drawn diverged too much from the expected statitstical properties!");
		}
		printf("Drawn: %d, at %f ns per number. Mean %f, StdDev %f (expected: %f and %f)\n",
			howManyToDraw, (double)periodNS/howManyToDraw, mean, stdDev, expectedMean, expectedStdDEv);
	}
	if (dumpPRNs) {

		FILE* fp = AZ::acquireFilePointerToSave(defaultNormalPRNGFilename, false, 
			                                                     defaultFilePath);

		fprintf(fp, "Drawn: %d, at %f ns per number. Mean %f, StdDev %f (expected: %f and %f)\n\n",
			howManyToDraw, (double)periodNS/howManyToDraw, mean, stdDev, expectedMean, expectedStdDEv);

		for (int i = 0; i < howManyToDraw; i++) {
			fprintf(fp, "%f\n", prns.at(i));
		}

		fclose(fp);
	}

	clearServer();

	return result;
}

bool AS::PRNserver::testRedDrawing(bool printResults, bool dumpPRNs) {

	LOG_DEBUG("Testing and benchmarking drawing red-ish PRNs...");
	LOG_WARN("THE ACTUAL TEST IS NOT YET IMPLEMENTED (will print/save data and return true)");
	GETCHAR_PAUSE;

	//TODO: implement actual tests

	int howManyToDraw = std::min(1000, MAX_PRNS);
	if (howManyToDraw < 100) {
		LOG_ERROR("MAX_PRNS is too low to make statistical analisys, will abort test");
		return false;
	}

	drawPRNs(0, 1, howManyToDraw * WHITE_PRNS_FOR_NORMAL_APPROX);

	std::vector<float> prns;
	prns.reserve(howManyToDraw);

	//We we'll test for 10 ref periods:
	float testPeriodPerDraw = 10.0f / howManyToDraw;

	float red = 0.5;

	auto start = std::chrono::steady_clock::now();
	for (int i = 0; i < howManyToDraw; i++) {
		red = getRedNextGivenTime(red, testPeriodPerDraw, 1.0f, 1.0f);
		prns.push_back(red);
	}
	auto end = std::chrono::steady_clock::now();

	auto periodNS = 
		(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)).count();

	double mean = 0;
	double meanOfSquares = 0;
	for (int i = 0; i < howManyToDraw; i++) {
		double prnByTotal = prns.at(i)/howManyToDraw;
		mean += prnByTotal;
		meanOfSquares += (prns.at(i) * prnByTotal);
	}

	assert(meanOfSquares >= 0);
	assert(meanOfSquares >= mean*mean);

	double stdDev = std::sqrt(meanOfSquares - mean*mean);

	if (printResults) {
		LOG_TRACE("Test not implemented: will return true either way. Check results summary:");

		printf("Drawn: %d, at %f ns per number. Mean %f, StdDev %f\n",
			    howManyToDraw, (double)periodNS/howManyToDraw, mean, stdDev);
	}
	if (dumpPRNs) {

		FILE* fp = AZ::acquireFilePointerToSave(defaultRedPRNGFilename, false, 
			                                                  defaultFilePath);

		fprintf(fp, "Drawn: %d, at %f ns per number. Mean %f, StdDev %f\n\n",
			howManyToDraw, (double)periodNS/howManyToDraw, mean, stdDev);

		for (int i = 0; i < howManyToDraw; i++) {
			fprintf(fp, "%f\n", prns.at(i));
		}

		fclose(fp);
	}

	clearServer();

	return true;
}