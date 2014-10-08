#ifndef __IW_SEARCH_HPP__
#define __IW_SEARCH_HPP__

#include "SearchTree.hpp"
#include "bit_matrix.hxx"
#include "../environment/ale_ram.hpp"


class IW1Search : public SearchTree {
    public:
	IW1Search(RomSettings *, Settings &settings, ActionVect &actions, StellaEnvironment* _env);

	virtual ~IW1Search();

	virtual void build(ALEState & state);
		
	virtual void update_tree();


	int expanded() const { return m_expanded_nodes; }
	int generated() const { return m_generated_nodes; }
	int pruned() const { return m_pruned_nodes; }

	virtual	void print_frame_data( int frame_number, float elapsed, Action curr_action, std::ostream& output );
protected:	

	void print_path(TreeNode *start, int a);

	virtual void expand_tree(TreeNode* start);

	void update_branch_return(TreeNode* node);

    	void set_terminal_root(TreeNode* node); 

	void	update_novelty_table( const ALERAM &machine_state );
	bool	check_novelty_1( const ALERAM& machine_state );

	virtual void	clear();
	virtual void	move_to_best_sub_branch();
	
	ALERAM 			m_ram;
	aptk::Bit_Matrix*	m_ram_novelty_table;
	unsigned		m_pruned_nodes;


};



#endif // __IW_SEARCH_HPP__
