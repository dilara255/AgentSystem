#include "miscStdHeaders.h"
#include "core.hpp"

#include "agentDataStructure.hpp"
#include "AS_internal.hpp"

/*
* * Note: all structures have a fixed size once maxNeighbours is defined;
* TO DO: create a MAX_NEIGHBORURS macro.
* 
* TO DO : Complete Rework.
* Use vectors to gold the Data, even though their size won't change after load.
* (so adding elements on load and clearing is very simple, and no(direct) malloc is used)
* (performance impact should be negligible with optimizations.TO DO : quick test of this)
*/

void allocateMemoryForAgentSystems(int numberLAs, int numberGAs, int neighborMaxLAs) {
	//Allocates the entire thing and sets pointers.
	//See agentDataStructure.hpp for the layout.
	
	AS::agentControlSystem_ptr.numberOfLAs = numberLAs;
	AS::agentControlSystem_ptr.numberOfGAs = numberGAs;
	AS::agentControlSystem_ptr.neighborMaxLAs = neighborMaxLAs;

	AS::agentControlSystem_ptr.lengthLAonOffList = 
		(int)ceil((float)numberLAs/(sizeof(AS::bitfield_t)*8));
	AS::agentControlSystem_ptr.lengthGAonOffList = 
		(int)ceil((float)numberGAs/(sizeof(AS::bitfield_t)*8));

	int lengthLAonOffList = AS::agentControlSystem_ptr.lengthLAonOffList;
	int lengthGAonOffList = AS::agentControlSystem_ptr.lengthGAonOffList;
	int neighborRelationBytes = 2*sizeof(float) + sizeof(int);
	int LAneighborRelationDataSize = neighborRelationBytes * neighborMaxLAs * numberLAs;
	int GAconnectionsLenght = lengthGAonOffList;

	AS::agentControlSystem_ptr.LAsystemsPtrs.nameList_ptr =
		(AS::agentName_t*)malloc(sizeof(AS::agentName_t) * numberLAs);
	AS::agentControlSystem_ptr.LAsystemsPtrs.onOffList_ptr =
		(AS::bitfield_t*)malloc(sizeof(AS::bitfield_t) * lengthLAonOffList);
	AS::agentControlSystem_ptr.LAsystemsPtrs.parameterList_ptr =
		(AS::parametersLA_t*)malloc(sizeof(AS::parametersLA_t) * numberLAs);
	//FIX: this is kinda hacky:
	AS::agentControlSystem_ptr.LAsystemsPtrs.locationAndConnectionData_ptr =
		(AS::locationAndConnectionDataLA_t*)malloc(
			(sizeof(AS::locationAndConnectionDataLA_t)+(numberLAs -1)*sizeof(AS::bitfield_t))
				* numberLAs);
	AS::agentControlSystem_ptr.LAsystemsPtrs.neighborRelationsList_ptr =
		(AS::neighborRelationPtrs_t*)malloc(LAneighborRelationDataSize);
	AS::agentControlSystem_ptr.LAsystemsPtrs.tresholdsAndInfluenceData_ptr =
		(AS::thresholdsAndGAinfluenceOnLA_t*)malloc(
			sizeof(AS::thresholdsAndGAinfluenceOnLA_t) * numberLAs);

	AS::agentControlSystem_ptr.GAsystemsPtrs.nameList_ptr =
		(AS::agentName_t*)malloc(sizeof(AS::agentName_t) * numberGAs);
	AS::agentControlSystem_ptr.GAsystemsPtrs.onOffList_ptr =
		(AS::bitfield_t*)malloc(sizeof(AS::bitfield_t) * lengthGAonOffList);
	//FIX: this is kinda hacky:
	AS::agentControlSystem_ptr.GAsystemsPtrs.LAparameterList_ptr =
		(AS::associatedLAparameters_t*)malloc(
			(sizeof(AS::associatedLAparameters_t)+(numberLAs-1)*sizeof(AS::bitfield_t))
				*numberGAs);
	//FIX: this is kinda hacky:
	AS::agentControlSystem_ptr.GAsystemsPtrs.GAconnectionsList_ptr =
		(AS::bitfield_t*)malloc(sizeof(AS::bitfield_t)*GAconnectionsLenght*numberGAs);
	AS::agentControlSystem_ptr.GAsystemsPtrs.neighborRelationsList_ptr =
		(AS::neighborRelationPtrs_t*)malloc(neighborRelationBytes*numberGAs);
	AS::agentControlSystem_ptr.GAsystemsPtrs.personalitiesList_ptr =
		(AS::GApersonality*)malloc(
			sizeof(AS::GApersonality) * numberGAs);

	//TO DO: set internal pointers 
	//actually, re-think pointers inside "internal" structs
	//TO DO: tie this to some initialization function and also set and verify some values
}



