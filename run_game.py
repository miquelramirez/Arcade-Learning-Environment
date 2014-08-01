#!/usr/bin/python

import os
import sys

def main() :

	if len(sys.argv) < 3 :
		print >> sys.stderr, "Missing parameters!"
		print >> sys.stderr, "Usage: ./run_game.py <internal agent name> <path to ROM>"
		sys.exit(1)

	agent_name = sys.argv[2]
	rom_path = sys.argv[3]

	if not os.path.exists( rom_path ) :
		print >> sys.stderr, "Could not find ROM:", rom_path
		sys.exit(1)


	command = './ale -display_screen true -game_controller internal -player_agent %(agent_name)s %(path_to_rom)'%globals()

	os.system( command)

if __name__ == '__main__' :
	main()
