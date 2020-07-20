#!/usr/bin/env python3
import json
import matplotlib.pyplot as plt
import os
import sys

# Hardcoded input folder
# Just so it doesn't get cleaned away by make
RESULTS_IN = 'results/'


#######################
# Reads a range of result files
# Based on a maximum terrain size and appended code to read
def getFiles(size, code):
    cSize = 3
    files = []
    while cSize <= size:
        rfs = RESULTS_IN + "results_{0}x{0}_{1}.json".format(cSize, code)
        cSize = (cSize-1)*2+1

        # Skip if result file does not exist
        if os.path.isfile(rfs):
            files.append(rfs)

    return files

# Reads a result file
def readResults(filename):
    f = open(filename)
    results = json.loads(f.read())
    f.close()
    return results


#######################
# Creates a label for a result
def label(result):
    return "{0}x{0}".format(result["size"])

# Calculates the EMD ratios of a result
def calcEMD(result):
    emd = []
    for s in range(0,result["samples"]):
        # Skip if no EMD info was recorded
        if result["emds"][s] != None:
            emd.append(result["emds"][s] / result["emds_opt"][s])

    return emd


#######################
# Draws the plot
# measure should tell us what to visualize
# filenames is a list of lists, each item is a separate data set we want to plot
def plot(measure, filenames):
    # First read everything then call the appropriate plotters
    results = [[readResults(f) for f in files] for files in filenames]

    if measure == "emd":
        # Visualize the EMD ratio
        # But first filter results without EMD info
        # Only take the first data set, it's only calculable for one anyway...
        data = list(filter(lambda r : len(r["emds_opt"]) > 0, results[0]))

        plt.title("p-approximation")
        plt.boxplot(
            [calcEMD(r) for r in data],
            labels=[label(r) for r in data])

    if measure == "iter":
        # Visualize iterations
        # Get the longest data set for the x-axis
        longest = max(results, key=len)
        num = len(results)

        plt.title("# Iterations")
        plt.yscale("log")
        for d in range(0,num):
            plt.boxplot(
                # Make sure to skip any sample that reached maximum iterations
                # We position each plot with 0.8 space inbetween
                [list(filter(lambda x : x != None, r["iterations"])) for r in results[d]],
                positions=[i*num + (d-(num-1)/2)*0.8 for i in range(0,len(results[d]))])

        plt.xticks(
            [i*num for i in range(0,len(longest))],
            [label(r) for r in longest])

    plt.grid(axis='y', linestyle=':')
    plt.show()


#######################
# Entry point, more or less
if __name__ == '__main__':
    if len(sys.argv) < 4:
        print("-- Not enough arguments were given, arguments are:")
        print("--   measure  Measure to visualize (emd, iter).")
        print("--   size     Maximum terrain size to display.")
        print("--   code     Code appended to the results .json file to read.")
        print("--   ...      More codes, treated as separate data sets.")
    else:
        # Get all arguments
        measure = sys.argv[1]
        size = int(sys.argv[2])
        filenames = [getFiles(size, c) for c in sys.argv[3:]]

        plot(measure, filenames)
