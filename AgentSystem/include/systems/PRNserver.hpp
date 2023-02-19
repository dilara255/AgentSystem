#pragma once

/*
Declares a class to hold the pseudo random numbers (PRNs) needed for each step.
Has a support structure to pass information about it's opperation, for testing and profiling.
Includes some related definitions.
*/

#include "network/parameters.hpp"

#define PRNS_PER_LA (LA_FIELDS_TO_DEDUCE_EXPECTED * MAX_LA_NEIGHBOURS + PRNS_TO_CHOOSE_ACTION)
#define PRNS_PER_GA (GA_FIELDS_TO_DEDUCE_EXPECTED * (MAX_GA_QUANTITY-1) + PRNS_TO_CHOOSE_ACTION)
#define MAX_LA_PRNS (PRNS_PER_LA*MAX_LA_QUANTITY)
#define MAX_GA_PRNS (PRNS_PER_GA*MAX_GA_QUANTITY)
#define MAX_ACT_PRNS (PRNS_PER_ACT*MAX_AGENTS)
#define MAX_PRNS (MAX_LA_PRNS + MAX_GA_PRNS + MAX_ACT_PRNS)
#define TOTAL_PRN_CHOPS AS_TOTAL_CHOPS

namespace AS{

	typedef struct {
		int draw4startIndex;
		int draw4IndexOffset;
		int draw4indexIsSmallerThan;
		int draw4IndexOffset;
		int draw1startIndex;
		int chopIndexIsSmallerThan;
		int prnsToDrawPerRegularChop;
		std::chrono::nanoseconds accumulatedDrawTime = std::chrono::nanoseconds(0);
	} drawInfo_t;

	//This class manages seeding, drawing and accessing PRNs in the range [0,1], as floats.
	//The drawn PRNs are also stored here. The generator is from elsewhere. 
	//It supports drawing in "chops": the PRNs to be drawn are divided in batches, according
	//to the total amount, how many have been drawn, and how many more mainLoop steps we have
	//remaining to get the total amount of PRNs.
	//Also includes methods to test, benchmark and dump the PRNs to a file.
	//TODO: get the PRNs in different formats/intervals
	class PRNserver {
	public:
		void drawPRNs(int chopIndex, int totalChops, int PRNsToDrawTotal);
		float getNext() {return PRNs[nextToUse++];}
		uint64_t getSeed(int index) const {return seeds[index];}
		void setSeed(int index, uint64_t newSeed) {seeds[index] = newSeed;}

		//Dumps to a file seeds, number of PRNs generated on this sequence of chops,
		//and the PRNs themselves. Has a default filename. Saves on default networks folder
		bool dumpData(std::string = "");

		//Tests the chopping for coverage of the whole expected range and overwriting.
		//Returns false if either fails. Also times the drawing. 
		//Optionally print results (default true) and dumps PRNs to file (default false)
		bool testAndBenchChoppedDrawing(int64_t howManyToDraw, int totalChops,
			                  bool printResults = true, bool dumpPRNs = false);

		void zeroAccumulatedDrawTime() {drawInfo.accumulatedDrawTime = std::chrono::nanoseconds(0);}
		std::chrono::nanoseconds getAccumulatedDrawTime(){return drawInfo.accumulatedDrawTime;}
		
	private:
		int drawn = 0;
		int nextToUse = 0;
		float PRNs[MAX_PRNS];
		uint64_t seeds[DRAW_WIDTH];
		drawInfo_t drawInfo; //for testing and benchmarking/profiling
	};
}