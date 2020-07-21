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
        if result["emds_opt"][s] != None:
            emd.append(result["emds"][s] / result["emds_opt"][s])

    return emd

# Calculates the percentage of unsatisfied constraints of a result
def calcUnsatisfied(result):
    return [
        (stats["u_s"] + stats["u_d"] + stats["u_r"] + stats["u_p"])/
        (stats["n_s"] + stats["n_d"] + stats["n_r"] + stats["n_p"])
        for stats in result["stats_H"]]


#######################
# Draws the plot
# measure should tell us what to visualize
# filenames is a list of lists, each item is a separate data set we want to plot
# Colors are per dataset
def plot(measure, filenames, colors):
    # First read everything
    results = [[readResult(f) for f in files] for files in filenames]

    # List of data sets, each data set containing a set of result files
    # This gets filled below...
    data = [[]]

    #######################
    # Fill in data for the EMD ratio
    if measure == "emd":
        plt.title("p-approximation")

        # Some preliminary filtering of the results
        # Make sure to skip any results without EMD info
        # We can do this cause there are no gaps inbetween
        # Also make sure to set results, so the x axis gets labeled correctly
        results = [
            list(filter(lambda r : len(r["emds_opt"]) > 0, d))
            for d in results]

        data = [[calcEMD(r) for r in d] for d in results]
        # Actual EMD, instead of ratio
        #data = [
        #    [list(filter(None, r["emds"])) for r in d]
        #    for d in results]

    # Fill in data for iterations
    if measure == "iter":
        plt.title("# iterations")
        plt.yscale("log")
        data = [
            # Make sure to skip any sample that reached maximum iterations
            [list(filter(None, r["iterations"])) for r in d]
            for d in results]

    # Fill in data for percentage of unsatisfied constraints
    if measure == "unsat":
        plt.title("% unsatisfied constraints")
        data = [[calcUnsatisfied(r) for r in d] for d in results]

    #######################
    # A plot for each data set
    num = len(data)
    for d in range(0,num):
        # Silent warning that there is no data
        # We only do it here so there is a visual cue of something missing :)
        if len(data[d]) == 0:
            print("-- Data set #{} has no appropriate data.".format(d+1))
            continue

        # We position each plot with 0.8 space inbetween
        pos = [i*num + (d-(num-1)/2)*0.8 for i in range(0,len(data[d]))]

        if measure != "unsat":
            # Boxplots
            plt.boxplot(
                data[d], positions=pos, showfliers=False, zorder=1,
                medianprops={"linewidth":2.5, "color":colors[d]},
                boxprops={"alpha":0.5},
                whiskerprops={"linestyle":"--", "alpha":0.5})

        # The scatter plot overlays
        for i in range(0,len(data[d])):
            if measure == "unsat":
                plt.scatter([pos[i]]*len(data[d][i]), data[d][i], 20, colors[d])
            else:
                x = np.random.normal(pos[i], 0.04, size=len(data[d][i]))
                plt.scatter(x, data[d][i], 10, colors[d], alpha=0.7, zorder=2)

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
        print("--   measure  Measure to visualize (emd, iter, unsat, dist).")
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
