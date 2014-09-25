#!/usr/bin/python

import os
import sys
import glob

def main() :

	if len(sys.argv) < 2 :
		print >> sys.stderr, "Missing parameters!"
		print >> sys.stderr, "Usage: ./evaluate_agents.py <path to ROMs>"
		sys.exit(1)

	rom_path = sys.argv[1]

	if not os.path.exists( rom_path ) :
		print >> sys.stderr, "Could not find ROM:", rom_path
		sys.exit(1)

	rom_bin_files = glob.glob( os.path.join( rom_path, '*.bin' ) )
	print >> sys.stdout, 'Found %d ROMs in %s'%(len(rom_bin_files), rom_path )

	games = []
	for filename in rom_bin_files :
		game_name = os.path.split(filename)[-1].replace('.bin','')
		games.append( (game_name, filename) )
		
	num_runs = 10

	agents = [ 	( 'random', '-player_agent random_agent' ),
			( 'brfs', '-player_agent search_agent -search_method brfs' ),
			( 'iw1', '-player_agent search_agent -search_method novelty' ),
			( 'uct', '-player_agent search_agent -search_method uct' )
		 ]

	command_template = './ale -display_screen false -max_sim_steps_per_frame 100000 %(agent_cmd)s %(rom_path)s'

	for game, rom_path in games :
		for agent, agent_cmd in agents :
			folder = 'experiments/%(game)s/%(agent)s'%locals()
			if not os.path.exists( folder ) :
				os.system( 'mkdir -p %(folder)s'%locals() )
			for i in range( 0, num_runs ) :
				res_filename = 	res_filename = os.path.join( folder, 'episode.%d'%(i+1) )
				os.system( command_template%locals() )
				if not os.path.exists( 'episode.1' ) :
					with open( res_filename) as output :
						print >> output, "Agent crashed"
				else :
					os.system( 'mv episode.1 %s'%res_filename )

				if agent == 'random' : continue
				trace_filename = 'episode.%d.trace'%(i+1)
				trace_filename = os.path.join( folder, trace_filename ) 
				os.system( 'mv %(agent)s.search-agent.trace %(trace_filename)s'%locals() )

if __name__ == '__main__' :
	main()
