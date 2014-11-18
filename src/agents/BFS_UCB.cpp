#include "BFS_UCB.hpp"
#include "SearchAgent.hpp"
#include <list>

BFS_UCB::BFS_UCB(RomSettings *rom_settings, Settings &settings,
			       ActionVect &actions, StellaEnvironment* _env) 
    : IW1Search( rom_settings, settings, actions, _env){
 
	m_max_reward = settings.getInt( "max_reward" );

	Q.resize( actions.size() );
	V.resize( actions.size() );
	N.resize( actions.size() );
	
	for ( unsigned a = 0; a < actions.size(); a++ ) {
		Q[a] = new Queue;
	}

}

BFS_UCB::~BFS_UCB() {
	for ( Queue* q : Q ) {
		delete q;
	}
}

int BFS_UCB::expand_node( TreeNode* curr_node )
{
	int num_simulated_steps =0;
	int num_actions = available_actions.size();
	static     int max_nodes_per_frame = max_sim_steps_per_frame / sim_steps_per_node;
	bool leaf_node = (curr_node->v_children.empty());
	m_expanded_nodes++;
	if(curr_node->novelty == 1)
	    m_exp_count_novelty1++;
	else
	    m_exp_count_novelty2++;
	// Expand all of its children (simulates the result)	
	if(leaf_node){ 
		curr_node->v_children.resize( num_actions );
		curr_node->available_actions = available_actions;
		if(m_randomize_successor)
			std::random_shuffle ( curr_node->available_actions.begin(), curr_node->available_actions.end() );

	
	}
	
	for (int a = 0; a < num_actions; a++) {
		Action act = curr_node->available_actions[a];
		
		TreeNode * child;
		// If re-expanding an internal node, don't creates new nodes
		if (leaf_node) {
			m_generated_nodes++;
			child = new TreeNode(	curr_node,	
						curr_node->state,
						this,
						act,
						sim_steps_per_node
						, discount_factor); 
	
			if ( check_novelty_1( child->state.getRAM() ) ) {
			    update_novelty_table( child->state.getRAM() );
			    child->novelty = 1;
			    m_gen_count_novelty1++;
			}
			else{
			    child->novelty = 2;
			    m_gen_count_novelty2++;
			}
			child->fn += ( m_max_reward - child->discounted_accumulated_reward ); // Miquel: add this to obtain Hector's BFS + m_max_reward * (720 - child->depth()) ;

			child->num_nodes_reusable = curr_node->num_nodes_reusable + num_actions;

			if (child->depth() > m_max_depth ) m_max_depth = child->depth();
			num_simulated_steps += child->num_simulated_steps;
					
			curr_node->v_children[a] = child;

		}
		else {
			
			child = curr_node->v_children[a];
			m_pruned_nodes++;
			// This recreates the novelty table (which gets resetted every time
			// we change the root of the search tree)
			if ( m_novelty_pruning ){
				if ( check_novelty_1( child->state.getRAM() ) ){
					update_novelty_table( child->state.getRAM() );
					child->novelty = 1;
					m_gen_count_novelty1++;
				}
				else{
					child->novelty = 2;
					m_gen_count_novelty2++;
					
				}
			}
			
			child->updateTreeNode();
			child->fn += ( m_max_reward - child->discounted_accumulated_reward ); // Miquel: add this to obtain Hector's BFS + m_max_reward * (720 - child->depth()) ;

			if (child->depth() > m_max_depth ) m_max_depth = child->depth();

			
			num_simulated_steps += child->num_simulated_steps;
		}
	
		// Don't expand duplicate nodes, or terminal nodes
		if (!child->is_terminal) {
		    if (! (ignore_duplicates && test_duplicate(child)) ){
				if( child->num_nodes_reusable < max_nodes_per_frame )
					Q[m_current_queue]->push(child);

		    }
		}
	
	}

	curr_node->already_expanded = true;
	return num_simulated_steps;
}

