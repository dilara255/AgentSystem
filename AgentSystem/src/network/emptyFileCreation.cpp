#include "miscStdHeaders.h"

#include "logAPI.hpp"

#include "network/fixedParameters.hpp"
#include "network/fileFormat.hpp"

int createEmptyNetworkFile(std::string name, std::string comment, int numberLAs,
    int numberGAs, int maxNeighbors, int maxActions, bool setDefaults) {

    LOG_TRACE("Creating new Network File");

    bool isWithinBounds = (numberLAs <= MAX_LA_QUANTITY) && (numberGAs <= MAX_GA_QUANTITY)
        && (maxNeighbors <= MAX_LA_NEIGHBOURS) && (maxActions <= MAX_ACTIONS_PER_AGENT);

    if (!isWithinBounds) {
        LOG_WARN("Parameters are out of the systems bounds, won't create file");
        return 0;
    }

    FILE* fp;

    name = defaultFilePath + name;
    fp = fopen(name.c_str(), "r");
    if (fp != NULL) {
        LOG_ERROR("File name already exists, aborting creation...");
        fclose(fp);
        return 0;
    }

    fp = fopen(name.c_str(), "w");
    if (fp == NULL) {
        LOG_ERROR("Couldn't create the file (check if folders exist), aborting creation...");
        return 0;
    }

    int result = 1;
    int resultAux = 0;

    //Header, with version control, network sizes and comment
    resultAux = fprintf(fp, headerLine,
        FILE_FORMAT_VERSION, numberGAs, numberLAs, maxNeighbors, maxActions, comment.c_str());

    result *= (resultAux > 0); //fprintf returns negative number on error

    if (setDefaults) {
        result *= insertGAsWithDefaults(numberGAs, fp);
        result *= insertLAsWithDefaults(numberLAs, maxNeighbors, numberGAs, fp);
        result *= insertActionsWithDefaults(numberLAs, numberGAs, maxActions, fp);
    }
    else {
        result *= insertGAsWithoutDefaults(numberGAs, fp);
        result *= insertLAsWithoutDefaults(numberLAs, maxNeighbors, fp);
        result *= insertActionsWithDefaults(numberLAs, numberGAs, maxActions, fp);
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

    for (int i = 0; i < (numberGAs - 1); i++) {
        std::string name = defaultGAnamePrefix;
        name += std::to_string(i);
        resultAux = fprintf(fp, GAidentity, i, name.c_str(), DEFAULT_ONOFF);
        if (resultAux <= 0) result = 0;

        resultAux = fprintf(fp, GAresources, DEFAULT_GA_RESOURCES);
        if (resultAux <= 0) result = 0;

        resultAux = fprintf(fp, connectedLAbitfield, 0, 0, 0, 0);
        if (resultAux <= 0) result = 0;

        //By default, all GAs are connected (except for the last, which is a dummy)
        int defaultConnectedGAs = (int)(pow(2, numberGAs - 1) - 1) & (~(1 << i));
        resultAux = fprintf(fp, connectedGAbitfield, defaultConnectedGAs);
        if (resultAux <= 0) result = 0;

        for (int j = 0; j < (numberGAs - 1); j++) {
            if (j != i) {
                resultAux = fprintf(fp, GArelationsInfo, j,
                    DEFAULT_GA_STANCE, DEFAULT_GA_DISPOSITION);
                if (resultAux <= 0) result = 0;
            }
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

        resultAux = fputs(GAresources, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(connectedLAbitfield, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(connectedGAbitfield, fp);
        if (resultAux == EOF) result = 0;

        for (int j = 0; j < (numberGAs - 1); j++) {
            if (j != i) {
                resultAux = fprintf(fp, GArelationsInfo, j, 0, 0);
                if (resultAux <= 0) result = 0;
            }
        }
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
        std::string name = defaultLAnamePrefix;
        name += std::to_string(i);
        resultAux = fprintf(fp, LAidentity, i, name.c_str(), numberGAs - 1, DEFAULT_ONOFF);
        if (resultAux <= 0) result = 0;

        int lineLenght = DEFAULT_LA_DISTANCE * (DEFAULT_LAs_PER_LINE);
        float x = (float)((i * DEFAULT_LA_DISTANCE) % lineLenght);
        float y = DEFAULT_LA_DISTANCE * (float)((i * DEFAULT_LA_DISTANCE) / lineLenght);
        resultAux = fprintf(fp, LAposition, x, y);
        if (resultAux <= 0) result = 0;

        resultAux = fprintf(fp, LAstrenght, DEFAULT_LA_STRENGHT,
            DEFAULT_LA_STR_THRESHOLD_FOR_UPKEEP);
        if (resultAux <= 0) result = 0;

        float upkeep = 0;
        if (DEFAULT_LA_STRENGHT > DEFAULT_LA_STR_THRESHOLD_FOR_UPKEEP) {
            upkeep = (DEFAULT_LA_STRENGHT - DEFAULT_LA_STR_THRESHOLD_FOR_UPKEEP);
            upkeep *= DEFAULT_LA_UPKEEP_PER_STRENGHT;
        }
        resultAux = fprintf(fp, LAresources, DEFAULT_LA_RESOURCES, DEFAULT_LA_INCOME,
            upkeep);
        if (resultAux <= 0) result = 0;

        resultAux = fprintf(fp, connectedLAbitfield, 0, 0, 0, 0);
        if (resultAux <= 0) result = 0;

        for (int j = 0; j < maxNeighbors; j++) {
            resultAux = fprintf(fp, LArelationsInfo, j,
                DEFAULT_LA_STANCE, DEFAULT_LA_DISPOSITION);
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

        resultAux = fputs(LAposition, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(LAstrenght, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(LAresources, fp);
        if (resultAux == EOF) result = 0;

        resultAux = fputs(connectedLAbitfield, fp);
        if (resultAux == EOF) result = 0;

        for (int j = 0; j < maxNeighbors; j++) {
                resultAux = fprintf(fp, LArelationsInfo, j, 0, 0);
                if (resultAux <= 0) result = 0;
        }
    }

    return result;
}

int insertActionsWithDefaults(int numberLAs, int numberGAs, int maxActions, FILE* fp) {
    int result = 1;
    int resultAux;

    resultAux = fputs(GAactionsSectionTittle, fp);
    if (resultAux == EOF) result = 0;

    int totalActions = (numberGAs - 1) * maxActions;
    for (int i = 0; i < totalActions; i++) {
        resultAux = fprintf(fp, GAaction, i, i / (maxActions));
        if (resultAux < 0) result = 0;
    }

    resultAux = fputs(LAactionsSectionTittle, fp);
    if (resultAux == EOF) result = 0;
    totalActions = (numberLAs)*maxActions;
    for (int i = 0; i < totalActions; i++) {
        resultAux = fprintf(fp, LAaction, i, i / (maxActions));
        if (resultAux < 0) result = 0;
    }

    return result;
}

int insertActionsWithoutDefaults(int numberLAs, int numberGAs, int maxActions, FILE* fp) {
    int result = 1;
    int resultAux;

    resultAux = fputs(GAactionsSectionTittle, fp);
    if (resultAux == EOF) result = 0;

    int totalActions = (numberGAs - 1) * maxActions;
    for (int i = 0; i < totalActions; i++) {
        resultAux = fprintf(fp, GAaction, i, i / (maxActions));
        if (resultAux < 0) result = 0;
    }

    resultAux = fputs(LAactionsSectionTittle, fp);
    if (resultAux == EOF) result = 0;
    totalActions = (numberLAs)*maxActions;
    for (int i = 0; i < totalActions; i++) {
        resultAux = fprintf(fp, LAaction, i, i / (maxActions));
        if (resultAux < 0) result = 0;
    }

    return result;
}