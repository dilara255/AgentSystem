/*
* Project Notes:
*
* 
* 
* Tests/Dev Milestones:
* (curr: A.II)
*
* ***** A. Basic Setup *****
*
* I. Initial setup:
* 1. AS can say hello, AS_API can be asked to say hello, CL can say and be asked
* to say hello.
* 2. CL has two APIs, one internal, one external. TA initializes AS,
* which initializes CL. AS creates a number and tells CL, TA querries CL for the number.
* 3. AS an CL have structure with pointer to (fixed size) data. AS copies it's data to CL.
* TA gets the data from CL.
* 4. Some cleanup so I can keep the basic tests working as I go along.
* 5. Make sure the system to upload to github is in order and that I can build from a clone.
*
* > II. AS data structure:
* 1. Create a structure with the expected data for LAs.
* 2. Create a structure with the expected data for GAs.
* 3. Allocate memory and populate an array for each.
* 4. Create a format for text file with information on each.
* 5. Read their information from a text file.
* 6. Create structures of relevant pointers.
* 7. Replicate memory structure on CL and transfer data to it.
* 8. Querry data on TA and check it.
*
* III. Actions, stub:
* 1. Create stub action structure and vector of those on AS.
* 2. Pass view into it to CL (point to const, I think).
* 3. Querry it or get a copy for TA trhough CL.
*
* IV. Cleanup and fresh clone+compile test
*
* ***** B. Runtime data insertion and removal *****
*
* I. AS timed loop:
* 1. Make AS work on a timed loop, incrementing some values in a predictable manner.
* 2. After each loop, make AS update CL's data.
* 3. At a different time interval, TA querries CL and compares data with expected values.
*
* II. Agent data injection:
* 1. Duplicate agent data structures on CL, plus an indexed list of fields with changes.
* Numerical fields can have absolute, additive, multiplicative or exponential changes.
* 2. Make it so TA can tranfer data to it (methods for field or agent).
* 3. When the data is received by the CL, it updates the list of changed fields (blocking).
* 4. Make it so AS can read the fields with changes and absorb them (pointer to const?).
* 5. Before each loop, AS retrieves data from CL (CL blocks and updates the list of changes).
* 6. Test running the AS and issuing changes.
*
* III. Actions, creation and end:
* 1. Create two vectors of new actions on the CL, and a int to tell wich to use.
* 2. Before each loop, AS asks CL to change the active vector (blocking) and adds the new
* actions to it's vector. CL erases this vector.
* 3. TA can ask CL to add new actions. These go to the opposite bank.
* 4. Create an index of active actions, which is to be kept updated by the AS.
* 5. Create two secondary indexes on the CL. The TA can mark actions for premature deletion.
* These indexes will "rotate" the same way as the vectors.
* 6. Before each loop, the AS takes a look at this and deletes any appropriate actions.
* 7. Test adding and ending Actions.
*
* IV. Cleanup and fresh clone+compile test
*
* ***** C. AS loop *****
*
* The goal here is to get the AS loop to work as expected, but no buisness logic yet.
*
* Cleanup and fresh clone+compile test
*
* ***** D. Tooling: inspection *****
*
* Implement some tooling to inspect the system.
*
* Cleanup and fresh clone+compile test
*
* ***** E. Basic decision making and actions *****
*
* A complete, but very simple version of the AS, with two possible actions for LAs and GAs.
*
* Fresh clone+compile test
*
* ***** F. More groundwork *****
*
* Any groundwork necessary before focusing on bechaviour.
*
* Cleanup and fresh clone+compile test
*
* ***** G. Behaviour *****
*
* Developing actions and decision procedures and checking the results.
* The goal is to have the initial minimum expected functionality in order. Fuck Equilibrium.
*
* Fresh clone+compile test
*
* ***** H. Tying things up *****
*
* A last pass and some more testing, including a final fresh clone+compile test.
*
* ***** I. Adjustments *****
*
* Any required adjustments.
*
*/