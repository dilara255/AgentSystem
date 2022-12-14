/*
Handles loading of network files. Initially only from .txt files with the right format.

NOTE: network specificed MUST be within confines of the fixed parameters (see includes)

WARNING: loading includes discarding any active networks. Asking to save them first, if
needed, is to be handled by the application, at least for now.

TO DO: should get the file name into the "network name" from the parameters file.
(same with comment)
*/

#include "miscStdHeaders.h"

#include "logAPI.hpp"

#include "systems/AScoordinator.hpp"
#include "network/fileManager.hpp"
#include "systems/actionSystem.hpp"

#include "network/parameters.hpp"
#include "network/fileFormat.hpp"

#include "data/agentDataControllers.hpp"

bool loadHeaderFromFp(FILE* fp, AS::networkParameters_t* pp) {
    
    LOG_TRACE("Will load the information from file's header...");
    int version;

    bool result = true;
    int tokensRead;

    tokensRead = fscanf(fp, headerLine, &version, &pp->numberGAs, &pp->numberLAs,
        &pp->maxLAneighbours, &pp->maxActions);
    result &= (tokensRead == 5); //TO DO: maybe bundle the number of tokens with the format?

    LOG_TRACE("Will load the comment line...");

    char tempComment[COMMENT_LENGHT];
    fgets(tempComment, COMMENT_LENGHT, fp);
    sscanf(tempComment, commentLine, pp->comment); //to get rid of the "initial #" in the format

    char separatorRead[COMMENT_LENGHT]; //will store a separator used after the comment line

    LOG_TRACE("And check if all of it was consumed...");
    //if the comment line wasn't consumed to the end, this will load the rest of it
    //instead of the expected separator
    tokensRead = fscanf(fp, commentSeparatorFormat, separatorRead);
    result &= (tokensRead == 1);
    if (!result) {
        LOG_ERROR("Failed to read part of the header. Will abort file loading.");
    }

#ifdef DEBUG
    printf("\nSeparator read:      %s\n", separatorRead);
#endif // DEBUG   

    //which fails this test*, pointing to the error
    //*unless the comment line goes out of it's way to make this work
    char expectedSeparator[COMMENT_LENGHT];
    sscanf(commentSeparator, commentSeparatorFormat, expectedSeparator);

#ifdef DEBUG
    printf("Expected separator: %s\n\n", expectedSeparator);
#endif // DEBUG   

    if (strcmp(separatorRead, expectedSeparator) != 0) {
        LOG_ERROR("Didn't properly consume the comment line. Aborting load");
        return false;
    }

    LOG_TRACE("File header information loaded.");
    return true;
}

