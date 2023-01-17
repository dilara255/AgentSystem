#pragma once

/*
Provides enums to all of the data structures of agents and actions, as well as for each field.
This is useful fo communication between Client, CL and AS.
*/

namespace AS {
    enum dataCategory { LA_COLD, LA_STATE, LA_DECISION, GA_COLD, GA_STATE, GA_DECISION,
                        LA_ACTION, GA_ACTION, NETWORK_PARAMS,
                        TOTAL_DATA_CATEGORIES };

    enum LAcoldFields { };

    enum LAstateFields { };

    enum LAdecisionFields { };

    enum GAcoldFields { };

    enum GAstateFields { };

    enum GAdecisionFields { };

    enum actionFields { };

    //plus enums for sub-fields of each field
}

