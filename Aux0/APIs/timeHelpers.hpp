#pragma once

#include "miscStdHeaders.h"

namespace AZ{
	inline std::chrono::microseconds nowMicros() {
		auto now = std::chrono::steady_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::microseconds>(now);
	}

	inline std::chrono::nanoseconds nowNanos() {
		auto now = std::chrono::steady_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::nanoseconds>(now);
	}
}