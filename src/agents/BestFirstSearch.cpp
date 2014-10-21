#include "BestFirstSearch.hpp"
#include "SearchAgent.hpp"
#include <list>

BestFirstSearch::BestFirstSearch(RomSettings *rom_settings, Settings &settings,
			       ActionVect &actions, StellaEnvironment* _env) 
    : IW1Search( rom_settings, settings, actions, _env){
 
	m_max_reward = settings.getInt( "max_reward" );
}

BestFirstSearch::~BestFirstSearch() {

}

class TreeNodeComparer
{
public:

	bool operator()( TreeNode* a, TreeNode* b ) const 
	{
		if ( b->fn < a->fn ) return true;
		return false;
	}
};

int BestFirstSearch::expand_node( TreeNode* curr_node )
{
	int num_simulated_steps =0;
	int num_actions = available_actions.size();
	bool leaf_node = (curr_node->v_children.empty());
	m_expanded_nodes++;
	// Expand all of its children (simulates the result)
	std::random_shuffle ( available_actions.begin(), available_actions.end() );
	if(leaf_node){
		curr_node->v_children.resize( num_actions );
		curr_node->available_actions = available_actions;
	}
	for (int a = 0; a < num_actions; a++) {
		Action act = available_actions[a];
		
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
			}
			else{
			    child->novelty = 2;
			}
			child->fn += ( m_max_reward - child->discounted_accumulated_reward ); // Miquel: add this to obtain Hector's BFS + m_max_reward * (720 - child->depth()) ;

			if (child->depth() > m_max_depth ) m_max_depth = child->depth();
			num_simulated_steps += child->num_simulated_steps;
					
			curr_node->v_children[a] = child;

		}
		else {
			
			child = curr_node->v_children[a];
		
			// This recreates the novelty table (which gets resetted every time
			// we change the root of the search tree)
			if ( m_novelty_pruning )
				if ( check_novelty_1( child->state.getRAM() ) ){
					update_novelty_table( child->state.getRAM() );
					child->novelty = 1;
				}
				else{
					child->novelty = 2;
					
				}
			
			child->updateTreeNode();
			child->fn += ( m_max_reward - child->discounted_accumulated_reward ); // Miquel: add this to obtain Hector's BFS + m_max_reward * (720 - child->depth()) ;

			if (child->depth() > m_max_depth ) m_max_depth = child->depth();

			
			num_simulated_steps += child->num_simulated_steps;
		}
	
		// Don't expand duplicate nodes, or terminal nodes
		if (!child->is_terminal) {
		    if (! (ignore_duplicates && test_duplicate(child)) ){
				q_exploration.push(child);
				q_exploitation.push(child);
		    }
		}
	}

	curr_node->already_expanded = true;
	return num_simulated_steps;
}

/* *********************************************************************
	update novelty_value to 0 to a node and all its children, all the way down the branch
 ******************************************************************* */
void BestFirstSearch::reset_branch(TreeNode* node) {
	if (!node->v_children.empty()) {
		for(size_t c = 0; c < node->v_children.size(); c++) {			
			reset_branch(node->v_children[c]);
			
		}
	}
	node->novelty = 0;
	node->fn = 0;	
	node->already_expanded = false;
}

unsigned BestFirstSearch::size_branch(TreeNode* node) {
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

void BestFirstSearch::expand_tree(TreeNode* start_node) {
    // If the root is terminal, we will not expand any of its children; deal with this
    //  appropriately
    if (start_node->is_terminal) {
	set_terminal_root(start_node);
	return;
    }
    
    if(!start_node->v_children.empty()) {
	    start_node->updateTreeNode();
	    reset_branch( start_node );
    }

    while(!q_exploration.empty() ) {	
	    q_exploration.pop();
    }
    while(!q_exploitation.empty() ) {	
	    q_exploitation.pop();
    }

    q_exploration.push(start_node);    
    
    bool explore = true;
    
    update_novelty_table( start_node->state.getRAM() );
    int num_simulated_steps = 0;

    m_expanded_nodes = 0;
    m_generated_nodes = 0;

    m_pruned_nodes = 0;
    

    while( ! (q_exploration.empty() && q_exploitation.empty()) ) {
	// Pop a node to expand
	TreeNode* curr_node;
	if(explore){	    
	    curr_node = q_exploration.top();
	    q_exploration.pop();
	    explore = false;

	}
	else{
		if(q_exploitation.top()->fn ==  m_max_reward){
			explore = true;
			continue;
		}

	
	    curr_node = q_exploitation.top();
	    q_exploitation.pop();
	    explore = true;
	}

	/**
	 * check nodes that have been expanded by other queue
	 */
	if(  curr_node->already_expanded ) 
		continue;
	
		

	//if(curr_node->novelty != 1)
	//	std::cout << curr_node->depth() << " " << curr_node->novelty << " " << curr_node->fn << " " << std::endl;

	if ( curr_node->depth() > m_reward_horizon - 1 ) continue;


	num_simulated_steps +=  expand_node( curr_node );
	// std::cout << "q_exploration size: "<< q_exploration.size() << std::endl;
	// std::cout << "q_exploitation size: "<< q_exploitation.size() << std::endl;
	// Stop once we have simulated a maximum number of steps
	if (num_simulated_steps >= max_sim_steps_per_frame) {
	    break;
	}
		
    }
   
    std::cout << "\tExpanded so far: " << m_expanded_nodes << std::endl;	
    std::cout << "\tPruned so far: " << m_pruned_nodes << std::endl;	
    std::cout << "\tGenerated so far: " << m_generated_nodes << std::endl;	

    if ( q_exploration.empty() && q_exploitation.empty() ) std::cout << "Search Space Exhausted!" << std::endl;
    std::cout << "q_exploration size: "<< q_exploration.size() << std::endl;
    std::cout << "q_exploitation size: "<< q_exploitation.size() << std::endl;

	
    update_branch_return(start_node);
}

