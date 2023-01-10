#pragma once

/*TO DO: write parser, build data, insert data, create agent object, add new agent from it.
* Write it all back on a file on this format. See if new agent is really there (one LA, one GA).
* Will need a control structure which knows the amount of LAs and GAs and coordinates controllers?
* 
*TO DO: Write Brief Description of the Format and of this file */

#include "miscStdHeaders.h"

#define FILE_FORMAT_VERSION 1
#define MAX_LINE_LENGHT 500
#define GA_CONNECTED_LA_FIELDS 4

//TO DO: Move all of this into some namespace!
//TO DO: Add decision data to the file format
//WARNING: multiple newlines can trip up loading system! (including start-of-string newlines)
//TO DO: CHECK wether this is eally the case. If so, eliminate it.

static const char* defaultGAnamePrefix = "GA_";
static const char* defaultLAnamePrefix = "LA_";
static const char* defaultFilePath = "networkFiles\\";

int insertGAsWithDefaults(int numberGAs, FILE* fp);
int insertGAsWithoutDefaults(int numberGAs, FILE* fp);
int insertLAsWithDefaults(int numberLAs, int maxNeighbors, int numberGAs, FILE* fp);
int insertLAsWithoutDefaults(int numberLAs, int maxNeighbors, FILE* fp);
int insertActionsWithDefaults(int numberLAs, int numberGAs, int maxActions, FILE* fp);
int insertActionsWithoutDefaults(int numberLAs, int numberGAs, int maxActions, FILE* fp);

static const char* headerLine = "Version: %i\nGAs: %i, LAs: %i, maxNeighbors: %i, maxActions: %i\n";
static const char* commentLine = "# %s\n";
static const char* commentSeparator = "commentSeparatorToCheckOverflowDontEdit\n";
static const char* commentSeparatorFormat = "%s\n";
static const char* GAsectiontittle = "#Global Agent Data:\n";
static const char* LAsectiontittle = "#Local Agent Data:\n";
static const char* GAactionsSectionTittle = "\n#GA Action Data (placeholder):\n\n";
static const char* LAactionsSectionTittle = "\n#LA Action Data (placeholder):\n\n";

static const char* GAidentity = "\nGA Id: %i, On? %d\n";
static const char* GAname = "Name: %s\n";
static const char* GAresources = "Resources: %f\n";
static const char* connectedLAbitfield = "Connected LAs (bitfield): %i %i %i %i\n";
static const char* connectedGAbitfield = "Connected GAs (bitfield): %i\n";
static const char* GArelationsInfo = "Relation with GA %i: stance: %i, disposition: %f\n";
static const char* lastGAwarning = "\n#The last GA is reserved for local agents not belonging to any GA\n\n";

static const char* LAidentity = "\nLA Id: %i, belongs to GA: %i, On? %d\n";
static const char* LAname = "Name: %s\n";
static const char* LAposition = "Pos: X %f, Y %f\n";
static const char* LAstrenght = "Strenght: %f, Threshold to cost upkeed: %f\n";
static const char* LAresources = "Current Resources: %f, income: %f, upkeep: %f\n";
static const char* LArelationsInfo = "Relation with Neighbor %i: stance: %i, disposition: %f\n";

static const char* LAaction = "#LA Action %i (LA %i): *Action Data* (placeholder)\n";
static const char* GAaction = "#GA Action %i (GA %i): *Action Data* (placeholder)\n";