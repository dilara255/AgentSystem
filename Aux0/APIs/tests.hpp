#pragma once

#include "prng.hpp"
#include "timeHelpers.hpp"

namespace AZ{
	//Draws howManyToDraw prns into a vector. Tests:
    //Avarage, amount of zeroes, periods 2, 4, 8 and 16
    //Returns true if all tests pass. Defaults to 100M draws
    static bool testDraw4spcg32s(int howManyToDraw = 100000000);

    //Draws howManyToDraw prns into a vector. Tests:
    //Avarage, amount of zeroes, periods 2, 4, 8 and 16
    //Returns true if all tests pass. Defaults to 25M draws
    static bool testDraw1spcg32(int howManyToDraw = 25000000);

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
			int howManyParallelLoadSteps = 3, double margin = 0.005, bool log = true);
}
