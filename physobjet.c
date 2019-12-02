#include "physobjet.h"

_sobject SimpleBox = {
	.entity_ID = 0,
	.radius[X] = 15,
	.radius[Y] = 15,
	.radius[Z] = 15
};

_sobject Torus = {
	.entity_ID = 1,
	.radius[X] = 15,
	.radius[Y] = 15,
	.radius[Z] = 15
};

//When loading from binary map data, dWorldObjects is what's loaded. objNEW value is also loaded and set each frame.
//That does mean every dWorldObjects entry is going to contain its own type data. That's ok. It's not like the map is going to be aware of its own RAM pointers.
_declaredObject dWorldObjects[256];
unsigned char objNEW = 0;
unsigned char objDRAW[256];
int numBoxChecks = 0;
short numGateChecks = 0;
FIXED planeDist[MAX_PHYS_PROXY];
FIXED lastPlaneDist[MAX_PHYS_PROXY];

void	declare_object_at_cell(short pixX, short pixY, _sobject type, ANGLE xrot, ANGLE yrot, ANGLE zrot, short height)
{
	dWorldObjects[objNEW].pix[X] = -pixX;
	dWorldObjects[objNEW].pix[Y] = -pixY;
	dWorldObjects[objNEW].type = type;
	dWorldObjects[objNEW].srot[X] = xrot;
	dWorldObjects[objNEW].srot[Y] = yrot;
	dWorldObjects[objNEW].srot[Z] = zrot;
	dWorldObjects[objNEW].height = height;
	objNEW++;
}

