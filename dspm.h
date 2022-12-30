#ifndef __DSPM_H__
# define __DSPM_H__

extern volatile int * dsp_input_addr;
extern volatile int * dsp_noti_addr;
extern volatile int * dsp_output_addr;
void	load_dsp_prog(void);
void	run_dsp(void);

#endif

