//pathing.c
//This file compiled with physobjet.c

#include <sl_def.h>
#include "def.h"
#include "pcmsys.h"
#include "sound.h"
#include "mymath.h"
#include "render.h"
#include "mloader.h"

#include "collision.h"
#include "particle.h"
#include "input.h"

#include "physobjet.h"



void	findSectorPathNodeCount(unsigned int sector_id)
{
	if(sector_id > MAX_SECTORS || sector_id == INVALID_SECTOR) return;
	_sector * sct = &sectors[sector_id];
	//Step 1: Is this sector populated?
	if(!sct->nbPolygon) return;
	if(!sct->ent) return;
	//Step 2: Is it pointing to a valid entity?
	if(sct->ent->type != MODEL_TYPE_SECTORED) return;
	if(!sct->ent->file_done) return;
	
	GVPLY * mesh = sct->ent->pol;
	//The concept is to generate path nodes that segregate adjacent floors between sectors of the level,
	//such that the path nodes represent a point that can be navigated towards to go from one sector to another.
	//The first problem is finding RAM suitable to allocate for this purpose.

	//Gotta be carefuL: hopefully this doesn't get misaligned.
	//we start over max, stop
	if(pathStackPtr > pathStackMax) return;
	//
	sct->paths = (_pathNodes*)pathStackPtr;
	//To make it simple, we are just going to guess on the number of possible paths being 6.
	pathStackPtr += sct->nbAdjacent * 2 * sizeof(_pathNodes);
	
	int t_plane[4][3];
	int t_center[3] = {0, 0, 0};
	int c_plane[4][3];
	int c_center[3] = {0, 0, 0};
	int vector_to_tc[3];
	int normal_to_tc[3];
	int y_min = 0;
	int y_max = 0;
	int within_span = 0;
	int within_shape_ct = 0;

	_sector * sct2;
	//GOTO Label:
	//On the first pass of this loop, we will count up the number of adjacents, then allocate memory based on that number.
	//On the second pass, we will fill that allocated memory with valid data.

for(int s = 0; s < sct->nbAdjacent; s++)
{
	sct2 = &sectors[sct->pvs[s+1]];
	sct->paths[s].numNodes = 0;
	for(int i = 0; i < sct->nbPolygon; i++)
	{
		int alias = sct->pltbl[i];
		if(mesh->maxtbl[alias] != N_Yn) continue;
		y_min = mesh->pntbl[mesh->pltbl[alias].vertices[0]][Y];
		y_max = mesh->pntbl[mesh->pltbl[alias].vertices[0]][Y];
		t_center[X] = 0;
		t_center[Y] = 0;
		t_center[Z] = 0;
		for(int k = 0; k < 4; k++)
		{
			t_plane[k][X] = mesh->pntbl[mesh->pltbl[alias].vertices[k]][X];
			t_plane[k][Y] = mesh->pntbl[mesh->pltbl[alias].vertices[k]][Y];
			t_plane[k][Z] = mesh->pntbl[mesh->pltbl[alias].vertices[k]][Z];
			t_center[X] += t_plane[k][X];
			t_center[Y] += t_plane[k][Y];
			t_center[Z] += t_plane[k][Z];
			y_min = (t_plane[k][Y] < y_min) ? t_plane[k][Y] : y_min;
			y_max = (t_plane[k][Y] > y_max) ? t_plane[k][Y] : y_max;
		}
		t_center[X] >>=4;
		t_center[Y] >>=4;
		t_center[Z] >>=4;
		
		//Add some margin of error
		//This will be a quarter meter, or (16<<16)
		y_min -= 16<<16;
		y_max += 16<<16;
		
		for(int p = 0; p < sct2->nbPolygon; p++)
		{
			int alias2 = sct2->pltbl[p];
			if(mesh->maxtbl[alias2] != N_Yn) continue;
			
			within_span = 0;
			c_center[X] = 0;
			c_center[Y] = 0;
			c_center[Z] = 0;
			for(int k = 0; k < 4; k++)
			{
				c_plane[k][X] = mesh->pntbl[mesh->pltbl[alias2].vertices[k]][X];
				c_plane[k][Y] = mesh->pntbl[mesh->pltbl[alias2].vertices[k]][Y];
				c_plane[k][Z] = mesh->pntbl[mesh->pltbl[alias2].vertices[k]][Z];
				if((t_plane[k][Y] > y_min) && (t_plane[k][Y] < y_max)) within_span = 1;
				c_center[X] += c_plane[k][X];
				c_center[Y] += c_plane[k][Y];
				c_center[Z] += c_plane[k][Z];
			}
			if(!within_span) continue;
			
			c_center[X] >>=4;
			c_center[Y] >>=4;
			c_center[Z] >>=4;
			
			vector_to_tc[X] = c_center[X] - t_center[X];
			vector_to_tc[Y] = c_center[Y] - t_center[Y];
			vector_to_tc[Z] = c_center[Z] - t_center[Z];
			normal_to_tc[X] = 0;
			normal_to_tc[Y] = 0;
			normal_to_tc[Z] = 0;
			
			accurate_normalize(vector_to_tc, normal_to_tc);
			//Method for detecting adjacency:
			//Hooray, it's the edge-wind test again. We check if two vertices are within the original polygon being tested.
			//If they are, they probably share an edge.
			within_shape_ct = 0;
			for(int k = 0; k < 4; k++)
			{
				//Aggravating Necessity: Push c_plane towards t_plane slightly
				//This is so the chirality test does not fail
				c_plane[k][X] -= normal_to_tc[X];
				c_plane[k][Y] -= normal_to_tc[Y];
				c_plane[k][Z] -= normal_to_tc[Z];
				//I need to know *exactly* which edge is adjacent.
				//I can do this by checking every vertex, and then seeing which pair is adjacent.
				if(edge_wind_test(t_plane[0], t_plane[1], t_plane[2], t_plane[3], c_plane[k], N_Yn, 12))
				{
					within_shape_ct++;
				}
			}
			if(within_shape_ct < 2) continue;
			
			sct->paths[s].numNodes++;
			sct->paths[s].fromSector = sector_id;
			sct->paths[s].toSector = sct->pvs[s+1];
			break;
			
		}
	}

}	

}