bool addGAfromFile(int id, FILE* fp, AS::dataControllerPointers_t* dp, int numEffectiveGAs) {
    //TO DO: extract functions

    int tokens; //to test how many have been read in each call to scanf

    GA::coldData_t cold;
    GA::stateData_t state;
    GA::decisionData_t decision;

    int onOff = -99;//TO DO: set a "TEST_INTEGER_VALUE" or something?

    tokens = fscanf(fp, GAidentity, &cold.id, &onOff);
    if (tokens != 2 ) {
        LOG_ERROR("Error reading identity tokens from GA. Aborting load.");
#ifdef DEBUG
        printf("GA: %i, Tokens read = %i, Cold.id = %i, onOff = %i\n\n", 
                id, tokens, cold.id, onOff);
#endif // DEBUG
        return false;
    }
    state.onOff = onOff;

    //Load name. Will check to make sure it's not too long
    size_t maxLenght = strlen(GAname) - 2 + NAME_LENGHT; //-2 to account for the "%s" (becomes "name")
    char buffer[COMMENT_LENGHT]; //should always be much longer, maybe set some "BUFFER_LENGHT"?
    fgets(buffer, COMMENT_LENGHT, fp); //buffer has the name line
 
    if (strlen(buffer) > maxLenght) {
        LOG_ERROR("GA Name line too long. Will abort loading.");
        return false;
    }

    tokens = sscanf(buffer, GAname, cold.name);
    if (tokens != 1) {
        LOG_ERROR("Error reading the name of the GA. Aborting load.");
        return false;
    }

    tokens = fscanf(fp, GApersonality, &decision.personality[0],
                    &decision.personality[1], &decision.personality[2], 
                    &decision.personality[3]);
    if (tokens != 4) {
        LOG_ERROR("Error reading personality tokens from GA. Aborting load.");
        return false;
    }

    tokens = fscanf(fp, GAresources, &state.parameters.GAresources);
    if (tokens != 1) {
        LOG_ERROR("Error reading resource tokens from GA. Aborting load.");
        return false;
    }
    
    if (state.localAgentsBelongingToThis.getNumberOfBlocks() != GA_CONNECTED_LA_FIELDS) {
        LOG_ERROR("File incompatible with FlagField Class used (different number of blocks). Aborting load.");
        return false;
    }

    uint32_t connectedLAfields[GA_CONNECTED_LA_FIELDS];
    tokens = fscanf(fp, connectedLAbitfield, &connectedLAfields[0], &connectedLAfields[1],
                        &connectedLAfields[2], &connectedLAfields[3]);
    if (tokens != GA_CONNECTED_LA_FIELDS) {
        LOG_ERROR("Error reading LA connection tokens from GA. Aborting load.");
        return false;
    }
    for (int i = 0; i < GA_CONNECTED_LA_FIELDS; i++) {
        state.localAgentsBelongingToThis.loadField(connectedLAfields[i], i);
    }
    
    uint32_t connectedGAsFlagField;
    tokens = fscanf(fp, connectedGAbitfield, &connectedGAsFlagField);
    if (tokens != 1) {
        LOG_ERROR("Error reading GA connection tokens from GA. Aborting load.");
        return false;
    }
    state.connectedGAs.loadField(connectedGAsFlagField);

    for (int i = 0; i < numEffectiveGAs; i++) {
        if (i != id) {
            int otherId, stance;
            float disposition, dispositionLastStep, infiltration;
            
            tokens = fscanf(fp, GArelationsInfo, &otherId, &stance, 
                            &disposition, &dispositionLastStep, &infiltration);
            if (tokens != 5) {
                LOG_ERROR("Error reading GA relation tokens. Aborting load.");
                return false;
            }
            if (otherId != i) {
                LOG_ERROR("Expected data relating to one GA but read from another. Aborting load.");
                return false;
            }

            state.relations.diplomaticStanceToNeighbors[otherId] = stance;
            state.relations.dispositionToNeighbors[otherId] = disposition;
            state.relations.dispositionToNeighborsLastStep[otherId] = dispositionLastStep;
            decision.infiltration[otherId] = infiltration;
        }
    }

    dp->GAcoldData_ptr->addAgentData(cold);
    dp->GAstate_ptr->addAgentData(state);
    dp->GAdecision_ptr->addAgentData(decision);

    return true;
}

bool setLAneighbourIDsAndFirst(AS::LAlocationAndConnectionData_t* data_ptr, int numberLAs) {
    int neighboursFound = 0;
    
    uint32_t i = 0;
    while ( (neighboursFound < data_ptr->numberConnectedNeighbors) 
                                      && (i < (uint32_t)numberLAs) ) {
        if (data_ptr->connectedNeighbors.isBitOn(i)) {
            data_ptr->neighbourIDs[neighboursFound] = i;
            neighboursFound++;
        }
        i++;
    }
    data_ptr->firstConnectedNeighborId = data_ptr->neighbourIDs[0];

    if (neighboursFound < data_ptr->numberConnectedNeighbors) {
        LOG_ERROR("Found less neighbours than expected when updating LA connection data!");
#ifdef DEBUG
        printf("\nFIELDS: %i, %i, %i, %i", data_ptr->connectedNeighbors.getField(0),
                                            data_ptr->connectedNeighbors.getField(1),
                                            data_ptr->connectedNeighbors.getField(2),
                                            data_ptr->connectedNeighbors.getField(3));
        printf("\nID first: %i, ID[0]: %i, ID[1] %i, ID[2] %i, ID[3]: %i, ID[%i]: %i",
                                data_ptr->firstConnectedNeighborId,
                                data_ptr->neighbourIDs[0],
                                data_ptr->neighbourIDs[1],
                                data_ptr->neighbourIDs[2],
                                data_ptr->neighbourIDs[3],
                                data_ptr->numberConnectedNeighbors - 1,
                                data_ptr->neighbourIDs[data_ptr->numberConnectedNeighbors - 1]);        
#endif // DEBUG
        return false;
    }
    
    return true;
}