//Note that this function is intentianally strict, and will not permit passing above or below between two objects.
bool	has_entity_passed_between(short obj_id1, short obj_id2, _boundBox * tgt)
{	
	//numGateChecks is a unique index into the planeDists and lastPlaneDists. The limit is the phys proxies. It's all good,
	//as we'll never be testing more gates than we have objects in the visible area.
	numGateChecks = (numGateChecks < MAX_PHYS_PROXY) ? numGateChecks + 1 : numGateChecks;
	//An objective check to see if either of the two objects are in the render area.
	//You could swap this for an objective pixel - cell distance check.
	if( (dWorldObjects[obj_id1].ext_dat & 32768) != 32768 && (dWorldObjects[obj_id2].ext_dat & 32768) != 32768)
	{
		planeDist[numGateChecks] = 0; //Data cleanup to prevent errant positive detection when culling logic is passed,
		lastPlaneDist[numGateChecks] = 0; //when previously it may noy have been.
		return false;
	};
	
	POINT prevPos;
	prevPos[X] = tgt->pos[X] - tgt->velocity[X];
	prevPos[Y] = tgt->pos[Y] - tgt->velocity[Y];
	prevPos[Z] = tgt->pos[Z] - tgt->velocity[Z];
	
	POINT fenceA;
	POINT fenceB;
	POINT fenceC;
	POINT fenceD;
	POINT tgtRelPos = {0, 0, 0};
	VECTOR rminusb = {0, 0, 0};
	VECTOR sminusb = {0, 0, 0};
	VECTOR cross = {0, 0, 0};
	VECTOR faceNormal = {0, 0, 0};
	VECTOR edgePrj0 = {0, 0, 0};
	POINT centerFace = {0, 0, 0};
	VECTOR outV0;
	register FIXED radius1;
	register FIXED radius2;
	register FIXED bigRadius;
	
	//Extrapolate a quad out of the pix given
	//	0 - 1 // B - D
	//	3 - 2 // A - C
	fenceA[X] = -dWorldObjects[obj_id1].pix[X] * (25<<16);
	fenceA[Y] = -(-(dWorldObjects[obj_id1].height<<16) - (main_map[ (-dWorldObjects[obj_id1].pix[X] + (main_map_x_pix * dWorldObjects[obj_id1].pix[Y]) + (main_map_total_pix>>1)) ]<<16));
	fenceA[Z] = -dWorldObjects[obj_id1].pix[Y] * (25<<16);
	
	fenceB[X] = fenceA[X];
	fenceB[Y] = fenceA[Y] + (dWorldObjects[obj_id1].type.radius[Y]<<17);
	fenceB[Z] = fenceA[Z];
	
	fenceC[X] = -dWorldObjects[obj_id2].pix[X] * (25<<16);
	fenceC[Y] = -(-(dWorldObjects[obj_id2].height<<16) - (main_map[ (-dWorldObjects[obj_id2].pix[X] + (main_map_x_pix * dWorldObjects[obj_id2].pix[Y]) + (main_map_total_pix>>1)) ]<<16));
	fenceC[Z] = -dWorldObjects[obj_id2].pix[Y] * (25<<16);
	
	fenceD[X] = fenceC[X];
	fenceD[Y] = fenceC[Y] + (dWorldObjects[obj_id2].type.radius[Y]<<17);
	fenceD[Z] = fenceC[Z];

		//Start math for projection
	centerFace[X] = (fenceA[X] + fenceC[X] + fenceB[X] + fenceD[X])>>2;
	centerFace[Y] = (fenceA[Y] + fenceC[Y] + fenceB[Y] + fenceD[Y])>>2;
	centerFace[Z] = (fenceA[Z] + fenceC[Z] + fenceB[Z] + fenceD[Z])>>2;
		//Get a relative position
	tgtRelPos[X] = tgt->pos[X] - centerFace[X];
	tgtRelPos[Y] = tgt->pos[Y] - centerFace[Y];
	tgtRelPos[Z] = tgt->pos[Z] - centerFace[Z];
		//If you are farther away from the face in X and Z than the face's big radius, you can't possibly have collided with it.
		//Note: Potential for this to be an incorrect conclusion on things moving really fast with small fences.
		radius1 = JO_ABS(fenceA[X] - fenceC[X]);
		radius2 = JO_ABS(fenceA[Z] - fenceC[Z]);
		bigRadius =  (radius1 > radius2) ? radius1 : radius2;
	if(JO_ABS(tgtRelPos[X]) > bigRadius || JO_ABS(tgtRelPos[Z]) > bigRadius)
	{
		planeDist[numGateChecks] = 0; //Data cleanup to prevent errant positive detection when culling logic is passed,
		lastPlaneDist[numGateChecks] = 0; //when previously it may noy have been.
		return false;
	};
		//Project to a horizontal edge of the face.
		//More specifically, this projects to the vector of that edge,
		//but since the projection is using a relative point rather than an absolute point,
		//it's already a projection in relation to the center of the face.
	project_to_segment(tgtRelPos, fenceA, fenceC, edgePrj0, outV0);
		//The first condition is if our projection is beyond the X or Z radius of the face, we want to stop now.
	if(JO_ABS(edgePrj0[X]) > JO_ABS(outV0[X]>>1) || JO_ABS(edgePrj0[Z]) > JO_ABS(outV0[Z]>>1) )
	{
		planeDist[numGateChecks] = 0; //Data cleanup to prevent errant positive detection when culling logic is passed,
		lastPlaneDist[numGateChecks] = 0; //when previously it may noy have been.
		return false;
	};
		//Now if we are above or below the Y radius of the face, defined by the radius of the objects, we want to stop.
	if( JO_ABS((tgtRelPos[Y]) - (edgePrj0[Y])) > (dWorldObjects[obj_id1].type.radius[Y]<<16) )
	{
		planeDist[numGateChecks] = 0; //Data cleanup to prevent errant positive detection when culling logic is passed,
		lastPlaneDist[numGateChecks] = 0; //when previously it may noy have been.
		return false;
	};
	
	//Then we do the math to find the normal of this area.
	//Makes a vector from point 3 to point 1.
	rminusb[X] = (fenceA[X] - fenceD[X]);
	rminusb[Y] = (fenceA[Y] - fenceD[Y]);
	rminusb[Z] = (fenceA[Z] - fenceD[Z]);
	//Makes a vector from point 2 to point 0.
	sminusb[X] = (fenceC[X] - fenceB[X]);
	sminusb[Y] = (fenceC[Y] - fenceB[Y]);
	sminusb[Z] = (fenceC[Z] - fenceB[Z]);
	
	cross_fixed(rminusb, sminusb, cross);
	
	cross[X] = cross[X]>>8;
	cross[Y] = cross[Y]>>8;
	cross[Z] = cross[Z]>>8;
	
	normalize(cross, faceNormal);
	//Then a collision detector.
	if(JO_ABS(tgtRelPos[X]) > (150<<16) || JO_ABS(tgtRelPos[Y]) > (150<<16) || JO_ABS(tgtRelPos[Z]) > (150<<16))
	{
	planeDist[numGateChecks] = ptalt_plane(tgt->pos, faceNormal, centerFace); //Returns on integer scale when value is large
	} else {
	planeDist[numGateChecks] = realpt_to_plane(tgt->pos, faceNormal, centerFace);
	}
	//slPrintFX(planeDist, slLocate(0, 8));
	if(fxm(planeDist[numGateChecks], lastPlaneDist[numGateChecks]) < 0)
	{
			lastPlaneDist[numGateChecks] = planeDist[numGateChecks];
		return true;
	} else {
			lastPlaneDist[numGateChecks] = planeDist[numGateChecks];
		return false;
	}
	
}

		//Presently function is unused so is technically incomplete (doesn't return or point to useful data).
		//For AI pathing, you.. uhh.. find a way.
