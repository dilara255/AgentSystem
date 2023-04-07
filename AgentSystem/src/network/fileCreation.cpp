/*
Handles creation of:
- "Empty" network file, given a few parameters, to be filled in later. Optionally include
some default values, or leave blank with format specifiers;
- "Saved" network, in a new file, taking in a reference to a data structure from which
to get the network data.
- Can save to a default or specified path. By default, doesn't overwrite, but creats new
file with a number appended to the end, but can be set to overwrite.

NOTE: for "empty" file, the network specificed MUST be within bounds of the fixed parameters

TODO: this has quite a bit of repetition. Text files may be palceholder, so this should be
reevaluated once the actual format and save system needs are known.
*/

#include "miscStdHeaders.h"

#include "logAPI.hpp"

#include "systems/AScoordinator.hpp"
#include "data/agentDataControllers.hpp"

#include "network/fileManager.hpp"

#include "data/actionData.hpp"
#include "network/parameters.hpp"
#include "network/fileFormat.hpp"

#include "fileHelpers.h"
#include "prng.hpp"

int AS::createEmptyNetworkFile(std::string fileName, std::string comment, int numberLAs,
    int numberGAs, int maxNeighbors, int maxActions, bool setDefaults, std::string filePath) {
    //TODO: add logic to insert decision data once that's added to the file format

    LOG_TRACE("Creating new Network File");

    bool isWithinBounds = (numberLAs <= MAX_LA_QUANTITY) && (numberGAs <= MAX_GA_QUANTITY)
        && (maxNeighbors <= MAX_LA_NEIGHBOURS) && (maxActions <= MAX_ACTIONS_PER_AGENT);

    if (!isWithinBounds) {
        LOG_WARN("Parameters are out of the systems bounds, won't create file");
        return 0;
    }

    FILE* fp = AS::acquireFilePointerToSave(fileName.c_str(), false, filePath);
    
    if (fp == NULL) {
        LOG_ERROR("Couldn't create the file (check if folders exist), aborting creation...");
        return 0;
    }

    uint64_t tickCount = DEFAULT_TICK_COUNT;
    double totalMultiplier = DEFAULT_TOTAL_MULTIPLIER;
    uint64_t seed0 = DEFAULT_PRNG_SEED0;
    uint64_t seed1 = DEFAULT_PRNG_SEED1;
    uint64_t seed2 = DEFAULT_PRNG_SEED2;
    uint64_t seed3 = DEFAULT_PRNG_SEED3;

    //Header
    int resultAux = 0;
    int result = 1;    

    resultAux = fprintf(fp, headerLine,
        FILE_FORMAT_VERSION, numberGAs, numberLAs, maxNeighbors, maxActions, tickCount,
                                           totalMultiplier, seed0, seed1, seed2, seed3);
    result *= (resultAux > 0); //fprintf returns negative number on error
    
    resultAux = fprintf(fp, commentLine, comment.c_str());
    result *= (resultAux > 0);

    resultAux = fputs(commentSeparator, fp);
    if (resultAux == EOF) result = 0; //fputs returns EOF on error

    if (setDefaults) {
        result *= insertGAsWithDefaults(numberGAs, fp);
        result *= insertLAsWithDefaults(numberLAs, maxNeighbors, numberGAs, fp);
        result *= insertActionsWithDefaults(numberLAs, numberGAs, maxActions, fp);
    }
    else {
        result *= insertGAsWithoutDefaults(numberGAs, fp);
        result *= insertLAsWithoutDefaults(numberLAs, maxNeighbors, fp);
        result *= insertActionsWithoutDefaults(numberLAs, numberGAs, maxActions, fp);
    }

    fclose(fp);

    if (result) {
        LOG_INFO("New Network File Created Sucessfully");
    }
    else {
        LOG_WARN("New Network File Creation Failed after opening the file");
    }

    return result;
}

