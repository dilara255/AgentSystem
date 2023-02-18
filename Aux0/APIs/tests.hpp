#pragma once

#include "prng.hpp"
#include "timeHelpers.hpp"

namespace AZ{
	//Draws howManyToDraw prns into a vector. Tests:
    //Avarage, amount of zeroes, periods 2, 4, 8 and 16.
    //Returns true if all tests pass. Defaults to 100M draws, minimum 16.
	//If log == true, logs to output (eg, console) the time per draw
	static bool testDraw4spcg32s(int64_t howManyTuplesToDraw = 250000, bool log = true) {
		const int mininumTotalDrawn = 16;

		if (howManyTuplesToDraw < (mininumTotalDrawn / DRAW_WIDTH)) {
			if (log) {
				puts("Too few tuples for test to run properly, aborting with false");
			}
			return false;
		}

		//Draws howManyToDraw prns into a vector, timing it:
		std::vector<uint32_t> drawn;
		int64_t totalDrawn = DRAW_WIDTH * howManyTuplesToDraw;
		drawn.reserve(totalDrawn);
		for (int i = 0; i < totalDrawn; i++) {
			drawn.push_back(0);
		}

		uint64_t seeds[DRAW_WIDTH];
		seeds[0] = DEFAULT_PRNG_SEED0;
		seeds[1] = DEFAULT_PRNG_SEED1;
		seeds[2] = DEFAULT_PRNG_SEED2;
		seeds[3] = DEFAULT_PRNG_SEED3;

		auto startTime = std::chrono::steady_clock::now();	
		
		for (int i = 0; i < howManyTuplesToDraw; i++) {
			
			draw4spcg32s(&seeds[0],&seeds[1],&seeds[2],&seeds[3],
							&drawn[i*DRAW_WIDTH], &drawn[(i*DRAW_WIDTH) + 1],
							&drawn[(i*DRAW_WIDTH) + 2], &drawn[(i*DRAW_WIDTH) + 3]);
		}

		auto endTime = std::chrono::steady_clock::now();
		auto timeElapsed = endTime - startTime;
		double nanosPerDraw = (double)timeElapsed.count()/totalDrawn;

		//Tests amount of zeroes:

		int zeroes = 0;
		for (int i = 0; i < totalDrawn; i++) {	
			zeroes += (drawn[i] == 0);
		}

		double expectedZeroes = (double)totalDrawn/UINT32_MAX;
		double proportionalDifference = fabs((zeroes - expectedZeroes))/expectedZeroes;
		int64_t graceZeroes = 1;
		double maxProportionalDifference = graceZeroes/expectedZeroes;

		bool result = (proportionalDifference <= maxProportionalDifference);
		
		//Tests difference in periods 2, 4, 8 and 16.
		//For values in (0,1), the expected difference is given by:
		//E[|X - Y|] = P(Y > X)E[Y - X | Y > X] + P(Y <= X)E[X - Y | X > Y] ,
		//which turns out to be 1/3. The expected variance is 1/18.
		double expectedAverageAbsoluteDifference = 1.0/3;

		//Will take to (0,1) first
		std::vector<double> drawnNormalized;
		drawnNormalized.reserve(totalDrawn);

		double inverseOfUint32max = 1.0/UINT32_MAX;
		for (int i = 0; i < totalDrawn; i++) {	
			drawnNormalized.push_back(drawn[i]*inverseOfUint32max);
		}

		double inverseOfTotalNumbersDrawn = 1.0/(totalDrawn - mininumTotalDrawn);
		double avarege = 0;
		double averageAbsoluteDifferencePeriod2 = 0;
		double averageAbsoluteDifferencePeriod4 = 0;
		double averageAbsoluteDifferencePeriod8 = 0;
		double averageAbsoluteDifferencePeriod16 = 0;
		for (int i = mininumTotalDrawn; i < totalDrawn; i++) {	
			avarege += drawnNormalized[i]*inverseOfTotalNumbersDrawn;
			averageAbsoluteDifferencePeriod2 +=
				fabs(drawnNormalized[i] - drawnNormalized[i-2])*inverseOfTotalNumbersDrawn;
			averageAbsoluteDifferencePeriod4 +=
				fabs(drawnNormalized[i] - drawnNormalized[i-4])*inverseOfTotalNumbersDrawn;
			averageAbsoluteDifferencePeriod8 +=
				fabs(drawnNormalized[i] - drawnNormalized[i-8])*inverseOfTotalNumbersDrawn;
			averageAbsoluteDifferencePeriod16 +=
				fabs(drawnNormalized[i] - drawnNormalized[i-16])*inverseOfTotalNumbersDrawn;
		}

		double margin = sqrt(inverseOfTotalNumbersDrawn);
		double differenceFromExpectedPeriod2 = 
			fabs(averageAbsoluteDifferencePeriod2 - expectedAverageAbsoluteDifference);
		double differenceFromExpectedPeriod4 = 
			fabs(averageAbsoluteDifferencePeriod4 - expectedAverageAbsoluteDifference);
		double differenceFromExpectedPeriod8 = 
			fabs(averageAbsoluteDifferencePeriod8 - expectedAverageAbsoluteDifference);
		double differenceFromExpectedPeriod16 = 
			fabs(averageAbsoluteDifferencePeriod16 - expectedAverageAbsoluteDifference);

		result &= (differenceFromExpectedPeriod2 <= margin);
		result &= (differenceFromExpectedPeriod4 <= margin);
		result &= (differenceFromExpectedPeriod8 <= margin);
		result &= (differenceFromExpectedPeriod16 <= margin);

		//Log and return result:

		if (log) {
			printf("\nPRNG test (draw4spcg32s): %lld drawn (normalized avg: %f)\n",
				                                               totalDrawn, avarege);
			printf("\tTiming: %f nanos per prn\n", nanosPerDraw);
			printf("\tZeroes: %d (expected: %f)\n", zeroes, expectedZeroes);
			printf("\tAvg. differences expected: %f (margin used: %f). Found:\n",
								expectedAverageAbsoluteDifference, margin);
			printf("\t\tPeriod 2: %f; Period 4: %f, Period 8: %f, Period 16: %f\n",
				averageAbsoluteDifferencePeriod2, averageAbsoluteDifferencePeriod4,
			   averageAbsoluteDifferencePeriod8, averageAbsoluteDifferencePeriod16);
		}

		return result;
	}

