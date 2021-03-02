#ifndef __AI__
#define __AI__

#include <stdint.h>
#include <unistd.h>
#include "utils.h"

void initialize_ai();

void find_solution( state_t* init_state);
void free_memory(unsigned expanded_nodes);
node_t* applyAction(node_t* n, position_s* selected_peg, move_t action); 

#endif
