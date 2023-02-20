#pragma once

#include "logAPI.hpp"

#include "miscStdHeaders.h"

#define AZ_MAX_FP_AQUISITION_TRIES 1000

#pragma warning(push) //pop at end of file
#pragma warning(disable : 4996) //TODO: change for safe functions
namespace AZ {
	
    //Opens a file for writting. If shouldOverwrite, will, no questions asked.
    //If not, will instead find the next integer which appendeding at the end of name
    //makes it unique (tries at most AZ_MAX_FP_AQUISITION_TRIES times).
    //NOTE: searches for delimiter ".", will leave without extension if not found.
	static FILE* acquireFilePointerToSave(std::string name, bool shouldOverwrite, 
		                                                   std::string filePath) {

        name = filePath + name;

        FILE* fp = fopen(name.c_str(), "r");

        if (fp == NULL) {
            return fopen(name.c_str(), "w");
        }
        else if (shouldOverwrite) {
            //Will overwrite existing file
            return fopen(name.c_str(), "w");
        }
    
        //File already exists and shouldn't overwrite. 
        //Will append a number to the end of the name, so we need to keep name in two parts:
        std::string tempName = "";
        int i = 0;
        const char delim = '.';
        while ( (name.c_str()[i] != '\0') && (name.c_str()[i] != delim)) {
            tempName += name.c_str()[i];
            i++;
        }

        std::string restOfName = "";
        while (name.c_str()[i] != '\0') {
            restOfName += name.c_str()[i];
            i++;
        }
   
        std::string newName;

        //appends the number to end of name. Stars trying at *2*
        int tries = 0;
        int append = 1;
        while ((fp != NULL) && (tries < AZ_MAX_FP_AQUISITION_TRIES)) {
            fclose(fp);

            append++;
            newName = tempName + std::to_string(append) + restOfName;

            fp = fopen(newName.c_str(), "r");

            tries++;
        }
        
        return fopen(newName.c_str(), "w");
	}
}
#pragma warning(pop)
