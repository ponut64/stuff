//ldata.c
//this file is compiled separately
#include <sl_def.h>
#include "def.h"
#include "bounder.h"
#include "physobjet.h"
#include "mloader.h"
#include "render.h"
#include "player_phy.h"
#include "tga.h"

#include "ldata.h"




Bool ldata_ready = false;

void	declarations(void)
{

/* level00 ?*/
//Will replace.

//The start stand contains the track data as well.
//So before declaring the start stand, change the track data parameters to as desired.
//It also contains the item manager for 7rings. No need to declare separately.

declare_object_at_cell(-(0 / 40) + 1, -15, (100 / 40), 16 /*kyob*/, 0, 0, 0, 0);

// declare_object_at_cell((0 / 40) + 1, -20, (0 / 40), 51 /*start stand*/, 0, 0, 0, 0);

// declare_object_at_cell(-(260 / 40) + 1, -69, (380 / 40), 18 /*post00*/, 0, 0, 0, 0);
// declare_object_at_cell(-(340 / 40) + 1, -69, (300 / 40), 18 /*post00*/, 0, 0, 0, 0);
// objList[18]->ext_dat = SET_GATE_POST_LINK(objList[18]->ext_dat, 1);
// declare_object_at_cell(-(220 / 40) + 1, -69, (180 / 40), 18 /*post00*/, 0, 0, 0, 0);
// declare_object_at_cell(-(140 / 40) + 1, -69, (260 / 40), 18 /*post00*/, 0, 0, 0, 0);
// objList[18]->ext_dat = SET_GATE_POST_LINK(objList[18]->ext_dat, 2);
// declare_object_at_cell((220 / 40) + 1, -69, -(180 / 40), 18 /*post00*/, 0, 0, 0, 0);
// declare_object_at_cell((140 / 40) + 1, -69, -(260 / 40), 18 /*post00*/, 0, 0, 0, 0);


// declare_object_at_cell((260 / 40), -10, (140 / 40), 1 /*t item*/, 0, 0, 0, 0);
// declare_object_at_cell((260 / 40), -10, (180 / 40), 2 /*t item*/, 0, 15, 0, 0);
// declare_object_at_cell((260 / 40), -10, (220 / 40), 3 /*t item*/, 0, 30, 0, 0);
// declare_object_at_cell((260 / 40), -10, (260 / 40), 4 /*t item*/, 0, 45, 0, 0);
// declare_object_at_cell((260 / 40), -10, (300 / 40), 5 /*t item*/, 0, 90, 0, 0);
// declare_object_at_cell((260 / 40), -10, (340 / 40), 6 /*t item*/, 0, 105, 0, 0);
// declare_object_at_cell((260 / 40), -10, (380 / 40), 7 /*t item*/, 0, 120, 0, 0);

// declare_object_at_cell(-(300 / 40) + 1, -4, -(340 / 40), 53 /*flag stand*/, 0, 0, 0, 0);

// declare_object_at_cell((340 / 40) + 1, -5, -(340 / 40), 54 /*goal stand*/, 0, 0, 0, 0);

// declare_object_at_cell((120 / 40) + 1, -100, -(0 / 40), 22 /*float03*/, 0, 0, 0, 0);

//declare_object_at_cell((220 / 40) + 1, -280, (-1060 / 40), 15 /*ADX sound trigger*/, 40, 40, 40, 7 | (7<<8) /* sound num & vol */);

/* level01? */

// declare_object_at_cell(-(180 / 40) + 1, -120, -(660 / 40), 61 /*start location*/, 0, 0, 0, 0);

// declare_object_at_cell(-(900 / 40) + 1, -248, -(460 / 40), 10 /*platf00*/, 0, 0, 0, 0);
// declare_object_at_cell(-(620 / 40) + 1, -248, (260 / 40), 10 /*platf00*/, 0, 0, 0, 0);
// declare_object_at_cell((340 / 40) + 1, -288, (660 / 40), 10 /*platf00*/, 0, 0, 0, 0);
// declare_object_at_cell((1060 / 40) + 1, -297, (620 / 40), 10 /*platf00*/, 0, 0, 0, 0);

// declare_object_at_cell(-(260 / 40) + 1, -368, (1100 / 40), 12 /*greece01*/, 0, 90, 0, 0);
// declare_object_at_cell(-(260 / 40) + 1, -368, (860 / 40), 13 /*greece02*/, 0, 270, 0, 0);
// declare_object_at_cell(-(260 / 40) + 1, -368, (1340 / 40), 13 /*greece02*/, 0, 90, 0, 0);


// declare_object_at_cell(-(260 / 40) + 1, -302, (580 / 40), 16 /*overhang*/, 0, 90, 0, 0);
// declare_object_at_cell((1540 / 40) + 1, -302, (500 / 40), 16 /*overhang*/, 0, 45, 0, 0);
// declare_object_at_cell((1740 / 40) + 1, -259, (1060 / 40), 16 /*overhang*/, 0, 45, 0, 0);
// declare_object_at_cell((1660 / 40) + 1, -313, -(940 / 40), 16 /*overhang*/, 0, 90, 0, 0);

// declare_object_at_cell(-(780 / 40) + 1, -275, -(780 / 40), 21 /*wall1*/, 0, 180, 10, 0);
// declare_object_at_cell(-(1020 / 40) + 1, -275, -(740 / 40), 21 /*wall1*/, 0, 0, 10, 0);

// declare_object_at_cell((660 / 40) + 1, -377, (1180 / 40), 25 /*float01*/, 0, 90, 0, 0);

// declare_object_at_cell(-(180 / 40) + 1, -179, -(340 / 40), 33 /*ramp01*/, 0, 0, 0, 0);
// declare_object_at_cell(-(100 / 40) + 1, -347, -(1260 / 40), 33 /*ramp01*/, 0, -90, 0, 0);

// declare_object_at_cell((700 / 40) + 1, -273, (660 / 40), 27 /*hiway01*/, 0, 0, 0, 0);
// declare_object_at_cell((1420 / 40) + 1, -351, (1140 / 40), 27 /*hiway01*/, 0, 0, 0, 0);

// declare_object_at_cell(-(180 / 40) + 1, -237, -(100 / 40), 27 /*hiway01*/, 0, 0, 0, 0);
// declare_object_at_cell((540 / 40) + 2, -237, -(100 / 40), 27 /*hiway01*/, 0, 0, 0, 0);
// declare_object_at_cell(-(900 / 40) + 1, -237, -(100 / 40), 27 /*hiway01*/, 0, 0, 0, 0);
// declare_object_at_cell(-(1580 / 40) + 1, -237, (340 / 40) + 1, 27 /*hiway01*/, 0, 90, 0, 0);

// declare_object_at_cell(-(1260 / 40) + 1, -237, -(100 / 40), 28 /*hiway02*/, 0, 0, 0, 0);
// declare_object_at_cell(-(540 / 40) + 1, -237, -(100 / 40), 28 /*hiway02*/, 0, 180, 0, 0);
// declare_object_at_cell((180 / 40) + 2, -237, -(100 / 40), 28 /*hiway02*/, 0, 0, 0, 0);
// declare_object_at_cell((900 / 40) + 2, -237, -(100 / 40), 28 /*hiway02*/, 0, 180, 0, 0);
// declare_object_at_cell((1140 / 40) + 2, -237, -(100 / 40), 28 /*hiway02*/, 0, 0, 0, 0);

// declare_object_at_cell(-(420 / 40) + 1, -237, -(100 / 40), 29 /*hiway03*/, 0, 0, 0, 0);
// declare_object_at_cell(-(1380 / 40) + 1, -237, -(100 / 40), 29 /*hiway03*/, 0, 0, 0, 0);
// declare_object_at_cell((60 / 40) + 2, -237, -(100 / 40), 29 /*hiway03*/, 0, 0, 0, 0);
// declare_object_at_cell((1020 / 40) + 2, -237, -(100 / 40), 29 /*hiway03*/, 0, 0, 0, 0);

// declare_object_at_cell(-(1020 / 40) + 1, -295, (1100 / 40), 29 /*hiway03*/, 0, 0, 0, 0);

// declare_object_at_cell((300 / 40) + 2, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0);
// declare_object_at_cell((660 / 40) + 2, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0);
// declare_object_at_cell((780 / 40) + 2, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0);
// declare_object_at_cell((1260 / 40) + 2, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0);
// declare_object_at_cell((1460 / 40) + 2, -237, -(300 / 40), 30 /*hiway04*/, 0, 90, 0, 0);
// declare_object_at_cell(-(60 / 40) + 1, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0);
// declare_object_at_cell(-(300 / 40) + 1, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0);
// declare_object_at_cell(-(660 / 40) + 1, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0);
// declare_object_at_cell(-(780 / 40) + 1, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0);
// declare_object_at_cell(-(1140 / 40) + 1, -237, -(100 / 40), 30 /*hiway04*/, 0, 0, 0, 0);
// declare_object_at_cell(-(1580 / 40) + 1, -237, (100 / 40) + 1, 30 /*hiway04*/, 0, 90, 0, 0);
// declare_object_at_cell(-(1580 / 40) + 1, -237, (220 / 40) + 1, 30 /*hiway04*/, 0, 90, 0, 0);

// declare_object_at_cell(-(1020 / 40) + 1, -237, -(100 / 40), 31 /*hiway04*/, 0, 0, 0, 0);
// declare_object_at_cell((420 / 40) + 2, -237, -(100 / 40), 31 /*hiway04*/, 0, 0, 0, 0);

// declare_object_at_cell((1420 / 40) + 2, -179, -(140 / 40), 32 /*hiway06*/, 0, 180, 0, 0);
// declare_object_at_cell(-(1540 / 40) + 1, -179, -(60 / 40), 32 /*hiway06*/, 0, 0, 0, 0);

// declare_object_at_cell((1460 / 40) + 2, -237, -(420 / 40), 34 /*hiway07*/, 0, 90, 0, 0);
// declare_object_at_cell((140 / 40) + 2, -405, -(1260 / 40), 34 /*hiway07*/, 0, 180, 0, 0);

// declare_object_at_cell(-(1140 / 40) + 1, -295, (1100 / 40), 34 /*hiway07*/, 0, 0, 0, 0);
// declare_object_at_cell(-(900 / 40) + 1, -295, (1100 / 40), 34 /*hiway07*/, 0, 180, 0, 0);

/* level2? */
// Likely will keep, with modification.

// declare_object_at_cell((1180 / 40) + 1, -400, -(3140 / 40), 61 /*start location*/, 0, 0, 0, 0);

// declare_object_at_cell((1660 / 40) + 1, -159, -(980 / 40), 10 /*platf00*/, 0, 0, 0, 0);
// declare_object_at_cell((1060 / 40) + 1, -251, -(2780 / 40), 10 /*platf00*/, 0, 0, 0, 0);

// declare_object_at_cell((1060 / 40) + 1, -344, -(1460 / 40), 11 /*bridge1*/, 0, 0, 0, 0);

// declare_object_at_cell(-(580 / 40) + 1, -177, -(300 / 40), 12 /*greece01*/, 0, 0, 0, 0);

// declare_object_at_cell(-(820 / 40) + 1, -177, -(300 / 40), 13 /*greece02*/, 0, 180, 0, 0);
// declare_object_at_cell(-(340 / 40) + 1, -177, -(140 / 40), 13 /*greece02*/, 0, 90, 0, 0);

// declare_object_at_cell(-(1580 / 40) + 1, -201, -(2860 / 40), 13 /*greece02*/, 0, 45, 0, 0);
// declare_object_at_cell(-(1300 / 40) + 1, -201, -(3140 / 40), 13 /*greece02*/, 0, 45, 0, 0);
// declare_object_at_cell(-(1220 / 40) + 1, -201, -(3220 / 40), 13 /*greece02*/, 0, 45, 0, 0);
// declare_object_at_cell(-(940 / 40) + 1, -201, -(3500 / 40), 13 /*greece02*/, 0, 45, 0, 0);

// declare_object_at_cell(-(340 / 40) + 1, -177, -(300 / 40), 14 /*greece03*/, 0, 0, 0, 0);

// declare_object_at_cell(-(1460 / 40) + 1, -200, -(3020 / 40), 15 /*greece04*/, 0, 0, 0, 0);
// declare_object_at_cell(-(1100 / 40) + 1, -200, -(3380 / 40), 15 /*greece04*/, 0, 0, 0, 0);

// declare_object_at_cell((1660 / 40) + 1, -117, -(1820 / 40), 16 /*overhang*/, 15, 0, 0, 0);
// declare_object_at_cell((1020 / 40) + 1, -146, -(220 / 40), 16 /*overhang*/, 0, 135, 0, 0);
// declare_object_at_cell((60 / 40) + 1, -209, (3180 / 40), 16 /*overhang*/, 0, 0, 0, 0);
// declare_object_at_cell(-(580 / 40) + 1, -238, (3340 / 40), 16 /*overhang*/, 0, 0, 0, 0);
// declare_object_at_cell((1540 / 40) + 1, -316, -(2940 / 40), 16 /*overhang*/, 0, 90, 0, 0);
// declare_object_at_cell(-(660 / 40) + 1, -190, -(2220 / 40), 16 /*overhang*/, 0, -45, 15, 0);
// declare_object_at_cell(-(420 / 40) + 1, -180, -(2580 / 40), 16 /*overhang*/, 0, -45, 15, 0);

// declare_object_at_cell((740 / 40) + 1, -167, -(60 / 40), 17 /*pier1*/, 0, 45, 0, 0);
// declare_object_at_cell((1180 / 40) + 1, -97, (2300 / 40), 17 /*pier1*/, 0, 45, 0, 0);

// declare_object_at_cell(-(1260 / 40) + 1, -304, (1620 / 40), 19 /*tunnel2*/, 0, 125, 0, 0);

// declare_object_at_cell((1300 / 40) + 1, -133, (860 / 40), 21 /*wall1*/, 0, 140, 0, 0);
// declare_object_at_cell(-(1420 / 40) + 1, -262, (2620 / 40), 21 /*wall1*/, 0, 180, 0, 0);
// declare_object_at_cell(-(340 / 40) + 1, -226, (740 / 40), 21 /*wall1*/, 0, 180, 0, 0);
// declare_object_at_cell(-(980 / 40) + 1, -208, (380 / 40), 21 /*wall1*/, 0, 40, 0, 0);
// declare_object_at_cell((300 / 40) + 1, -397, -(620 / 40), 21 /*wall1*/, 0, 40, 0, 0);
// declare_object_at_cell((620 / 40) + 1, -369, -(2500 / 40), 21 /*wall1*/, 0, 0, 0, 0);

// declare_object_at_cell(-(1340 / 40) + 1, -229, (3260 / 40), 24 /*OBSTCL1*/, 0, 135, 0, 0);

// declare_object_at_cell(-(1100 / 40) + 1, -258, -(1740 / 40), 27 /*hiway01*/, 0, 0, 0, 0);
// declare_object_at_cell(-(1500 / 40) + 1, -234 , -(2420 / 40), 27 /*hiway01*/, 0, 0, 0, 0);

// declare_object_at_cell((1660 / 40) + 1, -146, -(1340 / 40), 29 /*hiway03*/, 0, 90, 0, 0);
// declare_object_at_cell((1660 / 40) + 1, -146, -(1460 / 40), 34 /*hiway07*/, 0, 90, 0, 0);
// declare_object_at_cell((1660 / 40) + 1, -146, -(1220 / 40), 34 /*hiway07*/, 0, 270, 0, 0);

// declare_object_at_cell((100 / 40) + 1, -343, (2180 / 40), 35 /*tower01*/, 0, 0, 0, 0);

/* level3 */
//Decent. Will keep for testing/improvement.
//I'm getting better at this.

// declare_object_at_cell((1060 / 40) + 1, -420, (220 / 40), 61 /*start location*/, 0, 0, 0, 0);

// declare_object_at_cell((1020 / 40) + 1, -297, -(420 / 40), 25 /*float01*/, -15, 0, 0, 0);

// declare_object_at_cell(-(700 / 40) + 1, -350, -(1020 / 40), 26 /*float02*/, 0, 0, 0, 0);

// declare_object_at_cell(-(380 / 40) + 1, -325, -(1060 / 40), 10 /*platf00*/, 0, 0, -15, 0);
// declare_object_at_cell((740 / 40) + 1, -369, (500 / 40), 10 /*platf00*/, 0, 0, 0, 0);
// declare_object_at_cell(-(1020 / 40) + 1, -181, (220 / 40), 10 /*platf00*/, 0, 0, 0, 0);
// declare_object_at_cell((1020 / 40) + 1, -243, -(660 / 40), 10 /*platf00*/, -15, 0, 0, 0);
// declare_object_at_cell((1020 / 40) + 1, -326, -(140 / 40), 10 /*platf00*/, 0, 0, 0, 0);

// declare_object_at_cell((540 / 40) + 1, -345, -(1140 / 40), 16 /*overhang*/, 0, 0, 0, 0);

// declare_object_at_cell((500 / 40) + 1, -362, (740 / 40), 19 /*tunnel2*/, 0, -45, 0, 0);

// declare_object_at_cell(-(1140 / 40) + 1, -50, -(100 / 40), 21 /*wall1*/, 0, 180, 0, 0);
// declare_object_at_cell(-(1140 / 40) + 1, -82, -(540 / 40), 21 /*wall1*/, 0, 180, 0, 0);

// declare_object_at_cell(-(700 / 40) + 1, -107, (540 / 40), 15 /*greece04*/, 0, 90, 0, 0);
// declare_object_at_cell(-(420 / 40) + 1, -107, (820 / 40), 15 /*greece04*/, 0, 90, 0, 0);

// declare_object_at_cell(-(540 / 40) + 1, -107, (660 / 40), 13 /*greece02*/, 0, -45, 0, 0);
// declare_object_at_cell(-(860 / 40) + 1, -107, (420 / 40), 13 /*greece02*/, 0, 135, 0, 0);

/* level 4 ? */
// Hell Run level. I like it.

// declare_object_at_cell((460 / 40) + 1, -340, -(4820 / 40), 61 /*start location*/, 0, 0, 0, 0);

// declare_object_at_cell((140 / 40) + 1, -211,  -(1020 / 40), 10 /*platf00*/, 0, 0, 0, 0);
// declare_object_at_cell((180 / 40) + 1, -197,  -(480 / 40), 10 /*platf00*/, 0, 0, 0, 0);
// declare_object_at_cell((220 / 40) + 1, -351,  (1060 / 40), 10 /*platf00*/, 0, 0, 0, 0);

// declare_object_at_cell((260 / 40) + 1, -270,  -(4340 / 40), 23 /*bridge2*/, 0, 30, 0, 0);
// declare_object_at_cell((300 / 40) + 1, -426,  (860 / 40), 23 /*bridge2*/, 0, 30, 0, 0);
// declare_object_at_cell((300 / 40) + 1, -426,  (1260 / 40), 23 /*bridge2*/, 0, -30, 0, 0);

// declare_object_at_cell((140 / 40) + 1, -245, -(4140 / 40), 16 /*overhang*/, 0, 115, 0, 0);
// declare_object_at_cell((20 / 40) + 1, -289, -(3100 / 40), 16 /*overhang*/, 0, 90, 0, 0);

// declare_object_at_cell((260 / 40) + 1, -340, (2100 / 40), 16 /*overhang*/, 0, 90, 0, 0);

// declare_object_at_cell((180 / 40) + 1, -201, -(1220 / 40), 16 /*overhang*/, 20, 0, 0, 0);
// declare_object_at_cell((100 / 40) + 1, -201, -(1220 / 40), 16 /*overhang*/, 20, 0, 0, 0);

// declare_object_at_cell((220 / 40) + 1, -152, -(740 / 40), 16 /*overhang*/, 14, 0, 0, 0);
// declare_object_at_cell((140 / 40) + 1, -152, -(740 / 40), 16 /*overhang*/, 14, 0, 0, 0);

// declare_object_at_cell(-(140 / 40) + 1, -381, (3340 / 40), 24 /*OBSTCL1*/, 0, 0, 0, 0);
// declare_object_at_cell((60 / 40) + 1, -381, -(2300 / 40), 24 /*OBSTCL1*/, 0, 0, 0, 0);
// declare_object_at_cell((300 / 40) + 1, -383, (220 / 40), 24 /*OBSTCL1*/, 0, 0, 0, 0);

// declare_object_at_cell((100 / 40) + 1, -389, -(2300 / 40), 9 /*KYOOB*/, 0, 0, 0, 0);
// declare_object_at_cell((260 / 40) + 1, -393, (220 / 40), 9 /*KYOOB*/, 0, 0, 0, 0);

// declare_object_at_cell(-(20 / 40) + 1, -394, -(1900 / 40), 13 /*greece02*/, 0, 0, 0, 0);
// declare_object_at_cell((340 / 40) + 1, -310, -(300 / 40), 13 /*greece02*/, 0, 180, 0, 0);

// declare_object_at_cell(-(20 / 40) + 1, -398, (4420 / 40), 13 /*greece02*/, 0, 45, 0, 0);
// declare_object_at_cell(-(300 / 40) + 1, -398, (4700 / 40), 13 /*greece02*/, 0, 225, 0, 0);

// declare_object_at_cell(-(180 / 40) + 1, -398, (4540 / 40), 15 /*greece04*/, 0, 0, 0, 0);

// declare_object_at_cell(-(140 / 40) + 1, -296, (2940 / 40), 21 /*wall1*/, 0, 0, 60, 0);
// declare_object_at_cell(-(140 / 40) + 1, -277, (3820 / 40), 21 /*wall1*/, -60, -90, 0, 0);

/* level5 ? */
//Like this level, will keep.

// declare_object_at_cell(-(860 / 40) + 1, -330, (460 / 40), 61 /*start location*/, 0, 0, 0, 0);

// declare_object_at_cell((740 / 40) + 1, -148, (740 / 40), 16 /*overhang*/, 0, 45, 0, 0);
// declare_object_at_cell(-(1180 / 40) + 1, -126, (1020 / 40), 16 /*overhang*/, 0, 45, 0, 0);
// declare_object_at_cell((1740 / 40) + 1, -399, (1780 / 40), 16 /*overhang*/, 0, 45, 0, 0);

// declare_object_at_cell(-(500 / 40) + 1, -334,  (580 / 40), 10 /*platf00*/, 0, 0, 0, 0);
// declare_object_at_cell(-(340 / 40) + 1, -346,  (380 / 40), 10 /*platf00*/, 0, 0, 0, 0);
// declare_object_at_cell((1020 / 40) + 1, -196,  (460 / 40), 10 /*platf00*/, 0, 0, 0, 0);

// declare_object_at_cell((860 / 40) + 1, -22, -(700 / 40), 21 /*wall1*/, 0, -135, 0, 0);
// declare_object_at_cell((340 / 40) + 1, -181, -(1340 / 40), 21 /*wall1*/, 75, 90, 0, 0);

// declare_object_at_cell((1380 / 40) + 1, -284, (1380 / 40), 23 /*bridge2*/, 0, -135, 0, 0);
// declare_object_at_cell((660 / 40) + 1, -173, -(900 / 40), 23 /*bridge2*/, 0, -45, 0, 0);

// declare_object_at_cell(-(420 / 40) + 1, -164, -(1020 / 40), 17 /*pier1*/, 0, 120, 0, 0);

// declare_object_at_cell(-(860 / 40) + 1, -310, -(100 / 40), 20 /*tunnl3*/, 0, 90, 0, 0);

// declare_object_at_cell((300 / 40) + 1, -321, -(20 / 40), 14 /*greece03*/, 0, 0, 0, 0);

// declare_object_at_cell(-(60 / 40) + 1, -370, (180 / 40), 25 /*float01*/, 0, 0, 0, 0);

//Level 06 ?
// Cross level.
// Mostly just a track level, I don't have too much accomodation for the "seven rings".
// But I think the concept here is functional and fun.
// If I forget, path is:
// South -> North -> West -> East -> South (Ramp) -> West (Tunnel) -> North (Hill) -> East (NE Goal Corner)
// Now, onto the prospective level 07!

// declare_object_at_cell(-(0 / 40) + 1, -280, (0 / 40), 61 /*start location*/, 0, 0, 0, 0);

// declare_object_at_cell((500 / 40) + 1, -148, (980 / 40), 16 /*overhang*/, 0, 0, 0, 0);

// declare_object_at_cell(-(1120 / 40) + 1, -160, -(1080 / 40), 20 /*tunnel3*/, 0, 0, 0, 0);

// declare_object_at_cell(-(1340 / 40) + 1, -359, (900 / 40), 25 /*float01*/, 0, 45, 0, 0);

// declare_object_at_cell(-(100 / 40) + 1, -247, -(460 / 40), 34 /*hiway07*/, 0, 270, 0, 0);
// declare_object_at_cell(-(100 / 40) + 1, -247, -(1420 / 40), 34 /*hiway07*/, 0, 90, 0, 0);
// declare_object_at_cell(-(100 / 40) + 1, -247, (340 / 40), 34 /*hiway07*/, 0, 90, 0, 0);
// declare_object_at_cell(-(100 / 40) + 1, -247, (1300 / 40), 34 /*hiway07*/, 0, 270, 0, 0);
// declare_object_at_cell((260 / 40) + 1, -247, -(60 / 40), 34 /*hiway07*/, 0, 0, 0, 0);
// declare_object_at_cell((1220 / 40) + 1, -247, -(60 / 40), 34 /*hiway07*/, 0, 180, 0, 0);
// declare_object_at_cell(-(500 / 40) + 1, -247, -(60 / 40), 34 /*hiway07*/, 0, 180, 0, 0);
// declare_object_at_cell(-(1460 / 40) + 1, -247, -(60 / 40), 34 /*hiway07*/, 0, 0, 0, 0);

// declare_object_at_cell((500 / 40) + 1, -247, -(60 / 40), 30 /*hiway04*/, 0, 0, 0, 0);
// declare_object_at_cell((620 / 40) + 1, -247, -(60 / 40), 30 /*hiway04*/, 0, 0, 0, 0);
// declare_object_at_cell((860 / 40) + 1, -247, -(60 / 40), 30 /*hiway04*/, 0, 0, 0, 0);
// declare_object_at_cell((980 / 40) + 1, -247, -(60 / 40), 30 /*hiway04*/, 0, 0, 0, 0);

// declare_object_at_cell(-(740 / 40) + 1, -247, -(60 / 40), 30 /*hiway04*/, 0, 0, 0, 0);
// declare_object_at_cell(-(860 / 40) + 1, -247, -(60 / 40), 30 /*hiway04*/, 0, 0, 0, 0);
// declare_object_at_cell(-(1100 / 40) + 1, -247, -(60 / 40), 30 /*hiway04*/, 0, 0, 0, 0);
// declare_object_at_cell(-(1220 / 40) + 1, -247, -(60 / 40), 30 /*hiway04*/, 0, 0, 0, 0);

// declare_object_at_cell(-(100 / 40) + 1, -247, -(700 / 40), 30 /*hiway04*/, 0, 90, 0, 0);
// declare_object_at_cell(-(100 / 40) + 1, -247, -(820 / 40), 30 /*hiway04*/, 0, 90, 0, 0);
// declare_object_at_cell(-(100 / 40) + 1, -247, -(1060 / 40), 30 /*hiway04*/, 0, 90, 0, 0);
// declare_object_at_cell(-(100 / 40) + 1, -247, -(1180 / 40), 30 /*hiway04*/, 0, 90, 0, 0);

// declare_object_at_cell(-(100 / 40) + 1, -247, (580 / 40), 30 /*hiway04*/, 0, 90, 0, 0);
// declare_object_at_cell(-(100 / 40) + 1, -247, (700 / 40), 30 /*hiway04*/, 0, 90, 0, 0);
// declare_object_at_cell(-(100 / 40) + 1, -247, (940 / 40), 30 /*hiway04*/, 0, 90, 0, 0);
// declare_object_at_cell(-(100 / 40) + 1, -247, (1060 / 40), 30 /*hiway04*/, 0, 90, 0, 0);

// declare_object_at_cell(-(100 / 40) + 1, -247, -(580 / 40), 29 /*hiway03*/, 0, 90, 0, 0);
// declare_object_at_cell(-(100 / 40) + 1, -247, -(1300 / 40), 29 /*hiway03*/, 0, 90, 0, 0);
// declare_object_at_cell(-(100 / 40) + 1, -247, (460 / 40), 29 /*hiway03*/, 0, 90, 0, 0);
// declare_object_at_cell(-(100 / 40) + 1, -247, (1180 / 40), 29 /*hiway03*/, 0, 90, 0, 0);
// declare_object_at_cell(-(620 / 40) + 1, -247, -(60 / 40), 29 /*hiway03*/, 0, 0, 0, 0);
// declare_object_at_cell(-(1340 / 40) + 1, -247, -(60 / 40), 29 /*hiway03*/, 0, 0, 0, 0);
// declare_object_at_cell((380 / 40) + 1, -247, -(60 / 40), 29 /*hiway03*/, 0, 0, 0, 0);
// declare_object_at_cell((1100 / 40) + 1, -247, -(60 / 40), 29 /*hiway03*/, 0, 0, 0, 0);

// declare_object_at_cell((740 / 40) + 1, -247, -(60 / 40), 27 /*hiway01*/, 0, 0, 0, 0);
// declare_object_at_cell(-(980 / 40) + 1, -247, -(60 / 40), 27 /*hiway01*/, 0, 0, 0, 0);
// declare_object_at_cell(-(100 / 40) + 1, -247, -(940 / 40), 27 /*hiway01*/, 0, 0, 0, 0);
// declare_object_at_cell(-(100 / 40) + 1, -247, (820 / 40), 27 /*hiway01*/, 0, 0, 0, 0);

//Level 07
//Aside from level02, most levels have been small; less than 120x120.
//Now I don't feel like going for a big level "because I have to" is a good idea. Case in point, level02 probably isn't good,
//and will be re-done. But I think level07 should be bigger.

// declare_object_at_cell(-(1940 / 40) + 1, -100, -(2420 / 40), 61 /*start location*/, 0, 0, 0, 0);

// declare_object_at_cell((100 / 40) + 1, -405,  -(100 / 40), 10 /*platf00*/, 0, 0, 0, 0);
// declare_object_at_cell(-(20 / 40) + 1, -405,  -(20 / 40), 10 /*platf00*/, 0, 0, 0, 0);
// declare_object_at_cell(-(700 / 40) + 1, -357,  (1020 / 40), 10 /*platf00*/, 0, 0, 0, 0);
// declare_object_at_cell(-(580 / 40) + 1, -347,  (1300 / 40), 10 /*platf00*/, 0, 0, 0, 0);

// declare_object_at_cell(-(980 / 40) + 1, -242, (980 / 40), 13 /*greece02*/, 0, 225, 0, 0);
// declare_object_at_cell(-(1540 / 40) + 1, -342, (300 / 40), 13 /*greece02*/, 0, 45, 0, 0);
// declare_object_at_cell(-(980 / 40) + 1, -242, (980 / 40), 13 /*greece02*/, 0, 225, 0, 0);
// declare_object_at_cell((1260 / 40) + 1, -127, (1420 / 40), 13 /*greece02*/, 0, 0, 0, 0);
// declare_object_at_cell((1060 / 40) + 1, -131, (1780 / 40), 13 /*greece02*/, 0, 45, 0, 0);

// declare_object_at_cell((900 / 40) + 1, -170, -(2420 / 40), 15 /*greece04*/, 0, 90, 0, 0);
// declare_object_at_cell((1140 / 40) + 1, -170, -(2180 / 40), 15 /*greece04*/, 0, 90, 0, 0);
// declare_object_at_cell((2420 / 40) + 1, -170, -(2580 / 40), 15 /*greece04*/, 0, 0, 0, 0);
// declare_object_at_cell((2180 / 40) + 1, -170, -(2340 / 40), 15 /*greece04*/, 0, 0, 0, 0);

// declare_object_at_cell((180 / 40) + 1, -465, (20 / 40), 16 /*overhang*/, 0, 90, 0, 0);
// declare_object_at_cell(-(140 / 40) + 1, -465, -(100 / 40), 16 /*overhang*/, 0, 90, 0, 0);
// declare_object_at_cell((1020 / 40) + 1, -277, -(540 / 40), 16 /*overhang*/, 0, 90, 0, 0);
// declare_object_at_cell(-(2420 / 40) + 1, -174, (1700 / 40), 16 /*overhang*/, 0, 90, 0, 0);

// declare_object_at_cell((500 / 40) + 1, -245, (2300 / 40), 16 /*overhang*/, 0, 0, 0, 0);
// declare_object_at_cell(-(300 / 40) + 1, -245, (2300 / 40), 16 /*overhang*/, 0, 0, 0, 0);

// declare_object_at_cell((1020 / 40) + 1, -254, (900 / 40), 17 /*pier1*/, 0, 270, 0, 0);

// declare_object_at_cell(-(1260 / 40) + 1, -47, -(1260 / 40), 21 /*wall1*/, 0, 180, 0, 0);
// declare_object_at_cell(-(1020 / 40) + 1, -63, -(1740 / 40), 21 /*wall1*/, 0, 225, 0, 0);

// declare_object_at_cell((1540 / 40) + 1, -100, -(1260 / 40), 24 /*OBSTCL1*/, 0, 45, 0, 0);

// declare_object_at_cell(-(980 / 40) + 1, -352, -(540 / 40), 25 /*float01*/, 0, 45, 0, 0);
// declare_object_at_cell((540 / 40) + 1, -359, -(1220 / 40), 25 /*float01*/, 0, 0, 0, 0);

// declare_object_at_cell(-(420 / 40) + 1, -318, (1580 / 40), 27 /*hiway01*/, 0, 0, 0, 0);

// declare_object_at_cell((20 / 40) + 1, -221, -(2180 / 40), 29 /*hiway03*/, 0, 0, 0, 0);

// declare_object_at_cell(-(1340 / 40) + 1, -434, (2620 / 40), 30 /*hiway04*/, 0, 90, 0, 0);
// declare_object_at_cell(-(1340 / 40) + 1, -434, (2500 / 40), 30 /*hiway04*/, 0, 90, 0, 0);
// declare_object_at_cell(-(1340 / 40) + 1, -434, (2380 / 40), 30 /*hiway04*/, 0, 90, 0, 0);
// declare_object_at_cell(-(1340 / 40) + 1, -434, (2260 / 40), 30 /*hiway04*/, 0, 90, 0, 0);
// declare_object_at_cell(-(1340 / 40) + 1, -434, (2140 / 40), 30 /*hiway04*/, 0, 90, 0, 0);
// declare_object_at_cell(-(1340 / 40) + 1, -434, (2020 / 40), 30 /*hiway04*/, 0, 90, 0, 0);
// declare_object_at_cell(-(1340 / 40) + 1, -434, (1900 / 40), 30 /*hiway04*/, 0, 90, 0, 0);

// declare_object_at_cell(-(1380 / 40) + 1, -377, (2780 / 40), 32 /*hiway06*/, 0, 180, 0, 0);

// declare_object_at_cell(-(1340 / 40) + 1, -377, (1660 / 40), 33 /*ramp01*/, 0, 0, 0, 0);
// declare_object_at_cell(-(1660 / 40) + 1, -377, (2820 / 40), 33 /*ramp01*/, 0, 270, 0, 0);

// declare_object_at_cell(-(140 / 40) + 1, -221, -(2180 / 40), 34 /*hiway07*/, 0, 0, 0, 0);
// declare_object_at_cell((140 / 40) + 1, -221, -(2180 / 40), 34 /*hiway07*/, 0, 180, 0, 0);

// declare_object_at_cell(-(2340 / 40) + 1, -270, -(2420 / 40), 35 /*tower01*/, 0, 0, 0, 0);

//Level08
//Experimental
//Last level before I start going back and testing functionality

// declare_object_at_cell((1020 / 40) + 1, 459, (2580 / 40), 61 /*start location*/, 0, 0, 0, 0);

// declare_object_at_cell(-(1020 / 40) + 1, -435, (220 / 40), 10 /*platf00*/, 0, 0, 0, 0);

// declare_object_at_cell((3020 / 40) + 1, -299, -(1020 / 40), 13 /*greece02*/, 0, 135, 0, 0);
// declare_object_at_cell((2620 / 40) + 1, -284, -(620 / 40), 13 /*greece02*/, 0, 135, 0, 0);
// declare_object_at_cell((3380 / 40) + 1, -121, (2100 / 40), 13 /*greece02*/, 0, 25, 0, 0);
// declare_object_at_cell(-(3180 / 40) + 1, -479, -(2940 / 40), 13 /*greece02*/, 0, 90, 0, 0);
// declare_object_at_cell(-(3100 / 40) + 1, -256, (1660 / 40), 13 /*greece02*/, 0, 270, 0, 0);

// declare_object_at_cell((1460 / 40) + 1, -74, (3820 / 40), 15 /*greece04*/, 0, 0, 0, 0);
// declare_object_at_cell((3700 / 40) + 1, -76, -(60 / 40), 15 /*greece04*/, 0, 135, 0, 0);
// declare_object_at_cell((3740 / 40) + 1, -78, -(1140 / 40), 15 /*greece04*/, 0, 135, 0, 0);

// declare_object_at_cell((1660 / 40) + 1, -256, -(1020 / 40), 16 /*overhang*/, 0, 0, 0, 0);
// declare_object_at_cell((1180 / 40) + 1, -280, -(980 / 40), 16 /*overhang*/, 0, 0, 0, 0);
// declare_object_at_cell(-(1660 / 40) + 1, -108, -(780 / 40), 16 /*overhang*/, 0, 90, 0, 0);
// declare_object_at_cell(-(1100 / 40) + 1, -108, -(860 / 40), 16 /*overhang*/, 0, 90, 0, 0);
// declare_object_at_cell(-(820 / 40) + 1, -210, (1220 / 40), 16 /*overhang*/, 0, 0, 0, 0);
// declare_object_at_cell(-(2140 / 40) + 1, -218, (1260 / 40), 16 /*overhang*/, 0, 0, 0, 0);
// declare_object_at_cell(-(3180 / 40) + 1, -209, (3260 / 40), 16 /*overhang*/, 0, 115, 0, 0);
// declare_object_at_cell((2980 / 40) + 1, -172, (3220 / 40), 16 /*overhang*/, 0, 25, 0, 0);
// declare_object_at_cell((3820 / 40) + 1, -168, (1020 / 40), 16 /*overhang*/, 0, 25, 0, 0);
// declare_object_at_cell(-(860 / 40) + 1, -210, (3300 / 40), 16 /*overhang*/, 0, 0, 0, 0);


// declare_object_at_cell((460 / 40) + 1, -170, -(900 / 40), 19 /*tunnel2*/, 0, 0, 0, 0);

// declare_object_at_cell(-(3220 / 40) + 1, -180, -(1260 / 40), 21 /*wall1*/, 0, 330, 0, 0);
// declare_object_at_cell(-(2380 / 40) + 1, -136, -(660 / 40), 21 /*wall1*/, 0, 300, 0, 0);
// declare_object_at_cell(-(300 / 40) + 1, -410, (2500 / 40), 21 /*wall1*/, 0, 45, 0, 0);
// declare_object_at_cell(-(3700 / 40) + 1, -169, (3660 / 40), 21 /*wall1*/, 0, 300, 0, 0);

// declare_object_at_cell((140 / 40) + 1, -520, -(2860 / 40), 22 /*float03*/, 0, 0, 0, 0);
// declare_object_at_cell(-(620 / 40) + 1, -488, -(2860 / 40), 22 /*float03*/, 0, 0, 0, 0);
// declare_object_at_cell(-(300 / 40) + 1, -232, -(800 / 40), 22 /*float03*/, 0, 0, 0, 0);
// declare_object_at_cell((580 / 40) + 1, -474, (2420 / 40), 22 /*float03*/, 0, 0, 0, 0);

// declare_object_at_cell((3460 / 40) + 1, -191, (820 / 40), 23 /*bridge2*/, 0, 115, 0, 0);
// declare_object_at_cell((2980 / 40) + 1, -187, (1940 / 40), 23 /*bridge2*/, 0, 115, 0, 0);
// declare_object_at_cell((2580 / 40) + 1, -195, (3060 / 40), 23 /*bridge2*/, 0, 115, 0, 0);

// declare_object_at_cell(-(4020 / 40) + 1, -186, (2460 / 40), 24 /*OBSTCL1*/, 0, 0, 0, 0);

// declare_object_at_cell(-(3180 / 40) + 1, -373, -(2220 / 40), 25 /*float01*/, 0, 0, 0, 0);
// declare_object_at_cell((1460 / 40) + 1, -391, (100 / 40), 25 /*float01*/, 0, 0, 0, 0);

// declare_object_at_cell(-(660 / 40) + 1, -155, -(940 / 40), 27 /*hiway01*/, 0, 0, 0, 0);
// declare_object_at_cell((20 / 40) + 1, -155, -(860 / 40), 27 /*hiway01*/, 0, 0, 0, 0);

// declare_object_at_cell(-(2180 / 40) + 1, -214, (3660 / 40), 33 /*ramp01*/, 0, 180, 0, 0);
// declare_object_at_cell((1460 / 40) + 1, -244, (580 / 40), 33 /*ramp01*/, 0, 180, 0, 0);

// declare_object_at_cell(-(2180 / 40) + 1, -272, (3420 / 40), 34 /*hiway07*/, 0, 90, 0, 0);
// declare_object_at_cell((1460 / 40) + 1, -301, (340 / 40), 34 /*hiway07*/, 0, 90, 0, 0);

}


