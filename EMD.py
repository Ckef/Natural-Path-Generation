#!/usr/bin/env python3
import gurobipy as gp
import json
import math
from gurobipy import GRB

# Hardcoded input files for now
L_FILE = 'terrain_L.json'
H_FILE = 'terrain_H.json'


# Reads a terrain from filename into a list
def readTerrain(filename):
    f = open(filename)
    terr = json.loads(f.read())
    f.close()
    return terr


# Builds a cost dictionary
def buildCost(size, scale):
    cost = {}
    for ic in range(0,size):
        for ir in range(0,size):
            for jc in range(0,size):
                for jr in range(0,size):
                    hypot = math.sqrt((ic-jc)*(ic-jc) + (ir-jr)*(ir-jr))
                    cost[(ic*size+ir, jc*size+jr)] = hypot * scale

    return cost


# Implementation of the EMD problem, taking input from files
def EMD():
    # Read terrain
    L = readTerrain(L_FILE)
    H = readTerrain(H_FILE)

    if len(L) != len(H):
        print("-- Input and output terrain are not of the same dimensions.")
        return

    # size would be the width and height of the terrain
    # scale would be the distance between two adjacent vertices
    size = len(L)
    scale = (129-1)/(size-1) # See the C project for this rule

    # First flatten the data and get the cost dict
    L = [item for sublist in L for item in sublist]
    H = [item for sublist in H for item in sublist]
    cost = buildCost(size, scale)

    # Create a new model
    m = gp.Model("EMD")

    # Add flow variables
    flow = m.addVars(cost.keys(), obj=cost, vtype=GRB.CONTINUOUS, name="flow")

    # Flow direction constraint
    m.addConstrs((flow[i,j] >= 0 for i,j in cost.keys()), "flowdir")
    #for i,j in cost.keys():
    #    m.addConstr(flow[i,j] >= 0, "flow[%s, %s]" % (i,j))

    # Maximum flow
    m.addConstrs((flow.sum(i,'*') <= L[i] for i in range(0,len(L))), "flowout")
    m.addConstrs((flow.sum('*',j) <= H[j] for j in range(0,len(H))), "flowin")
    #for i in range(0,len(L)):
    #    m.addConstr((sum(flow[i,j] for j in range(0,len(H))) <= L[i]), "flowout[%s]" % i)
    #for j in range(0,len(H)):
    #    m.addConstr((sum(flow[i,j] for i in range(0,len(L))) <= H[j]), "flowin[%s]" % j)

    # Minimum flow
    m.addConstr(flow.sum('*','*') == min(sum(L), sum(H)), "minflow")

    # Compute optimal solution
    m.optimize()

    # Print solution
    if m.status == GRB.OPTIMAL:
        solution = m.getAttr('x', flow)
        for i,j in cost.keys():
            print("%s -> %s: %g" % (i,j, solution[i,j]))


# Entry point, more or less
if __name__ == '__main__':
    EMD()
