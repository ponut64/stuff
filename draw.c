
#include <jo/jo.h>

#include "draw.h"
#include "height.h"

FIXED sun_light[3] = {32768, 0, 0};
//Player Model
entity_t pl_model;
//Player's Shadow
entity_t shadow;
//Heightmap Matrix
MATRIX hmap_mtx;
MATRIX unit;

FIXED hmap_matrix_pos[XYZ] = {0, 0, 0};
FIXED hmap_actual_pos[XYZ] = {0, 0, 0};

//Note: global light is in order of BLUE, GREEN, RED. [BGR]
char globalLight[3] = {0, 0, 0};
unsigned char glblLightApplied = false;
unsigned char * backScrn = (unsigned char *)VDP2_RAMBASE;

void	computeLight(void)
{

	if(glblLightApplied == false)
	{
	unsigned char color[3] = {0, 0, 0};
	short finalColors[3] = {0, 0, 0};
	unsigned char * palPtrCpy = (unsigned char *)&sprPaletteCopy[0];
		
		
		finalColors[0] = backScrn[0] + globalLight[0];
		backScrn[0] = (finalColors[0] < 255) ? (finalColors[0] < 0) ? 0 : finalColors[0] : 255;
		finalColors[1] = backScrn[1] + globalLight[1];
		backScrn[1] = (finalColors[1] < 255) ? (finalColors[1] < 0) ? 0 : finalColors[1] : 255;
		finalColors[2] = backScrn[2] + globalLight[2];
		backScrn[2] = (finalColors[2] < 255) ? (finalColors[2] < 0) ? 0 : finalColors[2] : 255;
		
	for(int i = 0; i < 1024; ){
		/*
		Explanation:
		Endianess of 24-bits in 32-bits is that byte 0 of the 4-bytes is empty.
		That's why we start with i+1, then do i+2, i+3 etc.
		What we do:
		In Work RAM is a copy of the palette, called sprPaletteCopy.
		palPtrCopy is a byte-wise pointer to this array.
		The global light / global palette offset is to be added to ever member of this,
		and then shoved out to color RAM.
		**Without modifying the palette copy**
		sprPalette is a pointer to the palette in color RAM.
		The color is 24-bit but it's in a 32-bit space.
		1 byte blue (second, bytewise), 1 byte green (third, bytewise), 1 byte red (fourth, bytewise).
		*/
		color[0] = palPtrCpy[i+1];
		color[1] = palPtrCpy[i+2];
		color[2] = palPtrCpy[i+3];
		
		finalColors[0] = color[0] + globalLight[0];
		color[0] = (finalColors[0] < 255) ? (finalColors[0] < 0) ? 0 : finalColors[0] : 255;
		finalColors[1] = color[1] + globalLight[1];
		color[1] = (finalColors[1] < 255) ? (finalColors[1] < 0) ? 0 : finalColors[1] : 255;
		finalColors[2] = color[2] + globalLight[2];
		color[2] = (finalColors[2] < 255) ? (finalColors[2] < 0) ? 0 : finalColors[2] : 255;
		
		sprPalette[i+1] = color[0];
		sprPalette[i+2] = color[1];
		sprPalette[i+3] = color[2];
		
		i+=4;
	}
	
		//Next, set the sun light.
		active_lights[0].pop = 1;
		active_lights[0].ambient_light = &sun_light[0];

		glblLightApplied = true;
	}
}


void	initCamera(void)
{
	//slWindow(16, 8, 336, 232, draw_distance, JO_TV_WIDTH_2, JO_TV_HEIGHT_2);
	slWindow(0, 0, JO_TV_WIDTH-1, JO_TV_HEIGHT-1, 2000, JO_TV_WIDTH_2, JO_TV_HEIGHT_2);
	slZdspLevel(0);
	slPerspective(DEGtoANG(90)); //FOV
}

void	set_camera(void)
{
 POINT viewPnt = {0, 0, 1<<16};	

	slLookAt( viewPnt , zPt , 0 );
}

