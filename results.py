#!/usr/bin/env python3
import os
import sys

# Hardcoded input files for now
EMD_FILE   = 'emd_out.txt'
ITERS_FILE = 'iter_out.txt'


#######################
# Runs a system command
def run(cmd):
    print("\nRUNNING $ ", cmd, "\n")
    os.system(cmd)


#######################
# Runs a N experiments for a single config
# Set opt to True to calculate the optimum and its EMD
# Set opt to False if we just want the normal EMD
def results(size, opt, N):
    # First empty the .txt files
    open(EMD_FILE, 'w').close()
    open(ITERS_FILE, 'w').close()

    for i in range(0,N):
        # Run the iterative relaxation algorithm
        # This gives us all the terrain .json files and the iterations .txt
        run("./terr {} {} s 1".format(size, i+1))
        # Calculate the EMD of the above to the EMD .txt
        run("./EMD.py")
        if opt:
            # Calculate the EMD of the optimal solution, to the same EMD .txt
            run("./EMD.py 1")


#######################
# Entry point, more or less
if __name__ == '__main__':
    if len(sys.argv) < 4:
        print("-- Not enough arguments were given, arguments are:")
        print("--   size  Input size of the terrain.")
        print("--   opt   True to calculate the optimum and its EMD.")
        print("--   N     Sample size, i.e. number of random terrains to evaluate.")
    else:
        # Get all arguments
        size = int(sys.argv[1])
        opt = sys.argv[2].lower() in ['y', 'yes', 't', 'true', '1']
        N = int(sys.argv[3])

        results(size, opt, N)