bool addLAfromFile(int id, FILE* fp, AS::dataControllerPointers_t* dp, int maxNeighbours, 
                                                                            int numberLAs) {
    //TO DO: extract functions

    int tokens; //to test how many have been read in each call to scanf

    LA::coldData_t cold;
    LA::stateData_t state;
    LA::decisionData_t decision;

    int onOff = -99;//TO DO: set a "TEST_INTEGER_VALUE" or something?

    tokens = fscanf(fp, LAidentity, &cold.id, &state.GAid, &onOff);
    if (tokens != 3) {
        LOG_ERROR("Error reading identity tokens from LA. Aborting load.");
#ifdef DEBUG
        printf("LA: %i, Tokens read = %i, Cold.id = %i, state.GAid = %i, onOff = %i\n\n",
            id, tokens, cold.id, state.GAid, onOff);
#endif // DEBUG
        return false;
    }
    state.onOff = onOff;

    //Load name. Will check to make sure it's not too long
    size_t maxLenght = strlen(LAname) - 2 + NAME_LENGHT; //-2 to account for the "%s" (becomes "name")
    char buffer[COMMENT_LENGHT]; //should always be much longer, maybe set some "BUFFER_LENGHT"?
    fgets(buffer, COMMENT_LENGHT, fp); //buffer has the name line

    if (strlen(buffer) > maxLenght) {
        LOG_ERROR("LA Name line too long. Will abort loading.");
        return false;
    }

    tokens = sscanf(buffer, LAname, cold.name);
    if (tokens != 1) {
        LOG_ERROR("Error reading the name of the LA. Aborting load.");
        return false;
    }

    tokens = fscanf(fp, LAposition, &state.locationAndConnections.position.x,
                                    &state.locationAndConnections.position.y);
    if (tokens != 2) {
        LOG_ERROR("Error reading position tokens from GA. Aborting load.");
        return false;
    }
    

    tokens = fscanf(fp, LAstrenght, &state.parameters.strenght.current,
                                    &state.parameters.strenght.externalGuard,
                                    &state.parameters.strenght.thresholdToCostUpkeep);
    if (tokens != 3) {
        LOG_ERROR("Error reading resource tokens from GA. Aborting load.");
        return false;
    }

    tokens = fscanf(fp, LAresources, &state.parameters.resources.current,
        &state.parameters.resources.updateRate, &state.parameters.strenght.currentUpkeep);
    if (tokens != 3) {
        LOG_ERROR("Error reading resource tokens from GA. Aborting load.");
        return false;
    }

    //GA_CONNECTED_LA_FIELDS is also the number of blocks to hold flags for all LAs
    if (state.locationAndConnections.connectedNeighbors.getNumberOfBlocks() 
                                                 != GA_CONNECTED_LA_FIELDS) {
        LOG_ERROR("Format incompatible with FlagField Class used (different number of blocks). Aborting load.");
        return false;
    }

    uint32_t connectedLAfields[GA_CONNECTED_LA_FIELDS];
    tokens = fscanf(fp, connectedLAbitfield, &connectedLAfields[0], &connectedLAfields[1],
        &connectedLAfields[2], &connectedLAfields[3]);
    if (tokens != GA_CONNECTED_LA_FIELDS) {
        LOG_ERROR("Error reading LA connection tokens from LA. Aborting load.");
        return false;
    }
    for (int i = 0; i < GA_CONNECTED_LA_FIELDS; i++) {
        state.locationAndConnections.connectedNeighbors.loadField(connectedLAfields[i], i);
    }

    int connections = state.locationAndConnections.connectedNeighbors.howManyAreOn();
    state.locationAndConnections.numberConnectedNeighbors = connections;

    setLAneighbourIDsAndFirst(&state.locationAndConnections, numberLAs);

    for (int i = 0; i < connections; i++) {
        int otherId, stance;
        float disposition, infiltration;

        tokens = fscanf(fp, LArelationsInfo, &otherId, &stance,
                        &disposition, &infiltration);
        if (tokens != 4) {
            LOG_ERROR("Error reading LA relation tokens. Will Abort loading.");
            return false;
        }
        if (otherId != i) {
            LOG_ERROR("Expected data relating to one GA but read from another.Will Abort loading.");
            return false;
        }

        state.relations.diplomaticStanceToNeighbors[otherId] = stance;
        state.relations.dispositionToNeighbors[otherId] = disposition;
        decision.infiltration[otherId] = infiltration;
    }

    fscanf(fp, LAoffsetsTitle);

    constexpr int localAndGlobal = 2;
    float offsets[AS::TOTAL_CATEGORIES][AS::TOTAL_MODES][localAndGlobal];

    for (int i = 0; i < AS::TOTAL_CATEGORIES; i++) {
        int category;
        tokens = fscanf(fp, LAcategoryOffsets, &category,
                                               &offsets[i][0][0], &offsets[i][0][1],
                                               &offsets[i][1][0], &offsets[i][1][1],
                                               &offsets[i][2][0], &offsets[i][2][1]);
    }
    if (tokens != (AS::TOTAL_MODES*localAndGlobal + 1) ) {
        LOG_ERROR("Error reading LA offset tokens. Will Abort loading.");
        return false;
    }

    for (int i = 0; i < AS::TOTAL_CATEGORIES; i++) {
        for (int j = 0; j < AS::TOTAL_MODES; j++) {
            decision.offsets.personality[i][j] = offsets[i][j][AS::LOCAL];
            decision.offsets.incentivesAndConstraintsFromGA[i][j] 
                                                          = offsets[i][j][AS::GLOBAL];
        }
    }

    dp->LAcoldData_ptr->addAgentData(cold);
    dp->LAstate_ptr->addAgentData(state);
    dp->LAdecision_ptr->addAgentData(decision);

    return true;
}