void	accumulate_path_node_count_for_sectors(void)
{
	_sector * sct;

	//Initialize table
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		for(int k = 0; k < MAX_SECTORS; k++)
		{
			pathing->count[i][k] = 0;
		}
	}
	
	//Count up intersecting paths
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		sct = &sectors[i];
		if(sct->nbAdjacent == 0) continue;
		
		for(int j = 0; j < sct->nbAdjacent; j++)
		{
			int sctA = sct->paths[j].fromSector;
			int sctB = sct->paths[j].toSector;
			pathing->count[sctA][sctB]+=1;
		}
	}
	
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		sct = &sectors[i];
		if(sct->nbAdjacent <= 0) continue;
		
		for(int s = 0; s < sct->nbAdjacent; s++)
		{
			
			//So we now have: The sector and the source sector.
			// This part will compensate for cases where sctA knows it has a path to sctB, but sctB does not know it has a path to sctA.
			// We need to add the adjacent count in <sct2> to the adjacent count in <sct>
			// But, carefully: we do not want to do that twice, only once.
			// So the first thing we've done is stored the original path node count for each sector.
				int sctA = i;
				int sctB = sct->pvs[s+1];
			if(pathing->count[sctB][sctA])
			{
				sct->paths[s].fromSector = sctA;
				sct->paths[s].toSector = sctB;
				sct->paths[s].numNodes += pathing->count[sctB][sctA];
			}
			
		}
		
	}
	
	//Now that number is accumulated, we should be able to properly allocate memory for all sectors.
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		sct = &sectors[i];
		if(sct->nbAdjacent <= 0) continue;
		for(int s = 0; s < sct->nbAdjacent; s++)
		{
			//If we've exceeded the memory limit, abort.
			if(pathStackPtr > pathStackMax)
			{
				nbg_sprintf(0,0, "Error: path tables out of memory");
				return;
			}
			sct->paths[s].nodes = (POINT *)pathStackPtr;
			pathStackPtr += sct->paths[s].numNodes * sizeof(POINT);
			sct->paths[s].dir = (VECTOR *)pathStackPtr;
			pathStackPtr += sct->paths[s].numNodes * sizeof(VECTOR);
			//Initialize this memory (with an unlikely value)
			for(int k = 0; k < sct->paths[s].numNodes; k++)
			{
				sct->paths[s].nodes[k][X] = -1;
				sct->paths[s].nodes[k][Y] = -1;
				sct->paths[s].nodes[k][Z] = -1;
				sct->paths[s].dir[k][X] = -1;
				sct->paths[s].dir[k][Y] = -1;
				sct->paths[s].dir[k][Z] = -1;
			}
		}
	}
	//Now we can build the path nodes for all sectors.
	//Then after that, we will be adding the path nodes in backwards from [sctB] to [sctA] later.
	
}

