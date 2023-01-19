    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\
   //      WARNING: *AWFUL* REPETITION GOING ON HERE        \\
  //  TO DO: FIX IT BY PROPERLY EXPORTING THE WANTED CLASSES \\
 //                       (how, tho?)                         \\
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\

#include "data/mirrorDataControllers.hpp"

//TO DO: Copy the definitions here - and then take a shower :'(
//(also, eventually understand how to export stuff with std containers)

namespace CL {

    bool ActionMirrorController::initialize() {
        LOG_TRACE("Initializing ActionMirrorController");

        m_maxActionsPerAgent = MAX_ACTIONS_PER_AGENT;
        dataLAs.reserve((int64_t)m_maxActionsPerAgent * MAX_LA_QUANTITY);
        dataGAs.reserve((int64_t)m_maxActionsPerAgent * MAX_GA_QUANTITY);

        AS::actionData_t stubAction;

        for (int i = 0; i < MAX_LA_QUANTITY; i++) {
            dataLAs.push_back(stubAction);
        }

        for (int i = 0; i < MAX_GA_QUANTITY; i++) {
            dataGAs.push_back(stubAction);
        }

        if ((dataLAs.size() != MAX_LA_QUANTITY) || (dataGAs.size() != MAX_GA_QUANTITY)) {
            LOG_ERROR("Didn't populate mirror action data vectors with the right amount of stubs");
            return false;
        }

        m_isInitialized = true;

        LOG_INFO("ActionMirrorController initialized");
        return m_isInitialized;
    }

    bool ActionMirrorController::getAction(int localOrGlobal, uint32_t actionID,
                                                        actionData_t* recepient) const {
        
        LOG_WARN("This function is not implemented yet - returning false...");
        return false;
    }

    bool ActionMirrorController::getAgentData(int localOrGlobal, uint32_t agentID,
                                        int actionNumber, actionData_t* recepient) const {

        LOG_WARN("This function is not implemented yet - returning false...");
        return false;
    }

    bool ColdDataControllerLA::getAgentData(uint32_t agentID, 
                                            LA::coldData_t* recepient) const {
        
        LOG_WARN("This function is not implemented yet - returning false...");
        return false;
    }

    bool StateControllerLA::getAgentData(uint32_t agentID,
        LA::stateData_t* recepient) const {

        LOG_WARN("This function is not implemented yet - returning false...");
        return false;
    }

    bool DecisionSystemLA::getAgentData(uint32_t agentID,
        LA::decisionData_t* recepient) const {

        LOG_WARN("This function is not implemented yet - returning false...");
        return false;
    }

    bool ColdDataControllerGA::getAgentData(uint32_t agentID,
        GA::coldData_t* recepient) const {

        LOG_WARN("This function is not implemented yet - returning false...");
        return false;
    }

    bool StateControllerGA::getAgentData(uint32_t agentID,
        GA::stateData_t* recepient) const {

        LOG_WARN("This function is not implemented yet - returning false...");
        return false;
    }

    bool DecisionSystemGA::getAgentData(uint32_t agentID,
        GA::decisionData_t* recepient) const {

        LOG_WARN("This function is not implemented yet - returning false...");
        return false;
    }
}
