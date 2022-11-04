/*
1) Each LA's data will be comprised of:
 
-On/off boolean, to signal whether it should be taken in account or ignored by the system (say, because the location has been destroyed);
-A position;
-A name (of fixed maximum size);
-A quantity of resources and its update rate;
-A strength and a strength threshold to start costing upkeep;
-The current upkeep;
-The id of the GA it belongs to, if any;
-The information about which LAs it's connected to (as a bitfield);
-A list of the disposition towards each connected LA (can be positive or negative);
-The diplomatic stance towards each connected LA (eg: hostile, neutral, trade, ally);
-A list of how much information it has on each connected LA;
-A list of LA-decision-related thresholds, which act as the "personality" of the agent;
-A list of active constraints/incentives given by the associated GA.

2) Each GA's data will be comprised of:

-On/off, as in the LAs;
-A name (of fixed maximum size);
-The information about which LAs belong to it (as a bitfield);
-The information about which other GAs it’s connected to (as a bitfield);
-A list of the disposition towards each connected GA (can be positive or negative);
-The diplomatic stance towards each connected GA (eg: hostile, neutral, trade, ally);
-A list of how much information it has on each connected GA;
-Four traits (4 ids), which will influence the GAs "personality";
-Total resources and accumulation rates of the connected LAs;
-Total strength of the connected LAs.
*/

#define NAME_LENGHT 30

namespace AS{
	typedef struct {
		float x;
		float y;
	} pos_t;

	typedef struct {
		float current;
		float updateRate;
	} resources_t;

	typedef struct {
		float current;
		float thresholdToCostUpkeep;
		float currentUpkeep;
	} strenght_t;

	typedef struct{
		float thr1;
		float thr2;
		float thr3;
		float thr4;
		float thr5;
		float thr6;
		/*...*/
	} localAgentThresholds_t;

	typedef struct {
		float incentiveOrConstraint1;
		float incentiveOrConstraint2;
		float incentiveOrConstraint3;
		float incentiveOrConstraint4;
		float incentiveOrConstraint5;
		float incentiveOrConstraint6;
		/*...*/
	} incentivesAndConstraints_t;

	typedef int bitfield_t; //ver direito isso

	typedef struct {
		resources_t resources;
		strenght_t strenght;
	} parametersLA_t;

	typedef struct {
		pos_t position;
		int firstConnectedNeighborId;
		int numberConnectedNeighbors;
		bitfield_t connectedNeighbors;	
	} locationAndConnectionDataLA_t;

	typedef struct {
		int GAid;
		incentivesAndConstraints_t incentivesAndConstraintsFromGA;
		localAgentThresholds_t thresholds;
	} thresholdsAndGAinfluenceOnLA_t;

	typedef struct {
		float* dispositionsToNeighbors_list;
		int* diplomaticStanceToNeighbors_list;
		float* informationAboutNeighbors_list;
	} neighborRelationPtrs_t;

	typedef struct{
		//for "exporting"
		int id; //defined on load, trying to minimize the id-distance between neighbors
		
		char name[NAME_LENGHT];
		
		bool on;
		
		parametersLA_t resourcesAndStrenght;
		
		locationAndConnectionDataLA_t locationAndConnections;
		
		//arrays created on load, 
		//fixed size == locationAndConnections.numberConnectedNeighbors
		neighborRelationPtrs_t neighborRelation_ptrs;
		
		thresholdsAndGAinfluenceOnLA_t thresholdsAndGAinfluence;
	} localAgent_t;

	typedef int GApersonality[4];

	enum gaPersonalityTraits {/*add some*/};

	typedef struct{
		//for "exporting"
		int id;
		
		char name[NAME_LENGHT];
		
		bool on;
		
		bitfield_t localAgentsBelongingToThis;
		resources_t resourceTotalOfLAs;
		strenght_t strengthTotalOfLAs;
		
		pos_t position;
		int numberConnectedGAs; //set on load, never changes
		bitfield_t connectedGAs;
		
		//arrays created on load, fixed size == numberConnectedGAs
		float* dispositionsToConnectedGAs_list; 
		int* diplomaticStanceToConnectedGAs_list; 
		float* informationAboutToConnectedGAs_list;
			
		GApersonality traits;
	} globalAgent_t;

namespace LA{
	//memory layout for AS: each field is contiguous between all agents, on it's system.
	//6 "systems", indexed by id.
	//For AS "on" will actually be just a bitfield you check against the id. 
	//For the "export" structure it's a bool. 

	//calss LocalAgent with id as only member and constructors as only methods. 
	
	typedef struct{
		char** nameList_ptr; //fixed NAME_LENGHT size (maybe this can be cleaned up a bit)
		int* onOffList_ptr; //I need ceil(NUMBER_OF_LAs/sizeof(int*8)) ints
		parametersLA_t* parameterList_ptr;
		locationAndConnectionDataLA_t* locationAndConnectionData_ptr;
		neighborRelationPtrs_t* neighborRelationsList_ptr;
		thresholdsAndGAinfluenceOnLA_t* tresholdsAndInfluenceData_ptr;
	} systemsPointers_t;

	systemsPointers_t LAsystemPtrs;
}

namespace GA{
	//memory layout for AS: names / on / resources and strenghts / positions and connections  / 
	// neighbor stuff / personality
	//6 "systems", indexed by id, similar to LAs
	//For AS "on" will actually be just a bitfield you check against the id. For the "export" structure it's a bool.
}

/*
Notes:

Graph Reordering for Cache-Efficient Near Neighbor Search https://arxiv.org/pdf/2104.03221.pdf
... - neighbor4 - neighbor2 - maxNeighbors - neighbor1 - neighbor3 -n1n1 - n1n2 - n1n1n1 

TODO:
1. Simplify GA data structure (as LA is);
2. Finish creating the pointers and stuff for LAs;
3. Do the same for GAs;
4. Create simple initialization to allocate all this and relay the info;
*/