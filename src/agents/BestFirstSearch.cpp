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
						sim_steps_per_node); 
	
			if ( check_novelty_1( child->state.getRAM() ) ) {
			    update_novelty_table( child->state.getRAM() );
			    curr_node->novelty = 1;
			}
			else{
			    curr_node->novelty = 2;
			}
			child->fn += ( m_max_reward - child->accumulated_reward ); // Miquel: add this to obtain Hector's BFS + m_max_reward * (720 - child->depth()) ;

			if (child->depth() > m_max_depth ) m_max_depth = child->depth();
			num_simulated_steps += child->num_simulated_steps;
					
			curr_node->v_children.push_back(child);
		}
		else {
			child = curr_node->v_children[a];
			if ( !child->is_terminal )
				num_simulated_steps += child->num_simulated_steps;

			// This recreates the novelty table (which gets resetted every time
			// we change the root of the search tree)
			if ( m_novelty_pruning )
				update_novelty_table( child->state.getRAM() );
	
		}
	
		// Don't expand duplicate nodes, or terminal nodes
		if (!child->is_terminal) {
		    if (! (ignore_duplicates && test_duplicate(child)) ){
				q_exploration.push(child);
				q_exploitation.push(child);
		    }
		}
	}
	return num_simulated_steps;
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
    
    q_exploration.push(start_node);
    
    bool explore = true;
    
    update_novelty_table( start_node->state.getRAM() );
    int num_simulated_steps = 0;

    m_expanded_nodes = 0;
    m_generated_nodes = 0;

    m_pruned_nodes = 0;

    while( !q_exploration.empty() || !q_exploitation.empty() ) {
	// Pop a node to expand
	TreeNode* curr_node;
	if(explore){	    
	    curr_node = q_exploration.top();
	    q_exploration.pop();
	}
	else{
	    curr_node = q_exploitation.top();
	    q_exploitation.pop();
	
	}
	if ( curr_node->depth() > m_reward_horizon - 1 ) continue;
	if ( m_stop_on_first_reward && curr_node->node_reward != 0 ) 
	    {
		pivots.push_back( curr_node );
		continue;
	    }
	steps = expand_node( curr_node );	
	num_simulated_steps += steps;
	// Stop once we have simulated a maximum number of steps
	if (num_simulated_steps >= max_sim_steps_per_frame) {
	    break;
	}
		
    }
   
    std::cout << "\tExpanded so far: " << m_expanded_nodes << std::endl;	
    std::cout << "\tPruned so far: " << m_pruned_nodes << std::endl;	
    std::cout << "\tGenerated so far: " << m_generated_nodes << std::endl;	

    if (q.empty()) std::cout << "Search Space Exhausted!" << std::endl;
    

	
    update_branch_return(start_node);
}

