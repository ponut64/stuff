//vwmodelfunc.c
//this file compiled inside draw.c
_viewmodelSlot viewmodel_slots[2];
_viewmodelSlot * active_viewmodel;

_viewmodelData lever_pistol_vm;
_viewmodelData shorty_shotgun_vm;

void * viewmodel_0_workram_copy;
void * viewmodel_1_workram_copy;

//////////////////////////////////////////////////////////////////////////////
//Animation Structs
//////////////////////////////////////////////////////////////////////////////
backgroundAnimation * viewmodel_state;

backgroundAnimation shorty_idle;
backgroundAnimation shorty_fire;

backgroundAnimation leverpistol_idle;
backgroundAnimation leverpistol_fire;

void initialize_viewmodel_data(void)
{
	viewmodel_slots[0].buffer = viewmodel_0_workram_copy;
	viewmodel_slots[0].slot_in_slot_pointer = NULL; //(really could just be NULL)
	
	viewmodel_slots[1].buffer = viewmodel_1_workram_copy;
	viewmodel_slots[1].slot_in_slot_pointer = NULL; //(really could just be NULL)
	
	lever_pistol_vm.fid = GFS_NameToId((Sint8*)"LPISTOL.TGA");
	lever_pistol_vm.inSlot = -1; //initialized as not in a slot
	lever_pistol_vm.slot_data.idle_state = &leverpistol_idle;
	lever_pistol_vm.slot_data.use_state = &leverpistol_fire;
	
	shorty_shotgun_vm.fid = GFS_NameToId((Sint8*)"SHORTY.TGA");
	shorty_shotgun_vm.inSlot = -1; //initialized as not in a slot
	shorty_shotgun_vm.slot_data.idle_state = &shorty_idle;
	shorty_shotgun_vm.slot_data.use_state = &shorty_fire;
}

void load_viewmodel_to_slot(_viewmodelData * type, int slot)
{	
	_viewmodelSlot * vmslot = &viewmodel_slots[slot];
	//First, mark what is using this slot as unloaded, through this pointer.
	//(if it isn't trying to se
	*vmslot->slot_in_slot_pointer = -1;
	
	get_file_in_memory(type->fid, vmslot->buffer);
	type->inSlot = slot;
	vmslot->slot_in_slot_pointer = &type->inSlot;
	vmslot->idle_state = type->slot_data.idle_state;
	vmslot->use_state = type->slot_data.use_state;
	
}

void set_viewmodel_from_slot(int slot)
{
	active_viewmodel = &viewmodel_slots[slot];	
	set_8bpp_tga_to_nbg1_image_from_ram(active_viewmodel->buffer);
	viewmodel_state = active_viewmodel->idle_state;
} 

//what is the workflow desired?
// 1. The player will be able to decide, through an inventory system, which items are loaded into the slots.
// 2. The game must keep track of which items are loaded into which slot and be ready to swap in an instant.
// 3. The inventory menu will be the place where new viewmodels are loaded.
// 4. The player will have a "swap" button, which will instantly swap between the two loaded slots.
// 5. Thusly, the game needs to identify the loaded viewmodel's association with the item's function data.
// Right now, there are only two viewmodels and no associated item function structures.
// So this code is more or less as far as it is going to go right now.
//

void use_viewmodel(void)
{
	viewmodel_state = active_viewmodel->use_state;
}
