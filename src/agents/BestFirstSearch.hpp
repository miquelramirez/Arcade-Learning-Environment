#ifndef __BEST_FIRST_SEARCH_HPP__
#define __BEST_FIRST_SEARCH_HPP__

#include "IW1Search.hpp"

class BestFirstSearch : public IW1Search {
public:
    BestFirstSearch(RomSettings *, Settings &settings, ActionVect &actions, StellaEnvironment* _env);

    virtual ~BestFirstSearch();


    class TreeNodeComparerExploration
    {
    public:
	
	bool operator()( TreeNode* a, TreeNode* b ) const 
	{
	    if ( a->novelty < b->novelty ) return true;
	    return false;
	}
    };


    class TreeNodeComparerExploitation
    {
    public:
	
	bool operator()( TreeNode* a, TreeNode* b ) const 
	{
	    if ( b->fn < a->fn ) return true;
	    return false;
	}
    };

    virtual int  expand_node( TreeNode* n ); 

protected:	


    virtual void expand_tree(TreeNode* start);

    std::priority_queue<TreeNode*, std::vector<TreeNode*>, TreeNodeComparerExploration > q_exploration;
    std::priority_queue<TreeNode*, std::vector<TreeNode*>, TreeNodeComparerExploitation > q_exploitation;

    reward_t		m_max_reward;
};



#endif // __IW_DIJKSTRA_SEARCH_HPP__
