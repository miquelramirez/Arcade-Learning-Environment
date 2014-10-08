#!/usr/bin/python
import os
import sys
import csv
import math

class Episode :
	
	def __init__( self, game, algorithm ) :
		self.game = game
		self.algorithm = algorithm
		self.score = None
		self.time = None
		self.crashed = False

	def __lt__( self, other ) :
		if self.score < other.score : return True
		return False

def retrieve_episodes( experiments_folder ) :
	episodes = []
	for root, dirs, files in os.walk('experiments') :
		game = os.path.split( root )[-2]
		algorithm = os.path.split(root)[-1]
		for f in files :
			if "trace" in f : continue
			if "episode" in f :
				full_path = os.path.join( root, f )
				with open( full_path ) as instream :
					for line in instream :
						if "Agent crashed" in line :
							e = Episode( game, algorithm )
							e.crashed = True
							episodes.append( e )
							break
						time = line.strip().split('=')[1].split(',')[0]
						score = line.strip().split('=')[2]
						e = Episode( game, algorithm )
						e.time = float( time )
						e.score = float( score )
						episodes.append(e)
	return episodes

class Algorithm_Performance :

	def __init__( self ) :
		self.average = 0.0
		self.median = 0.0
		self.std_dev = 0.0
		self.episodes = []
		self.crashes = []

	def num_episodes( self ) :
		return len(self.episodes)
	
	def num_crashes( self ) :
		return len(self.crashes)

	def add_episode( self, episode ) :
		if episode.crashed :
			self.crashes.append(episode)
		else :
			self.episodes.append(episode)

	def compute_stats( self ) :
		if len( self.episodes ) == 0 : # no stats available
			self.average = 'n/a'
			self.median = 'n/a'
			self.std_dev = 'n/a'
			return
		# 1. Sort non-crashing episodes
		self.episodes.sort()
		# 2. compute average
		self.average = 0.0
		for e in self.episodes :
			self.average += e.score
		self.average /= float(self.num_episodes())
		# 3. compute median
		self.median = self.episodes[ self.num_episodes()/2 ].score
		# 4. compute standard deviation
		if len( self.episodes ) == 1 :
			self.std_dev = 'n/a'
			return
		self.std_dev = 0.0
		for e in self.episodes :
			# a. substract average
			s = e.score - self.average
			# b. square
			s *= s
			# c. sum
			self.std_dev += s
		# d. divide by n-1 to obtain the sample standard deviation
		self.std_dev /= float(self.num_episodes()-1)
		self.std_dev = math.sqrt( self.std_dev )
		

if __name__ == '__main__':

	episodes = retrieve_episodes('experiments')
	print >> sys.stdout, len(episodes), 'episodes retrieved'
	# the experimental data set is represented as a nested dictionary
	# { game, { algorithm, instance of Algorithm Performance } }
	games = {}
	
	for e in episodes :
		algorithms = None
		try :
			algorithms = games[e.game]
		except KeyError :
			games[e.game] = {}
			algorithms = games[e.game]
		perf = None
		try :
			perf = algorithms[e.algorithm]
		except KeyError :
			algorithms[e.algorithm] = Algorithm_Performance()
			perf = algorithms[e.algorithm]
		perf.add_episode( e )

	# now we compute the statistics
	for _, algs in games.iteritems() :
		for _, perf in algs.iteritems() :
			perf.compute_stats()

	algorithms = [ 'random', 'brfs', 'iw1', 'uct' ]
	# and finally, we write the table summarizing the results
	with open( 'results.csv', 'w' ) as outstream :
		writer = csv.writer( outstream, delimiter = ',' )
		header = [ 'Game' ]
		for alg_name in algorithms :
			header += [ alg_name ] + [ '' ] * 4
		writer.writerow( header )
		header = [ '' ]
		for alg_name in algorithms :
			header += [ 'runs', 'crashes', 'avg', 'median', 'std. dev.' ]
		writer.writerow( header )

		for game_name, algs in games.iteritems() :
			row = [ game_name ]
			for alg_name in algorithms :
				try :
					perf = algs[ alg_name ]
					row += [ str( perf.num_episodes() ) ]
					row += [ str( perf.num_crashes() ) ]
					row += [ str( perf.average ) ]
					row += [ str( perf.median ) ]
					row += [ str( perf.std_dev ) ]
				except KeyError :
					row += [ 'n/a' ] * 5
			writer.writerow( row )


