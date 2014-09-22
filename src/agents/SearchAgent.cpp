/* *****************************************************************************
 * A.L.E (Arcade Learning Environment)
 * Copyright (c) 2009-2012 by Yavar Naddaf, Joel Veness, Marc G. Bellemare
 * Released under GNU General Public License www.gnu.org/licenses/gpl-3.0.txt
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  SearchAgent.cpp
 *
 * The implementation of the SearchAgent class, which uses Search Algorithms
 * to act in the game
 **************************************************************************** */

#include "SearchAgent.hpp"
//#include "game_controller.h"
#include "random_tools.h"
#include "Serializer.hxx"
#include "Deserializer.hxx"
#include "System.hxx"
#include <sstream>

#include "FullSearchTree.hpp"
//#include "UCTSearchTree.hpp"

SearchAgent::SearchAgent(OSystem* _osystem, RomSettings* _settings, StellaEnvironment* _env) : 
    PlayerAgent(_osystem, _settings),
  m_curr_action(UNDEFINED)
{
    search_method = p_osystem->settings().getString("search_method", true); 
    
    // Depending on the configuration, create a SearchTree of the requested type
    if (search_method == "fulltree") {
	search_tree = new FullSearchTree(_settings, _osystem->settings(),
					 available_actions, _env);
    // } else if (search_method == "uct") {
    // 	search_tree = new UCTSearchTree(_settings, _osystem->settings(),
    // 					available_actions);
    } else {
	cerr << "Unknown search Method: " << search_method << endl;
		exit(-1);
    }
    m_rom_settings = _settings;
    m_env = _env;
    
    Settings &settings = _osystem->settings();
    sim_steps_per_node = settings.getInt("sim_steps_per_node", true);
}

SearchAgent::~SearchAgent() {
}

int SearchAgent::num_available_actions() {
  return available_actions.size();
}

ActionVect& SearchAgent::get_available_actions() {
  return available_actions;
}

Action SearchAgent::agent_step() {
  Action act = PlayerAgent::agent_step();

  state.incrementFrame();

  return act;
}

/* *********************************************************************
    Returns a random action from the set of possible actions
 ******************************************************************** */
Action SearchAgent::act() {
	// Generate a new action every sim_steps_per node; otherwise return the
	//  current selected action 
		
	if (frame_number % sim_steps_per_node != 0)
		return m_curr_action;
	
	std::cout << "Search Agent action selection: frame=" << frame_number << std::endl;
	std::cout << "Evaluating actions: " << std::endl;

	state = m_env->cloneState();

	if (search_tree->is_built) {
		// Re-use the old tree
		search_tree->move_to_best_sub_branch();
   
		assert(search_tree->get_root()->state.equals(state));
		assert (search_tree->get_root_frame_number() == state.getFrameNumber());
		search_tree->update_tree();
	} else {
		// Build a new Search-Tree
		search_tree->clear(); 
		search_tree->build(state);
	}

	m_curr_action = search_tree->get_best_action();
	std::cout << "Best Action: " << action_to_string( m_curr_action );	
	std::cout << " branch_reward: " << search_tree->get_root_value() << std::endl;

	m_env->restoreState( state );
	
	return m_curr_action;
}



/* *********************************************************************
    This method is called when the game ends. 
 ******************************************************************** */
void SearchAgent::episode_end(void) {
  PlayerAgent::episode_end();
	// Our search-tree is useless now. Clear it
	search_tree->clear();
}

Action SearchAgent::episode_start(void) {
  Action a = PlayerAgent::episode_start();

  state.incrementFrame();

  return a;
}

