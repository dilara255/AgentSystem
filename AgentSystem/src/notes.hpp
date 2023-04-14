#pragma once

/*
* Project Notes:
*
* Some of the Specs have been changed to better accomodate the decision-making proccess.
* LA Thresholds have become "score offsets" instead. The spec doc is NOT updated yet.
* TODO: Mark areas of the specification needing update version 0.6, and finish with 1.0
* TODO: On these notes, correct uses of array when actually we're using vectors;
*
* TECHNICAL DEBT:
* 
* Being tracked on a separate file 
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
* Cleanup, some (in-code) documentation, and fresh pull+compile test
*
* ***** Minor 3. Runtime data insertion and removal *****
*
* 1. AS timed loop:
* 
* a. Make AS work on a timed loop, incrementing some of its values in a predictable manner.
* b. After each loop, make AS update CL's data.
* c. At a different time interval, TA querries CL and compares data with expected values.
*
* 2. Data injection:
* 
* a. TA-managed Cold, State and Action data area on CL, plus list of fields with changes.
* b. On each loop, AS retrieves data from CL which was changed after its last check.
* 
* Complete for some fields, and stub for the rest:
* c. Make it so TA can tranfer data to it (methods for field or whole State or Action from
* Agent Object). When the data is received by the CL, it updates the list of changed fields.
* d. Make it so AS can read the fields with changes and absorb them (pointer to const?).
* e. Test running the AS, issuing changes from the TA and reading the data. 

* Note: changing an action to "innactive" essentially ends it, but it is also possible 
* to advance actions to the moment before their completion. The TA can then check 
* again a bit later if this is part of a multi-step process.
*
* Cleanup and bug-fixing
* 
* ***** > Minor 4. AS loop *****
*
* -Understand why x86 versions are getting crazy tick values;
* -Understand why saving can sometimes take a while (but don't loose sleep over it);
* 
* 1. Basic structure:
* 
* -Review notes, sketch on paper;
* -Implement skeleton of main loop, with dummy loop stubs;
* --Implement timing of main loop (and lastTickTime to params);
* --Main loop implemented except for "step()" (for which there will be stubs)
* 
* 2. PRNG:
* 
* -Create PRNG system (and add them to their step and time it);
* --System on AS, to draw numbers as required;
* --Actual PRNG, on Aux0;
* --Seeds: default and loading;
* 
* 3. Updates:
* 
* -Implement state self-updating (ie, from income and upkeep, no actions involved);
* -Add tests;
* 
* 4. Information and Expected Values, plus notions and action score data structures:
* 
* -Update information and expected values;
* --Add data structure for expected values;
* --WILL HAVE TO SOLVE HOW TO SAVE THIS (at the bottom of the file seems good);
* -add shouldMakeDecisions flag;
* -Add "notions" and "actionScores" data structures;
* --will be calculated per-step on the stack, so no saving and loading;
* 
* 5. More handling and updating:
* 
* -Complete 3.2.c-e for any necessary fields;
* --All new fields on mirror, shouldMakeDecisions and infiltration on ClientDataHandler;
* -Info and relation updates on LA and GA updateSteps;
* -Some general maintenence and fixes (sav/load of neighbor data, prn distribution, etc);
* 
* 6. "MVP":
* 
* >-Stubs of decisions and actions (no actual logic);
* -Couple of simplified decisions and actions (and watever is necessary for that);
* --Should just run their course and have a simple resolution;
*  
* Some cleanup
*
* ***** Minor 5. Tooling: inspection *****
* 
* -Text-mode visualization of a couple agents running;
* 
* Quick system review and any pending groundwork.
* Do some benchmarking.
* Implement some tooling to inspect the system.
* Client inuput limited by whats already implemented (focus on reading/output).
*
* General testing
*
* * ***** Minor 6. More groundwork, Technical Debt/future refactor organization*****
*
* - Quick technical debt organization pass and refactor planning;
* -- Take a look at technical debt and TODOs (especially CRITICALs);
* -- Prioritize what to deal with now, only if really necessary, and document the rest;
* -- Plan a proposed first refactor around that, as well as the following steps to delivery;
* 
* - Any groundwork still necessary before focusing on behaviour;
* --Especially action-related;
* -- Also complete 3.2.c-e for a few more of the most relevant fields and update tooling;
*
* ***** Minor 7. Basic decision making and actions: vertical slice *****
*
* A complete vertical slice of the AS.
* -- All planned functionality in place: some may be partially implemented, but validated.
* Very simple versions of all initially expected actions.
* 
* Validation of the system and initial exploration of the decision process:
* - Minimum necessary detailed behaviour + reasonable behaviour for the rest, so that:
* -- We have the initial minimum expected functionality in order. Fuck Equilibrium.
* -- We can check that agent growth can be kept under control;
* -- We can check the emergency of one key "complex" scnario with the detailed behaviour;
* *
* ***** Major 1. Tying things up *****
*
* - A last pass on TODO's, documenting any left behind;
* - A last cleanup and comments pass;
* - A last pass on tests and possible bugs;
* - Fresh clone+compile, plus testing on other pcs;
* - Updated specification, diagrams and results;
* - Video with overview of results;
* - Delivery;
*
* *************************************
*
* ***** Minor 1. Adjustments *****
*
* Any required adjustments.
*
*/