void	master_draw_stats(void)
{
	slPrintFX(you.pos[X], slLocate(9, 1));
	slPrintFX(you.pos[Y], slLocate(19, 1));
	slPrintFX(you.pos[Z], slLocate(29, 1));

	jo_printf(18, 0, "(File System Status)");
	jo_printf(27, 2, "Pts :%x:", you.points);
	jo_printf(10, 2, "throttle:(%i)", you.IPaccel);
	slPrintFX(you.sanics, slLocate(26, 3));
	jo_printf(8, 25, "TRPLY:                ");
	jo_printf(8, 25, "TRPLY:%i", transPolys[0]);
	jo_printf(8, 26, "SNTPL:                ");
	jo_printf(8, 26, "SNTPL:%i", ssh2SentPolys[0] + msh2SentPolys[0]);
	jo_printf(8, 27, "VERTS:                ");
	jo_printf(8, 27, "VERTS:%i", transVerts[0]);
	jo_printf(37, 26, "cX(%i)", you.cellPos[X]);
	jo_printf(37, 27, "cY(%i)", you.cellPos[Y]);    
}

void	player_draw(void)
{
	slPushMatrix();
	{
		
		static MATRIX mat;

		mat[X][X] = pl_RBB.UVX[X];
		mat[X][Y] = pl_RBB.UVX[Y];
		mat[X][Z] = pl_RBB.UVX[Z];
		mat[3][0] = -pl_RBB.pos[X]; //POS
		
		mat[Y][X] = pl_RBB.UVY[X];
		mat[Y][Y] = pl_RBB.UVY[Y];
		mat[Y][Z] = pl_RBB.UVY[Z];
		mat[3][1] = -pl_RBB.pos[Y]; //POS
		
		mat[Z][X] = pl_RBB.UVZ[X];
		mat[Z][Y] = pl_RBB.UVZ[Y];
		mat[Z][Z] = pl_RBB.UVZ[Z];
		mat[3][2] = -pl_RBB.pos[Z]; //Position

		slMultiMatrix(mat); //Multiplies bound box matrix parameters by global view rotation parameters (really nice!)
		
		pl_model.prematrix = &pl_RBB.UVX[0];
		pl_model.isPlayer = 'Y';
		
//Animation Chains
					static int airTimer = 0;
			if(pl_model.file_done == true){
				if(you.onSurface == true){
				airTimer = 0;
					if(you.setSlide != true && airTimer == 0){
						if(you.Velocity[X] == 0 && you.Velocity[Y] == 0 && you.Velocity[Z] == 0){
							ssh2DrawAnimation(&idle, &pl_model, you.pos, false);
						} else if( (you.Velocity[X] != 0 || you.Velocity[Z] != 0) && you.dirInp){
						if(you.IPaccel < 0){
							ssh2DrawAnimation(&stop, &pl_model, you.pos, false);
							}
						if(you.sanics < 2<<16 && you.IPaccel > 0){
							ssh2DrawAnimation(&walk, &pl_model, you.pos, true);
							}
						if(you.sanics < 3<<16 && you.sanics > 2<<16){
							ssh2DrawAnimation(&run, &pl_model, you.pos, true);
							}
						if(you.sanics >= 3<<16){
							ssh2DrawAnimation(&dbound, &pl_model, you.pos, true);
							}
						} else if((you.Velocity[X] != 0 || you.Velocity[Z] != 0) && !you.dirInp){
						ssh2DrawAnimation(&stop, &pl_model, you.pos, false);
						} else {
						ssh2DrawAnimation(&idle, &pl_model, you.pos, false);
						}	
					} else {//IF NOT SLIDE ENDIF
						if(is_key_pressed(DIGI_RIGHT)){
						ssh2DrawAnimation(&slideRln, &pl_model, you.pos, false);
						} else if(is_key_pressed(DIGI_LEFT)){
						ssh2DrawAnimation(&slideLln, &pl_model, you.pos, false);
						} else if(is_key_pressed(DIGI_UP)){
						ssh2DrawAnimation(&slideIdle, &pl_model, you.pos, false);
						} else if(is_key_pressed(DIGI_DOWN)){
						ssh2DrawAnimation(&slideIdle, &pl_model, you.pos, false);
						} else {
						ssh2DrawAnimation(&slideIdle, &pl_model, you.pos, false);
						}
						}//IF SLIDE ENDIF
						
					
				} else {//IF SURFACE ENDIF
						airTimer++;
						if(airTimer < 8 && airTimer != 0 && you.Velocity[Y] != 0){
							ssh2DrawAnimation(&jump, &pl_model, you.pos, false);
						} else if(is_key_pressed(DIGI_RIGHT)){
						ssh2DrawAnimation(&airRight, &pl_model, you.pos, false);
						} else if(is_key_pressed(DIGI_LEFT)){
						ssh2DrawAnimation(&airLeft, &pl_model, you.pos, false);
						} else if(is_key_pressed(DIGI_DOWN)){
						ssh2DrawAnimation(&airIdle, &pl_model, you.pos, false);
						} else {
						ssh2DrawAnimation(&airIdle, &pl_model, you.pos, false);
						}
				}//IF AIR ENDIF
			} //IF MODEL LOADED ENDIF
	}

	slPopMatrix();

}

