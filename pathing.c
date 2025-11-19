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
			
			accurate_normalize(vector_to_tc, normal_to_tc, 5);
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
			pathing->count[sctA][sctB] += sct->paths[j].numNodes;
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
			
			accurate_normalize(vector_to_tc, normal_to_tc, 5);
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
				normal_to_tc[X] = -((t_center[X] - (guidance_point[X]>>2))>>4);
				normal_to_tc[Y] = -((t_center[Y] - (guidance_point[Y]>>2))>>4);
				normal_to_tc[Z] = -((t_center[Z] - (guidance_point[Z]>>2))>>4);
				accurate_normalize(normal_to_tc, sct->paths[s].dir[num_paths], 5);
				num_paths++;
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
			//Now we need to check for an unpopulared intersection.
			//In this case, the pathing system needs the number zeroed out such that it can quickly know there is no valid path.
			if((sct2->paths[bt].nodes[0][X] & sct2->paths[bt].nodes[0][Y] & sct2->paths[bt].nodes[0][Z]) == -1)
			{
				sct->paths[s].numNodes = 0;
				sct2->paths[bt].numNodes = 0;
				continue;
			}
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
			pathing->count[sctA][sctB] += sct->paths[j].numNodes;
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
			//(must account for cases wherein there is no valid path for adjacent sectors)
			if(sct->paths[j].numNodes)
			{
				//The path itself contains a number of nodes, so no need to index the array here; it should always be at zero.
				//The caveat for that would be in a case where [sctA][sctB] must have the same address as [sctB][sctA],
				//but other code handles that requirement.
				pathing->guides[sctA][sctB][0] = &sct->paths[j];
				pathing->count[sctA][sctB] += sct->paths[j].numNodes;
			}
		}
	}
	
	//Be aware that in the process of building these lists, arranging, and copying data,
	//we do not remove duplicates, nor is there an easy point to check for duplicates.
	//For instance, we can check and prevent copying over duplicates at the point where nodes are copied between sectors.
	//But in doing that, the order of nodes is broken, as is the total assumed node count. The memory will get allocated anyway.
	//So you'd have to allocate the memory, then scan each set for duplicates, remove the duplicates, then consolidate the lists.
	//At that point, like I said, you will still have allocated the memory for every node anyway, but at least the path search will be faster.
	//(And that point in the code where that could be done is right here)
	_pathNodes * cpy = (_pathNodes*)dirty_buf;
	cpy->dir = (POINT*)((unsigned int)dirty_buf + 1024);
	cpy->nodes = (POINT*)((unsigned int)dirty_buf + 2048);
	
	for(int i = 0; i < MAX_SECTORS; i++)
	{
		sct = &sectors[i];
		if(sct->nbAdjacent == 0 || sct->nbVisible == 0) continue;
		for(int j = 0; j < sct->nbAdjacent; j++)
		{
			if(!sct->paths[j].numNodes) continue;
			int unduped_nodes = 0;
			cpy->fromSector = sct->paths[j].fromSector;
			cpy->toSector = sct->paths[j].toSector;
			
			for(int k = 0; k < sct->paths[j].numNodes; k++)
			{
				int * chk_pos = sct->paths[j].nodes[k];
				
				//We've now captured data for the node we want to check if it is or isn't a duplicate of other nodes.
				//Now we need to flip through every other node to capture data for them, and compare them.
				for(int f = 0; f < sct->paths[j].numNodes; f++)
				{
					if(f == k) continue;
					int * nxt_pos = sct->paths[j].nodes[f];
					int dist = approximate_distance(chk_pos, nxt_pos);
					if(nxt_pos[X] == -1 && nxt_pos[Y] == -1 && nxt_pos[Z] == -1) continue;
					if(dist < (64<<16))
					{
						//In this case, we've found a (positional) duplicate.
						//Destroy the duplicate.
						nxt_pos[X] = -1;
						nxt_pos[Y] = -1;
						nxt_pos[Z] = -1;
					}
				}
			}
			//At this point, all duplicates have been destroyed.
			//What we need to do is copy what's left to a re-ordered list, copy it back, then update the total.
			for(int k = 0; k < sct->paths[j].numNodes; k++)
			{
				int * chk_pos = sct->paths[j].nodes[k];
				int * chk_dir = sct->paths[j].dir[k];
				if(chk_pos[X] == -1 && chk_pos[Y] == -1 && chk_pos[Z] == -1) continue;
				cpy->nodes[unduped_nodes][X] = chk_pos[X];
				cpy->nodes[unduped_nodes][Y] = chk_pos[Y];
				cpy->nodes[unduped_nodes][Z] = chk_pos[Z];
				cpy->dir[unduped_nodes][X] = chk_dir[X];
				cpy->dir[unduped_nodes][Y] = chk_dir[Y];
				cpy->dir[unduped_nodes][Z] = chk_dir[Z];
				unduped_nodes++;
			}
			sct->paths[j].numNodes = unduped_nodes;
			int sctA = sct->paths[j].fromSector;
			int sctB = sct->paths[j].toSector;
			pathing->count[sctA][sctB] = unduped_nodes;
			for(int k = 0; k < sct->paths[j].numNodes; k++)
			{
				int * src_pos = cpy->nodes[k];
				int * src_dir = cpy->dir[k];
				
				int * dst_pos = sct->paths[j].nodes[k];
				int * dst_dir = sct->paths[j].dir[k];
				
				dst_pos[X] = src_pos[X];
				dst_pos[Y] = src_pos[Y];
				dst_pos[Z] = src_pos[Z];
				dst_dir[X] = src_dir[X];
				dst_dir[Y] = src_dir[Y];
				dst_dir[Z] = src_dir[Z];
			}
			
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
	//Re-seat the path node pointer (in case level is re-loaded, this pointer must be re-set)
	pathStackPtr = sectorPathHeap;
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


void	actor_hit_wall(_actor * act, int * wall_norm)
{
	
	int deflectionFactor = -fxdot(act->velocity, wall_norm);
		
	act->velocity[X] += fxm(wall_norm[X], deflectionFactor + REBOUND_ELASTICITY);
	act->velocity[Y] += fxm(wall_norm[Y], deflectionFactor + REBOUND_ELASTICITY);
	act->velocity[Z] += fxm(wall_norm[Z], deflectionFactor + REBOUND_ELASTICITY);
	//Small push to secure surface release
	act->dV[X] += wall_norm[X];
	act->dV[Y] += wall_norm[Y];
	act->dV[Z] += wall_norm[Z];
	
	act->info.flags.hitWall = 1;
	
}


int		actorMoveToPos(_actor * act, int * target, int rate, int gap)
{
	static int target_dif[3];
	static int dif_norm[3];
	
	target_dif[X] = (target[X] - act->pos[X])>>4;
	target_dif[Y] = (target[Y] - act->pos[Y])>>4;
	target_dif[Z] = (target[Z] - act->pos[Z])>>4;
	
	//Flatten the dif according to the allowable movement of the actor
	target_dif[Y] = 0;
	
	quick_normalize(target_dif, dif_norm);
	
	target_dif[X] = JO_ABS(target_dif[X]>>12);
	target_dif[Y] = JO_ABS(target_dif[Y]>>12);
	target_dif[Z] = JO_ABS(target_dif[Z]>>12);
	
	if((target_dif[X] + target_dif[Y] + target_dif[Z]) < gap) return 1;

	act->dV[X] += fxm(dif_norm[X], rate);
	act->dV[Y] += fxm(dif_norm[Y], rate);
	act->dV[Z] += fxm(dif_norm[Z], rate);
	
	return 0;
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
		//If there is no valid path between these adjacent sectors, don't bother checking with it.
		if(!pathing->count[from_sector][check_id]) continue;
		if(check_id == to_sector) return 1;
		
		if(actor_recursive_path_from_sector_to_sector(check_id, to_sector, iterations, iter_limit)) return 1;
	}
	
	return 0;
}



int		actorLineOfSight(_actor * act, int * pos)
{
	//Goal:
	//Check line-of-sight from (actor) to (pos)
	//This involves all collision-enabled proxies
	
	static int vector_to_pos[3] = {0,0,0};
	static int normal_to_pos[3] = {0,0,0};
	static int vector_to_hit[3] = {0,0,0};
	static int hit[3] = {0,0,0};
	//static int nHit[3];
	static int hitPly = 0;
	int possibleObstruction = 0;
	
	hit[X] = 32767<<16;
	hit[Y] = 32767<<16;
	hit[Z] = 32767<<16;
	
	vector_to_pos[X] = (pos[X] - act->pos[X])>>4;
	vector_to_pos[Y] = (pos[Y] - act->pos[Y])>>4;
	vector_to_pos[Z] = (pos[Z] - act->pos[Z])>>4;
	
	quick_normalize(vector_to_pos, normal_to_pos);
	
	//Methods needed:
	//well i have them
	
/* 	for(int c = 0; c < MAX_PHYS_PROXY; c++)
	{
		//nbg_sprintf(0, 0, "(PHYS)"); //Debug ONLY
		if(RBBs[c].status[1] != 'C') continue;
		if(RBBs[c].boxID == act->box->boxID) continue;
		unsigned short edata = dWorldObjects[activeObjects[c]].type.ext_dat;
		unsigned short boxType = edata & (0xF000);
		//Check if object # is a collision-approved type
		switch(boxType)
		{
			case(OBJPOP):
			case(SPAWNER):
			possibleObstruction += hitscan_vector_from_position_box(normal_to_pos, act->pos, hit, nHit, &RBBs[c]);
			break;
			case(ITEM | OBJPOP):
			break;
			case(BUILD | OBJPOP):
			possibleObstruction += hitscan_vector_from_position_building(normal_to_pos, act->pos, hit, &hitPly, &entities[dWorldObjects[activeObjects[c]].type.entity_ID], RBBs[c].pos, NULL);
			break;
			default:
			break;
		}
	} */
	

	//Check sectors for LOS
	_sector * sct = &sectors[act->curSector];
	//Rather than check everything in the sector's PVS for collision,
	//we will only check the sector itself + primary adjacents.
	for(int s = 0; s < (sct->nbAdjacent+1); s++)
	{
		possibleObstruction += hitscan_vector_from_position_building(normal_to_pos, act->pos, hit, &hitPly, sct->ent, levelPos, &sectors[sct->pvs[s]]);
		if(possibleObstruction) break;
		hit[X] = 0;
		hit[Y] = 0;
		hit[Z] = 0;
	}
	
	//What we should have returned now is the closest hit point to the actor (hit).
	//We need to know if (hit) is between (actor) and (pos).
	//What we will do is see if the hit is on the right side of pos using the normal to it.
	//Of course, if there was no hit, there is no obstruction to line-of-sight.
	if(!possibleObstruction)
	{
		return 1;
	}
	
	vector_to_hit[X] = (pos[X] - hit[X]);
	vector_to_hit[Y] = (pos[Y] - hit[Y]);
	vector_to_hit[Z] = (pos[Z] - hit[Z]);
	
	//(1<<16 being used as some tolerance in case the target position is exactly the hit position, as may sometimes happen)
	if(fxdot(vector_to_hit, normal_to_pos) > (1<<16))
	{
		act->blockedLOSNorm[X] = sct->ent->pol->nmtbl[hitPly][X];
		act->blockedLOSNorm[Y] = sct->ent->pol->nmtbl[hitPly][Y];
		act->blockedLOSNorm[Z] = sct->ent->pol->nmtbl[hitPly][Z];
		return 0;
	}
	//No obstruction conditions were met; line-of-sight is achieved.
	return 1;
	
}


int	actorCheckPathOK(_actor * act, int * path_dir)
{
	//Step 1: Create an arbitrary point some distance in the direction the actor is moving, scaled by velocity.
	static int actorPathProxy[3];
	static int towardsFloor[3] = {0, (1<<16), 0};
	static int floorProxy[3];
	//static int floorNorm[3];
	static int hitFloorPly = 0;
	static int losToProxy = 0;
	
	//First Pass
	//If a wall is likely to be in the way in the first place, consider that the path is not OK.
	//In this case, the path target is significantly above the actor; it cannot reach it directly.
	if((JO_ABS((act->pos[Y] + act->box->radius[Y]) - act->pathTarget[Y]) > (act->box->radius[Y]<<1)) && act->curPathStep == 0 && act->curSector != act->goalSector)
	{
		return 0;
	}
	
	//(we add the Z axis because that's forward)
	actorPathProxy[X] = act->pos[X] + fxm(act->velocity[X]<<1, time_fixed_scale) + fxm(act->box->radius[Z]<<1, path_dir[X]);
	actorPathProxy[Z] = act->pos[Z] + fxm(act->velocity[Z]<<1, time_fixed_scale) + fxm(act->box->radius[Z]<<1, path_dir[Z]);
	
	//We are going to do something that in many circumstances we would not want to do.
	//We are going to ignore the Y axis of the path proxy; it will retain the Y axis of the actor.
	//This is a simplification of representing the allowable movement of the actor.
	//You don't want them to think they can path to a point up a vertical wall; the wall should block them,
	//and make them find another path.
	actorPathProxy[Y] = act->pos[Y];
	
	//Addendum: Check if the actor's distance to the path target is low enough to be used as the path target.
	if(approximate_distance(act->pos, act->pathTarget) < (128<<16))
	{
		actorPathProxy[X] = act->pathTarget[X];
		actorPathProxy[Y] = act->pathTarget[Y];
		actorPathProxy[Z] = act->pathTarget[Z];
		
	}

	sprite_prep.info.drawMode = SPRITE_TYPE_BILLBOARD;
	sprite_prep.info.drawOnce = 1;
	sprite_prep.info.mesh = 0;
	sprite_prep.info.sorted = 0;
	static short sprSpan[3] = {10,10,10};
	add_to_sprite_list(actorPathProxy, sprSpan, 'A', 5, sprite_prep, 0, 0);
	
	//Step 2: Check line-of-sight to this point.
	losToProxy = actorLineOfSight(act, actorPathProxy);
	
	//If no line of sight, path is not ok.
	if(!losToProxy) return 0;
	
	//Step 3: Get a floor position/normal.
	int possibleFloor = 0;

	//Check sectors for LOS
	_sector * sct = &sectors[act->curSector];
	//We can't forget to re-set these.
	floorProxy[X] = -(32765<<16);
	floorProxy[Y] = -(32765<<16);
	floorProxy[Z] = -(32765<<16);
	//Rather than check everything in the sector's PVS for collision,
	//we will first check the sector itself + primary adjacents.
	for(int s = 0; s < (sct->nbAdjacent+1); s++)
	{
		possibleFloor += hitscan_vector_from_position_building(towardsFloor, actorPathProxy, floorProxy, &hitFloorPly, sct->ent, levelPos, &sectors[sct->pvs[s]]);
		//floorNorm[X] = sct->ent->pol->nmtbl[hitFloorPly][X];
		//floorNorm[Y] = sct->ent->pol->nmtbl[hitFloorPly][Y];
		//floorNorm[Z] = sct->ent->pol->nmtbl[hitFloorPly][Z];
	}
	


	//If there was no possible floor, path is not OK.
	//if(!possibleFloor) return 0;
	

	
	//If the floor height difference is outside the tolerance, path is not OK.
	// I'll have to use some other method to validate whether or not the guidance point is on a floor or not.
	if(JO_ABS((act->pos[Y] + act->box->radius[Y]) - floorProxy[Y]) > (act->box->radius[Y]<<1))
	{
		possibleFloor = 0;
		//Next, we need to check all bounding boxes.
		//Bounding boxes as recorded should already be limited to what is nearby, so this should be OK.
		//Further, we only want this for recording if there is a valid surface to stand on and there wasn't already one.
		for(int i = 0; i < MAX_PHYS_PROXY; i++)
		{
			_boundBox * box = &RBBs[i];
			if(box->status[1] != 'C') continue;
			if(box->pos[Y] < act->pos[Y]) continue; 
			if(entities[objPREP[i]].type == MODEL_TYPE_BUILDING)
			{
			possibleFloor += hitscan_vector_from_position_building(towardsFloor, actorPathProxy, floorProxy, &hitFloorPly, &entities[objPREP[i]], box->pos, NULL);
			}
		}
		if(!possibleFloor)
		{
		//Stand-in data
		act->blockedLOSNorm[X] = -path_dir[X];
		act->blockedLOSNorm[Y] = 0;
		act->blockedLOSNorm[Z] = -path_dir[Z];
		actor_hit_wall(act, act->blockedLOSNorm);
		return 0;
		}
	}	
	//If this wasn't actually a floor at all, path is not OK.
	//if(floorNorm[Y] > (-32768)) return 0;
	
	//Otherwise, path should be OK.
	return 1;
	
}


void	pathing_exception(int actor_id)
{
	//In case where the actor lost LOS to the navigation proxy during normal pathing, an exception must be created.
	//To do so, we will be adding to the actor's navigation list.
	//What we must do in this case is find a unique data set to apply to a navigation step to mark it as the exception case.
	_actor * act = &spawned_actors[actor_id];
	int new_step_flag = 0;
	if(act->exceptionStep == INVALID_SECTOR)
	{
		act->exceptionStep = act->curPathStep+1;
		act->exceptionDir[X] = act->pathUV[X];
		act->exceptionDir[Y] = act->pathUV[Y];
		act->exceptionDir[Z] = act->pathUV[Z];
		new_step_flag = 1;
	}
	
	act->exceptionTimer = 0;
	act->curPathStep = act->exceptionStep;
	
	//quick_normalize(rotationSet, act->exceptionDir);
	
	// nbg_sprintf_decimal(3, 10, act->exceptionDir[X]);                     
	// nbg_sprintf_decimal(3, 11, act->exceptionDir[Y]);                       
	// nbg_sprintf_decimal(3, 12, act->exceptionDir[Z]);
	// nbg_sprintf_decimal(3, 13, fxdot(act->exceptionDir, act->exceptionDir));
	
	
	_pathStep * step = &pathStepHeap.steps[actor_id][act->curPathStep];
	
	int rotationSet[3] = {act->exceptionDir[X], act->exceptionDir[Y], act->exceptionDir[Z]};
	
	if(!step->winding)
	{
	fxrotY(rotationSet, act->exceptionDir, (30 * 182));
	} else {
	fxrotY(rotationSet, act->exceptionDir, -(30 * 182));
	}
	
	act->exceptionPos[X] = (fxm(act->exceptionDir[X], 256<<16) + act->pos[X] - levelPos[X]);
	act->exceptionPos[Y] = (fxm(act->exceptionDir[Y], 256<<16) + act->pos[Y] - levelPos[Y]) + act->box->radius[Y];
	act->exceptionPos[Z] = (fxm(act->exceptionDir[Z], 256<<16) + act->pos[Z] - levelPos[Z]);

	//We just need to find a way to set this high again if the actor is **really** thrown off (collision with another actor or impulse)
	if(!new_step_flag)
	{
		step->winding = 1;
	} else {
		step->winding = 0;
	}

	step->pos = act->exceptionPos;
	step->dir = act->exceptionDir;
	step->fromSector = act->curSector;
	step->toSector = act->curSector; //(may be incorrect)
	step->actorID = actor_id;
	pathStepHeap.numStepsUsed[actor_id] = act->exceptionStep;
	
}


void	runPath(int actor_id)
{
	//register which path we are looking at
	_pathStep * stepList = &pathStepHeap.steps[actor_id][0];
	
	int cap_sct = stepList[1].toSector;
	int lst_sct = stepList[1].fromSector;
	
	_actor * act = &spawned_actors[actor_id];
	
	if(act->curSector == act->goalSector) return;
	
	_sector * sctA = &sectors[cap_sct];
	
	int next_step = INVALID_SECTOR;
	static int iterations = 0;
	iterations = 0;
	do{
		for(int i = 0; i < sctA->nbAdjacent; i++)
		{
			if(!pathing->count[cap_sct][sctA->pvs[i+1]]) continue;
			//It stands to reason that the last sector we were in would show up on the list first,
			//so we have to specifically exclude it.
			if(sctA->pvs[i+1] == lst_sct) continue;
			if(actor_recursive_path_from_sector_to_sector(sctA->pvs[i+1], act->goalSector, 0, iterations))
			{
				next_step = sctA->pvs[i+1];
				break;
			}
		}
	iterations++;
	}while(iterations <= MAX_PATHING_STEPS && next_step == INVALID_SECTOR);
	
	//nbg_sprintf(3, 20, "pth(%i)", next_step);
	
	//In case no valid path was found, make no changes to the path table.
	if(next_step == INVALID_SECTOR) return;
	
	stepList[1].fromSector = cap_sct;
	stepList[1].toSector = next_step;
	
	//Between two sectors (and thus on each path step), there can be a number of valid pathNodes.
	//However, we only want to use the pathNode that is closest to the actor.
	//This is an oversimplification; we want to use the path node that is closest to the next path node down the list.
	//However, due to the way the system is built, we don't know which sector is next right now. So we use the approx. closest.
	_pathNodes * path = pathing->guides[cap_sct][next_step][0];
	int * active_node = path[0].nodes[0];
	int * active_dir = path[0].dir[0];
	if(path[0].numNodes > 1)
	{
		int cur_dist = approximate_distance(active_node, act->pos);
		int low_dist = cur_dist;
		for(int j = 1; j < path[0].numNodes; j++)
		{
			cur_dist = approximate_distance(active_node, path[0].nodes[j]);
			//(yes the logic is inverted because the vector spaces are inverted)
			if(cur_dist > low_dist)
			{
				active_node = path[0].nodes[j];
				active_dir = path[0].dir[j];
				low_dist = cur_dist;
			}
		}
	}
	stepList[1].pos = active_node;
	stepList[1].dir = active_dir;
	stepList[1].actorID = actor_id;
	act->curPathStep = 1;
}



void	checkInPathSteps(int actor_id)
{
	_actor * act = &spawned_actors[actor_id];
	if(act->curPathStep == INVALID_SECTOR) return;
	char * numSteps = &pathStepHeap.numStepsUsed[actor_id];
	//nbg_sprintf(3, 14, "total_step(%i)", *numSteps);
	if(act->curPathStep > *numSteps) act->curPathStep = *numSteps;
	//register which path we are looking at
	_pathStep * stepList = &pathStepHeap.steps[actor_id][0];
	//path steps
	_pathStep * step;
	
	static int onPathNode = 0;
	
	if(act->exceptionTimer >= ACTOR_PATH_EXCEPTION_TIME && act->curPathStep > 0 && act->exceptionStep != INVALID_SECTOR)
	{
		//We will use the path checking function to evaluate the line-of-sight to the original path direction.
		//If the path to the original point is clear, we will release the exception.
		//If it is not, we will reset the timer, but we won't release the exception.
		step = &stepList[act->curPathStep-1];
		int path_delta[3] = {((levelPos[X] + step->pos[X]) - act->pos[X])>>4, 0, ((levelPos[Z] + step->pos[Z]) - act->pos[Z])>>4};
		int path_dUV[3] = {0,0,0};
		quick_normalize(path_delta, path_dUV);
	
		if(actorCheckPathOK(act, path_dUV) || onPathNode)
		{
			act->curPathStep--;
			act->exceptionStep = INVALID_SECTOR;
			if(onPathNode)
			{
				onPathNode = 0;
				runPath(actor_id);
			}
		} else {
			act->exceptionPos[X] += fxm(act->exceptionDir[X], 256<<16);
			//(flying actors might need the Y axis)
			act->exceptionPos[Z] += fxm(act->exceptionDir[Z], 256<<16);
		}
		act->exceptionTimer = 0;
	}
	
	//register the step
	step = &stepList[act->curPathStep];
	//simple test: if we are in the sector that this step is going towards, we should change steps.
	if((act->curSector == step->toSector) && act->curPathStep > 0 && act->exceptionStep == INVALID_SECTOR)
	{
		act->curPathStep--;
		act->exceptionStep = INVALID_SECTOR;
		runPath(actor_id);
		//since the current step has changed, register it again
		step = &stepList[act->curPathStep];
		onPathNode = 0;
		act->exceptionTimer = 0;
	}
	
	if(act->curSector != step->fromSector && act->curSector != step->toSector)
	{
		onPathNode = 0;
		act->exceptionTimer = 0;
		act->curPathStep = 0;
		act->pathingLatch = 0;
		step = &stepList[act->curPathStep];
		step->winding = 0;
	}

	
	// nbg_sprintf(3, 15, "stp(%i)", act->curPathStep);
	// nbg_sprintf(3, 16, "sc1(%i)", step->fromSector);
	// nbg_sprintf(3, 17, "sc2(%i)", step->toSector);
	// nbg_sprintf(3, 18, "exc(%i)", act->exceptionStep);
	
	// nbg_sprintf_decimal(3, 10, step->dir[X]);                     
	// nbg_sprintf_decimal(3, 11, step->dir[Y]);                       
	// nbg_sprintf_decimal(3, 12, step->dir[Z]);
	// nbg_sprintf_decimal(3, 13, fxdot(step->dir, step->dir));
	
	act->exceptionTimer += delta_time;
	if(act->curPathStep > 0)
	{
		
		act->pathTarget[X] = levelPos[X] + step->pos[X];
		act->pathTarget[Y] = levelPos[Y] + (step->pos[Y] - act->box->radius[Y]);
		act->pathTarget[Z] = levelPos[Z] + step->pos[Z];
		
		//iterate towards the step
		onPathNode += actorMoveToPos(act, act->pathTarget, 32768, act->box->radius[X]>>16);
		//if on the path node ( = 1), we need to do something else.
		//each path node has a direction; we need to follow that direction until we are in the sector of the next node.
		// ^^ this exception needs to be added - we only want to follow this particular exception when we are not in the target sector.
		// Once we're in it, we need to abandon this and go towards the target, or else we'll get stuck.
		if(onPathNode && act->exceptionStep == INVALID_SECTOR && act->exceptionTimer >= ACTOR_PATH_EXCEPTION_TIME && step->toSector != act->curSector)
		{
			//Grab the direction from the last path node, presumed to not be an exception path node as restricted by the "if"
			int * pathNodeDir = step->dir;
			int lsSct = step->toSector;

			act->exceptionStep = act->curPathStep+1;
			act->curPathStep = act->exceptionStep;
			act->exceptionTimer = 0;
			
			step = &stepList[act->exceptionStep];
			
			step->dir = pathNodeDir;

			act->exceptionDir[X] = step->dir[X];
			act->exceptionDir[Y] = step->dir[Y];
			act->exceptionDir[Z] = step->dir[Z];

			act->exceptionPos[X] = (fxm(act->exceptionDir[X], 256<<16) + act->pos[X] - levelPos[X]);
			act->exceptionPos[Y] = (fxm(act->exceptionDir[Y], 256<<16) + act->pos[Y] - levelPos[Y]) + act->box->radius[Y];
			act->exceptionPos[Z] = (fxm(act->exceptionDir[Z], 256<<16) + act->pos[Z] - levelPos[Z]);
			
			step->pos = act->exceptionPos;
			step->dir = act->exceptionDir;
			step->fromSector = act->curSector;
			step->toSector = lsSct; 
			step->actorID = actor_id;
			step->winding = 0;
			pathStepHeap.numStepsUsed[actor_id] = act->exceptionStep;
			
			//Curious?
			//This should *mostly* resolve this issue; the exception exists to set the actor on course into a sector if it may have reached the node outside of it.
			//In case the actor is already safely inside the sector, this exception isn't needed.
			//What this does is it flags the exception as invalid and it will be discarded ahead of the exception timer if the actor is within the sector.
			act->exceptionStep = INVALID_SECTOR;
		}
	} else {
		if(act->info.flags.losTarget)
		{
			act->pathTarget[X] = act->pathGoal[X];
			act->pathTarget[Y] = act->pathGoal[Y];
			act->pathTarget[Z] = act->pathGoal[Z];
			act->atGoal = actorMoveToPos(act, act->pathTarget, 32768, act->box->radius[X]>>16);
		}
		return;
	}

	
	
}

void	findPathTo(int targetSector, int actor_id)
{
	_actor * act = &spawned_actors[actor_id];
	if(act->curSector == targetSector || act->pathingLatch == 1)
	{
		pathing_exception(actor_id);
		return;
	}
	_sector * sctA = &sectors[act->curSector];
	_sector * sctB = &sectors[targetSector];
	if(sctA->nbAdjacent == 0) return;
	if(sctB->nbAdjacent == 0) return;
	
	act->pathingLatch = 1;
	act->exceptionStep = INVALID_SECTOR;
	
	char * numSteps = &pathStepHeap.numStepsUsed[actor_id];
	*numSteps = -1;
	//register which path we are looking at
	_pathStep * stepList = &pathStepHeap.steps[actor_id][0];
	
	//reconciliation: the final path step is always to the pathGoal, so add it
	stepList[0].fromSector = targetSector;
	stepList[0].toSector = targetSector;
	stepList[0].pos = &act->pathGoal[0];
	stepList[0].dir = &act->dirUV[0];
	stepList[0].actorID = actor_id;
	*numSteps += 1;

	//first a sanity check
	//if sector A is adjacent to sector B, then our path table is one step
	//but **ONLY** if a path exists from sector A to sector B
	if(pathing->count[act->curSector][targetSector])
	{
		for(int i = 0; i < sctA->nbAdjacent; i++)
		{
			if(sctA->pvs[i+1] == targetSector)
			{
				//the target sector was found as adjacent to the actor's current sector
				//add this to the path table
				stepList[1].fromSector = act->curSector;
				stepList[1].toSector = targetSector;
				_pathNodes * path = pathing->guides[act->curSector][targetSector][0];
				stepList[1].pos = path[0].nodes[0];
				stepList[1].dir = path[0].dir[0];
				stepList[1].actorID = actor_id;
				*numSteps += 1;
				act->curPathStep = *numSteps;
				return;
			}
		}
	}
	//Objective:
	//Iterate through each adjacent sector to find the targetSector. 
	//This process must be done in away where each iteration is limited such that every path is allowed to process up to the same limit.
	//This is so that the shortest path is found within the limit, which increases every iteration.
	int next_step = INVALID_SECTOR;
	static int iterations = 0;
	iterations = 0;
	do{
		for(int i = 0; i < sctA->nbAdjacent; i++)
		{
			if(!pathing->count[act->curSector][sctA->pvs[i+1]]) continue;
			if(actor_recursive_path_from_sector_to_sector(sctA->pvs[i+1], targetSector, 0, iterations))
			{
				next_step = sctA->pvs[i+1];
				break;
			}
		}
	iterations++;
	}while(iterations <= MAX_PATHING_STEPS && next_step == INVALID_SECTOR);
	
	//In case no valid path was found, make no changes to the path table.
	if(next_step == INVALID_SECTOR) return;
	
	//Otherwise, start pathing with the set sector as the next step.
	//Note that within this function, we can only start a path; perhaps I need a code cleanup so there is:
	//clearPath
	//findPath (<- this function)
	//runPath (<- will operate if toSector is not targetSector)
	
	stepList[1].fromSector = act->curSector;
	stepList[1].toSector = next_step;
	_pathNodes * path = pathing->guides[act->curSector][next_step][0];
	stepList[1].pos = path[0].nodes[0];
	stepList[1].dir = path[0].dir[0];
	stepList[1].actorID = actor_id;
	*numSteps += 1;
	act->curPathStep = *numSteps;
	
	return;
	
}

