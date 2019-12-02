
#include <jo/jo.h>

#include "draw.h"
#include "height.h"
#include "game.c"

FIXED surf_lit[3] = {0, 65535, 0};
FIXED light[3] = {0, 65535, 0};
FIXED hmap_matrix_pos[XYZ] = {0, 0, 0};
//Player Model
entity_t pl_model;
//Player's Shadow
entity_t shadow;
//Heightmap Matrix
MATRIX hmap_mtx;
MATRIX unit;

void	computeLight(void)
{
	//RIP
}


void	initCamera(jo_camera * curCam)
{
	(*curCam).viewpoint[X] = (0.0);
	(*curCam).viewpoint[Y] = (0.0);
	(*curCam).viewpoint[Z] = 65536;
	(*curCam).z_angle = DEGtoANG(0.0);
	(*curCam).target[X] = 0;
	(*curCam).target[Y] = 0;
	(*curCam).target[Z] = 0;
	//slWindow(16, 8, 336, 232, draw_distance, JO_TV_WIDTH_2, JO_TV_HEIGHT_2);
	slWindow(0, 0, JO_TV_WIDTH-1, JO_TV_HEIGHT-1, 1200, JO_TV_WIDTH_2, JO_TV_HEIGHT_2);
	slZdspLevel(0);
	slPerspective(DEGtoANG(90)); //FOV
}

void	set_camera(void)
{
//current player variable input type of _player has component type rcam which has viewpoint target and zangle	
	slLookAt( you.rcam.viewpoint , you.rcam.target , you.rcam.z_angle );
}

void	master_draw_stats(void)
{
	slPrintFX(you.pos[X], slLocate(0, 1));
	slPrintFX(you.pos[Y], slLocate(10, 1));
	slPrintFX(you.pos[Z], slLocate(20, 1));

	jo_printf(18, 0, "(File System Status)");
	jo_printf(14, 4, "Y rot:(%i)", you.rot[Y] / 182);
	jo_printf(0, 4, "throttle:(%i)", you.IPaccel);
	jo_printf(0, 25, "Total quads :                      ");
	jo_printf(0, 25, "Total quads : %d", transVerts[0]);
	jo_printf(0, 26, "Disp quads :                  ");
	jo_printf(0, 26, "Disp quads :  %d", ssh2SentPolys[0] + msh2SentPolys[0]);
	jo_printf(0, 27, "Vertices :                  ");
	jo_printf(0, 27, "Vertices :    %d", transVerts[0]);
	jo_printf(27, 26, "Cell Pos X (%i)", you.cellPos[X]);
	jo_printf(27, 27, "Cell Pos Z (%i)", you.cellPos[Y]);    
}