void	obj_draw_queue(void)
{
		static MATRIX mat;
		static MATRIX matSt;
		
	for( unsigned char i = 0; i < MAX_PHYS_PROXY; i++){
		if(RBBs[i].isBoxPop != true) continue;
		
		unsigned short objType = (dWorldObjects[activeObjects[i]].type.ext_dat & 0x7000);
		
	slPushMatrix();
		slGetMatrix(matSt);
		mat[X][X] = RBBs[i].UVX[X];
		mat[X][Y] = RBBs[i].UVX[Y];
		mat[X][Z] = RBBs[i].UVX[Z];
		mat[3][0] = RBBs[i].pos[X]; //POS
		
		mat[Y][X] = RBBs[i].UVY[X];
		mat[Y][Y] = RBBs[i].UVY[Y];
		mat[Y][Z] = RBBs[i].UVY[Z];
		mat[3][1] = RBBs[i].pos[Y]; //POS
		
		mat[Z][X] = RBBs[i].UVZ[X];
		mat[Z][Y] = RBBs[i].UVZ[Y];
		mat[Z][Z] = RBBs[i].UVZ[Z];
		mat[3][2] = RBBs[i].pos[Z]; //Position
	
					if( objType != ITEM && objType != LDATA ){ //Check if entity is NOT ITEM or LDATA

		slMultiMatrix(mat); //Multiplies bound box matrix parameters by global view rotation parameters (really nice!)

		entities[objDRAW[i]].prematrix = &RBBs[i].UVX[0];
		
		ssh2DrawModel(&entities[objDRAW[i]], RBBs[i].pos);
		
					} else if( objType == ITEM ){ //if entity IS ITEM
		
			if( !(dWorldObjects[activeObjects[i]].type.ext_dat & 8) ) //Check if root entity still exists
			{
		slMultiMatrix(mat);
		
		entities[objDRAW[i]].prematrix = &RBBs[i].UVX[0];
		
		ssh2DrawModel(&entities[objDRAW[i]], RBBs[i].pos);
			}
		
					}
	slPopMatrix();
	}
	
}

void	shadow_draw(void)
{
	static char first_run;
		if(shadow.file_done == true)
		{
			if(first_run != true){
	//Special Shadow Param (just for MESHon lol)
	for(int i = 0; i < shadow.pol[0]->nbPolygon; i++){
	shadow.pol[0]->attbl[i] = (ATTR)ATTRIBUTE(Dual_Plane, SORT_CEN, shadow.pol[0]->attbl[i].texno, 0, No_Gouraud,Window_In|MESHoff|HSSon|ECdis | SPenb |CL64Bnk |MSBon,sprNoflip,UseNearClip);
	shadow.pol[0]->pltbl[i].norm[X] = 0;
	shadow.pol[0]->pltbl[i].norm[Y] = 0;
	shadow.pol[0]->pltbl[i].norm[Z] = 0;
	}
			first_run = true;
			}
			
	slPushMatrix();

		static MATRIX mat;
		//Make shadow match player rotation. I mean, it's not a perfect solution, but it mostly works.
		mat[X][X] = pl_RBB.UVX[X];
		mat[X][Y] = pl_RBB.UVX[Y];
		mat[X][Z] = pl_RBB.UVX[Z];
		mat[3][0] = -you.shadowPos[X]; //POS
		
		mat[Y][X] = pl_RBB.UVY[X];
		mat[Y][Y] = pl_RBB.UVY[Y];
		mat[Y][Z] = pl_RBB.UVY[Z];
		mat[3][1] = -(you.shadowPos[Y] + 8196); //POS
		
		mat[Z][X] = pl_RBB.UVZ[X];
		mat[Z][Y] = pl_RBB.UVZ[Y];
		mat[Z][Z] = pl_RBB.UVZ[Z];
		mat[3][2] = -you.shadowPos[Z]; //Position

		slMultiMatrix(mat); //Multiplies bound box matrix parameters by global view rotation parameters (really nice!)

	ssh2DrawModel(&shadow, you.shadowPos);
	
	slPopMatrix();
		}
}

