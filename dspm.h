#pragma once

extern volatile int * dsp_input_addr;
extern volatile int * dsp_noti_addr;
extern volatile int * dsp_output_addr;
void	init_dsp_programs(void);

void	load_winder_prog(void);

void	run_winder_prog(int num_verts, int * num_portals, void * vertices);

