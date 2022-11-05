#pragma once

/*
* Project Notes:
*
* 
* 
* Versioning:
* 
* - Major version: 0, until deliverable;
* - Minors and Subs described below (numbered);
* - Versions increment when complete (that is, we start at v0.0.0 working on 0.1.1);
* - Build #s will be the incremental commits in a given Sub;
* 
* - Current Version: 0.1.0
* - Working on: finishing 0.2.1
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
* //Ideally, I want to have data in diferent "banks", so all the names are in one
* //place, all the positions in another, etc. The exact design should reflect how
* //the data is expected to be used, so at first it will be just a reasonable guess.
* //There will be "Agent Entities" which are IDs to find the specific data, as well
* //as access functions for the data, taking in the id. The IDs will (directly converted to)
* //indexes on the areas were the data are. There will be a method to contruct an Agent
* //Struct, with all it's information, which can be used for local processing, for example.
* 
* >> a. Create a structure with the expected data for LAs.
* >> b. Create a structure with the expected data for GAs.
* >> c. Allocate memory and populate an array for each (on initialization).
* >> d. Create structures of relevant pointers.
* 
* 2. Communication and loading:
* 
* a. Create a format for text file with information on each.
* b. Read their information from a text file on initialization or request.
* c. Replicate memory structure on CL and transfer data to it.
* d. Export data to binary file and load from it.
* e. Querry data on TA and check it.
*
* 3. Actions, stub:
* 
* a. Create stub action structure and vector of those on AS.
* b. Pass view into it to CL (point to const, I think).
* c. Querry it or get a copy for TA trhough CL.
* d. Add support for this on loading and exporting.
*
* Cleanup, documentation, and fresh pull+compile test
*
* ***** Minor 3. Runtime data insertion and removal *****
*
* 1. AS timed loop:
* 
* a. Make AS work on a timed loop, incrementing some values in a predictable manner.
* b. After each loop, make AS update CL's data.
* c. At a different time interval, TA querries CL and compares data with expected values.
*
* 2. Agent data injection:
* 
* a. Duplicate agent data structures on CL, plus an indexed list of fields with changes.
* Numerical fields can have absolute, additive, multiplicative or exponential changes.
* b. Make it so TA can tranfer data to it (methods for field or agent).
* c. When the data is received by the CL, it updates the list of changed fields (blocking).
* d. Make it so AS can read the fields with changes and absorb them (pointer to const?).
* e. Before each loop, AS retrieves data from CL (CL blocks and updates the list of changes).
* f. Test running the AS and issuing changes.
*
* 3. Actions, creation and end:
* 
* a. Create two vectors of new actions on the CL, and a int to tell wich to use.
* b. Before each loop, AS asks CL to change the active vector (blocking) and adds the new
* actions to it's vector. CL erases this vector.
* c. TA can ask CL to add new actions. These go to the opposite bank.
* d. Create an index of active actions, which is to be kept updated by the AS.
* e. Create two secondary indexes on the CL. The TA can mark actions for premature deletion.
* These indexes will "rotate" the same way as the vectors.
* f. Before each loop, the AS takes a look at this and deletes any appropriate actions.
* g. Test adding and ending Actions.
*
* Cleanup, documentation, and fresh pull+compile test
*
* ***** Minor 4. AS loop *****
*
* The goal here is to get the AS loop to work as expected, but no decision making yet.
*
* Cleanup, documentation, and fresh pull+compile test
*
* ***** Minor 5. Tooling: inspection *****
*
* Implement some tooling to inspect the system.
*
* Cleanup, documentation, and fresh pull+compile test
*
* ***** Minor 6. Basic decision making and actions *****
*
* A complete, but very simple version of the AS, with two possible actions for LAs and GAs.
*
* Cleanup, documentation, and fresh pull+compile test
*
* ***** Minor 7. More groundwork *****
*
* Any groundwork necessary before focusing on behaviour.
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