void	buildSectorPathNodes(unsigned int sector_id)
{
	if(sector_id > MAX_SECTORS || sector_id == INVALID_SECTOR) return;
	_sector * sct = &sectors[sector_id];
	//Step 1: Is this sector populated?
	if(!sct->nbPolygon) return;
	if(!sct->ent) return;
	//Step 2: Is it pointing to a valid entity?
	if(sct->ent->type != MODEL_TYPE_SECTORED) return;
	if(!sct->ent->file_done) return;
	
	GVPLY * mesh = sct->ent->pol;

	//Gotta be carefuL: hopefully this doesn't get misaligned.
	//we start over max, stop
	if(pathStackPtr > pathStackMax) return;

	int t_plane[4][3];
	int t_center[3] = {0, 0, 0};
	int c_plane[4][3];
	int c_center[3] = {0, 0, 0};
	int vector_to_tc[3];
	int normal_to_tc[3];
	int y_min = 0;
	int y_max = 0;
	int within_span = 0;
	int within_shape_ct = 0;
	int vert_within_shape[4] = {0,0,0,0};
	int guidance_point[3] = {0,0,0};
	int num_paths = 0;
	
	_sector * sct2;
	//GOTO Label:
	//On the first pass of this loop, we will count up the number of adjacents, then allocate memory based on that number.
	//On the second pass, we will fill that allocated memory with valid data.

for(int s = 0; s < sct->nbAdjacent; s++)
{
	sct2 = &sectors[sct->pvs[s+1]];
	num_paths = 0;
	for(int i = 0; i < sct->nbPolygon; i++)
	{
		int alias = sct->pltbl[i];
		if(mesh->maxtbl[alias] != N_Yn) continue;
		y_min = mesh->pntbl[mesh->pltbl[alias].vertices[0]][Y];
		y_max = mesh->pntbl[mesh->pltbl[alias].vertices[0]][Y];
		t_center[X] = 0;
		t_center[Y] = 0;
		t_center[Z] = 0;
		for(int k = 0; k < 4; k++)
		{
			t_plane[k][X] = mesh->pntbl[mesh->pltbl[alias].vertices[k]][X];
			t_plane[k][Y] = mesh->pntbl[mesh->pltbl[alias].vertices[k]][Y];
			t_plane[k][Z] = mesh->pntbl[mesh->pltbl[alias].vertices[k]][Z];
			t_center[X] += t_plane[k][X];
			t_center[Y] += t_plane[k][Y];
			t_center[Z] += t_plane[k][Z];
			y_min = (t_plane[k][Y] < y_min) ? t_plane[k][Y] : y_min;
			y_max = (t_plane[k][Y] > y_max) ? t_plane[k][Y] : y_max;
		}
		t_center[X] >>=4;
		t_center[Y] >>=4;
		t_center[Z] >>=4;
		
		for(int p = 0; p < sct2->nbPolygon; p++)
		{
			int alias2 = sct2->pltbl[p];
			if(mesh->maxtbl[alias2] != N_Yn) continue;
			
			within_span = 0;
			c_center[X] = 0;
			c_center[Y] = 0;
			c_center[Z] = 0;
			for(int k = 0; k < 4; k++)
			{
				c_plane[k][X] = mesh->pntbl[mesh->pltbl[alias2].vertices[k]][X];
				c_plane[k][Y] = mesh->pntbl[mesh->pltbl[alias2].vertices[k]][Y];
				c_plane[k][Z] = mesh->pntbl[mesh->pltbl[alias2].vertices[k]][Z];
				if((c_plane[k][Y] >= y_min) && (c_plane[k][Y] <= y_max)) within_span++;
				c_center[X] += c_plane[k][X];
				c_center[Y] += c_plane[k][Y];
				c_center[Z] += c_plane[k][Z];
			}
			if(within_span < 2) continue;
			
			c_center[X] >>=4;
			c_center[Y] >>=4;
			c_center[Z] >>=4;
			
			vector_to_tc[X] = c_center[X] - t_center[X];
			vector_to_tc[Y] = c_center[Y] - t_center[Y];
			vector_to_tc[Z] = c_center[Z] - t_center[Z];
			normal_to_tc[X] = 0;
			normal_to_tc[Y] = 0;
			normal_to_tc[Z] = 0;
			
			accurate_normalize(vector_to_tc, normal_to_tc);
			//Method for detecting adjacency:
			//Hooray, it's the edge-wind test again. We check if two vertices are within the original polygon being tested.
			//If they are, they probably share an edge.
			within_shape_ct = 0;
			for(int k = 0; k < 4; k++)
			{
				//Aggravating Necessity: Push c_plane towards t_plane slightly
				//This is so the chirality test does not fail
				c_plane[k][X] -= (normal_to_tc[X]);
				c_plane[k][Y] -= (normal_to_tc[Y]);
				c_plane[k][Z] -= (normal_to_tc[Z]);
				vert_within_shape[k] = 0;
				//I need to know *exactly* which edge is adjacent.
				//I can do this by checking every vertex, and then seeing which pair is adjacent.
				if(edge_wind_test(t_plane[0], t_plane[1], t_plane[2], t_plane[3], c_plane[k], N_Yn, 12))
				{
					within_shape_ct++;
					vert_within_shape[k] = 1;
				}
			}
			if(within_shape_ct < 2) continue;

			//This logic chunk assigns which edge was adjacent (0->1, 1->2, 2->3, 3->0)
			int fk = 0;
			int sk = 0;
			if(vert_within_shape[0] && vert_within_shape[1])
			{
				fk = 0;
				sk = 1;
			} else if(vert_within_shape[1] && vert_within_shape[2])
			{
				fk = 1;
				sk = 2;
			} else if(vert_within_shape[2] && vert_within_shape[3])
			{
				fk = 2;
				sk = 3;
			} else if(vert_within_shape[3] && vert_within_shape[0])
			{
				fk = 3;
				sk = 0;
			}
			

			if(within_shape_ct)
			{
				//Mark the guidance point of (adjacent_planes) as the center of [FK]->[SK]
				guidance_point[X] = (c_plane[fk][X] + c_plane[sk][X])>>1;
				guidance_point[Y] = (c_plane[fk][Y] + c_plane[sk][Y])>>1;
				guidance_point[Z] = (c_plane[fk][Z] + c_plane[sk][Z])>>1;		
				sct->paths[s].nodes[num_paths][X] = guidance_point[X];
				sct->paths[s].nodes[num_paths][Y] = guidance_point[Y];
				sct->paths[s].nodes[num_paths][Z] = guidance_point[Z];
				//add a direction to the path node - first calculate a vector from guidance pt to center
				sct->paths[s].dir[num_paths][X] = normal_to_tc[X];//JO_ABS((t_center[X] - (guidance_point[X]>>2))>>4);
				sct->paths[s].dir[num_paths][Y] = normal_to_tc[Y];//JO_ABS((t_center[Y] - (guidance_point[Y]>>2))>>4);
				sct->paths[s].dir[num_paths][Z] = normal_to_tc[Z];//JO_ABS((t_center[Z] - (guidance_point[Z]>>2))>>4);
				break;
			}
			
		}
	}

}	

}