bool addGAactionFromFile(int id, FILE* fp, AS::ActionDataController* ap) {
    
    AS::actionData_t action;

    int actionID, agentID;
    int tokens = fscanf(fp, GAaction, &actionID, &agentID, &action.ids, 
                                      &action.ticks.initial, &action.ticks.lastProcessed, 
                                      &action.details.intensity, &action.details.processingAux);
    if (tokens != 7) {
        LOG_ERROR("Error reading GA Action tokens from file. Aborting load.");
        return false;
    }

    return ap->addActionData(action);
}

bool addLAactionFromFile(int id, FILE* fp, AS::ActionDataController* ap) {
   
    AS::actionData_t action;

    int actionID, agentID;
    int tokens = fscanf(fp, LAaction, &actionID, &agentID, &action.ids,
                                      &action.ticks.initial, &action.ticks.lastProcessed,
                                      &action.details.intensity, &action.details.processingAux);
    if (tokens != 7) {
        LOG_ERROR("Error reading GA Action tokens from file. Aborting load.");
        return false;
    }

    return ap->addActionData(action);
}

//TO DO: extect functions?
bool loadDataFromFp(FILE* fp, AS::networkParameters_t* pp, AS::dataControllerPointers_t* dp,
                                                               AS::ActionDataController* ap) {
    bool result;

    LOG_TRACE("Will load Agent and Action Data...");

    char buffer[COMMENT_LENGHT];
    fgets(buffer, COMMENT_LENGHT, fp);

    //Since fgets has no easy way of checking wether we reached a \n or COMMENT_LENGHT:
    //We'll check a "comment separator line" to see if the comment was properly red:

    if (strcmp(buffer, GAsectiontittle) != 0) { //try again (don't abort because of trailing newline)
#ifdef DEBUG
        printf("\n\nLeu %s em vez de \"%s\"\nVamos tentar mais uma linha...",
            buffer, GAsectiontittle);
#endif // DEBUG
        fgets(buffer, COMMENT_LENGHT, fp);
        if (strcmp(buffer, GAsectiontittle) != 0) { //two failures -> abort, format should be wrong
            LOG_ERROR("Start of GA data section is not where it`s expected. Aborting...");
            return false;
        }
    }    

    LOG_TRACE("Loading Global Agent Data...");

    int numEffectiveGAs = pp->numberGAs - 1;//one GA is left to represent "belongs to no GA"
    for (int i = 0; i < numEffectiveGAs; i++) {
        result = addGAfromFile(i, fp, dp, numEffectiveGAs);
        if (!result) break;
    }
    if (!result) {
        LOG_ERROR("Couldn't load all GAs. Aborting");
        return false;
    }

    LOG_TRACE("Global Agent Data Loaded. Will load Local Agent Data...");

    fscanf(fp, lastGAwarning);
    fscanf(fp, LAsectiontittle);

    for (int i = 0; i < pp->numberLAs; i++) {
        result = addLAfromFile(i, fp, dp, pp->maxLAneighbours, pp->numberLAs);

        if (!result) break;
    }
    if (!result) {
        LOG_ERROR("Couldn't load all LAs. Aborting");
        return false;
    }

    LOG_TRACE("Local Agent Data loaded. Will load Action Data...");
    
    fscanf(fp, GAactionsSectionTittle);

    for (int i = 0; i < numEffectiveGAs * pp->maxActions; i++) {
        result = addGAactionFromFile(i, fp, ap);
       
        if (!result) break;
    }
    if (!result) {
        LOG_ERROR("Couldn't load all GA actions. Aborting");
        return false;
    }

    LOG_TRACE("Local Actions loaded. Will load Global Action Data...");

    fscanf(fp, LAactionsSectionTittle);

    for (int i = 0; i < pp->numberLAs * pp->maxActions; i++) {
        result = addLAactionFromFile(i, fp, ap);

        if (!result) break;
    }
    if (!result) {
        LOG_ERROR("Couldn't load all LA actions. Aborting");
        return false;
    }

    LOG_TRACE("Local and Global Agent DATA and ACTIONS loaded.");
    return true;
}

