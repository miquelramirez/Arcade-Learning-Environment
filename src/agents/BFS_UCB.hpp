#ifndef __BFS_UCB_HPP__
#define __BFS_UCB_HPP__

#include "IW1Search.hpp"

class BFS_UCB : public IW1Search {
public:
    
	BFS_UCB(RomSettings *, Settings &settings, ActionVect &actions, StellaEnvironment* _env);

	virtual ~BFS_UCB();


	class TreeNodeComparerExploitation
	{
	public:
		
		bool operator()( TreeNode* a, TreeNode* b ) const 
		{
			if ( b->fn < a->fn ) return true;
			//else if( b->fn == a->fn && b->m_depth < a->m_depth ) return true;
			else if( b->fn == a->fn && b->novelty < a->novelty ) return true;
			else if( b->novelty == a->novelty && b->m_depth < a->m_depth ) return true;
			return false;
		}
	};

	typedef 	std::priority_queue<TreeNode*, std::vector<TreeNode*>, TreeNodeComparerExploitation >
			Queue;

	typedef		std::vector< Queue* >
			Queue_Vector;

	virtual int  	expand_node( TreeNode* n ); 

	void 		clear_queues()	{
			for ( Queue* q : Q ) {
				delete q;
				q = new Queue;
			}
	}

	float		exploration_bonus( unsigned k ) {
		return sqrt( ( 2.0f * logf((float)m_num_iterations) ) / (float)N[k] );
	}

	void		select_queue() {
		m_current_queue = Q.size();
		float best_value = -1.0f;

		for ( unsigned k = 0; k < V.size(); k++ ) {
			if ( Q[k]->empty() ) continue;
			float f_k = V[k] + exploration_bonus(k);
			if ( f_k > best_value ) {
				m_current_queue = k;
				best_value = f_k;
			}
		}
	}

	bool	queues_empty() {
		for ( Queue* q : Q )
			if ( !q->empty() ) return false;
		return true;
	}

protected:	

	void 			reset_branch(TreeNode* node);
	unsigned 		size_branch(TreeNode* node);
	
	virtual void 		expand_tree(TreeNode* start);

	Queue_Vector		Q;
	std::vector<float>	V;
	std::vector<unsigned>	N;
	unsigned		m_num_iterations;
	unsigned		m_current_queue;

	reward_t		m_max_reward;
	unsigned 		m_gen_count_novelty2;
	unsigned 		m_gen_count_novelty1;
	unsigned 		m_exp_count_novelty2;
	unsigned 		m_exp_count_novelty1;
};



#endif // __BFS_UCB__
