#include "miscStdHeaders.h"

#include "systems/actionSystem.hpp"
#include "data/agentDataControllers.hpp"

#include "systems/actionHelpers.hpp"

namespace AS::Decisions {

	float calcNotionS0(scope scope, int agentID, AS::dataControllerPointers_t* dp) {

		return 1.0f;
	}

	float calcNotionS1(scope scope, int agentID, AS::dataControllerPointers_t* dp) {

		float timeWaster = calcNotionS0(scope, agentID, dp);

		return 1.0f;
	}

	float calcNotionS2(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionS0(scope, agentID, dp);

		return 1.0f;
	}

	float calcNotionS3(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionS0(scope, agentID, dp);

		return 1.0f;
	}

	float calcNotionS4(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionS0(scope, agentID, dp);

		return 1.0f;
	}

	float calcNotionS5(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionS0(scope, agentID, dp);

		return 1.0f;
	}

	float calcNotionS6(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionS0(scope, agentID, dp);

		return 1.0f;
	}

	float calcNotionS7(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionS0(scope, agentID, dp);

		return 1.0f;
	}


	float calcNotionN0(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		return 1.0f;
	}

	float calcNotionN1(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionN0(scope, agentID, dp);

		return 1.0f;
	}

	float calcNotionN2(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionN0(scope, agentID, dp);

		return 1.0f;
	}

	float calcNotionN3(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionN0(scope, agentID, dp);

		return 1.0f;
	}

	float calcNotionN4(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionN0(scope, agentID, dp);

		return 1.0f;
	}

	float calcNotionN5(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionN0(scope, agentID, dp);

		return 1.0f;
	}

	float calcNotionN6(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionN0(scope, agentID, dp);

		return 1.0f;
	}

	float calcNotionN7(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionN0(scope, agentID, dp);

		return 1.0f;
	}

	float calcNotionN8(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionN0(scope, agentID, dp);

		return 1.0f;
	}

	float calcNotionN9(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionN0(scope, agentID, dp);

		return 1.0f;
	}

	float calcNotionN10(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionN0(scope, agentID, dp);

		return 1.0f;
	}

	float calcNotionN11(scope scope, int agentID, AS::dataControllerPointers_t* dp) {
		
		float timeWaster = calcNotionN0(scope, agentID, dp);

		return 1.0f;
	}

	float calculateNotion(notionsSelf notion, scope scope, int agentID, 
		                              AS::dataControllerPointers_t* dp) {

		switch ((int)notion)
		{
		case 0:
			return calcNotionS0(scope, agentID, dp);
		case 1:
			return calcNotionS1(scope, agentID, dp);
		case 2:
			return calcNotionS2(scope, agentID, dp);
		case 3:
			return calcNotionS3(scope, agentID, dp);
		case 4:
			return calcNotionS4(scope, agentID, dp);
		case 5:
			return calcNotionS5(scope, agentID, dp);
		case 6:
			return calcNotionS6(scope, agentID, dp);
		case 7:
			return calcNotionS7(scope, agentID, dp);
		default:
			return 0.0f;
		}
	}

	float calculateNotion(notionsNeighbor notion, scope scope, int agentID, 
		                                 AS::dataControllerPointers_t* dp) {

		switch ((int)notion)
		{
		case 0:
			return calcNotionN0(scope, agentID, dp);
		case 1:
			return calcNotionN1(scope, agentID, dp);
		case 2:
			return calcNotionN2(scope, agentID, dp);
		case 3:
			return calcNotionN3(scope, agentID, dp);
		case 4:
			return calcNotionN4(scope, agentID, dp);
		case 5:
			return calcNotionN5(scope, agentID, dp);
		case 6:
			return calcNotionN6(scope, agentID, dp);
		case 7:
			return calcNotionN7(scope, agentID, dp);
		case 8:
			return calcNotionN8(scope, agentID, dp);
		case 9:
			return calcNotionN9(scope, agentID, dp);
		case 10:
			return calcNotionN10(scope, agentID, dp);
		case 11:
			return calcNotionN11(scope, agentID, dp);
		default:
			return 0.0f;
		}
	}
}