#pragma once

#include "multipleCategoryCounter.hpp"
#include "logAPI.hpp"

namespace AS {

	//TODO: collections of strings of the warnings and error messages, which can be shown on check
	//(eventually could collect source info as well to display together)

	enum class warnings {TEST, DS_FINISH_IN_LESS_THAN_ONE_CHOP, TOTAL};
	enum class errors {TEST, RS_FAILED_RECEIVING, RS_FAILED_SENDING, PS_FAILED_TO_DRAW_PRNS,
					   DS_RECEIVED_BAD_CHOP_INDEX, DS_NEGATIVE_DECISIONS_MADE, 
					   DS_NEGATIVE_NUMBER_OF_AGENTS, 
		               TOTAL};
	//PS: preparation step; RS: receive/send; DS: decision step;

	class WarningsAndErrorsCounter {

	public:
		WarningsAndErrorsCounter(uint64_t currentTick, uint64_t ticksPerDisplay): 
				m_warnings((int)warnings::TOTAL), m_errors((int)errors::TOTAL) {
			
			m_lastTickDisplayed = currentTick;
			m_ticksPerDisplay = ticksPerDisplay;
		};

		uint64_t getLastTickDisplayed() {
			return m_lastTickDisplayed;

		}

		void setLastTickDisplayed(uint64_t tick) {
			m_lastTickDisplayed = tick;
		}

		uint64_t getTicksPerDisplay() {
			return m_ticksPerDisplay;
		}

		void setTicksPerDisplay(uint64_t ticksPerDisplay) {
			m_ticksPerDisplay = ticksPerDisplay;
		}

		int incrementWarning(warnings id) {
			m_warnings.increment((int)id);
			return m_warnings.howMany((int)id);
		}

		int incrementError(errors id) {
			m_errors.increment((int)id);
			return m_errors.howMany((int)id);
		}

		int getWarnings(warnings id) {
			return m_warnings.howMany((int)id);
		}

		int getErrors(errors id) {
			return m_errors.howMany((int)id);
		}

		int totalWarnings() {
			return m_warnings.totalCount();
		}

		int totalErrors() {
			return m_errors.totalCount();
		}

		int total() {
			return (m_warnings.totalCount() + m_errors.totalCount());
		}

		int totalWarningsAlreadyDisplayed() {
			return m_countWarningsDisplayed;
		}

		int totalErrorsAlreadyDisplayed() {
			return m_countErrorsDisplayed;
		}

		int timesDisplayed() {
			return m_timesDisplayed;
		}

		void printWarningCounts() {
			m_warnings.printCounts();
		}

		void printErrorCounts() {
			m_errors.printCounts();
		}

		void clear() {
			m_warnings.clearAll();
			m_errors.clearAll();
		}

		//Returns wether there was something to display.
		bool showPendingIfEnoughTicksPassedAndClear(uint64_t currentTick) {
			if (currentTick < (m_lastTickDisplayed + m_ticksPerDisplay)) {return false;}
			if (total() == 0) {return false;}
			
			m_timesDisplayed++;
			m_lastTickDisplayed = currentTick;
			
			int totalWarnings = m_warnings.totalCount();
			if (totalWarnings > 0) {
				LOG_WARN("Some warnings happened since last display");
				#if (defined AS_DEBUG) || VERBOSE_RELEASE
					printf("\t%d Warnings (index: count): ", totalWarnings);
					m_warnings.printCounts();
				#endif

				m_countWarningsDisplayed += totalWarnings;
			}

			int totalErrors = m_errors.totalCount();
			if (totalErrors > 0) {
				LOG_ERROR("Some errors happened since last display");
				printf("\t%d Errors (index: count): ", totalErrors);
				m_errors.printCounts();

				m_countErrorsDisplayed += totalErrors;
			}

			clear();		

			return true;
		}

	private:
		uint64_t m_lastTickDisplayed;
		uint64_t m_ticksPerDisplay;
		int m_timesDisplayed = 0;
		int m_countWarningsDisplayed = 0;
		int m_countErrorsDisplayed = 0;
		MultipleCategoryCounter m_warnings;
		MultipleCategoryCounter m_errors;
	};
}