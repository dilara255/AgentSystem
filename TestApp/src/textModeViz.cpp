#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "logAPI.hpp"
#include "timeHelpers.hpp"

#include "AS_API.hpp"
#include "CL_externalAPI.hpp"

#include "textViz.hpp"

const char* initialNetworkFilename = "textModeVizBase.txt";
const char* networkFilenameSaveName = "textModeViz_run0.txt";

const std::chrono::seconds testTime = std::chrono::seconds(30);

int TV::textModeVisualizationLoop(std::chrono::seconds loopTime) {

	auto start = std::chrono::steady_clock::now();
	auto timePassed = std::chrono::seconds(0);
	printf("\n\n\n\nWill run test for %llu seconds...\nSeconds remaining: %llu...", 
						  					    loopTime.count(), loopTime.count());

	while (timePassed < loopTime) {

		AZ::hybridBusySleepForMicros(std::chrono::microseconds(MICROS_IN_A_SECOND));
		auto now = std::chrono::steady_clock::now();
		timePassed = 
			std::chrono::duration_cast<std::chrono::seconds>(now - start);
		printf(" %llu...", (loopTime - timePassed).count());
	}
	printf("\nDone! Leaving Main Loop...\n\n\n");

	return 0;
}

int TV::textModeVisualizationEntry() {

	LOG_DEBUG("Initializing...\n",1);
	if (!AS::initializeASandCL()) {
		LOG_CRITICAL("Couldn't initialize the AS");
		GETCHAR_FORCE_PAUSE;
		return 1;
	}

	LOG_DEBUG("Loading...\n",1);
	if (!AS::loadNetworkFromFile(initialNetworkFilename)) {
		LOG_CRITICAL("Couldn't load the test network");
		GETCHAR_FORCE_PAUSE;
		return 1;
	}

	LOG_DEBUG("Running network...\n",1);
	if (!AS::run()) {
		LOG_CRITICAL("Failed to run network");
		GETCHAR_FORCE_PAUSE;
		return 1;
	}

	LOG_DEBUG("Starting visualization Main Loop...\n",20);
	int result = textModeVisualizationLoop(testTime);
	GETCHAR_FORCE_PAUSE;
	
	LOG_DEBUG("Stopping AS...\n",1);
	AS::stop();

	LOG_DEBUG("Saving results...\n",1);
	AS::saveNetworkToFile(networkFilenameSaveName, false, false, true);

	LOG_DEBUG("Quitting AS...\n\n",1);
	AS::quit();

	LOG_DEBUG("Done. Press enter to exit",1);
	GETCHAR_FORCE_PAUSE;
	return 0;
}