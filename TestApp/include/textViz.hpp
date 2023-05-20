#pragma once

namespace TV {

	int textModeVisualizationEntry();

	typedef struct actionChanges_st {
		AS::actionData_t data;
		bool hasChanged = false;
	} actionChanges_t;

	typedef struct decisionHasChanges_st {
		int lastAmountOfDecisions = 0;
		bool hasChanged = false;
	} decisionHasChanges_t;

}