void	walk_map_between_objects(short obj_id1, short obj_id2)
{
		//arrays below contain the in-order X and Y coordinates of cells that draw a line between the two objects.
		short pixXs[64];
		short pixYs[64];
		
		short cellDif[XY];
		cellDif[X] = JO_ABS(JO_ABS(dWorldObjects[obj_id1].pix[X]) - JO_ABS(dWorldObjects[obj_id2].pix[X]));
		cellDif[Y] = JO_ABS(JO_ABS(dWorldObjects[obj_id1].pix[Y]) - JO_ABS(dWorldObjects[obj_id2].pix[Y]));
		
		short totalCell = 0;
		short newCell[XY] = {dWorldObjects[obj_id1].pix[X], dWorldObjects[obj_id1].pix[Y]};
		Sint8 AddBool[XY];
		
		AddBool[X] = (dWorldObjects[obj_id1].pix[X] > dWorldObjects[obj_id2].pix[X]) ? -1 : 1;
		AddBool[Y] = (dWorldObjects[obj_id1].pix[Y] > dWorldObjects[obj_id2].pix[Y]) ? -1 : 1;
		
		//Less Strict Version. Simpler, faster code, but doesn't result in a "touch any cell" pathway.
		//It is a logical copy of how Jo Engine draws lines in the background layer.
		//This method makes more sense there since it is more visibly pleasing.
  		short error = 0;
		short e2 = 0;
		error = (cellDif[X] == cellDif[Y]) ? 0 : (cellDif[X] > cellDif[Y] ? cellDif[X] : -cellDif[Y])>>1;
    for (;;)
    {
				//First cell is where we started [0]
        pixXs[totalCell] = newCell[X];
		pixYs[totalCell] = newCell[Y];
		totalCell++;
				//Last cell is where we end up at
		if (newCell[X] == dWorldObjects[obj_id2].pix[X] && newCell[Y] == dWorldObjects[obj_id2].pix[Y]) break;
		e2 = error;
		
			error -= (e2 > -cellDif[X]) ? cellDif[Y] : 0;
			newCell[X] += (e2 > -cellDif[X]) ? AddBool[X] : 0;

			error += (e2 < cellDif[Y]) ? cellDif[X] : 0;
			newCell[Y] += (e2 < cellDif[Y]) ? AddBool[Y] : 0;
    } 
	
}

void	declarations(void){
	declare_object_at_cell(-10, -3, SimpleBox, 0, 0, 0, 3);
	declare_object_at_cell(10, 3, SimpleBox, 0, 0, 0, 3);
	declare_object_at_cell(3, 4, SimpleBox, 0, DEGtoANG(45), 0, 3);
	declare_object_at_cell(0, 3, SimpleBox, DEGtoANG(45), 0, 0, 3);
}

//I'm not sure if this whole system is ideal.
//But if I really do end up limited to 256 objects, really.. honestly... it should be okay, it's not logically intensive, and not intense on the bus either.
// Yet for collectibles, on these big maps, is that enough?
// .. Yeah. Yeah, it probably is. Even if it's 512.
void	update_object(Uint8 boxNumber, int pixX, int pixY, FIXED Ydist, ANGLE rotx, ANGLE roty, ANGLE rotz, FIXED radx, FIXED rady, FIXED radz)
{
	
	make2AxisBox((25 * pixX)<<16, -Ydist - (main_map[ (-pixX + (main_map_x_pix * pixY) + (main_map_total_pix>>1)) ]<<16), (25 * pixY)<<16,
	rotx, roty, rotz,
	radx, rady, radz, &RBBs[boxNumber]);
}


