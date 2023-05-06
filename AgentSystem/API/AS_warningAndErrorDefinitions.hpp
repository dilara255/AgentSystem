#pragma once

namespace AS {

	enum class warnings {TEST, DS_FINISH_IN_LESS_THAN_ONE_CHOP,
						 DP_LA_TRADE_PARTNER_HAS_ZERO_SAT,                 
						 DP_GA_TRADE_PARTNER_HAS_ZERO_SAT, 
		                 DS_GA_GOT_BAD_ACT_COUNT, DS_LA_GOT_BAD_ACT_COUNT,
		                 DS_LA_NOTIONS_RMS_BLEW_UP, DS_GA_NOTIONS_RMS_BLEW_UP,
		                 DS_TRIED_TO_SPAWN_TOO_MANY_ACTIONS,
		                 TOTAL};
	enum class errors {TEST, RS_FAILED_RECEIVING, RS_FAILED_SENDING, PS_FAILED_TO_DRAW_PRNS,
					   DS_RECEIVED_BAD_CHOP_INDEX, DS_NEGATIVE_DECISIONS_MADE, 
					   DS_NEGATIVE_NUMBER_OF_AGENTS, AC_FAILED_PROCESS_LA_ACT,
		               AC_FAILED_PROCESS_GA_ACT, AS_LA_STATE_PTR_NULL, AS_GA_STATE_PTR_NULL,
					   AS_LA_NOT_NEIGHBOR_OF_NEIGHBOR, AS_GA_NOT_NEIGHBOR_OF_NEIGHBOR,
					   AS_GA_INFILTRATION_NOT_FINITE, AS_LA_INFILTRATION_NOT_FINITE,
					   AS_GA_DISPOSITION_NOT_FINITE, AS_LA_DISPOSITION_NOT_FINITE,
		               AS_GA_INFILTRATION_FROM_LAS_NOT_FINITE, 
		               AS_GA_DISPOSITION_FROM_LAS_NOT_FINITE, AC_COULDNT_GET_ACTIONS_CPTR, 
					   DS_CHOSE_INVALID_GA_TARGET, DS_CHOSE_INVALID_LA_TARGET,
					   DS_NEIGHBOR_MARKED_SELF_WRONG_MODE_ON_LEAST_HARM,
					   DS_NEIGHBOR_MARKED_SELF_WRONG_MODE_ON_TRY_BEST,
		               DS_FAILED_TO_FIND_NEIGHBORS_INDEX,
					   AC_RECEIVED_BAD_SYSTEM_POINTERS, AC_RECEIVED_BAD_ACTION_PTR,
					   DS_LAST_ACTION_SCORED_IS_INVALID, 
					   DS_FIRST_UNSCORED_ACTION_NOT_AS_EXPECTED, DS_CHOSE_INVALID_VARIATION,
		               TOTAL};
	//PS: Preparation Step; RS: Receive/Send; DS: Decision Step; AC: Action; AS: Agent Step;
	//DP: Diplomacy; DS: Decision Step
}