bool AS::loadNetworkFromFileToDataControllers(FILE* fp, 
                                dataControllerPointers_t agentDataControllers, 
                                networkParameters_t* currentNetworkParams_ptr, 
                                ActionDataController* actionDataController_ptr) {    
    LOG_TRACE("Will rewind the file pointer to begin loading");
    rewind(fp);

    bool result = loadHeaderFromFp(fp, currentNetworkParams_ptr);
    if (!result) {
        LOG_ERROR("Couldn't read all tokens from the file header area. Aborting load");
        return false;
    }
    LOG_TRACE("Header read, parameters updated. Will load data."); 

    actionDataController_ptr->setMaxActionsPerAgent(currentNetworkParams_ptr->maxActions);
    
    result = loadDataFromFp(fp, currentNetworkParams_ptr, &agentDataControllers,
                                                          actionDataController_ptr);
    if (!result) {
        LOG_ERROR("Couldn't load data, loading aborted");
        return false;
    }
    else {
        LOG_INFO("File Loaded.");
        return true;
    }   
}

FILE* AS::acquireFilePointerToLoad(std::string name) {
    name = defaultFilePath + name;

    return fopen(name.c_str(), "r");
}

bool AS::fileIsCompatible(FILE* fp) {
    
    LOG_TRACE("Will check wether the file is compatible...");

    int version, GAs, LAs, maxNeighbours, maxActions;

    int tokens = fscanf(fp, headerLine, &version, &GAs, &LAs, &maxNeighbours, &maxActions);
    if (tokens != 5) {
        LOG_ERROR("Couldn't read all tokens from header to validade file format. Aborting load.");
        return false;
    }

    if (version != FILE_FORMAT_VERSION) {
        LOG_ERROR("Incompatible file version, aborting load");
        return false;
    }

    bool withinBounds = (GAs <= MAX_GA_QUANTITY) && (LAs <= MAX_LA_QUANTITY)
        && (maxNeighbours <= MAX_LA_NEIGHBOURS) && (maxActions <= MAX_ACTIONS_PER_AGENT);

    if (!withinBounds) {
        LOG_ERROR("The file's network size is not within the fixed bounds");
        return false;
    }

    LOG_TRACE("File seems compatible, will procceed loading...");
    return true;
}