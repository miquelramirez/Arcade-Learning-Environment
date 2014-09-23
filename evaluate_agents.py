#!/usr/bin/python

import os
import sys

def main() :

	if len(sys.argv) < 3 :
		print >> sys.stderr, "Missing parameters!"
		print >> sys.stderr, "Usage: ./evaluate_agents.py <game> <path to ROM>"
		sys.exit(1)

	game_name = sys.argv[1]
	rom_path = sys.argv[2]

	if not os.path.exists( rom_path ) :
		print >> sys.stderr, "Could not find ROM:", rom_path
		sys.exit(1)

	num_runs = 10


	command_brfs = './ale -display_screen false -max_sim_steps_per_frame 12000 -player_agent search_agent -search_method brfs %(rom_path)s'%locals()
	command_novelty = './ale -display_screen false -max_sim_steps_per_frame 12000 -player_agent search_agent -search_method novelty %(rom_path)s'%locals()
	command_random = './ale -display_screen false -max_sim_steps_per_frame 12000 -player_agent random_agent %(rom_path)s'%locals()

	random_folder = 'experiments/%(game_name)s/random'%locals()
	brfs_folder = 'experiments/%(game_name)s/brfs'%locals()
	novelty_folder = 'experiments/%(game_name)s/novelty'%locals()

	if not os.path.exists( random_folder ) :
		os.system( 'mkdir -p %(random_folder)s'%locals() )
	if not os.path.exists( brfs_folder ) :
		os.system( 'mkdir -p %(brfs_folder)s'%locals() )
	if not os.path.exists( novelty_folder ) :
		os.system( 'mkdir -p %(novelty_folder)s'%locals() )

	# random
	for i in range(0, num_runs ) :
		res_filename = os.path.join( random_folder, 'episode.%d'%(i+1) )
		os.system( command_random )
		if not os.path.exists( 'episode.1' ) :
			with open( res_filename) as output :
				print >> output, "Agent crashed"
		else :
			os.system( 'mv episode.1 %s'%res_filename )

	# brfs
	for i in range(0, num_runs ) :
		res_filename = os.path.join( brfs_folder, 'episode.%d'%(i+1) )
		os.system( command_brfs )
		if not os.path.exists( 'episode.1' ) :
			with open( res_filename ) as output :
				print >> output, "Agent crashed"
		else :
			os.system( 'mv episode.1 %s'%res_filename )
		trace_filename = 'episode.%d.trace'%(i+1)
		trace_filename = os.path.join( brfs_folder, trace_filename ) 
		os.system( 'mv brfs.search-agent.trace %s'%trace_filename )

	# novelty
	for i in range(0, num_runs ) :
		res_filename = os.path.join( novelty_folder, 'episode.%d'%(i+1) )
		os.system( command_brfs )
		if not os.path.exists( 'episode.1' ) :
			with open( res_filename ) as output :
				print >> output, "Agent crashed"
		else :
			os.system( 'mv episode.1 %s'%res_filename )
		trace_filename = 'episode.%d.trace'%(i+1)
		trace_filename = os.path.join( noveltys_folder, trace_filename ) 
		os.system( 'mv novelty.search-agent.trace %s'%trace_filename )

	

if __name__ == '__main__' :
	main()
