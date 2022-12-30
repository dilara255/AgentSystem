#pragma once

/*
These parameters are fixed and represent a "kind" of network.
* These are essentially constrainst for any network to be loaded.
* They help make things predictable and sizes static.
* Future versions of the AS may try to add more flexibity.
*/

#define NAME_LENGHT 30
#define NUMBER_LA_ACTIONS 18
#define NUMBER_LA_OFFSETS (NUMBER_LA_ACTIONS + 5) //5: for specific diplomatic states
#define MAX_LA_NEIGHBOURS 10
#define MAX_GA_QUANTITY 15
#define MAX_LA_QUANTITY 128

namespace AS {
	typedef AZ::FlagField128 LAflagField_t;
	typedef AZ::FlagField32 GAflagField_t;
}

enum gaPersonalityTraits {/*add some*/ };