    //Draws howManyToDraw prns into a vector. Tests:
    //Avarage, amount of zeroes, periods 2, 4, 8 and 16
    //Returns true if all tests pass. Defaults to 25M draws
	//If log == true, logs to output (eg, console) the time per draw
	static bool testDraw1spcg32(int64_t howManyToDraw = 1000000, bool log = true) {
			const int mininumTotalDrawn = 16;

		if (howManyToDraw < mininumTotalDrawn) {
			if (log) {
				puts("Too few tuples for test to run properly, aborting with false");
			}
			return false;
		}

		//Draws howManyToDraw prns into a vector, timing it:
		std::vector<uint32_t> drawn;
		int64_t totalDrawn = howManyToDraw;
		drawn.reserve(totalDrawn);
		for (int i = 0; i < totalDrawn; i++) {	
			drawn.push_back(0);
		}

		uint64_t seed = DEFAULT_PRNG_SEED0;

		auto startTime = std::chrono::steady_clock::now();

		for (int i = 0; i < totalDrawn; i++) {	
			drawn[i] = draw1spcg32(&seed);
		}

		auto endTime = std::chrono::steady_clock::now();
		auto timeElapsed = endTime - startTime;
		double nanosPerDraw = (double)timeElapsed.count()/totalDrawn;

		//Tests amount of zeroes:
		int zeroes = 0;
		for (int i = 0; i < totalDrawn; i++) {	
			zeroes += (drawn[i] == 0);
		}

		double expectedZeroes = (double)totalDrawn/UINT32_MAX;
		double proportionalDifference = fabs((zeroes - expectedZeroes))/expectedZeroes;
		int64_t graceZeroes = 1;
		double maxProportionalDifference = graceZeroes/expectedZeroes;

		bool result = (proportionalDifference <= maxProportionalDifference);
		
		//Tests difference in periods 2, 4, 8 and 16.
		//For values in (0,1), the expected difference is given by:
		//E[|X - Y|] = P(Y > X)E[Y - X | Y > X] + P(Y <= X)E[X - Y | X > Y] ,
		//which turns out to be 1/3. The expected variance is 1/18.
		double expectedAverageAbsoluteDifference = 1.0/3;

		//Will take to (0,1) first
		std::vector<double> drawnNormalized;
		drawnNormalized.reserve(totalDrawn);

		double inverseOfUint32max = 1.0/UINT32_MAX;
		for (int i = 0; i < totalDrawn; i++) {	
			drawnNormalized.push_back(drawn[i]*inverseOfUint32max);
		}

		double inverseOfTotalNumbersTested = 1.0/(totalDrawn - mininumTotalDrawn);
		double avarege = 0;
		double averageAbsoluteDifferencePeriod2 = 0;
		double averageAbsoluteDifferencePeriod4 = 0;
		double averageAbsoluteDifferencePeriod8 = 0;
		double averageAbsoluteDifferencePeriod16 = 0;

		for (int i = mininumTotalDrawn; i < totalDrawn; i++) {	
			avarege += drawnNormalized[i]*inverseOfTotalNumbersTested;
			averageAbsoluteDifferencePeriod2 +=
				fabs(drawnNormalized[i] - drawnNormalized[i-2])*inverseOfTotalNumbersTested;
			averageAbsoluteDifferencePeriod4 +=
				fabs(drawnNormalized[i] - drawnNormalized[i-4])*inverseOfTotalNumbersTested;
			averageAbsoluteDifferencePeriod8 +=
				fabs(drawnNormalized[i] - drawnNormalized[i-8])*inverseOfTotalNumbersTested;
			averageAbsoluteDifferencePeriod16 +=
				fabs(drawnNormalized[i] - drawnNormalized[i-16])*inverseOfTotalNumbersTested;
		}

		double margin = sqrt(inverseOfTotalNumbersTested);
		double differenceFromExpectedPeriod2 = 
			fabs(averageAbsoluteDifferencePeriod2 - expectedAverageAbsoluteDifference);
		double differenceFromExpectedPeriod4 = 
			fabs(averageAbsoluteDifferencePeriod4 - expectedAverageAbsoluteDifference);
		double differenceFromExpectedPeriod8 = 
			fabs(averageAbsoluteDifferencePeriod8 - expectedAverageAbsoluteDifference);
		double differenceFromExpectedPeriod16 = 
			fabs(averageAbsoluteDifferencePeriod16 - expectedAverageAbsoluteDifference);

		result &= (differenceFromExpectedPeriod2 <= margin);
		result &= (differenceFromExpectedPeriod4 <= margin);
		result &= (differenceFromExpectedPeriod8 <= margin);
		result &= (differenceFromExpectedPeriod16 <= margin);

		//Log and return result:

		if (log) {
			printf("\nPRNG test (draw4spcg32s): %lld drawn (normalized avg: %f)\n",
				                                               totalDrawn, avarege);
			printf("\tTiming: %f nanos per prn\n", nanosPerDraw);
			printf("\tZeroes: %d (expected: %f)\n", zeroes, expectedZeroes);
			printf("\tAvg. differences expected: %f (margin used: %f). Found:\n",
								expectedAverageAbsoluteDifference, margin);
			printf("\t\tPeriod 2: %f; Period 4: %f, Period 8: %f, Period 16: %f\n",
				averageAbsoluteDifferencePeriod2, averageAbsoluteDifferencePeriod4,
			   averageAbsoluteDifferencePeriod8, averageAbsoluteDifferencePeriod16);
		}

		return result;
	}

    //Sleeps from minimumSleepTimeMicros to maximumSleepTimeMicros, increasing by
	//multiplications of sleepTimeMultiplierPerStep. Always includes maximumSleepTimeMicros.
	//For each sleep time, tests howManyThresholdSteps different thresholds and
	//howManyParallelLoadSteps different parallel loads.
	//Always tests *at least* no threshold, standard threshold and "full threshold" (just
	//nusy waiting, as a baseline). Also always tests no parrallel load as well as moderate 
	//(~35% cpu time, period a quarter of sleep time) and high (~90% cpu time, period 1/20
	//sleep time). For each test, calculates how much time was slept over what was expected.
	//Returns the proportion of tests which had excess <= margin*(sleeptime + baseline).
	//If log == true, logs to output (eg, console) the results of each test.
	//Has default values for all parameters.
	static double testHybridBusySleeping(
			int minimumSleepTimeMicros = 10, int maximumSleepTimeMicros = 20000,
			double sleepTimeMultiplierPerStep = 4, int howManyThresholdSteps = 3,
			int howManyParallelLoadSteps = 3, double margin = 0.005, bool log = true) {

		return 0;
	}
}
