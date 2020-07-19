#!/usr/bin/env python3
import json
import os
import sys

# Hardcoded input files for now
EMD_FILE     = 'emd_out.txt'
ITERS_FILE   = 'iter_out.txt'
L_STATS_FILE = 'stats_out_l.txt'
H_STATS_FILE = 'stats_out_h.txt'


#######################
# Runs a system command
def run(cmd):
    print("\nRUNNING $", cmd, "\n")
    os.system(cmd)

# Reads all lines from a file
def readLines(filename):
    f = open(filename)
    lines = f.readlines()
    f.close()
    return lines


#######################
# Runs a N experiments for a single config
# Set emd to True to calculate the EMD
# If emd is False, opt will be False as well
# Set opt to True to calculate the optimum and its EMD
# Set opt to False if we just want the normal EMD
def results(size, emd, opt, Ns, filecode):
    # First empty the .txt files
    open(ITERS_FILE, 'w').close()
    open(L_STATS_FILE, 'w').close()
    open(H_STATS_FILE, 'w').close()
    if emd:
        open(EMD_FILE, 'w').close()

    # Now run all Ns samples
    # Giving a new seed to the terrain generator every time
    for i in range(0,Ns):
        # Run the iterative relaxation algorithm
        # This gives us all the terrain .json files and the iterations and stats .txt
        run("./terr {} {} s 1".format(size, i+1))
        if emd:
            # Calculate the EMD of the above to the EMD .txt
            run("./EMD.py")
            if opt:
                # Calculate the EMD of the optimal solution, to the same EMD .txt
                run("./EMD.py 1")

    # Now we want to summarize all our results in a nice results file
    # Create a JSON object that summarizes all results
    iters = readLines(ITERS_FILE)
    statsL = readLines(L_STATS_FILE)
    statsH = readLines(H_STATS_FILE)

    results = {
        "samples" : Ns,
        "emd" : [],
        "emd_opt" : [],
        "iterations" : [int(i) for i in iters],
        "stats_L" : [json.loads(s) for s in statsL],
        "stats_H" : [json.loads(s) for s in statsH]
    }

    if emd:
        # Some extra formatting for EMDs
        # Splits up the list into two lists, depending on the input arguments
        emds = readLines(EMD_FILE)
        toFloat = lambda x : None if x == "-\n" else float(x)
        if opt:
            results["emd"] = [toFloat(emds[i*2]) for i in range(0,Ns)]
            results["emd_opt"] = [toFloat(emds[i*2+1]) for i in range(0,Ns)]
        else:
            results["emd"] = [toFloat(emds[i]) for i in range(0,Ns)]

    # Construct an output file and write the results
    rfs = "results_{0}x{0}_{1}.json".format(size, filecode)
    rf = open(rfs, 'w')
    rf.write(json.dumps(results, indent=2))
    rf.close()

    print("\nFINISHED -- Results have been written to file:", rfs)


#######################
# Entry point, more or less
if __name__ == '__main__':
    if len(sys.argv) < 6:
        print("-- Not enough arguments were given, arguments are:")
        print("--   size  Input size of the terrain.")
        print("--   emd   True to calculate the EMD.")
        print("--   opt   True to calculate the optimum and its EMD (False if emd is False).")
        print("--   Ns    Sample size, i.e. number of random terrains to evaluate.")
        print("--   code  Code to append to the output results.json file.")
    else:
        bStrs = ['y', 'yes', 't', 'true', 'on', '1']

        # Get all arguments
        size = int(sys.argv[1])
        emd = sys.argv[2].lower() in bStrs
        opt = sys.argv[3].lower() in bStrs
        Ns = int(sys.argv[4])
        code = sys.argv[5]

        results(size, emd, opt, Ns, code)
