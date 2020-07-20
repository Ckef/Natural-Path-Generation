#!/usr/bin/env python3
import json
import matplotlib.pyplot as plt
import os
import sys

# Hardcoded input folder
# Just so it doesn't get cleaned away by make
RESULTS_IN = 'results/'


#######################
# Reads a result file
def readResults(filename):
    f = open(filename)
    results = json.loads(f.read())
    f.close()
    return results

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
def plot(measure, filenames):
    # First read everything then call the appropriate plotters
    results = [readResults(f) for f in filenames]

    if measure == "emd":
        # Visualize the EMD ratio
        # But first filter results without EMD info
        results = list(filter(lambda r : len(r["emds_opt"]) > 0, results))
        plt.title("p-approximation")
        plt.boxplot(
            [calcEMD(r) for r in results],
            labels=["{0}x{0}".format(r["size"]) for r in results])

    if measure == "iter":
        # Visualize iterations
        plt.title("# Iterations")
        plt.boxplot(
            [r["iterations"] for r in results],
            labels=["{0}x{0}".format(r["size"]) for r in results])
        plt.yscale("log")

    plt.show()


#######################
# Entry point, more or less
if __name__ == '__main__':
    if len(sys.argv) < 4:
        print("-- Not enough arguments were given, arguments are:")
        print("--   measure  Measure to visualize (emd, iter).")
        print("--   size     Maximum terrain size to display.")
        print("--   code     Code appended to the results .json file to read.")
    else:
        # Get all arguments
        measure = sys.argv[1]
        size = int(sys.argv[2])
        code = sys.argv[3]

        # Get all results to plot
        cSize = 3
        files = []
        while cSize <= size:
            rfs = RESULTS_IN + "results_{0}x{0}_{1}.json".format(cSize, code)
            cSize = (cSize-1)*2+1

            # Skip if result file does not exist
            if os.path.isfile(rfs):
                files.append(rfs)

        plot(sys.argv[1], files)
