#pragma once

/*These classes are meant as a convenient way to work with the entire Data relating to
* a single Agent (Local or Global) or to export Data ordered by Agent (with Cold, State
* and Decision Data together). 
*
* They are not for internal use of the AS (except for loading/exporting).
* 
* They should have a field for each Data Category and a Constructor to populate them
* given a pointer to each.
*/

#include "logAPI.hpp"
#include "data/agentDataStructures.hpp"

namespace AS {

	class LAdata {
	
	public:
		LAdata(LA::coldData_t coldData, LA::stateData_t state, LA::decisionData_t decisionData) {
			m_coldData = coldData;
			m_state = state;
			m_decisionData = decisionData;
			LOG_TRACE("Instance of LAdata Class created");
		}

		LA::coldData_t m_coldData;
		LA::stateData_t m_state;
		LA::decisionData_t m_decisionData;
	};

	class GAdata {

	public:
		GAdata(GA::coldData_t coldData, GA::stateData_t state, GA::decisionData_t decisionData) {
			m_coldData = coldData;
			m_state = state;
			m_decisionData = decisionData;
			LOG_TRACE("Instance of GAdata Class created");
		}

		GA::coldData_t m_coldData;
		GA::stateData_t m_state;
		GA::decisionData_t m_decisionData;
	};
}