void	cross_compartment_path_nodes(int sector_num)
{
	//Using pathing->count[sector_num][sctB], we can find how many path nodes a sector generated for itself.
	//Conversely, pathing->count[sctB][sector_num], we can find how many path nodes we need to add to a sector.
	int original_ct = 0;
	int add_ct = 0;
	_sector * sct = &sectors[sector_num];
	if(sct->nbAdjacent == 0) return;
	
	for(int s = 0; s < sct->nbAdjacent; s++)
	{
		_sector * sct2 = &sectors[sct->pvs[s+1]];
		int sctA = sct->paths[s].fromSector;
		int sctB = sct->paths[s].toSector;
		
		original_ct = pathing->count[sctA][sctB];
		add_ct = pathing->count[sctB][sctA];

		if(original_ct == 0 && add_ct == 0) continue;
		int bt = INVALID_SECTOR;
		for(int k = 0; k < sct2->nbAdjacent; k++)
		{
			if(sct2->pvs[k+1] == sector_num)
			{
				bt = k;
				break;
			}
		}
		if(bt == INVALID_SECTOR)
		{
			nbg_sprintf(0, 0, "Error in path graph");
			while(1);
		}
		
		//All path nodes allocated are initialized at -1 in xyz of nodes and dir.
		//The system will note the accumulation of path nodes in the node count before it allocates memory for them.
		//Because of this, even unpopulated intersections will come through to this point with at least 1 in addCt and originalCt.
		//To detect if we are adding from an unpopulated interesection (e.g. sctA knew it had a path to sctB, but sctB did not),
		//we thus must check to see if the node has been initialized yet or not.
		//We have to do this because of the way the system checks for paths (by chirality checking floor vertices).
		if((sct->paths[s].nodes[0][X] & sct->paths[s].nodes[0][Y] & sct->paths[s].nodes[0][Z]) == -1)
		{
			//(setting this to zero overwrites entry 0, which is uninitialized)
			original_ct = 0;
		}
		
		for(int i = 0; i < add_ct; i++)
		{
			sct->paths[s].nodes[original_ct+i][X] = sct2->paths[bt].nodes[i][X];
			sct->paths[s].nodes[original_ct+i][Y] = sct2->paths[bt].nodes[i][Y];
			sct->paths[s].nodes[original_ct+i][Z] = sct2->paths[bt].nodes[i][Z];
			
			sct->paths[s].dir[original_ct+i][X] = -sct2->paths[bt].dir[i][X];
			sct->paths[s].dir[original_ct+i][Y] = -sct2->paths[bt].dir[i][Y];
			sct->paths[s].dir[original_ct+i][Z] = -sct2->paths[bt].dir[i][Z];
		}
		
	}	
}


