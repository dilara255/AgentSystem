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
#include "fileManager.hpp"

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

    fgets(pp->comment, COMMENT_LENGHT, fp);

    char readSeparator[COMMENT_LENGHT]; //will store a separator used after the comment line

    LOG_TRACE("And check if all of it was consumed...");
    //if the comment line wasn't consumed to the end, this will load the rest of it
    //instead of the expected separator
    tokensRead = fscanf(fp, "%s", readSeparator);
    result &= (tokensRead == 1);

    //which USUALLY fails this test, pointing to the error
    if (readSeparator[0] != commentSeparator[0]) {
        LOG_ERROR("Didn't properly consume the comment line. Aborting load");
        return false;
    } //TO DO: this test is kinda bad (can fail with an strategically placed "!" in the
      //current version).

    if (!result) {
        LOG_ERROR("Header not properly loaded. Will abort file loading.");
    }
    else {
        LOG_TRACE("File header information loaded.");
    }

    return result;
}

bool addGAfromFile() {
    return true;
}

bool addLAfromFile() {
    return true;
}

bool addGAactionFromFile() {
    return true;
}

bool addLAactionFromFile() {
    return true;
}

bool loadDataFromFp(FILE* fp, AS::networkParameters_t* pp, AS::dataControllerPointers_t* dp) {
    bool result;

    LOG_TRACE("Will load the actual network data...");

    fscanf(fp, GAsectiontittle);

    for (int i = 0; i < pp->numberGAs; i++) {
        result = addGAfromFile();
        if (!result) break;
    }
    if (!result) {
        LOG_ERROR("Couldn't load all GAs. Aborting");
        return false;
    }

    fscanf(fp, LAsectiontittle);

    for (int i = 0; i < pp->numberLAs; i++) {
        result = addLAfromFile();

        if (!result) break;
    }
    if (!result) {
        LOG_ERROR("Couldn't load all LAs. Aborting");
        return false;
    }
    
    fscanf(fp, GAactionsSectionTittle);

    for (int i = 0; i < pp->numberGAs * pp->maxActions; i++) {
        result = addGAactionFromFile();
       
        if (!result) break;
    }
    if (!result) {
        LOG_ERROR("Couldn't load all GA actions. Aborting");
        return false;
    }

    fscanf(fp, LAactionsSectionTittle);

    for (int i = 0; i < pp->numberLAs * pp->maxActions; i++) {
        result = addLAactionFromFile();

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
                               networkParameters_t currentNetworkParams) {
    
    LOG_TRACE("Will rewind the file pointer to begin loading");
    rewind(fp);

    bool result = loadHeaderFromFp(fp, &currentNetworkParams);
    if (!result) {
        LOG_ERROR("Couldn't read all tokens from the file header area. Aborting load (will clear network)");
        AS::clearNetwork();
        return false;
    }
    LOG_TRACE("Header read, parameters updated. Will load data.");

    result = loadDataFromFp(fp, &currentNetworkParams, &agentDataControllers);    
    if (!result) {
        LOG_ERROR("Loading aborted - will clear network");
        AS::clearNetwork();
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