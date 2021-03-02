#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "ai.h"
#include "utils.h"
#include "hashtable.h"
#include "stack.h"


void copy_state(state_t* dst, state_t* src){
	
	//Copy field
	memcpy( dst->field, src->field, SIZE*SIZE*sizeof(int8_t) );

	dst->cursor = src->cursor;
	dst->selected = src->selected;
}

/**
 * Saves the path up to the node as the best solution found so far
*/
void save_solution( node_t* solution_node ) {
	node_t* n = solution_node;
	while( n->parent != NULL ){
		copy_state( &(solution[n->depth]), &(n->state) );
		solution_moves[n->depth-1] = n->move;

		n = n->parent;
	}
	solution_size = solution_node->depth;
}


node_t* create_init_node( state_t* init_state ) {
	node_t * new_n = (node_t *) malloc(sizeof(node_t));
	new_n->parent = NULL;	
	new_n->depth = 0;
	copy_state(&(new_n->state), init_state);
	return new_n;
}

node_t* add_node(node_t *parent, position_s* selected, move_t action) {
	node_t * new_n = (node_t *) malloc(sizeof(node_t));
	// Copy the state from the parent node provided
	copy_state(&(new_n->state), &(parent->state));
	// Copy the action provided to the new node. 
	new_n->move = action;
	// Point the parent to the new node
	new_n->parent = parent;	
	// Increasing depth by one from the previous one. 
	new_n->depth = (parent->depth) + 1;
	// Pointed the cursor of the new node to the selected peg. 
	new_n->state.cursor = *selected; 
	return new_n;
}

/**
 * Apply an action to node n and return a new node resulting from executing the action
*/
node_t* applyAction(node_t* n, position_s* selected_peg, move_t action) {
	// Creates a new node 
    node_t* new_node = add_node(n, selected_peg, action);	
	// Executes the move and then returns 
    execute_move_t(&(new_node->state), &(new_node->state.cursor), action);
	return new_node;
}

/**
 * Find a solution path as per algorithm description in the handout
 */

void find_solution( state_t* init_state) {

	HashTable table;
	
	// Choose initial capacity of PRIME NUMBER 
	// Specify the size of the keys and values you want to store once 
	ht_setup(&table, sizeof(int8_t) * SIZE * SIZE, sizeof(int8_t) * SIZE * SIZE, 16769023);

	// Initialize Stack
	initialize_stack();

	// Initialise the list to add the nodes 
	list_t *list; 
	list = make_empty_list();
	
	// Add the initial node
	node_t* firstNode = create_init_node(init_state);

	// Push the first node in the stack 
	stack_push(firstNode); 
	
	// Keeping a count of all pegs 
	int remainPegs = num_pegs(&(firstNode->state)); 
	
	while (!is_stack_empty()) {
		node_t *n = stack_top(); 
		list = insert_at_head(list, n); 
		stack_pop(); 
		expanded_nodes++; 
		
		if (num_pegs(&(n->state)) < remainPegs) {
			save_solution(n);
			remainPegs = num_pegs(&(n->state)); 
		}
		
		for (int x=0; x < SIZE; x++) {
			for (int y=0; y < SIZE; y++) {
				// Choosing a peg position to check
				position_s peg; 
				peg.x = x; 
				peg.y = y; 

				// Checking every possible direction in the sequence defined 
				for (move_t direction = left; direction <= down; direction++) {
					
					if (can_apply(&(n->state), &peg, direction)) {
						node_t *newNode = applyAction(n, &peg, direction); 
						generated_nodes++; 
						
						// Game ends if won
						if (won(&(newNode->state))) {
							save_solution(newNode); 
							remainPegs = num_pegs(&(newNode->state)); 
							
							// If the game has ended then we free the stack and destroy the hashtable 
							free_stack(); 
							ht_destroy(&table); 
							free_list(list); 
							// Freeing the last node that wasnt ended into the list
							free(newNode); 
							return; 
						}
						// Making sure we are not adding duplicates so a hashtable is used. 
						if (!ht_contains(&table, &((newNode->state.field)))) {
							stack_push(newNode); 
							ht_insert(&table, &((newNode->state.field)), &((newNode->state.field))); 
						}
						else {
							list = insert_at_head(list, newNode); 
						}
					}
				}
			}
		}
		if (expanded_nodes >= budget) {
			// If the game has ended then we free the stack and destroy the hashtable 
			ht_destroy(&table); 
			free_list(list); 
			free_stack(); 
			return; 
		}
	}
}