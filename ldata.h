#pragma once

extern Bool ldata_ready;

void	fetch_and_load_leveldata(Sint8 * filename);
void	process_binary_ldata(void * source_data);
void	testing_level_data(Sint8 * filename, void * destination);
void	level_data_basic(void);