void	object_control_loop(int ppos[XY])
{
	
	static char difX = 0;
	static char difY = 0;
	int objUP = 0;
	numGateChecks = 0;

	for(unsigned char i = 0; i < objNEW; i++){
		difX = ppos[X] + dWorldObjects[i].pix[X]; //I have no idea how my coordinate systems get so reliably inverted...
		difY = ppos[Y] + dWorldObjects[i].pix[Y];
		if(difX > -12 && difX < 12 && difY > -12 && difY < 12){
			update_object(objUP, dWorldObjects[i].pix[X], dWorldObjects[i].pix[Y],
			(dWorldObjects[i].height + dWorldObjects[i].type.radius[Y])<<16, dWorldObjects[i].srot[X], dWorldObjects[i].srot[Y], dWorldObjects[i].srot[Z],
			dWorldObjects[i].type.radius[X]<<16, dWorldObjects[i].type.radius[Y]<<16, dWorldObjects[i].type.radius[Z]<<16);
			RBBs[objUP].isBoxPop = true;
			dWorldObjects[i].ext_dat |= 32768; //Bit 15 of ext_dat is a flag that will tell the system if the object is on or not.
			objDRAW[objUP] = dWorldObjects[i].type.entity_ID;
			objUP++;
		} else {
			RBBs[i].isBoxPop = false;
			dWorldObjects[i].ext_dat &= 32767; //Axe bit 15 but keep all other data.
		}
	}
	//objNEW = 0;
}

void	set_from_this_normal(Uint8 normID, _boundBox stator, VECTOR setNormal)
{
	
	//jo_printf(12, 9, "(%i)", normID);
	
	if(normID == 1){
		setNormal[X] = stator.UVX[X];
		setNormal[Y] = stator.UVX[Y];
		setNormal[Z] = stator.UVX[Z];
	} else if(normID == 2){
		setNormal[X] = stator.UVNX[X];
		setNormal[Y] = stator.UVNX[Y];
		setNormal[Z] = stator.UVNX[Z];
	} else if(normID == 3){
		setNormal[X] = stator.UVY[X];
		setNormal[Y] = stator.UVY[Y];
		setNormal[Z] = stator.UVY[Z];
	} else if(normID == 4){
		setNormal[X] = stator.UVNY[X];
		setNormal[Y] = stator.UVNY[Y];
		setNormal[Z] = stator.UVNY[Z];
	} else if(normID == 5){
		setNormal[X] = stator.UVZ[X];
		setNormal[Y] = stator.UVZ[Y];
		setNormal[Z] = stator.UVZ[Z];
	} else if(normID == 6){
		setNormal[X] = stator.UVNZ[X];
		setNormal[Y] = stator.UVNZ[Y];
		setNormal[Z] = stator.UVNZ[Z];
	}

}

