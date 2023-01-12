#pragma once

/*
* Project Notes:
*
* Some of the Specs have been changed to better accomodate the decision-making proccess.
* LA Thresholds have become "score offsets" instead. The spec doc is NOT updated yet.
* 
* TO DO: Update it after version 0.6
* TO DO: Use constexpr instead of defines where possible; 
* TO DO: On these notes, correct uses of array when actually we're using vectors;
* 
* Versioning:
* 
* - Major version: 0, until deliverable;
* - Minors and Subs described below (numbered);
* - Versions increment when complete (that is, we start at v0.0.0 working on 0.1.1);
* - Build #s will be the incremental commits in a given Sub;
* 
* - Current Version: 0.2.0
* - Working on: finishing 0.3.1
*
* ***** Minor 1. Basic Setup *****
*
* 1. Initial setup:
* 
* a. AS can say hello, AS_API can be asked to say hello, CL can say and be asked
* to say hello.
* b. CL has two APIs, one internal, one external. TA initializes AS,
* which initializes CL. AS creates a number and tells CL, TA querries CL for the number.
* c. AS an CL have structure with pointer to (fixed size) data. AS copies it's data to CL.
* TA gets the data from CL.
* d. Some cleanup so I can keep the basic tests working as I go along.
* e. Make sure the system to upload to github is in order and that I can build from a clone.
*
* ***** Minor 2. Basic Data Structure, loading and exporting *****
*
* 1. AS data structure:
* 
* //Most of the AS Data will be in vectors. These will be separated in PRNs, Control,
* //and, with a version each for LAs and GAs: Action, Decision, State, and Cold Data.
* //These will be controlled by a Control System (PRNs, Control and Cold Data), an Action 
* //System, a State System and a Decision System.
* //The exact design should reflect how the data is expected to be used, so at first it 
* //will be just a reasonable guess.
* //There will be access functions for the data, taking in the id of the agent. 
* //The IDs are (directly converted to) indexes on the areas were the data are. 
* //There will be a method to contruct an Agent Class, with all its information, 
* //which can be used for local processing, for example.
* 
* a. Create structures with the expected State and Cold Data for LAs.
* b. Create structures with the expected State and Cold Data for GAs.
* c. Create LA and GA Decision Data structure stub (just part of the necessary Data will
* be known at this point, specially expected values and last step relations for each
* neighbor. Will assume maxNeighbors for all agents.
* d. Create an array of each, with an element for each agent.
* e. Create initial version of relevant control systems.
* f. Allocate memory and populate an array for each (on initialization).
* 
* Note: State and Decision data will assume maxNeighbours.
* Note: first parts of Decision data structures are actually implemented here.
* 
* 2. Basic saving and loading:
* 
* a. Create a format for text file with network information.
* b. Create file with empty or default fields.
* c. Read information from a text file on initialization or request.
* d. Save data to a text file on request.
* e. Add to file format and functions the decision data already in place.
*
* 3. Actions (stub), communication, more saving and loading:
* 
* a. Create Action Data Structure and controller.
* b. Update file format and file functions to support it.
* c. Create functionality to update data and parameters derived from loaded data
*    (eg: number of connections, neighbour ids, actionController.m_maxNeighbours)
* d. Replicate all data and params on CL (transfer on command).
* e. Querry data on TA and check it.
* f. Add simple functionality to change data DIRECTLY and test load/save/read.
*
* Cleanup, documentation (in-code), and fresh pull+compile test
*
* ***** Minor 3. Runtime data insertion and removal *****
*
* >> 1. AS timed loop:
* 
* a. Make AS work on a timed loop, incrementing some of its values in a predictable manner.
* b. After each loop, make AS update CL's data.
* c. At a different time interval, TA querries CL and compares data with expected values.
*
* 2. Data injection:
* 
* a. Duplicate State and Action Data on CL, plus an indexed list of fields with the tick of 
* their last changes.
* TO DO: SHOULD DUPLICATE Decision Data too?
* b. Make it so TA can tranfer data to it (methods for field or whole State or Action from
* Agent Object).
* c. When the data is received by the CL, it updates the list of changed fields.
* d. Make it so AS can read the fields with changes and absorb them (pointer to const?).
* e. Before each loop, AS retrieves data from CL which was changed after its last check.
* f. Test running the AS, issuing changes from the TA and reading the data. 

* Note: changing an action to "innactive" essentially ends it, but there could also be 
* methods to advance actions to the moment before tehir completion. The TA can then chack 
* again a bit later if this is part of a multi-step process.
*
* Cleanup, documentation, and fresh pull+compile test
*
* ***** Minor 4. AS loop *****
*
* The goal here is to get the AS loop to work as expected.
* No actual decision making yet, just a couple stub decisions, but updating expected values.
* Couple of stub actions as well, that simply run their course and have a simple resolution.
*
* Cleanup, documentation, and fresh pull+compile test
*
* ***** Minor 5. Tooling: inspection *****
*
* Implement some tooling to inspect the system.
*
* Cleanup, documentation, and fresh pull+compile test
*
* * ***** Minor 6. More groundwork *****
*
* Any groundwork necessary before focusing on behaviour.
* Specially the sub-loops of different systems (eg Decision and Action).
*
* Cleanup, documentation (update specs), and fresh pull+compile test
* 
* ***** Minor 7. Basic decision making and actions *****
*
* A complete, but very simple version of the AS.
* Very simple versions of all initially expected actions.
* One Action for GAs and another for LAs made in more detail.
* 
* Validation of the basic system and initial exploration of the decision process.
*
* Cleanup, documentation, and fresh pull+compile test
*
* ***** Minor 8. Behaviour *****
*
* Developing actions and decision procedures and checking the results.
* The goal is to have the initial minimum expected functionality in order. Fuck Equilibrium.
*
* Cleanup, documentation, and fresh pull+compile test
*
* ***** Major 1. Tying things up *****
*
* A last pass and some more testing, including a final fresh clone+compile test.
*
* *************************************
*
* ***** Minor 1. Adjustments *****
*
* Any required adjustments.
*
*/