void	player_draw(void)
{
	slPushMatrix();
	{
		
		static MATRIX mat;

		mat[X][X] = pl_RBB.UVX[X];
		mat[X][Y] = pl_RBB.UVX[Y];
		mat[X][Z] = pl_RBB.UVX[Z];
		mat[3][0] = 0; //POS
		
		mat[Y][X] = pl_RBB.UVY[X];
		mat[Y][Y] = pl_RBB.UVY[Y];
		mat[Y][Z] = pl_RBB.UVY[Z];
		mat[3][1] = 0; //POS
		
		mat[Z][X] = pl_RBB.UVZ[X];
		mat[Z][Y] = pl_RBB.UVZ[Y];
		mat[Z][Z] = pl_RBB.UVZ[Z];
		mat[3][2] = 0; //Position

		slMultiMatrix(mat); //Multiplies bound box matrix parameters by global view rotation parameters (really nice!)
		
		POINT plLit = {mat[X][Y], mat[Y][Y], mat[Z][Y]};
		
//Animation Chains
					static int airTimer = 0;
			if(pl_model.file_done == true){
				if(you.onSurface == true){
				airTimer = 0;
					if(you.setSlide != true && airTimer == 0){
						if(you.Velocity[X] == 0 && you.Velocity[Y] == 0 && you.Velocity[Z] == 0){
							ssh2DrawAnimation(&idle, &pl_model, plLit);
						} else if( (you.Velocity[X] != 0 || you.Velocity[Z] != 0) && you.dirInp){
						if(you.IPaccel < 0){
							ssh2DrawAnimation(&stop, &pl_model, plLit);
							}
						if(you.IPaccel < 13000 && you.IPaccel > 0){
							ssh2DrawAnimation(&forward, &pl_model, plLit);
							}
						if(you.IPaccel < 24000 && you.IPaccel > 13000){
							ssh2DrawAnimation(&run, &pl_model, plLit);
							}
						if(you.IPaccel >= 24000){
							ssh2DrawAnimation(&dbound, &pl_model, plLit);
							}
						} else if((you.Velocity[X] != 0 || you.Velocity[Z] != 0) && !you.dirInp){
						ssh2DrawAnimation(&stop, &pl_model, plLit);
						} else {
						ssh2DrawAnimation(&idle, &pl_model, plLit);
						}	
					} else {//IF NOT SLIDE ENDIF
						if(jo_is_input_key_pressed(0, JO_KEY_RIGHT)){
						ssh2DrawAnimation(&slideRln, &pl_model, plLit);
						} else if(jo_is_input_key_pressed(0, JO_KEY_LEFT)){
						ssh2DrawAnimation(&slideLln, &pl_model, plLit);
						} else if(jo_is_input_key_pressed(0, JO_KEY_UP)){
						ssh2DrawAnimation(&slideFwd, &pl_model, plLit);
						} else if(jo_is_input_key_pressed(0, JO_KEY_DOWN)){
						ssh2DrawAnimation(&slideRvs, &pl_model, plLit);
						} else {
						ssh2DrawAnimation(&slideIdle, &pl_model, plLit);
						}
						}//IF SLIDE ENDIF
						
					
				} else {//IF SURFACE ENDIF
						airTimer++;
						if(airTimer < 8 && airTimer != 0 && you.Velocity[Y] != 0){
							if(you.setSlide == true){
							ssh2DrawAnimation(&jump2, &pl_model, light);
							} else {
							ssh2DrawAnimation(&jump, &pl_model, light);
							}
						} else if(jo_is_input_key_pressed(0, JO_KEY_RIGHT)){
						ssh2DrawAnimation(&airRight, &pl_model, light);
						} else if(jo_is_input_key_pressed(0, JO_KEY_LEFT)){
						ssh2DrawAnimation(&airLeft, &pl_model, light);
						} else if(jo_is_input_key_pressed(0, JO_KEY_DOWN)){
						ssh2DrawAnimation(&airBack, &pl_model, light);
						} else {
						ssh2DrawAnimation(&airIdle, &pl_model, light);
						}
				}//IF AIR ENDIF
			} //IF MODEL LOADED ENDIF
	}

	slPopMatrix();

}

void	disp_pt_as_2D(POINT inpPt, POINT pl_location, Sint32 oldDispPix[XY])
{
//Input Point needs to be put as relative to the screen, which is where you are, so player location is used.
//If it is a point already relative to the player, ignore player location.
//In addition to this, for some reason I need to invert the matrix to use slConvert3Dto2D.
	slPushMatrix();

		static MATRIX mat;
		mat[X][X] = -65536;
		mat[Y][Y] = -65536;
		mat[Z][Z] = -65536;
		slMultiMatrix(mat); 
		
		static Sint32 dispPix[XY] = {0, 0};
		POINT usedPt = { (inpPt[X] - pl_location[X]), (inpPt[Y] - pl_location[Y]), (inpPt[Z] - pl_location[Z]) };
		if(usedPt[X] != 0 && usedPt[Y] != 0 && usedPt[Z] != 0){
		slConvert3Dto2D(usedPt, dispPix);
		}
		// jo_printf(0, 16, "(%i)", dispPix[X]);
		// jo_printf(0, 17, "(%i)", dispPix[Y]);
		jo_draw_background_square(dispPix[X]+172, dispPix[Y]+108, 8, 8, JO_COLOR_Yellow);
		if(dispPix[X] != oldDispPix[X] || dispPix[Y] != oldDispPix[Y]){
		jo_draw_background_square(oldDispPix[X]+172, oldDispPix[Y]+108, 8, 8, JO_COLOR_Transparent);
		}
		oldDispPix[X] = dispPix[X];
		oldDispPix[Y] = dispPix[Y];
	slPopMatrix();
}