void	object_draw(void)
{
	
			//Test Light
		active_lights[0].location[X] = you.pos[X];
		active_lights[0].location[Y] = you.pos[Y] + (15<<16);
		active_lights[0].location[Z] = you.pos[Z];
		active_lights[0].bright = 8000;
			//
	
	slPushMatrix();
	{	
	slTranslate((VIEW_OFFSET_X<<16), (VIEW_OFFSET_Y<<16), (VIEW_OFFSET_Z<<16) );
	
	//Take care about the order of the matrix transformations!
	slRotX((you.viewRot[X]));
	slRotY((you.viewRot[Y]));

		//
	slTranslate(you.pos[X], you.pos[Y], you.pos[Z]);
	player_draw();
	shadow_draw();
		
	obj_draw_queue();
	}
	slPopMatrix();
	
}

void	map_draw(void){	

	while(dsp_noti_addr[0] == 0){}; //"DSP Wait"
update_hmap(hmap_mtx, sun_light);

}

void	prep_map_mtx(void) //Uses SGL to prepare the matrix for the map, so it doesn't mess up the matrix stack when the map draws
{
	slInitMatrix();
	set_camera();
	slPushMatrix();
	{
	slTranslate((VIEW_OFFSET_X<<16), (VIEW_OFFSET_Y<<16), (VIEW_OFFSET_Z<<16) );
	slRotX((you.viewRot[X]));
	slRotY((you.viewRot[Y]));
	slTranslate(hmap_matrix_pos[X], you.pos[Y], hmap_matrix_pos[Z]);
	slGetMatrix(hmap_mtx);
	}
	slPopMatrix();
}

void	master_draw(void)
{
	prep_map_mtx();
	computeLight();
	slSlaveFunc(object_draw, 0); //Get SSH2 busy with its drawing stack ASAP
	slCashPurge();

	hmap_cluster();
	map_draw();
	
	you.prevCellPos[X] = you.cellPos[X];
	you.prevCellPos[Y] = you.cellPos[Y];
	you.prevDispPos[X] = you.dispPos[X];
	you.prevDispPos[Y] = you.dispPos[Y];
	//View Distance Extention -- Makes turning view cause performance issue, beware?
	you.cellPos[X] = (fxm((INV_CELL_SIZE), you.pos[X])>>16);
	you.cellPos[Y] = (fxm((INV_CELL_SIZE), you.pos[Z])>>16);
	int sineY = fxm(slSin(-you.viewRot[Y]), 275<<16);
	int sineX = fxm(slCos(-you.viewRot[Y]), 275<<16);
	you.dispPos[X] = (fxm((INV_CELL_SIZE), you.pos[X] +  sineY)>>16);
	you.dispPos[Y] = (fxm((INV_CELL_SIZE), you.pos[Z] +  sineX)>>16);
	//
	hmap_matrix_pos[X] = (you.pos[X] + you.Velocity[X]) - ((you.dispPos[X] * CELL_SIZE_INT)<<16);
	hmap_matrix_pos[Z] = (you.pos[Z] + you.Velocity[Z]) - ((you.dispPos[Y] * CELL_SIZE_INT)<<16);
	
	hmap_actual_pos[X] = hmap_matrix_pos[X] - (you.pos[X] + you.Velocity[X]);
	hmap_actual_pos[Y] = 0;
	hmap_actual_pos[Z] = hmap_matrix_pos[Z] - (you.pos[Z] + you.Velocity[Z]);
	
	//No Touch Order -- Affects animations/mechanics
	player_phys_affect();
		mypad();
	player_collision_test_loop();		//These are here because actually, the MSH2 is getting pretty hammered.
	collide_with_heightmap(&pl_RBB);
	object_control_loop(you.dispPos);	//It does reduce the max poly # but the MSH2 is very focused on the map and must draw it, so we are freed up there.
	//
	
	
	run_dsp(); //Run the DSP now to give it maximum time to complete (minimize sh2 wait)
	slSlaveFunc(sort_master_polys, 0);	
}

