#include "miscStdHeaders.h"
#include "miscDefines.hpp"

#include "logAPI.hpp"
#include "timeHelpers.hpp"

#include "AS_API.hpp"
#include "CL_externalAPI.hpp"

#include "textViz.hpp"

const char* initialNetworkFilename = "textModeVizBase.txt";
const char* networkFilenameSaveName = "textModeViz_run.txt";

const std::chrono::seconds testTime = std::chrono::seconds(20);

int TV::textModeVisualization() {

	if (!AS::initializeASandCL()) {
		LOG_CRITICAL("Couldn't initialize the AS");
		return 0;
	}

	if (!AS::loadNetworkFromFile(initialNetworkFilename)) {
		LOG_CRITICAL("Couldn't load the test network");
		return 0;
	}

	if (!AS::run()) {
		LOG_CRITICAL("Failed to run network");
		return 0;
	}

	auto start = std::chrono::steady_clock::now();
	auto now = start;
	while (std::chrono::duration_cast<std::chrono::seconds>(now - start) < testTime) {
		AZ::hybridBusySleepForMicros(std::chrono::microseconds(MICROS_IN_A_SECOND));
	}
	
	AS::pauseMainLoop();

	AS::saveNetworkToFile(networkFilenameSaveName, false, false, true);

	AS::quit();

	return 1;
}