#pragma once

/*TODO: write parser, build data, insert data, create agent object, add new agent from it.
*Write it all back on a file on this format. See if new agent is really there (one LA, one GA).
* Will need a control structure which knows the amount of LAs and GAs and coordinates controllers.

GLOBAL
agents int

name max_30_chars
onOff 1_ou_0
relations0 int_stance float_dispo
[...]
relationsMAX_GA int_stance float_dispo
listLAs int int int int
GAresources float
totalLAstrenght float
totalLAresources float_current float_update
connections int

*data second GA*

[...]

*data last GA*

LOCAL
agents int

name max_30_chars
onOff 1_ou_0
relations0 int_stance float_dispo
[...]
relationsMAX_NEIGHBOURS int_stance float_dispo
pos float_x float_y
connections int
listNeighbours int int int int
strenght float
upkeep float_current float_strenghtThresholdToCost
resources float_current float_update
GA int

*data second LA*

[...]

*data last LA*
*/