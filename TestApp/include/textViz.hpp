#pragma once

namespace TV {

	typedef struct actionChanges_st {
		AS::actionData_t data;
		bool hasChanged = false;
	} actionChanges_t;

	int textModeVisualizationEntry();

}