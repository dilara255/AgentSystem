#pragma once

#include "logAPI.hpp"

#include "miscStdHeaders.h"


namespace AZ {
	
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
        int append = 1;
        while (fp != NULL) {
            fclose(fp);

            append++;
            newName = tempName + std::to_string(append) + restOfName;

            fp = fopen(newName.c_str(), "r");
        }
        
        return fopen(newName.c_str(), "w");
	}

}
