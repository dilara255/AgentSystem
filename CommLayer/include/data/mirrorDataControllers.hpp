#pragma once    

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\
   //      WARNING: *AWFUL* REPETITION GOING ON HERE        \\
  //  TO DO: FIX IT BY PROPERLY EXPORTING THE WANTED CLASSES \\
 //                       (how, tho?)                         \\
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\

#include "AS_internal.hpp"

namespace CL {
	using namespace AS;

	class ActionMirrorController {
	public:

		bool initialize() { size_t size = MAX_ACTIONS_PER_AGENT*sizeof(actionData_t);
							dataLAs.reserve(size * MAX_LA_QUANTITY);
							dataGAs.reserve(size * MAX_GA_QUANTITY);
							m_maxActionsPerAgent = MAX_ACTIONS_PER_AGENT;
							m_isInitialized = true;
	    }

		bool replaceDataLAs(const std::vector <actionData_t>& newData) { dataLAs = newData; }
		bool replaceDataGAs(const std::vector <actionData_t>& newData) { dataGAs = newData; }

		bool getAction(int localOrGlobal, uint32_t actionID, actionData_t* recepient) const;
		bool getAgentData(int localOrGlobal, uint32_t agentID, int actionNumber,
			actionData_t* recepient) const;

		size_t sizeOfDataInBytesLAs() const {return dataLAs.size() * sizeof(dataLAs[0]);}
		size_t sizeOfDataInBytesGAs() const {return dataLAs.size() * sizeof(dataLAs[0]);}
		size_t capacityForDataInBytesLAs() const {return dataLAs.capacity() * sizeof(dataLAs[0]); }
		size_t capacityForDataInBytesGAs() const {return dataGAs.capacity()*sizeof(dataGAs[0]);}

		void clearData() { dataLAs.clear(); dataGAs.clear(); }

		bool isInitialized() const { return m_isInitialized; }
		bool hasData() const { return m_hasData; }
		int getMaxActionsPerAgent() const { return m_maxActionsPerAgent; }

		std::vector <actionData_t> dataLAs;
		std::vector <actionData_t> dataGAs;

	private:
		bool m_isInitialized = false;
		bool m_hasData = false;
		int m_maxActionsPerAgent = 0;
	};
}

namespace CL{

	class ColdDataControllerLA {
	public:
		ColdDataControllerLA() { data.reserve(MAX_LA_QUANTITY*sizeof(LA::coldData_t)); }

		bool replaceData(const std::vector <LA::coldData_t> &newData) { data = newData; }

		bool getAgentData(uint32_t agentID, LA::coldData_t* recepient) const;

		size_t sizeOfDataInBytes() const { return data.size() * sizeof(data[0]); }
		size_t capacityForDataInBytes() const { return data.capacity() * sizeof(data[0]); }

		void clearData() { data.clear(); }
	
		std::vector <LA::coldData_t> data;
	};

	class StateControllerLA {
	public:
		StateControllerLA() { data.reserve(MAX_LA_QUANTITY * sizeof(LA::stateData_t)); }

		bool replaceData(const std::vector <LA::stateData_t>& newData) { data = newData; }

		bool getAgentData(uint32_t agentID, LA::stateData_t* recepient) const;

		size_t sizeOfDataInBytes() const { return data.size() * sizeof(data[0]); }
		size_t capacityForDataInBytes() const { return data.capacity() * sizeof(data[0]); }

		void clearData() { data.clear(); }
	
		std::vector <LA::stateData_t> data;
	};

	class DecisionSystemLA {
	public:
		DecisionSystemLA() { data.reserve(MAX_LA_QUANTITY * sizeof(LA::decisionData_t)); }

		bool replaceData(const std::vector <LA::decisionData_t>& newData) { data = newData; }

		bool getAgentData(uint32_t agentID, LA::decisionData_t* recepient) const;

		size_t sizeOfDataInBytes() const { return data.size() * sizeof(data[0]); }
		size_t capacityForDataInBytes() const { return data.capacity() * sizeof(data[0]); }

		void clearData() { data.clear(); }
	
		std::vector <LA::decisionData_t> data;
	};
}
	
namespace CL {
	class ColdDataControllerGA {
	public:
		ColdDataControllerGA() { data.reserve(MAX_LA_QUANTITY * sizeof(GA::coldData_t)); }

		bool replaceData(const std::vector <GA::coldData_t>& newData) { data = newData; }

		bool getAgentData(uint32_t agentID, GA::coldData_t* recepient) const;

		size_t sizeOfDataInBytes() const { return data.size() * sizeof(data[0]); }
		size_t capacityForDataInBytes() const { return data.capacity() * sizeof(data[0]); }

		void clearData() { data.clear(); }
	
		std::vector <GA::coldData_t> data;
	};

	class StateControllerGA {
	public:
		StateControllerGA() { data.reserve(MAX_LA_QUANTITY * sizeof(GA::stateData_t)); }

		bool replaceData(const std::vector <GA::stateData_t>& newData) { data = newData; }

		bool getAgentData(uint32_t agentID, GA::stateData_t* recepient) const;

		size_t sizeOfDataInBytes() const { return data.size() * sizeof(data[0]); }
		size_t capacityForDataInBytes() const { return data.capacity() * sizeof(data[0]); }

		void clearData() { data.clear(); }
	
		std::vector <GA::stateData_t> data;
	};

	class DecisionSystemGA {
	public:
		DecisionSystemGA() { data.reserve(MAX_LA_QUANTITY * sizeof(GA::decisionData_t)); }

		bool replaceData(const std::vector <GA::decisionData_t>& newData) { data = newData; }

		bool getAgentData(uint32_t agentID, GA::decisionData_t* recepient) const;

		size_t sizeOfDataInBytes() const { return data.size() * sizeof(data[0]); }
		size_t capacityForDataInBytes() const { return data.capacity() * sizeof(data[0]); }

		void clearData() { data.clear(); }
	
		std::vector <GA::decisionData_t> data;
	};
}

namespace CL {
	typedef struct {
		bool haveBeenCreated;
		bool haveData;
		ColdDataControllerLA* LAcoldData_ptr;
		StateControllerLA* LAstate_ptr;
		DecisionSystemLA* LAdecision_ptr;
		ColdDataControllerGA* GAcoldData_ptr;
		StateControllerGA* GAstate_ptr;
		DecisionSystemGA* GAdecision_ptr;
	} agentMirrorControllerPtrs_t;

	bool createAgentDataControllers(agentMirrorControllerPtrs_t* agentDataControllers_ptr);

	typedef struct {
		AS::networkParameters_t networkParams;
		CL::agentMirrorControllerPtrs_t agentMirrorPtrs;
		CL::ActionMirrorController actionMirror;
	}mirror_t;
}