void	reconcile_pathing_lists(void)
{
	//this function reconciles the sector-specific path lists into an easier-to-navigate table of paths from pathing->guides[sctA][sctB].
	_sector * sct;
	pathStackPtr = align_4(pathStackPtr);
	if(pathStackPtr > pathStackMax)
	{
		nbg_sprintf(0,0, "Error: path tables out of memory");
		return;
	}
	
	//Initialize table
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		for(int k = 0; k < MAX_SECTORS; k++)
		{
			pathing->count[i][k] = 0;
		}
	}
	
	//Count up intersecting paths
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		sct = &sectors[i];
		if(sct->nbAdjacent == 0 || sct->nbVisible == 0) continue;
		
		for(int j = 0; j < sct->nbAdjacent; j++)
		{
			int sctA = sct->paths[j].fromSector;
			int sctB = sct->paths[j].toSector;
			pathing->count[sctA][sctB]+=1;
		}
	}
	
	//Allocate memory for each intersection
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		for(int k = 0; k < MAX_SECTORS; k++)
		{
			if(pathing->count[i][k] != 0)
			{
				if(pathStackPtr > pathStackMax)
				{
					nbg_sprintf(0,0, "Error: path tables out of memory");
					return;
				}
				pathing->guides[i][k] = (_pathNodes**)pathStackPtr;
				pathStackPtr += pathing->count[i][k] * sizeof(void *);
				//Initialize
				for(int f = 0; f < pathing->count[i][k]; f++)
				{
					pathing->guides[i][k][f] = sectors[0].paths;
				}
			}
			pathing->count[i][k] = 0;
		}
	}
	
	//Populate the allocated memory
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		sct = &sectors[i];
		if(sct->nbAdjacent == 0 || sct->nbVisible == 0) continue;
		
		for(int j = 0; j < sct->nbAdjacent; j++)
		{
			int sctA = sct->paths[j].fromSector;
			int sctB = sct->paths[j].toSector;
			pathing->guides[sctA][sctB][pathing->count[sctA][sctB]] = &sct->paths[j];
			pathing->count[sctA][sctB]++;
		}
	}
	
	
}

