#include "miscStdHeaders.h"

#include "systems/actionSystem.hpp"
#include "data/agentDataControllers.hpp"

#include "data/agentDataStructures.hpp"

#include "systems/actionHelpers.hpp"

namespace PURE_LA = LA;
namespace PURE_GA = GA;

namespace AS::Decisions::LA {

	//STUB: pretend this is something like "I'm afraid I have too much upkeep / income"
	float calcNotionS0(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {

		return 1.0f;
	}

	float calcNotionS1(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {

		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS2(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS3(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS4(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS5(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS6(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS7(int agentID, AS::dataControllerPointers_t* dp, 
					        PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}


	float calcNotionN0(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		return 1.0f;
	}

	float calcNotionN1(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN2(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN3(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN4(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN5(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN6(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN7(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN8(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN9(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										  PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN10(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										   PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN11(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
										   PURE_LA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}
}

namespace AS::Decisions::GA {

	//STUB: pretend this is something like "I'm afraid I have too much upkeep / income"
	float calcNotionS0(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {

		return 1.0f;
	}

	float calcNotionS1(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {

		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS2(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS3(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS4(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS5(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS6(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionS7(int agentID, AS::dataControllerPointers_t* dp, 
		                    PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionS0(agentID, dp, refReads_ptr);

		return 1.0f;
	}


	float calcNotionN0(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		return 1.0f;
	}

	float calcNotionN1(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN2(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN3(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN4(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN5(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN6(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN7(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN8(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN9(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                  PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN10(int neighbor, int agentID, AS::dataControllerPointers_t* dp,
						                   PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}

	float calcNotionN11(int neighbor, int agentID, AS::dataControllerPointers_t* dp, 
						                   PURE_GA::readsOnNeighbor_t* refReads_ptr) {
		
		float timeWaster = calcNotionN0(neighbor, agentID, dp, refReads_ptr);

		return 1.0f;
	}
}


namespace AS::Decisions {

	float calculateNotionSelfLA(notionsSelf notion, int agentID, 
		                      AS::dataControllerPointers_t* dp,
		                      PURE_LA::readsOnNeighbor_t* refReads_ptr) {

		switch ((int)notion)
		{
		case 0:
			return LA::calcNotionS0(agentID, dp, refReads_ptr);
		case 1:
			return LA::calcNotionS1(agentID, dp, refReads_ptr);
		case 2:
			return LA::calcNotionS2(agentID, dp, refReads_ptr);
		case 3:
			return LA::calcNotionS3(agentID, dp, refReads_ptr);
		case 4:
			return LA::calcNotionS4(agentID, dp, refReads_ptr);
		case 5:
			return LA::calcNotionS5(agentID, dp, refReads_ptr);
		case 6:
			return LA::calcNotionS6(agentID, dp, refReads_ptr);
		case 7:
			return LA::calcNotionS7(agentID, dp, refReads_ptr);
		default:
			return 0.0f;
		}
	}

	float calculateNotionNeighborLA(notionsNeighbor notion, int neighbor, 
		                          int agentID, AS::dataControllerPointers_t* dp,
		                          PURE_LA::readsOnNeighbor_t* refReads_ptr) {

		switch ((int)notion)
		{
		case 0:
			return LA::calcNotionN0(neighbor, agentID, dp, refReads_ptr);
		case 1:
			return LA::calcNotionN1(neighbor, agentID, dp, refReads_ptr);
		case 2:
			return LA::calcNotionN2(neighbor, agentID, dp, refReads_ptr);
		case 3:
			return LA::calcNotionN3(neighbor, agentID, dp, refReads_ptr);
		case 4:
			return LA::calcNotionN4(neighbor, agentID, dp, refReads_ptr);
		case 5:
			return LA::calcNotionN5(neighbor, agentID, dp, refReads_ptr);
		case 6:
			return LA::calcNotionN6(neighbor, agentID, dp, refReads_ptr);
		case 7:
			return LA::calcNotionN7(neighbor, agentID, dp, refReads_ptr);
		case 8:
			return LA::calcNotionN8(neighbor, agentID, dp, refReads_ptr);
		case 9:
			return LA::calcNotionN9(neighbor, agentID, dp, refReads_ptr);
		case 10:
			return LA::calcNotionN10(neighbor, agentID, dp, refReads_ptr);
		case 11:
			return LA::calcNotionN11(neighbor, agentID, dp, refReads_ptr);
		default:
			return 0.0f;
		}
	}

	float calculateNotionSelfGA(notionsSelf notion, int agentID, 
		                      AS::dataControllerPointers_t* dp,
		                      PURE_GA::readsOnNeighbor_t* refReads_ptr) {

		switch ((int)notion)
		{
		case 0:
			return GA::calcNotionS0(agentID, dp, refReads_ptr);
		case 1:
			return GA::calcNotionS1(agentID, dp, refReads_ptr);
		case 2:
			return GA::calcNotionS2(agentID, dp, refReads_ptr);
		case 3:
			return GA::calcNotionS3(agentID, dp, refReads_ptr);
		case 4:
			return GA::calcNotionS4(agentID, dp, refReads_ptr);
		case 5:
			return GA::calcNotionS5(agentID, dp, refReads_ptr);
		case 6:
			return GA::calcNotionS6(agentID, dp, refReads_ptr);
		case 7:
			return GA::calcNotionS7(agentID, dp, refReads_ptr);
		default:
			return 0.0f;
		}
	}

	float calculateNotionNeighborGA(notionsNeighbor notion, int neighbor, 
		                          int agentID, AS::dataControllerPointers_t* dp,
		                          PURE_GA::readsOnNeighbor_t* refReads_ptr) {

		switch ((int)notion)
		{
		case 0:
			return GA::calcNotionN0(neighbor, agentID, dp, refReads_ptr);
		case 1:
			return GA::calcNotionN1(neighbor, agentID, dp, refReads_ptr);
		case 2:
			return GA::calcNotionN2(neighbor, agentID, dp, refReads_ptr);
		case 3:
			return GA::calcNotionN3(neighbor, agentID, dp, refReads_ptr);
		case 4:
			return GA::calcNotionN4(neighbor, agentID, dp, refReads_ptr);
		case 5:
			return GA::calcNotionN5(neighbor, agentID, dp, refReads_ptr);
		case 6:
			return GA::calcNotionN6(neighbor, agentID, dp, refReads_ptr);
		case 7:
			return GA::calcNotionN7(neighbor, agentID, dp, refReads_ptr);
		case 8:
			return GA::calcNotionN8(neighbor, agentID, dp, refReads_ptr);
		case 9:
			return GA::calcNotionN9(neighbor, agentID, dp, refReads_ptr);
		case 10:
			return GA::calcNotionN10(neighbor, agentID, dp, refReads_ptr);
		case 11:
			return GA::calcNotionN11(neighbor, agentID, dp, refReads_ptr);
		default:
			return 0.0f;
		}
	}
}