int insertGAsWithDefaults(int numberGAs, FILE* fp) {
    int result = 1;
    int resultAux;

    resultAux = fputs(GAsectiontittle, fp);
    if (resultAux == EOF) result = 0; //fputs returns EOF on error

    for (int thisGA = 0; thisGA < (numberGAs - 1); thisGA++) {
        
        resultAux = fprintf(fp, GAidentity, thisGA, DEFAULT_ONOFF, DEFAULT_SHOULD_DECIDE);
        if (resultAux <= 0) result = 0;

        std::string name = defaultGAnamePrefix;
        name += std::to_string(thisGA);
        resultAux = fprintf(fp, GAname, name.c_str());
        if (resultAux <= 0) result = 0;

        resultAux = fprintf(fp, GApersonality, DEFAULT_GA_PERSONA_0,
                            DEFAULT_GA_PERSONA_1, DEFAULT_GA_PERSONA_2,
                            DEFAULT_GA_PERSONA_3);
        if (resultAux <= 0) result = 0;

        //DEFAULT: each GA is connected to *connections* LAs
        int connections = TST_NUMBER_LAS / TST_NUMBER_GAS;
        float defaultTotalLAres = DEFAULT_LA_RESOURCES * connections;

        float targetTime = (float)AS_MILLISECONDS_PER_STEP / MILLIS_IN_A_SECOND;

        resultAux = fprintf(fp, GAresources, DEFAULT_GA_RESOURCES, DEFAULT_REQUESTS,
            defaultTotalLAres * GA_TAX_RATE_PER_SECOND * targetTime, DEFAULT_REQUESTS,
            defaultTotalLAres * GA_TAX_RATE_PER_SECOND * TRADE_FACTOR_PER_SECOND * targetTime,
            DEFAULT_REQUESTS);
        if (resultAux <= 0) result = 0;

        float defaultTotalLAincome = DEFAULT_LA_INCOME * connections;
        float defaultTotalLAstrenght = DEFAULT_LA_STRENGHT * connections;
        float defaultTotalLAguard = DEFAULT_REINFORCEMENT * connections;

        resultAux = fprintf(fp, GAtotals, defaultTotalLAres, defaultTotalLAincome, 
                                         defaultTotalLAstrenght, DEFAULT_REQUESTS, 
                                            defaultTotalLAguard, DEFAULT_REQUESTS);
        if (resultAux <= 0) result = 0;
        
        AZ::FlagField128 connectionField;
        for (int j = 0; j < connections; j++) {
            int laID = (thisGA*connections + j) % TST_NUMBER_LAS; //which warps around if necessary   
            connectionField.setBitOn(laID);
        }

        resultAux = fprintf(fp, connectedLAbitfield, connectionField.getField(0),
                                                    connectionField.getField(1), 
                                                    connectionField.getField(2), 
                                                    connectionField.getField(3));
        if (resultAux <= 0) result = 0;

        //By default, all GAs are connected (except for the last, which is a dummy)
        int allButLast = (int)(pow(2, numberGAs - 1) - 1);
        int notThisOne = (~(1 << thisGA));
        int defaultConnectedGAs = allButLast & notThisOne;
        int defaultTotalConnectedGAs = numberGAs - 2; //same idea

        resultAux = fprintf(fp, connectedGAbitfield, defaultConnectedGAs);
        if (resultAux <= 0) result = 0;

        int otherID = 0;
        for (int j = 0; j < defaultTotalConnectedGAs; j++, otherID++) {

            if(otherID == thisGA) {
                otherID++;
            }

            resultAux = fprintf(fp, GArelationsInfo, j, otherID,
                DEFAULT_GA_STANCE, DEFAULT_GA_DISPOSITION, DEFAULT_GA_DISPOSITION,
                                                            DEFAULT_GA_INFILTRATION);
            if (resultAux <= 0){
                result = 0;
            }

            resultAux = fprintf(fp, GAreadsOnNeighbor,
                DEFAULT_GA_RESOURCES, DEFAULT_REQUESTS, 
                defaultTotalLAres * GA_TAX_RATE_PER_SECOND * targetTime, DEFAULT_REQUESTS,
                defaultTotalLAres * GA_TAX_RATE_PER_SECOND * TRADE_FACTOR_PER_SECOND * targetTime,
                DEFAULT_REQUESTS, defaultTotalLAstrenght, DEFAULT_REQUESTS,
                defaultTotalLAguard, DEFAULT_REQUESTS);
            if (resultAux <= 0) result = 0;
        }
    }

    resultAux = fputs(lastGAwarning, fp);
    if (resultAux == EOF) result = 0;

    return result;
}

