#!/usr/bin/env python3
import json
import matplotlib.pyplot as plt
import sys


#######################
# Reads a result file
def readResults(filename):
    f = open(filename)
    results = json.loads(f.read())
    f.close()
    return results


#######################
# Draws the plot
# measure should tell us what to visualize
def plot(measure, filenames):
    # First read everything then call the appropriate plotters
    results = [readResults(f) for f in filenames]

    if measure == "emd":
        # Visualize the EMD ratio
        return

    if measure == "iter":
        # Visualize iterations
        plt.boxplot(
            [r["iterations"] for r in results],
            labels=["{0}x{0}".format(r["size"]) for r in results])
        plt.yscale("log")

    plt.show()


#######################
# Entry point, more or less
if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("-- Not enough arguments were given, arguments are:")
        print("--   mes    Measure to visualize (emd, iter)")
        print("--   files  Any number of result .json files to make the plot out of.")
    else:
        plot(sys.argv[1], sys.argv[2:])
