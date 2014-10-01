#!/usr/bin/python
import os
import sys, tarfile, glob, time

def create_csv():
	folders = []

	csv_file = open( 'results.csv', 'w' )
	for root, dirs, files in os.walk('experiments'):

            for f in files:
                if f.find(".trace") != -1: continue;
                if f.find("episode") > -1:
                    fullpath = os.path.join(root, f)
                    res = open(  fullpath )
                    for line in res:

                        if line.find("Agent crashed") == -1:
                            game = root.split("/")[-2]
                            alg = root.split("/")[-1] 
                            time = line.strip().split("=")[1].split(",")[0]
                            score = line.strip().split("=")[2]              
                            csv_file.write("%s,%s,%s,%s"%(game,alg,time,score ))
                        else:
                            game = root.split("/")[-2]
                            alg = root.split("/")[-1] 
                            csv_file.write("%s,%s,-1"%(game,alg))
                        csv_file.write("\n");
        csv_file.close()

if __name__ == '__main__':

	create_csv()