int insertGAsWithoutDefaults(int numberGAs, FILE* fp) {
    int result = 1;
    int resultAux;

    resultAux = fputs(GAsectiontittle, fp);
    if (resultAux == EOF) result = 0;

    for (int i = 0; i < (numberGAs - 1); i++) {
        resultAux = fputs(GAidentity, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(GAname, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(GApersonality, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(GAresources, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(GAtotals, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(connectedLAbitfield, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(connectedGAbitfield, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(GArelationsInfo, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(GAreadsOnNeighbor, fp);
        if (resultAux == EOF) result = 0;
    }

    resultAux = fputs(lastGAwarning, fp);
    if (resultAux == EOF) result = 0;

    return result;
}

int insertLAsWithDefaults(int numberLAs, int maxNeighbors, int numberGAs, FILE* fp) {
    int result = 1;
    int resultAux;

    resultAux = fputs(LAsectiontittle, fp);
    if (resultAux == EOF) result = 0;

    for (int i = 0; i < numberLAs; i++) {
        
        //Default GA: last effective actual last doesn't count, so numberGAs - 2
        //TODO: REFACTOR: CHANGE numberGAs TO BE THE NUMBER OF EFFECTIVE GAs
        resultAux = fprintf(fp, LAidentity, i, numberGAs - 2, DEFAULT_ONOFF, DEFAULT_SHOULD_DECIDE);
        if (resultAux <= 0) result = 0;

        std::string name = defaultLAnamePrefix;
        name += std::to_string(i);

        resultAux = fprintf(fp, LAname, name.c_str());
        if (resultAux <= 0) result = 0;

        int lineLenght = DEFAULT_LA_DISTANCE * (DEFAULT_LAs_PER_LINE);
        float x = (float)((i * DEFAULT_LA_DISTANCE) % lineLenght);
        float y = DEFAULT_LA_DISTANCE * (float)((i * DEFAULT_LA_DISTANCE) / lineLenght);
        resultAux = fprintf(fp, LAposition, x, y);
        if (resultAux <= 0) result = 0;

        resultAux = fprintf(fp, LAstrenght, DEFAULT_LA_STRENGHT, DEFAULT_REQUESTS, 
                                          DEFAULT_REINFORCEMENT, DEFAULT_REQUESTS,
                                              DEFAULT_LA_STR_THRESHOLD_FOR_UPKEEP);
        if (resultAux <= 0) result = 0;

        float upkeep = 0;
        if (DEFAULT_LA_STRENGHT > DEFAULT_LA_STR_THRESHOLD_FOR_UPKEEP) {
            float guardCost = (float)DEFAULT_REINFORCEMENT*EXTERNAL_GUARD_UPKEEP_RATIO_BY_DEFENDED;
            upkeep = (float)(guardCost + DEFAULT_LA_STRENGHT - DEFAULT_LA_STR_THRESHOLD_FOR_UPKEEP);
            upkeep *= LA_UPKEEP_PER_EXCESS_STRENGHT;
        }

        resultAux = fprintf(fp, LAresources, DEFAULT_LA_RESOURCES, DEFAULT_REQUESTS, 
                                                          DEFAULT_LA_INCOME, upkeep);
        if (resultAux <= 0) result = 0;

        AZ::FlagField128 connectionField;
        int connections = MAX_LA_NEIGHBOURS / DEFAULT_LA_NEIGHBOUR_QUOTIENT;
        //DEFAULT: each LA is connected to the next *connections* LAs after it
        for (int j = 0; j < connections; j++) {
            
            //TODO: extract, repeated bellow
            int other = (i + 1 + j) % numberLAs; //which warps around if necessary   
            connectionField.setBitOn(other);
        }

        resultAux = fprintf(fp, connectedLAbitfield, connectionField.getField(0),
                                                    connectionField.getField(1), 
                                                    connectionField.getField(2), 
                                                    connectionField.getField(3));
        if (resultAux <= 0) result = 0;

        for (int j = 0; j < connections; j++) {

            //TODO: extract, repeated above
            int other = (i + 1 + j) % numberLAs; //which warps around if necessary   

            resultAux = fprintf(fp, LArelationsInfo, j, other,
                DEFAULT_LA_STANCE, DEFAULT_LA_DISPOSITION, DEFAULT_LA_INFILTRATION);
            if (resultAux <= 0) result = 0;

             resultAux = fprintf(fp, LAreadsOnNeighbor, 
                                DEFAULT_LA_RESOURCES, DEFAULT_REQUESTS, 
                                DEFAULT_LA_INCOME,
                                DEFAULT_LA_STRENGHT, DEFAULT_REQUESTS, 
                                DEFAULT_REINFORCEMENT, DEFAULT_REQUESTS);
            if (resultAux <= 0) result = 0;
        }

        resultAux = fputs(LAoffsetsTitle, fp);
        if (resultAux == EOF) result = 0;
     
        for (int i = 0; i < (int)AS::actCategories::TOTAL; i++) {

            resultAux = fprintf(fp, LAcategoryOffsets, i,
                DEFAULT_LA_OFFSET, DEFAULT_LA_OFFSET,
                DEFAULT_LA_OFFSET, DEFAULT_LA_OFFSET,
                DEFAULT_LA_OFFSET, DEFAULT_LA_OFFSET);
            if (resultAux <= 0) result = 0;
        }
    }

    return result;
}

int insertLAsWithoutDefaults(int numberLAs, int maxNeighbors, FILE* fp) {
    int result = 1;
    int resultAux;

    resultAux = fputs(LAsectiontittle, fp);
    if (resultAux == EOF) result = 0;

    for (int i = 0; i < numberLAs; i++) {
        resultAux = fputs(LAidentity, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(LAname, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(LAposition, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(LAstrenght, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(LAresources, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(connectedLAbitfield, fp);
        if (resultAux == EOF) result = 0;

        for (int j = 0; j < maxNeighbors; j++) {

            resultAux = fputs(LArelationsInfo, fp);
            if (resultAux == EOF) result = 0;

            resultAux = fputs(LAreadsOnNeighbor, fp);
            if (resultAux == EOF) result = 0;
        }

        resultAux = fputs(LAoffsetsTitle, fp);
        if (resultAux == EOF) result = 0;

        for (int i = 0; i < (int)AS::actCategories::TOTAL; i++) {

            resultAux = fputs(LAcategoryOffsets, fp);
            if (resultAux == EOF) result = 0;
        }
    }

    return result;
}

int insertActionsWithDefaults(int numberLAs, int numberGAs, int maxActions, FILE* fp) {
    int result = 1;
    int resultAux;

    resultAux = fputs(GAactionsSectionTittle, fp);
    if (resultAux == EOF) result = 0;

    int totalGlobalActions = (numberGAs - 1) * maxActions;

    AS::ids_t actionID; //this is a bitfield
    uint32_t* actionID_ptr = (uint32_t*)&actionID;
    *actionID_ptr = 0; //makes sure sampleID is all zeroes to start
    actionID.scope = (int)AS::scope::GLOBAL; //turns the scope bit to global
    
    for (int i = 0; i < totalGlobalActions; i++) {
        resultAux = fprintf(fp, GAaction, i, i / (maxActions),
                                    actionID, DEFAULT_FIRST_TICK, DEFAULT_LAST_TICK,
                                    DEFAULT_INTENSITY, DEFAULT_ACTION_AUX);
        if (resultAux < 0) result = 0;
    }

    resultAux = fputs(LAactionsSectionTittle, fp);
    if (resultAux == EOF) result = 0;
    
    int totalLocalActions = (numberLAs)*maxActions;

    *actionID_ptr = 0; //makes sure sampleID is all zeroes to start
    actionID.scope = (uint32_t)AS::scope::LOCAL; //turns the scope bit to local

    for (int i = 0; i < totalLocalActions; i++) {
        resultAux = fprintf(fp, LAaction, i, i / (maxActions),
                                    actionID, DEFAULT_FIRST_TICK, DEFAULT_LAST_TICK,
                                    DEFAULT_INTENSITY, DEFAULT_ACTION_AUX);
        if (resultAux < 0) result = 0;
    }

    return result;
}

int insertActionsWithoutDefaults(int numberLAs, int numberGAs, int maxActions, FILE* fp) {
    int result = 1;
    int resultAux;

    resultAux = fputs(GAactionsSectionTittle, fp);
    if (resultAux == EOF) result = 0;

    int totalGlobalActions = (numberGAs - 1) * maxActions;

    for (int i = 0; i < totalGlobalActions; i++) {
        resultAux = fputs(GAaction, fp);
        if (resultAux == EOF) result = 0;
    }

    resultAux = fputs(LAactionsSectionTittle, fp);
    if (resultAux == EOF) result = 0;

    int totalLocalActions = (numberLAs)*maxActions;

    for (int i = 0; i < totalLocalActions; i++) {
        resultAux = fputs(LAaction, fp);
        if (resultAux == EOF) result = 0;
    }

    return result;
}

FILE* AS::acquireFilePointerToSave(std::string name, bool shouldOverwrite, std::string filePath) {
    
    if (filePath == "") {
        name = defaultFilePath + name;
    }
    else {
        name = filePath + name;
    }

    return AZ::acquireFilePointerToSave(name, shouldOverwrite, filePath);
}

bool insertGAsFromNetwork(FILE* fp, const AS::dataControllerPointers_t* dp,
    const AS::networkParameters_t* pp) {

    LOG_TRACE("Will write the GAs to file...");

    int result = 1;
    int resultAux;

    resultAux = fputs(GAsectiontittle, fp);
    if (resultAux == EOF) result = 0; //fputs returns EOF on error

    for (int thisGA = 0; thisGA < (pp->numberGAs - 1); thisGA++) {

        GA::coldData_t cold;
        GA::stateData_t state;
        GA::decisionData_t decision;

        dp->GAcoldData_ptr->getAgentData(thisGA, &cold);
        dp->GAstate_ptr->getAgentData(thisGA, &state);
        dp->GAdecision_ptr->getAgentData(thisGA, &decision);

        resultAux = fprintf(fp, GAidentity, cold.id, state.onOff, decision.shouldMakeDecisions);
        if (resultAux <= 0) result = 0;

        resultAux = fprintf(fp, GAname, cold.name);
        if (resultAux <= 0) result = 0;

        resultAux = fprintf(fp, GApersonality, decision.personality[0],
                            decision.personality[1], decision.personality[2],
                            decision.personality[3]);
        if (resultAux <= 0) result = 0;

        auto expecsSelf = &(decision.requestsForSelf.expected[0]);
        int expecResID = (int)GA::requestExpectations_t::fields::GA_RESOURCES;
        int expecTaxID = (int)GA::requestExpectations_t::fields::TAX_INCOME;
        int expecTradeID = (int)GA::requestExpectations_t::fields::TRADE_INCOME;

        resultAux = fprintf(fp, GAresources, state.parameters.GAresources,
                            expecsSelf[expecResID], state.parameters.lastTaxIncome, 
                            expecsSelf[expecTaxID], state.parameters.lastTradeIncome, 
                            expecsSelf[expecTradeID]);
        if (resultAux <= 0) result = 0;

        int expecStrenghtID = (int)GA::requestExpectations_t::fields::STRENGHT_LAS;
        int expecGuardID = (int)GA::requestExpectations_t::fields::GUARD_LAS;

        resultAux = fprintf(fp, GAtotals, state.parameters.LAesourceTotals.current,
                                          state.parameters.LAesourceTotals.updateRate, 
                                                     state.parameters.LAstrenghtTotal,
                           expecsSelf[expecStrenghtID], state.parameters.LAguardTotal, 
                                                             expecsSelf[expecGuardID]);
        if (resultAux <= 0) result = 0;

        resultAux = fprintf(fp, connectedLAbitfield, 
                            state.localAgentsBelongingToThis.getField(0),
                            state.localAgentsBelongingToThis.getField(1), 
                            state.localAgentsBelongingToThis.getField(2), 
                            state.localAgentsBelongingToThis.getField(3));
        if (resultAux <= 0) result = 0;
       
        resultAux = fprintf(fp, connectedGAbitfield, state.connectedGAs.getField());
        if (resultAux <= 0) result = 0;

        state.connectedGAs.updateHowManyAreOn();
        int totalConnected = state.connectedGAs.howManyAreOn();
        for (int neighbor = 0; neighbor < totalConnected; neighbor++) {

            int neighborID = state.neighbourIDs[neighbor];

            resultAux = fprintf(fp, GArelationsInfo, neighbor, neighborID,
                state.relations.diplomaticStanceToNeighbors[neighbor], 
                state.relations.dispositionToNeighbors[neighbor],
                state.relations.dispositionToNeighborsLastStep[neighbor],
                decision.infiltration[neighbor]);
            if (resultAux <= 0) result = 0;
                
            auto expecsNeighbor = &(decision.requestsForNeighbors[neighbor].expected[0]);
            int readResID = (int)GA::readsOnNeighbor_t::fields::GA_RESOURCES;
            int readTaxID = (int)GA::readsOnNeighbor_t::fields::TAX_INCOME;
            int readTradeID = (int)GA::readsOnNeighbor_t::fields::TRADE_INCOME;
            int readStrenghtID = (int)GA::readsOnNeighbor_t::fields::STRENGHT_LAS;
            int readGuardID = (int)GA::readsOnNeighbor_t::fields::GUARD_LAS;

            resultAux = fprintf(fp, GAreadsOnNeighbor,
                decision.reads[neighbor].readOf[readResID], expecsNeighbor[expecResID], 
                decision.reads[neighbor].readOf[readTaxID], expecsNeighbor[expecTaxID], 
                decision.reads[neighbor].readOf[readTradeID], expecsNeighbor[expecTradeID], 
                decision.reads[neighbor].readOf[readStrenghtID], expecsNeighbor[expecStrenghtID], 
                decision.reads[neighbor].readOf[readGuardID], expecsNeighbor[expecGuardID]);
            if (resultAux <= 0) result = 0;
        }
    }

    resultAux = fputs(lastGAwarning, fp);
    if (resultAux == EOF) result = 0;

    return (bool)result;
}

bool insertLAsFromNetwork(FILE* fp, const AS::dataControllerPointers_t* dp,
    const AS::networkParameters_t* pp) {

    LOG_TRACE("Will write the LAs to file...");

    int result = 1;
    int resultAux;

    resultAux = fputs(LAsectiontittle, fp);
    if (resultAux == EOF) result = 0;

    for (int thisLA = 0; thisLA < pp->numberLAs; thisLA++) {

        LA::coldData_t cold;
        LA::stateData_t state;
        LA::decisionData_t decision;

        dp->LAcoldData_ptr->getAgentData(thisLA, &cold);
        dp->LAstate_ptr->getAgentData(thisLA, &state);
        dp->LAdecision_ptr->getAgentData(thisLA, &decision);

        resultAux = fprintf(fp, LAidentity, cold.id, state.GAid, state.onOff, decision.shouldMakeDecisions);
        if (resultAux <= 0) result = 0;

        resultAux = fprintf(fp, LAname, cold.name);
        if (resultAux <= 0) result = 0;

        resultAux = fprintf(fp, LAposition, state.locationAndConnections.position.x, 
                                            state.locationAndConnections.position.y);
        if (resultAux <= 0) result = 0;

        int expecStrenghtID = (int)LA::requestExpectations_t::fields::STRENGHT;
        int expecGuardID = (int)LA::requestExpectations_t::fields::GUARD;

        resultAux = fprintf(fp, LAstrenght, state.parameters.strenght.current, 
                        decision.requestsForSelf.expected[expecStrenghtID],
                        state.parameters.strenght.externalGuard,
                        decision.requestsForSelf.expected[expecGuardID],
                        state.parameters.strenght.thresholdToCostUpkeep);
        if (resultAux <= 0) result = 0;

        int expecResID = (int)LA::requestExpectations_t::fields::RESOURCES;

        resultAux = fprintf(fp, LAresources, state.parameters.resources.current,
                                  decision.requestsForSelf.expected[expecResID],
                                          state.parameters.resources.updateRate, 
                                        state.parameters.strenght.currentUpkeep);
        if (resultAux <= 0) result = 0;

        resultAux = fprintf(fp, connectedLAbitfield, 
                    state.locationAndConnections.connectedNeighbors.getField(0),
                    state.locationAndConnections.connectedNeighbors.getField(1),
                    state.locationAndConnections.connectedNeighbors.getField(2),
                    state.locationAndConnections.connectedNeighbors.getField(3));
        if (resultAux <= 0) result = 0;

        int totalNeighbors = state.locationAndConnections.numberConnectedNeighbors;
        for (int neighbor = 0; neighbor < totalNeighbors; neighbor++) {
            
            int neighborID = state.locationAndConnections.neighbourIDs[neighbor];

            resultAux = fprintf(fp, LArelationsInfo, neighbor, neighborID,
                                state.relations.diplomaticStanceToNeighbors[neighbor],
                                state.relations.dispositionToNeighbors[neighbor],
                                decision.infiltration[neighbor]);
            if (resultAux <= 0) result = 0;

            auto expecsNeighbor = &(decision.requestsForNeighbors[neighbor].expected[0]);
            auto readsNeighbor = &(decision.reads[neighbor].readOf[0]);

            int readResID = (int)LA::readsOnNeighbor_t::fields::RESOURCES;
            int readIncomeID = (int)LA::readsOnNeighbor_t::fields::INCOME;
            int readStrenghtID = (int)LA::readsOnNeighbor_t::fields::STRENGHT;
            int readGuardID = (int)LA::readsOnNeighbor_t::fields::GUARD;

            resultAux = fprintf(fp, LAreadsOnNeighbor, 
                            readsNeighbor[readResID], expecsNeighbor[expecResID], 
                            readsNeighbor[readIncomeID],
                            readsNeighbor[readStrenghtID], expecsNeighbor[expecStrenghtID], 
                            readsNeighbor[readGuardID], expecsNeighbor[expecGuardID]);
            if (resultAux <= 0) result = 0;
        }

        resultAux = fputs(LAoffsetsTitle, fp);
        if (resultAux == EOF) result = 0;

        for (int i = 0; i < (int)AS::actCategories::TOTAL; i++) {

            resultAux = fprintf(fp, LAcategoryOffsets, i,
                            decision.offsets.personality[i][(int)AS::actModes::IMMEDIATE], 
                            decision.offsets.incentivesAndConstraintsFromGA[i][(int)AS::actModes::IMMEDIATE],
                            decision.offsets.personality[i][(int)AS::actModes::REQUEST],
                            decision.offsets.incentivesAndConstraintsFromGA[i][(int)AS::actModes::REQUEST],
                            decision.offsets.personality[i][(int)AS::actModes::SELF],
                            decision.offsets.incentivesAndConstraintsFromGA[i][(int)AS::actModes::SELF]);
            if (resultAux <= 0) result = 0;
        }
    }

    return (bool)result;
}

bool insertActionsFromNetwork(FILE* fp, const AS::dataControllerPointers_t* dp,
    const AS::networkParameters_t* pp, const AS::ActionDataController* ap) {

    LOG_TRACE("Will write the Actions to file...");
    //TODO: Actually write once the data structures and loading are in place

    int result = 1;
    int resultAux;

    resultAux = fputs(GAactionsSectionTittle, fp);
    if (resultAux == EOF) result = 0;

    int totalGlobalActions = (pp->numberGAs - 1) * pp->maxActions;

    for (int i = 0; i < totalGlobalActions; i++) {
        
        AS::actionData_t action;
        if (!ap->getAction(AS::scope::GLOBAL, i, &action)) { return false; }

        resultAux = fprintf(fp, GAaction, i, i / (pp->maxActions),
                                action.ids, action.ticks.initial, action.ticks.lastProcessed,
                                action.details.intensity, action.details.processingAux);
        if (resultAux < 0) result = 0;
    }

    resultAux = fputs(LAactionsSectionTittle, fp);
    if (resultAux == EOF) result = 0;

    int totalLocalActions = (pp->numberLAs)*pp->maxActions;

    for (int i = 0; i < totalLocalActions; i++) {
        AS::actionData_t action;
        ap->getAction(AS::scope::LOCAL, i, &action);

        resultAux = fprintf(fp, LAaction, i, i / (pp->maxActions),
                                action.ids, action.ticks.initial, action.ticks.lastProcessed,
                                action.details.intensity, action.details.processingAux);
        if (resultAux < 0) result = 0;
    }

    return (bool)result;
}

bool AS::createNetworkFileFromData(FILE* fp,
                            const AS::dataControllerPointers_t* dp,
                            const AS::networkParameters_t* pp,
                            const AS::ActionDataController* ap) {

    bool result = true;
    int resultAux = 0;

    //Header, with version control, network sizes and comment
    resultAux = fprintf(fp, headerLine, FILE_FORMAT_VERSION, pp->numberGAs, 
                        pp->numberLAs, pp->maxLAneighbours, pp->maxActions, pp->mainLoopTicks,
                                                                    pp->accumulatedMultiplier,
                                       pp->seeds[0], pp->seeds[1], pp->seeds[2], pp->seeds[3]);
    result &= (resultAux > 0); //fprintf returns negative number on error

    resultAux = fprintf(fp, commentLine, pp->comment);
    result &= (resultAux > 0);

    resultAux = fputs(commentSeparator, fp);
    if (resultAux == EOF) result = 0; //fputs returns EOF on error

    result &= insertGAsFromNetwork(fp, dp, pp);
    result &= insertLAsFromNetwork(fp, dp, pp);
    result &= insertActionsFromNetwork(fp, dp, pp, ap);

    return result;
}
