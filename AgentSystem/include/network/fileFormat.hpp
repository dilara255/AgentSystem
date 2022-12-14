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

static const char* defaultGAnamePrefix = "GA_";
static const char* defaultLAnamePrefix = "LA_";
static const char* defaultFilePath = "networkFiles\\";

int insertGAsWithDefaults(int numberGAs, FILE* fp);
int insertGAsWithoutDefaults(int numberGAs, FILE* fp);
int insertLAsWithDefaults(int numberLAs, int maxNeighbors, int numberGAs, FILE* fp);
int insertLAsWithoutDefaults(int numberLAs, int maxNeighbors, FILE* fp);
int insertActionsWithDefaults(int numberLAs, int numberGAs, int maxActions, FILE* fp);
int insertActionsWithoutDefaults(int numberLAs, int numberGAs, int maxActions, FILE* fp);

static const char* headerLine = "Version: %d\nGAs: %d, LAs: %d, maxNeighbors: %d, maxActions: %d\n";
static const char* commentLine = "# %s\n";
static const char* commentSeparator = "commentSeparatorToCheckOverflowDontEdit\n";
static const char* commentSeparatorFormat = "%s\n";
static const char* GAsectiontittle = "#Global Agent Data:\n";
static const char* LAsectiontittle = "#Local Agent Data:\n";
static const char* GAactionsSectionTittle = "\n#GA Action Data:\n\n";
static const char* LAactionsSectionTittle = "\n#LA Action Data:\n\n";
static const char* LAoffsetsTitle = "Action Offsets {from LA's personality | from GA's actions}:\n";

static const char* GAidentity = "\nGA Id: %d, On? %d\n";
static const char* GAname = "Name: %s\n";
static const char* GApersonality = "Personality Traits: %d %d %d %d\n";
static const char* GAresources = "Resources: %f\n";
static const char* connectedLAbitfield = "Connected LAs (bitfield): %d %d %d %d\n";
static const char* connectedGAbitfield = "Connected GAs (bitfield): %d\n";
static const char* GArelationsInfo = "Relation with GA %d: stance: %d, disposition: %f (last step: %f), infiltration: %f\n";
static const char* lastGAwarning = "\n#The last GA is reserved for local agents not belonging to any GA\n\n";

static const char* LAidentity = "\nLA Id: %d, belongs to GA: %d, On? %d\n";
static const char* LAname = "Name: %s\n";
static const char* LAposition = "Pos: X %f, Y %f\n";
static const char* LAstrenght = "Strenght: %f (plus %f reinforcement), Threshold to cost upkeed: %f\n";
static const char* LAresources = "Current Resources: %f, income: %f, upkeep: %f\n";
static const char* LArelationsInfo = "Relation with Neighbor %d: stance: %d, disposition: %f, infiltration: %f\n";
static const char* LAcategoryOffsets = "Category %d: immediate: {%f | %f}, request:  {%f | %f}, self: {%f | %f}\n";

static const char* LAaction = "#LA Action %d (LA %d): IDs: %d, Ticks: %lu, %lu, Intensity: %d, Aux: %d\n";
static const char* GAaction = "#GA Action %d (GA %d): IDs: %d, Ticks: %lu, %lu, Intensity: %d, Aux: %d\n";