void	pl_physics_handler(_boundBox * stator, _boundBox * mover, POINT hitPt, Uint8 hitFace)
{

	/*
	
Floor collisions pass the boolean "onSurface" that is processed in "collide with heightmap" function in player_phy.c 
Wall collisions pass the boolean "hitWall" that is processed in "player phys affect" function in player_phy.c
	
	*/

	if(hitFace == 4){
			if(stator->UVNY[Y] < -32768){
		
		you.floorNorm[X] = -stator->UVNY[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = -stator->UVNY[Y];
		you.floorNorm[Z] = -stator->UVNY[Z];
		
	sort_angle_to_domain(stator->UVY, alwaysLow, you.rot);
		
	you.floorPos[X] = (-(hitPt[X]) - (mover->Yneg[X]));
	you.floorPos[Y] = (-(hitPt[Y]) - (mover->Yneg[Y]));
	you.floorPos[Z] = (-(hitPt[Z]) - (mover->Yneg[Z]));
	you.shadowPos[X] = -hitPt[X];
	you.shadowPos[Y] = -hitPt[Y];
	you.shadowPos[Z] = -hitPt[Z];
		
	you.hitWall = false;
	you.hitSurface = true;
			} else {
	you.wallNorm[X] = stator->UVNY[X];
	you.wallNorm[Y] = stator->UVNY[Y];
	you.wallNorm[Z] = stator->UVNY[Z];
	you.wallPos[X] = hitPt[X];
	you.wallPos[Y] = hitPt[Y];
	you.wallPos[Z] = hitPt[Z];
		
	you.hitWall = true;
	you.hitSurface = false;
			}
	} else if(hitFace == 3){
		
			if(stator->UVY[Y] < -32768){
		
		you.floorNorm[X] = -stator->UVY[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = -stator->UVY[Y];
		you.floorNorm[Z] = -stator->UVY[Z];
		
	sort_angle_to_domain(stator->UVY, alwaysLow, you.rot);
		
	you.floorPos[X] = (-(hitPt[X]) - (mover->Yneg[X]));
	you.floorPos[Y] = (-(hitPt[Y]) - (mover->Yneg[Y]));
	you.floorPos[Z] = (-(hitPt[Z]) - (mover->Yneg[Z]));
	you.shadowPos[X] = -hitPt[X];
	you.shadowPos[Y] = -hitPt[Y];
	you.shadowPos[Z] = -hitPt[Z];
		
	you.hitWall = false;
	you.hitSurface = true;
			} else {
	you.wallNorm[X] = stator->UVY[X];
	you.wallNorm[Y] = stator->UVY[Y];
	you.wallNorm[Z] = stator->UVY[Z];
	you.wallPos[X] = hitPt[X];
	you.wallPos[Y] = hitPt[Y];
	you.wallPos[Z] = hitPt[Z];
		
	you.hitWall = true;
	you.hitSurface = false;
			}
	} else if(hitFace == 5){
		
			if(stator->UVZ[Y] < -32768){
		
		you.floorNorm[X] = -stator->UVZ[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = -stator->UVZ[Y];
		you.floorNorm[Z] = -stator->UVZ[Z];
		
	sort_angle_to_domain(stator->UVZ, alwaysLow, you.rot);
		
	you.floorPos[X] = (-(hitPt[X]) - (mover->Yneg[X]));
	you.floorPos[Y] = (-(hitPt[Y]) - (mover->Yneg[Y]));
	you.floorPos[Z] = (-(hitPt[Z]) - (mover->Yneg[Z]));
	you.shadowPos[X] = -hitPt[X];
	you.shadowPos[Y] = -hitPt[Y];
	you.shadowPos[Z] = -hitPt[Z];
		
	you.hitWall = false;
	you.hitSurface = true;
			} else {
	you.wallNorm[X] = stator->UVZ[X];
	you.wallNorm[Y] = stator->UVZ[Y];
	you.wallNorm[Z] = stator->UVZ[Z];
	you.wallPos[X] = hitPt[X];
	you.wallPos[Y] = hitPt[Y];
	you.wallPos[Z] = hitPt[Z];
		
	you.hitWall = true;
	you.hitSurface = false;
			}
	} else if(hitFace == 6){
		
			if(stator->UVNZ[Y] < -32768){
		
		you.floorNorm[X] = -stator->UVNZ[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = -stator->UVNZ[Y];
		you.floorNorm[Z] = -stator->UVNZ[Z];
		
	sort_angle_to_domain(stator->UVZ, alwaysLow, you.rot);
		
	you.floorPos[X] = (-(hitPt[X]) - (mover->Yneg[X]));
	you.floorPos[Y] = (-(hitPt[Y]) - (mover->Yneg[Y]));
	you.floorPos[Z] = (-(hitPt[Z]) - (mover->Yneg[Z]));
	you.shadowPos[X] = -hitPt[X];
	you.shadowPos[Y] = -hitPt[Y];
	you.shadowPos[Z] = -hitPt[Z];
		
	you.hitWall = false;
	you.hitSurface = true;
			} else {
	you.wallNorm[X] = stator->UVNZ[X];
	you.wallNorm[Y] = stator->UVNZ[Y];
	you.wallNorm[Z] = stator->UVNZ[Z];
	you.wallPos[X] = hitPt[X];
	you.wallPos[Y] = hitPt[Y];
	you.wallPos[Z] = hitPt[Z];
		
	you.hitWall = true;
	you.hitSurface = false;
			}
	} else if(hitFace == 1){

			if(stator->UVX[Y] < -32768){
		
		you.floorNorm[X] = -stator->UVX[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = -stator->UVX[Y];
		you.floorNorm[Z] = -stator->UVX[Z];
		
	sort_angle_to_domain(stator->UVX, alwaysLow, you.rot);
		
	you.floorPos[X] = (-(hitPt[X]) - (mover->Yneg[X]));
	you.floorPos[Y] = (-(hitPt[Y]) - (mover->Yneg[Y]));
	you.floorPos[Z] = (-(hitPt[Z]) - (mover->Yneg[Z]));
	you.shadowPos[X] = -hitPt[X];
	you.shadowPos[Y] = -hitPt[Y];
	you.shadowPos[Z] = -hitPt[Z];
		
	you.hitWall = false;
	you.hitSurface = true;
			} else {
	you.wallNorm[X] = stator->UVX[X];
	you.wallNorm[Y] = stator->UVX[Y];
	you.wallNorm[Z] = stator->UVX[Z];
	you.wallPos[X] = hitPt[X];
	you.wallPos[Y] = hitPt[Y];
	you.wallPos[Z] = hitPt[Z];
		
	you.hitWall = true;
	you.hitSurface = false;
			}
	} else if(hitFace == 2){

			if(stator->UVNX[Y] < -32768){
		
		you.floorNorm[X] = -stator->UVNX[X]; //[could just use UVY instead of -UVNY]
		you.floorNorm[Y] = -stator->UVNX[Y];
		you.floorNorm[Z] = -stator->UVNX[Z];
		
	sort_angle_to_domain(stator->UVX, alwaysLow, you.rot);
		
	you.floorPos[X] = (-(hitPt[X]) - (mover->Yneg[X]));
	you.floorPos[Y] = (-(hitPt[Y]) - (mover->Yneg[Y]));
	you.floorPos[Z] = (-(hitPt[Z]) - (mover->Yneg[Z]));
	you.shadowPos[X] = -hitPt[X];
	you.shadowPos[Y] = -hitPt[Y];
	you.shadowPos[Z] = -hitPt[Z];
		
	you.hitWall = false;
	you.hitSurface = true;
			} else {
	you.wallNorm[X] = stator->UVNX[X];
	you.wallNorm[Y] = stator->UVNX[Y];
	you.wallNorm[Z] = stator->UVNX[Z];
	you.wallPos[X] = hitPt[X];
	you.wallPos[Y] = hitPt[Y];
	you.wallPos[Z] = hitPt[Z];
		
	you.hitWall = true;
	you.hitSurface = false;
			}
	}
}

void	player_shadow_object(_boundBox * stator, POINT centerDif)
{
	
	//Again, I have no idea how my coordinate systems get so reliably inverted...
	POINT below_player = {-you.pos[X], -(you.pos[Y] - (1<<16)), -you.pos[Z]};
	POINT negative_pos = {-you.pos[X], -(you.pos[Y]), -you.pos[Z]};
	POINT xHit;
	POINT yHit;
	POINT zHit;
	char hitBools[XYZ];
	POINT highHit = {0, 0, 0};

	if( centerDif[X] < 0){
		//This means we draw to X+
		line_hit_plane_here(negative_pos, below_player, stator->Xplus, stator->UVX, stator->pos, xHit);
	} else {
		//This means we draw to X-
		line_hit_plane_here(negative_pos, below_player, stator->Xneg, stator->UVX, stator->pos, xHit);
	}
	
	if( centerDif[Y] < 0){
		//This means we draw to Y+
		line_hit_plane_here(negative_pos, below_player, stator->Yplus, stator->UVY, stator->pos, yHit);
	} else {
		//This means we draw to Y-
		line_hit_plane_here(negative_pos, below_player, stator->Yneg, stator->UVY, stator->pos, yHit);
	}
	
	if( centerDif[Z] < 0){
		//This means we draw to Z+
		line_hit_plane_here(negative_pos, below_player, stator->Zplus, stator->UVZ, stator->pos, zHit);
	} else {
		//This means we draw to Z-
		line_hit_plane_here(negative_pos, below_player, stator->Zneg, stator->UVZ, stator->pos, zHit);
	}
	
	if(simple_collide(xHit, stator) == true){
		hitBools[X] = true;
		highHit[X] = xHit[X];
		highHit[Y] = xHit[Y];
		highHit[Z] = xHit[Z];
	} else {
		hitBools[X] = false;
	}
	
	if(simple_collide(yHit, stator) == true){
		hitBools[Y] = true;
		highHit[X] = (JO_ABS(yHit[Y]) > highHit[Y]) ? yHit[X] : highHit[X];
		highHit[Y] = (JO_ABS(yHit[Y]) > highHit[Y]) ? yHit[Y] : highHit[Y];
		highHit[Z] = (JO_ABS(yHit[Y]) > highHit[Y]) ? yHit[Z] : highHit[Z];
	} else {
		hitBools[Y] = false;
	}
	
	if(simple_collide(zHit, stator) == true){
		hitBools[Z] = true;
		highHit[X] = (JO_ABS(zHit[Y]) > highHit[Y]) ? zHit[X] : highHit[X];
		highHit[Y] = (JO_ABS(zHit[Y]) > highHit[Y]) ? zHit[Y] : highHit[Y];
		highHit[Z] = (JO_ABS(zHit[Y]) > highHit[Y]) ? zHit[Z] : highHit[Z];
	} else {
		hitBools[Z] = false;
	}
	
	if((hitBools[X] == true || hitBools[Y] == true || hitBools[Z] == true) && you.pos[Y] > -highHit[Y])
	{
		//Inverted coordinates...
		you.shadowPos[X] = -highHit[X];
		you.shadowPos[Y] = -highHit[Y];
		you.shadowPos[Z] = -highHit[Z];
		you.aboveObject = true;
	} else {
		you.aboveObject = false;
	}

}

bool	player_collide_boxes(_boundBox * stator, _boundBox * mover)
{

static FIXED bigRadius = 0;

static POINT centerDif = {0, 0, 0};

static POINT lineEnds[9];

static bool lineChecks[9];

static Uint8 hitFace;
		
static FIXED bigDif = 0;


//Box Populated Check
if(stator->isBoxPop != true){
	return false;
}

//Box Distance Culling Check
bigRadius = JO_MAX(JO_MAX(JO_MAX(JO_MAX(JO_ABS(stator->Xplus[X]), JO_ABS(stator->Xplus[Y])), JO_ABS(stator->Xplus[Z])),
		JO_MAX(JO_MAX(JO_ABS(stator->Yplus[X]), JO_ABS(stator->Yplus[Y])), JO_ABS(stator->Yplus[Z]))),
		JO_MAX(JO_MAX(JO_ABS(stator->Zplus[X]), JO_ABS(stator->Zplus[Y])), JO_ABS(stator->Zplus[Z])));
		

centerDif[X] = stator->pos[X] + mover->pos[X];
centerDif[Y] = stator->pos[Y] + mover->pos[Y];
centerDif[Z] = stator->pos[Z] + mover->pos[Z];


bigDif = JO_MAX(JO_MAX(JO_ABS(centerDif[X]), JO_ABS(centerDif[Y])),JO_ABS(centerDif[Z]));

if(bigDif > (bigRadius + (20<<16))) return false;

numBoxChecks++;

//Box Collision Check
_lineTable moverCFs = {
	.xp0[X] = mover->Xplus[X] - mover->pos[X],
	.xp0[Y] = mover->Xplus[Y] - mover->pos[Y],
	.xp0[Z] = mover->Xplus[Z] - mover->pos[Z],
	.xp1[X] = mover->Xneg[X] - mover->pos[X],
	.xp1[Y] = mover->Xneg[Y] - mover->pos[Y],
	.xp1[Z] = mover->Xneg[Z] - mover->pos[Z],
	.yp0[X] = mover->Yplus[X] - mover->pos[X],
	.yp0[Y] = mover->Yplus[Y] - mover->pos[Y],
	.yp0[Z] = mover->Yplus[Z] - mover->pos[Z],
	.yp1[X] = mover->Yneg[X] - mover->pos[X],
	.yp1[Y] = mover->Yneg[Y] - mover->pos[Y],
	.yp1[Z] = mover->Yneg[Z] - mover->pos[Z],
	.zp0[X] = mover->Zplus[X] - mover->pos[X],
	.zp0[Y] = mover->Zplus[Y] - mover->pos[Y],
	.zp0[Z] = mover->Zplus[Z] - mover->pos[Z],
	.zp1[X] = mover->Zneg[X] - mover->pos[X],
	.zp1[Y] = mover->Zneg[Y] - mover->pos[Y],
	.zp1[Z] = mover->Zneg[Z] - mover->pos[Z]
}; //Why Subtract? Why Coordinate System, Bruh?!

		/*
	ABSOLUTE PRIORITY: Once again, the normal hit during collision must be found ABSOLUTELY. ACCURATELY.
	How we did this best before:
	Step 0: Determine which faces face you.
	Step 1: Draw lines to a point from every pair of CFs on the mover to every facing face of the stator->
	Step 2: Test the points and find which one collides and which face it collided with.
	Step 3: Use the normal of that face for collision.

		*/
		

//	Step 0: Determine which faces face you.

	POINT negCenter = {-mover->pos[X], -mover->pos[Y], -mover->pos[Z]};
	//This math figures out what side of the box we're on, with respect to its rotation, too.
	centerDif[X] = realpt_to_plane(negCenter, stator->UVX, stator->pos);
	centerDif[Y] = realpt_to_plane(negCenter, stator->UVY, stator->pos);
	centerDif[Z] = realpt_to_plane(negCenter, stator->UVZ, stator->pos);
//	Step 1: Draw lines to a point from every pair of CFs on the mover to every face of the stator->
	if( centerDif[X] < 0){
		//This means we draw to X+
		lineChecks[0] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Xplus, stator->UVX, stator->pos, lineEnds[0]);
		lineChecks[1] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Xplus, stator->UVX, stator->pos, lineEnds[1]);
		lineChecks[2] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Xplus, stator->UVX, stator->pos, lineEnds[2]);
	} else {
		//This means we draw to X-
		lineChecks[0] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Xneg, stator->UVX, stator->pos, lineEnds[0]);
		lineChecks[1] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Xneg, stator->UVX, stator->pos, lineEnds[1]);
		lineChecks[2] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Xneg, stator->UVX, stator->pos, lineEnds[2]);
	}
	
	if( centerDif[Y] < 0){
		//This means we draw to Y+
		lineChecks[3] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Yplus, stator->UVY, stator->pos, lineEnds[3]);
		lineChecks[4] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Yplus, stator->UVY, stator->pos, lineEnds[4]);
		lineChecks[5] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Yplus, stator->UVY, stator->pos, lineEnds[5]);
	} else {
		//This means we draw to Y-
		lineChecks[3] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Yneg, stator->UVY, stator->pos, lineEnds[3]);
		lineChecks[4] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Yneg, stator->UVY, stator->pos, lineEnds[4]);
		lineChecks[5] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Yneg, stator->UVY, stator->pos, lineEnds[5]);
	}
	
	if( centerDif[Z] < 0){
		//This means we draw to Z+
		lineChecks[6] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Zplus, stator->UVZ, stator->pos, lineEnds[6]);
		lineChecks[7] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Zplus, stator->UVZ, stator->pos, lineEnds[7]);
		lineChecks[8] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Zplus, stator->UVZ, stator->pos, lineEnds[8]);
	} else {
		//This means we draw to Z-
		lineChecks[6] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Zneg, stator->UVZ, stator->pos, lineEnds[6]);
		lineChecks[7] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Zneg, stator->UVZ, stator->pos, lineEnds[7]);
		lineChecks[8] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Zneg, stator->UVZ, stator->pos, lineEnds[8]);
	}
	
//	jo_printf(13, 12, "(%i)", hitFace);
				player_shadow_object(stator, centerDif);
		//Step 2: Test the points and find which one collides and which face it collided with.
	for(int i = 0; i < 9; i++){
		if(lineChecks[i] == true){
			if(sort_collide(lineEnds[i], stator, &hitFace, -HIT_TOLERANCE) == true){
				//Step 3: Use the normal of that face for collision.
				pl_physics_handler(stator, mover, lineEnds[i], hitFace);
				return true;
			}
		}
		
	}

				you.hitWall = false;
				you.hitSurface = false; //Neccessary to release from surface
		return false;
}

void	player_collision_test_loop(void)
{
	
	for(Uint8 i = 0; i < MAX_PHYS_PROXY; i++)
	{
		if(player_collide_boxes(&RBBs[i], &pl_RBB) == true) return;
	}
	//jo_printf(0, 14, "(%i)E", numBoxChecks);
	numBoxChecks = 0;
}