/* *********************************************************************
	update novelty_value to 0 to a node and all its children, all the way down the branch
 ******************************************************************* */
void BFS_UCB::reset_branch(TreeNode* node) {
	if (!node->v_children.empty()) {
		for(size_t c = 0; c < node->v_children.size(); c++) {	
		    //node->v_children[c]->updateTreeNode();		
		    reset_branch(node->v_children[c]);
			
		}
	}
	//node->novelty = 0;
	//node->fn = 0;	
	node->already_expanded = false;
}

unsigned BFS_UCB::size_branch(TreeNode* node) {
	unsigned size = 1;

	if (!node->v_children.empty()) {
		for(size_t c = 0; c < node->v_children.size(); c++) {			
			size += size_branch(node->v_children[c]);
			
		}
	}
	return size;	

}

/* *********************************************************************
   Expands the tree from the given node until i_max_sim_steps_per_frame
   is reached
	
   ******************************************************************* */

void BFS_UCB::expand_tree(TreeNode* start_node) {
	// If the root is terminal, we will not expand any of its children; deal with this
	//  appropriately
	if (start_node->is_terminal) {
		set_terminal_root(start_node);
		return;
	}
	
	int num_simulated_steps = 0;
	bool explore = true;
	
	int max_nodes_per_frame = max_sim_steps_per_frame / sim_steps_per_node;
	clear_queues();

	for ( unsigned k = 0; k < V.size(); k++ ) {
		V[k] = 0.0f;
		N[k] = 1;
	}
	
	m_num_iterations = 1;
	m_current_queue = 0;
	
	update_novelty_table( start_node->state.getRAM() );
	int num_actions = available_actions.size();

	if ( !start_node->v_children.empty() ) {
		start_node->updateTreeNode();
		for(size_t c = 0; c < start_node->v_children.size(); c++) {			
			TreeNode* child = start_node->v_children[c];				
				
			// This recreates the novelty table (which gets resetted every time
			// we change the root of the search tree)
			if( child->is_terminal ){						
				if ( check_novelty_1( child->state.getRAM() ) ){
					update_novelty_table( child->state.getRAM() );
					if(!child->already_expanded){
						child->novelty = 1;
					}
				}
				else{
					if(!child->already_expanded)
						child->novelty = 2;
							
						
				}
			}
				
			child->updateTreeNode();
			child->fn += ( m_max_reward - child->discounted_accumulated_reward ); // Miquel: add this to obtain Hector's BFS + m_max_reward * (720 - child->depth()) ;
				
			//First applicable action
			if(child->depth() == 1)
    				child->num_nodes_reusable = child->num_nodes();
			else
				child->num_nodes_reusable = start_node->num_nodes_reusable;
			if (child->depth() > m_max_depth ) m_max_depth = child->depth();
			
			// Don't expand duplicate nodes, or terminal nodes
			if (!child->is_terminal) {
				if (! (ignore_duplicates && test_duplicate(child)) ){
					Q[ c ]->push(child );
				}
				
			}
		}
	}
	else {

		start_node->v_children.resize( num_actions );
		start_node->available_actions = available_actions;
		if(m_randomize_successor)
			std::random_shuffle ( start_node->available_actions.begin(), start_node->available_actions.end() );
		for (int a = 0; a < num_actions; a++) {
			Action act = start_node->available_actions[a];
			TreeNode * child;
			m_generated_nodes++;
			child = new TreeNode(	start_node,	
						start_node->state,
						this,
						act,
						sim_steps_per_node
						, discount_factor); 
	
			if ( check_novelty_1( child->state.getRAM() ) ) {
				update_novelty_table( child->state.getRAM() );
				child->novelty = 1;
				m_gen_count_novelty1++;
			}
			else{
				child->novelty = 2;
				m_gen_count_novelty2++;
			}
			child->fn += ( m_max_reward - child->discounted_accumulated_reward ); // Miquel: add this to obtain Hector's BFS + m_max_reward * (720 - child->depth()) ;
	
			child->num_nodes_reusable = start_node->num_nodes_reusable + num_actions;
	
			if (child->depth() > m_max_depth ) m_max_depth = child->depth();
			num_simulated_steps += child->num_simulated_steps;
					
			start_node->v_children[a] = child;
			Q[a]->push(child);
		}
	}	

	m_expanded_nodes = 0;
	m_generated_nodes = 0;
	m_exp_count_novelty1 = 0;
	m_exp_count_novelty2 = 0;
	m_gen_count_novelty1 = 0;
	m_gen_count_novelty2 = 0;
	m_pruned_nodes = 0;
	
	
	while( (num_simulated_steps < max_sim_steps_per_frame) && !queues_empty() ) {
		// Pop a node to expand
		TreeNode* curr_node;
		m_num_iterations++;
		select_queue();
		N[ m_current_queue ]++;

		if ( Q[ m_current_queue ]->empty() )
			continue;

		curr_node = Q[ m_current_queue ]->top();
		Q[ m_current_queue ]->pop();
		V[ m_current_queue ] = std::max( V[ m_current_queue ], (float)curr_node->accumulated_reward );

		if ( curr_node->depth() > m_max_depth ) m_max_depth = curr_node->depth();

		/**
		* check if subtree is bigger than max_budget of nodes
		*/
		if(  curr_node->num_nodes_reusable > max_nodes_per_frame  ) {
			continue;
		}

		if ( curr_node->depth() > m_reward_horizon - 1 ) continue;
		
		if ( curr_node->v_children.empty() )
			num_simulated_steps +=  expand_node( curr_node );
		else {
			curr_node->updateTreeNode();
			for(size_t c = 0; c < curr_node->v_children.size(); c++) {			
				TreeNode* child = curr_node->v_children[c];				
					
				// This recreates the novelty table (which gets resetted every time
				// we change the root of the search tree)
				if( child->is_terminal ){						
					if ( check_novelty_1( child->state.getRAM() ) ){
						update_novelty_table( child->state.getRAM() );
						if(!child->already_expanded){
							child->novelty = 1;
						}
					}
					else{
						if(!child->already_expanded)
							child->novelty = 2;
								
							
					}
				}
					
				child->updateTreeNode();
				child->fn += ( m_max_reward - child->discounted_accumulated_reward ); // Miquel: add this to obtain Hector's BFS + m_max_reward * (720 - child->depth()) ;
					
				//First applicable action
				if(child->depth() == 1)
					child->num_nodes_reusable = child->num_nodes();
				else
					child->num_nodes_reusable = curr_node->num_nodes_reusable;
				if (child->depth() > m_max_depth ) m_max_depth = child->depth();
				
				// Don't expand duplicate nodes, or terminal nodes
				if (!child->is_terminal) {
					if (! (ignore_duplicates && test_duplicate(child)) ){
						Q[ m_current_queue ]->push(child );
					}
					
				}
			}
		}
			
	}
   
	std::cout << "\tExpanded so far: " << m_expanded_nodes << std::endl;	
	std::cout << "\tExpanded Novelty 1: " << m_exp_count_novelty1 << std::endl;	
	std::cout << "\tExpanded Novelty 2: " << m_exp_count_novelty2 << std::endl;	
	std::cout << "\tPruned so far: " << m_pruned_nodes << std::endl;	
	std::cout << "\tGenerated so far: " << m_generated_nodes << std::endl;	
	std::cout << "\tGenerated Novelty 1: " << m_gen_count_novelty1 << std::endl;	
	std::cout << "\tGenerated Novelty 2: " << m_gen_count_novelty2 << std::endl;	
	
	if ( queues_empty() ) std::cout << "Search Space Exhausted!" << std::endl;
	for ( unsigned k = 0; k < Q.size(); k++ ) {
		std::cout << "Size(Q[" << k << "]) = " << Q[k]->size()  << " V[" << k << "]=" << V[k] << " N[" << k << "]=" << N[k] << std::endl;
	}
		
	update_branch_return(start_node);
}
	
