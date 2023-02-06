#pragma once

#include "miscStdHeaders.h"

#define AZ_MICROS_TO_BUSY_WAIT 25
#define MICROS_IN_A_SECOND 1000000
#define MILLIS_IN_A_SECOND 1000

namespace AZ{

	const static std::chrono::microseconds standardThreshold(AZ_MICROS_TO_BUSY_WAIT);

	//takes just about 0.22 microseconds on test system
	inline std::chrono::microseconds nowMicros() {
		auto now = std::chrono::steady_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::microseconds>(now);
	}

	inline std::chrono::nanoseconds nowNanos() {
		auto now = std::chrono::steady_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::nanoseconds>(now);
	}

	//This loops while yielding via Sleep(0) up to targetWakeTime - threshold,
	//then busy-waits until threshold. Does nothing if now >= targetWakeTime.
	//If no threshold is especified, uses standard value defined by Aux0.
	//WARNING: NO IDEA how portable this is (HACKY?)
	//TODO-CRITICAL: Implement testing of this specifically
	static void hybridBusySleep(std::chrono::steady_clock::time_point targetWakeTime,
		                     std::chrono::microseconds threshold = standardThreshold) {

		int dummy = 0;
		static std::chrono::microseconds zeroMicro(0);

		while ((targetWakeTime - std::chrono::steady_clock::now()) > threshold) {
			std::this_thread::sleep_for(zeroMicro);
		}
		while (std::chrono::steady_clock::now() < targetWakeTime){dummy++;}
	}

	//This loops while yielding via Sleep(0) for microsToSleep - threshold,
	//then busy-waits to complete microsToSleep. 
	//If no threshold is especified, uses standard value defined by Aux0.
	static void hybridBusySleepForMicros(std::chrono::microseconds microsToSleep,
		                 std::chrono::microseconds threshold = standardThreshold){

		auto targetWakeTime = std::chrono::steady_clock::now() + microsToSleep;

		hybridBusySleep(targetWakeTime, threshold);
	}
}