void	process_tga_as_ldata(void * source_data)
{
	
	unsigned char * readByte = (unsigned char *)source_data;
	
	unsigned char id_field_size = readByte[0];
	unsigned char col_map_type = readByte[1]; 
	unsigned char data_type = readByte[2];	
	
	unsigned char xSizeLoBits = readByte[12];
	unsigned char xSizeHiBits = readByte[13];
	unsigned char ySizeLoBits = readByte[14];
	unsigned char ySizeHiBits = readByte[15];
	unsigned short xSize = xSizeLoBits | (xSizeHiBits<<8);
	unsigned short ySize = ySizeLoBits | (ySizeHiBits<<8);
	unsigned char byteFormat = readByte[16];
	
	if(col_map_type != 0){
		nbg_sprintf(0, 0, "(REJECTED NON-RGB TGA)");
		return;
	}
	
	if(data_type != 2) {
		nbg_sprintf(0, 0, "(REJECTED RLE TGA)");
		return;
	}
	//Color Map Specification Data is ignored.
	
	//X / Y origin data is ignored.
	
	if(byteFormat != 24){
		nbg_sprintf(0, 0, "(TGA NOT 24BPP)");
		return; //File Typing Demands 24 bpp.
	}
	
	
//I want to make a command line executable that can parse an image file into these declarations.
//Conceptual: 24bit RGB TGA
//Bits 0-4: Object type
//Bits 5-8: (Vertical offset)
//Bits 9-11: Rotation sign bits
//Bits 12-15: (x rotation)<<8 [degrees]
//Bits 16-19: (y rotation)<<12
//Bits 20-23: (z rotation)<<16
//Object types: 32 max
//Height increments: 4[<<16] | 60[<<16] max, unsigned
//Rotational increments: 16 degrees / 240 max, signed [via sign bits]
	
	unsigned int imdat = id_field_size + 18;
	//int totalPix = xSize * ySize;
	//int yspot;
	
	//	nbg_sprintf(0, 0, "(%i)", totalPix);
	
	objNEW = 0;
	for(int k = 0; k < 8; k++)
	{
		link_starts[k] = -1; //Re-set link starts to no links conidition
	} 
	
		for(int i = 0; i < ySize; i++)
		{
		//nbg_sprintf(0, 0, "(DECL)"); //Debug ONLY
			//yspot = i * ySize;
			for(int k = 0; k < xSize; k++)
			{
		//If the pixel is all high (white), dont use it.
		if(readByte[imdat] != 0xFF && readByte[imdat+1] != 0xFF && readByte[imdat+2] != 0xFF) 
		{
			//Item location x y z, item type, item rotation x y z.
	declare_object_at_cell(k-(xSize>>1), -((readByte[imdat] & 0xE0) | ((readByte[imdat+1] & 1)<<8)), i-(ySize>>1), readByte[imdat] & 0x1F, 
	(readByte[imdat+1] & 2) ? -(readByte[imdat+1] & 0xF0) : (readByte[imdat+1] & 0xF0),
	(readByte[imdat+1] & 4) ? -(readByte[imdat+2] & 0xF)<<4 : (readByte[imdat+2] & 0xF)<<4,
	(readByte[imdat+1] & 8) ? -(readByte[imdat+2] & 0xF0) : (readByte[imdat+2] & 0xF0), 0);
		}
			imdat += 3;
			}
		}

}

void	level_data_basic(void)
{
	
	declarations();
	ldata_ready = true;

	post_ldata_init_building_object_search();
	
		//////////////////////////////////
		//
		// Send player to map's start location
		// Notice: Code chunk must run after objects are declared
		// Notice: Only finds first start location, uses it, then flags it as used.
		//////////////////////////////////
	for(int i = 0; i < objNEW; i++)
	{
		//The following condition should indicate that we've found a declared player start and it hasn't been used yet.
		if((dWorldObjects[i].type.ext_dat & LDATA_TYPE) == PSTART && (dWorldObjects[i].type.ext_dat & ETYPE) == LDATA)
		{
			//They're negative because of COORDINATE SYSTEM MAYHEM.
			you.startPos[X] = -dWorldObjects[i].pos[X];
			you.startPos[Y] = -dWorldObjects[i].pos[Y];
			you.startPos[Z] = -dWorldObjects[i].pos[Z];
			reset_player();
			dWorldObjects[i].type.ext_dat |= 0x8000;
		}
	}
}


