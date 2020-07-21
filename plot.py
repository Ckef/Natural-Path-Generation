#!/usr/bin/env python3
import json
import matplotlib.pyplot as plt
import numpy as np
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
def readResult(filename):
    f = open(filename)
    result = json.loads(f.read())
    f.close()
    return result


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
# Helper plotter to plot a scatter overlay
def plotIndividuals(data, positions, color):
    for i in range(0,len(data)):
        x = np.random.normal(positions[i], 0.04, size=len(data[i]))
        plt.scatter(x, data[i], 10, color, zorder=3, alpha=0.8)

# Draws the plot
# measure should tell us what to visualize
# filenames is a list of lists, each item is a separate data set we want to plot
# Colors are per dataset
def plot(measure, filenames, colors):
    # First read everything then call the appropriate plotters
    results = [[readResult(f) for f in files] for files in filenames]

    if measure == "emd":
        # Visualize the EMD ratio
        # But first filter results without EMD info
        # Only take the first data set, it's only calculable for one anyway...
        data = list(filter(lambda r : len(r["emds_opt"]) > 0, results[0]))
        emd = [calcEMD(r) for r in data]
        #emd = [list(filter(None, r["emds"])) for r in data], # Actual EMD, instead of ratio
        plt.title("p-approximation")

        plotIndividuals(emd, list(range(1,len(emd)+1)), colors[0])
        plt.boxplot(
            emd, showfliers=False,
            labels=[label(r) for r in data],
            medianprops={"linewidth":2.5, "color":colors[0]},
            boxprops={"alpha":0.5},
            whiskerprops={"linestyle":"--", "alpha":0.5})

    if measure == "iter":
        # Visualize iterations
        plt.title("# Iterations")
        plt.yscale("log")

        # For each data set
        num = len(results)
        for d in range(0,num):
            # Make sure to skip any sample that reached maximum iterations
            # We position each plot with 0.8 space inbetween
            data = [list(filter(None, r["iterations"])) for r in results[d]]
            pos = [i*num + (d-(num-1)/2)*0.8 for i in range(0,len(results[d]))]

            plotIndividuals(data, pos, colors[d])
            plt.boxplot(
                data, positions=pos, showfliers=False,
                medianprops={"linewidth":2.5, "color":colors[d]},
                boxprops={"alpha":0.5},
                whiskerprops={"linestyle":"--", "alpha":0.5})

        # Get the longest data set for the x-axis
        longest = max(results, key=len)
        plt.xticks(
            [i*num for i in range(0,len(longest))],
            [label(r) for r in longest])

    # And get us a nice plot
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
        # A color dictionary...
        codecolor = {
            "deriv" : "gold",
            "rough" : "limegreen",
            "deriv_border" : "orange",
            "rough_border" : "darkgreen"
        }

        # Get all arguments
        measure = sys.argv[1]
        size = int(sys.argv[2])
        filenames = [getFiles(size, c) for c in sys.argv[3:]]
        colors = [codecolor.get(c, "black") for c in sys.argv[3:]]

        plot(measure, filenames, colors)