//This whole process is required to:
//1. Find which sectors know they have a path to another sector
//2. Share that information such that all sectors know when they have a path to them and make it also a path from them
//3. Allocate memory such that all sectors have room to store pathing data to them and from them
//4. Reconcile the sector-specific pathing lists into a 2D array that points to data contained within each sector
void	init_pathing_system(void)
{
	nbg_sprintf(0,0, "Finding path tables...");
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		findSectorPathNodeCount(i);
	}
	accumulate_path_node_count_for_sectors();
	nbg_sprintf(0,0, "Building path tables...");
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		buildSectorPathNodes(i);
	}
	nbg_sprintf(0,0, "Node graph out of date...");
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		cross_compartment_path_nodes(i);
	}
		
	// nbg_sprintf(0,1, "szStepHost(%i)", pathing);
	// nbg_sprintf(0,2, "szPathHost(%i)", pathStepHeap);
	nbg_sprintf(0,0, "Reconciling...");
	reconcile_pathing_lists();
}


//Allows each search for the target sector to iterate through each sector's adjacent sector list up to the iteration limit.
//There are really two iteration limits; the one set here for this function to allow each branch to find the shortest path,
//(by limiting each recursive path to a certain number of iterations such that the longer iterations return 0 and thus allow to continue),
//and a second limit of MAX_PATHING_STEPS, dictated by the use of the function.
//In both cases, <iterations> shall start at 0 upon initial all and <iter_limit> shall be the # of iterations your search must stop at.
//As noted earlier, your search must stop at your shortest path limit.
//Your shortest path limit is the shortest path you already know does NOT bridge <from_sector> and <to_sector>.
//(which is probably the number of times you searched all sectors for <to_sector>)
int	actor_recursive_path_from_sector_to_sector(int from_sector, int to_sector, int iterations, int iter_limit)
{
	_sector * source = &sectors[from_sector];
	
	if(iterations > iter_limit) return 0;
	iterations++;
	
	for(int i = 0; i < source->nbAdjacent; i++)
	{
		int check_id = source->pvs[i+1];
		
		if(check_id == to_sector) return 1;
		
		if(actor_recursive_path_from_sector_to_sector(check_id, to_sector, iterations, iter_limit)) return 1;
	}
	
	return 0;
}