void	box_phys_helper(_boundBox * tbox, FIXED offset[XYZ]){
		static Sint32 op1[XY] = {0, 0};
		static Sint32 op2[XY] = {0, 0};
		static Sint32 op3[XY] = {0, 0};
		static Sint32 op4[XY] = {0, 0};
		static Sint32 op5[XY] = {0, 0};
		static Sint32 op6[XY] = {0, 0};
		disp_pt_as_2D(tbox->Xplus, offset, op1);
		disp_pt_as_2D(tbox->Xneg, offset, op2);
		disp_pt_as_2D(tbox->Yplus, offset, op3);
		disp_pt_as_2D(tbox->Yneg, offset, op4);
		disp_pt_as_2D(tbox->Zplus, offset, op5);
		disp_pt_as_2D(tbox->Zneg, offset, op6);
}

void	obj_draw_queue(void)
{
	
		static MATRIX mat;
	for( unsigned char i = 0; i < MAX_PHYS_PROXY; i++){
		if(RBBs[i].isBoxPop != true) continue;
	slPushMatrix();
		
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

		slMultiMatrix(mat); //Multiplies bound box matrix parameters by global view rotation parameters (really nice!)
		//Why use my own matrix function? 1. Some separation from SGL, 2. Easier to port (if ever neccessary), 3. Simplified rotation order 
		
		POINT objLit = {mat[X][Y], mat[Y][Y], mat[Z][Y]};
		
	ssh2DrawModel(&entities[objDRAW[i]], objLit);
	
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

	ssh2DrawModel(&shadow, light);
	
	slPopMatrix();
		}
}

void	object_draw(void)
{
	slPushMatrix();
	{	
	slTranslate((VIEW_OFFSET_X<<16), (VIEW_OFFSET_Y<<16), (VIEW_OFFSET_Z<<16) );
	
	//Take care about the order of the matrix transformations!
	slRotX((you.viewRot[X]));
	slRotY((you.viewRot[Y]));
	player_draw();

	//box_phys_helper(&pl_RBB, zPt);
		//
	slTranslate(you.pos[X], you.pos[Y], you.pos[Z]);
	shadow_draw();
		
	obj_draw_queue();


	}
	slPopMatrix();
}

void	map_draw(void){	
/*
Heightmap Occlusion Idea:
If between the projection window and an entity there exists any yValue between the entitie's bounding box vertices greater than all of the vertices, do not bother sending the putpolygon command.
First of course we need to figure out how to walk the heightmap in a particular direction.
*/
entity_t MapDat;

MapDat.pol[0] = (XPDATA*)polymap;
MapDat.file_done = true;

msh2DrawModel(&MapDat, hmap_mtx, surf_lit);

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
	slSlaveFunc(object_draw, 0); //Get SSH2 busy with its drawing stack ASAP
	slCashPurge();

	hmap_cluster();
	computeLight();

	map_draw();

	you.prevCellPos[X] = you.cellPos[X];
	you.prevCellPos[Y] = you.cellPos[Y];
	you.prevDispPos[X] = you.dispPos[X];
	you.prevDispPos[Y] = you.dispPos[Y];
	//View Distance Extention -- Makes turning view cause performance issue, beware?
	you.cellPos[X] = (fxm((INV_CELL_SIZE), you.pos[X])>>16);
	you.cellPos[Y] = (fxm((INV_CELL_SIZE), you.pos[Z])>>16);
	you.dispPos[X] = (fxm((INV_CELL_SIZE), you.pos[X] + fxm(slSin(-you.viewRot[Y]), 225<<16) )>>16);
	you.dispPos[Y] = (fxm((INV_CELL_SIZE), you.pos[Z] + fxm(slCos(-you.viewRot[Y]), 225<<16) )>>16);
	//
	hmap_matrix_pos[X] = (you.pos[X]+you.Velocity[X]) - ((you.dispPos[X] * CELL_SIZE_INT)<<16);
	hmap_matrix_pos[Z] = (you.pos[Z]+you.Velocity[Z]) - ((you.dispPos[Y] * CELL_SIZE_INT)<<16);
	
	slSlaveFunc(sort_master_polys, 0);	
}

