/* *****************************************************************************
 * A.L.E (Arcade Learning Environment)
 * Copyright (c) 2009-2012 by Yavar Naddaf, Joel Veness, Marc G. Bellemare
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  FullSearchTree.cpp
 *
 *  Implementation of the FullSearchTree class, which represents search a
 *  Full-Search-Tree (i.e. a search-tree that expands all its nodes until
 *  the given depth), used by the Search Agent
 **************************************************************************** */

#include "FullSearchTree.hpp"
#include "SearchAgent.hpp"
#include <queue>

/* *********************************************************************
   Constructor
   Generates a whole search tree from the current state, until 
   max_frame_num is reached.
   ******************************************************************* */
FullSearchTree::FullSearchTree(RomSettings *rom_settings, Settings &settings,
			       ActionVect &actions, StellaEnvironment* _env) :
    SearchTree(rom_settings, settings, actions, _env) {
	
}

/* *********************************************************************
   Destructor
   ******************************************************************* */
FullSearchTree::~FullSearchTree() {

}

/* *********************************************************************
   Builds a new tree
   ******************************************************************* */
void FullSearchTree::build(ALEState & state) {	
    assert(p_root == NULL);
    p_root = new TreeNode(NULL, state, NULL, UNDEFINED, 0);
    update_tree();
    is_built = true;	
    std::cout << "Expanded: "<< m_expanded_nodes  << " Generated: "<< m_generated_nodes;
			
}

void FullSearchTree::print_path(TreeNode * node, int a) {
    cerr << "Path, return " << node->v_children[a]->branch_return << endl;

    while (!node->is_leaf()) {
	TreeNode * child = node->v_children[a];

	cerr << "\tAction " << a << " Reward " << child->node_reward << 
	    " Return " << child->branch_return << 
	    " Terminal " << child->is_terminal << endl;
    
	node = child;
	if (!node->is_leaf())
	    a = node->best_branch;
    }
}

/* *********************************************************************
   Re-Expands the tree until the new max_frame_num is reached on all 
   leaf nodes
   ******************************************************************* */
void FullSearchTree::update_tree() {
    expand_tree(p_root);
}

/* *********************************************************************
   Expands the tree from the given node until i_max_sim_steps_per_frame
   is reached
	
   ******************************************************************* */
void FullSearchTree::expand_tree(TreeNode* start_node) {
    // If the root is terminal, we will not expand any of its children; deal with this
    //  appropriately
    if (start_node->is_terminal) {
	set_terminal_root(start_node);
	return;
    }

    queue<TreeNode*> q;
    q.push(start_node);

    m_ram.reset();
    m_ram.or_op(start_node->state.getRAM());


    int num_simulated_steps = 0;
    int num_actions = available_actions.size();

    m_expanded_nodes = 0;
    m_generated_nodes = 0;
    int m_new_RAM = 0;
    while(!q.empty()) {
	// Pop a node to expand
	TreeNode* curr_node = q.front();
	q.pop();

       
	bool leaf_node = (curr_node->v_children.empty());
	m_expanded_nodes++;
	// Expand all of its children (simulates the result)
	for (int a = 0; a < num_actions; a++) {
	    Action act = available_actions[a];

	    TreeNode * child;

	    // If re-expanding an internal node, don't creates new nodes
	    if (leaf_node) {
		child = new TreeNode(curr_node,	
				     curr_node->state,
				     this,
				     act,
				     sim_steps_per_node); 

		
		if( m_ram.new_bit( child->state.getRAM()) )
			{
				m_new_RAM++;
				m_ram.or_op(child->state.getRAM());
				std::cout << "screen size: "<<child->state.getScreen().arraySize() << std::endl;
				
				
			}
		else{
			//delete child;
			curr_node->v_children.push_back(child);
			child->is_terminal = true;
			m_generated_nodes++;
			continue;
				
		}


		num_simulated_steps += child->num_simulated_steps;

		curr_node->v_children.push_back(child);
		m_generated_nodes++;
	    }
	    else
		child = curr_node->v_children[a];

	    // Don't expand duplicate nodes, or terminal nodes
	    if (!child->is_terminal) {
		if (! (ignore_duplicates && test_duplicate(child)) )
		    q.push(child);
	    }
	}
	
	// Stop once we have simulated a maximum number of steps
	if (num_simulated_steps >= max_sim_steps_per_frame) {
	    break;
	}
		
    }
    
    if (q.empty()) std::cout << "Search Space Exhausted!" << std::endl;
    
    std::cout << "new_Rams "<<m_new_RAM << " out of " << m_generated_nodes << std::endl;
    update_branch_return(start_node);
}



/* *********************************************************************
   Updates the branch reward for the given node
   which equals to: node_reward + max(children.branch_return)
   ******************************************************************* */
void FullSearchTree::update_branch_return(TreeNode* node) {
    // Base case (leaf node): the return is the immediate reward
    if (node->v_children.empty()) {
	node->branch_return = node->node_reward;
	node->best_branch = -1;
	return;
    }

    // First, we have to make sure that all the children are updated
    for (unsigned int c = 0; c < node->v_children.size(); c++) {
	TreeNode* curr_child = node->v_children[c];
		
	if (ignore_duplicates && curr_child->is_duplicate()) continue;
    
	update_branch_return(curr_child);
    }
	
    // Now that all the children are updated, we can update the branch-reward
    float best_return = -1;
    int best_branch = -1;
    return_t avg = 0;

    // Terminal nodes encur no reward beyond immediate
    if (node->is_terminal) {
	best_return = node->node_reward;
	best_branch = 0;
    } else {
	    
	for (size_t a = 0; a < node->v_children.size(); a++) {
	    return_t child_return = node->v_children[a]->branch_return;
	    if (best_branch == -1 || child_return > best_return) {
		best_return = child_return;
		best_branch = a;
		avg+=child_return;
	    }
	    if( node->v_children.size() ) avg/=node->v_children.size();
	}
    }

    node->branch_return = node->node_reward + best_return * discount_factor; 
	//node->branch_return = node->node_reward + avg * discount_factor; 
	
    node->best_branch = best_branch;
}

void FullSearchTree::set_terminal_root(TreeNode * node) {
    node->branch_return = node->node_reward; 

    if (node->v_children.empty()) {
	// Expand one child; add it to the node's children
	TreeNode* new_child = new TreeNode(node,node->state, 
					   this, PLAYER_A_NOOP, sim_steps_per_node);

	node->v_children.push_back(new_child);
    }
  
    // Now we have at least one child, set the 'best branch' to the first action
    node->best_branch = 0; 
}
