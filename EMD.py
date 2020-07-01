#!/usr/bin/env python3
import gurobipy as gp
import json
import math
import sys
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


# Builds a distance dictionary
def buildDist(size, scale):
    dist = {}
    for ic in range(0,size):
        for ir in range(0,size):
            for jc in range(0,size):
                for jr in range(0,size):
                    hypot = math.sqrt((ic-jc)*(ic-jc) + (ir-jr)*(ir-jr))
                    dist[(ic*size+ir, jc*size+jr)] = hypot * scale

    return dist


# Implementation of the EMD problem, taking input from files
# set pathGen to False if you just want to calculate the EMD
# set pathGen to True to set the output as a decision variable too
def EMD(pathGen):
    # Read terrain
    L = readTerrain(L_FILE)
    H = []

    if not pathGen:
        H = readTerrain(H_FILE)

    # size would be the width and height of the terrain
    # scale would be the distance between two adjacent vertices
    # sigma is the cost of creating/destroying, in this case, sigma >= all d_ij
    size = len(L)
    scale = (129-1)/(size-1) # See the C project for this rule
    sigma = (size-1)*2

    # First flatten the data and get the distance dict
    L = [item for sublist in L for item in sublist]
    if not pathGen:
        H = [item for sublist in H for item in sublist]

        # Check dimensions
        if len(L) != len(H):
            print("-- Input and output terrain are not of the same dimensions.")
            return

    ran = range(0,len(L))
    dist = buildDist(size, scale)

    # Create a new model
    m = gp.Model("EMD")

    # Add flow and output terrain H variables
    flow = m.addVars(dist.keys(), vtype=GRB.CONTINUOUS, name="flow")
    if pathGen:
        H = m.addVars(ran, vtype=GRB.CONTINUOUS, name="H")

    # Define the objective
    obj = gp.quicksum(flow[i,j] * dist[i,j] for i,j in dist.keys())
    if pathGen:
        # Add slack variables Sp and Sn to be able to take the absolute value |L-H|
        Sp = m.addVar(lb=0.0, vtype=GRB.CONTINUOUS)
        Sn = m.addVar(lb=0.0, vtype=GRB.CONTINUOUS)
        obj.add(Sp + Sn, sigma)
        m.addConstr(H.sum('*') - sum(L) <= Sp, name="Sp")
        m.addConstr(sum(L) - H.sum('*') <= Sn, name="Sn")

    m.setObjective(obj, GRB.MINIMIZE)

    # Flow direction constraint
    m.addConstrs((flow[i,j] >= 0 for i,j in dist.keys()), "flowdir")
    #for i,j in dist.keys():
    #    m.addConstr(flow[i,j] >= 0, "flow[%s, %s]" % (i,j))

    # Maximum flow
    m.addConstrs((flow.sum(i,'*') <= L[i] for i in ran), "flowout")
    m.addConstrs((flow.sum('*',j) <= H[j] for j in ran), "flowin")
    #for i in range(0,len(L)):
    #    m.addConstr((sum(flow[i,j] for j in ran) <= L[i]), "flowout[%s]" % i)
    #for j in range(0,len(H)):
    #    m.addConstr((sum(flow[i,j] for i in ran) <= H[j]), "flowin[%s]" % j)

    # Minimum flow
    if pathGen:
        # TODO: Apparently a general expression can only be equal to a single var
        # So at this point no creation/destruction is allowed"
        #m.addConstr(flow.sum('*','*') == gp.min_(sum(L), H.sum('*')), "minflow")
        m.addConstr(flow.sum('*','*') == sum(L), "totflowout")
        #m.addConstr(flow.sum('*','*') <= H.sum('*'), "totflowin")
    else:
        m.addConstr(flow.sum('*','*') == min(sum(L), sum(H)), "minflow")

    # Compute optimal solution
    m.optimize()

    # Print solution
    if m.status == GRB.OPTIMAL:
        Cost = m.getAttr("ObjVal")
        Flow = sum(m.getAttr('x', flow).values())
        print("-- Cost =", Cost)
        print("-- Flow =", Flow)

        if not pathGen:
            print("-- EMD =", (Cost/Flow))
        else:
            print("-- H =", m.getAttr('x', H).values())


# Entry point, more or less
if __name__ == '__main__':
    # So really if any argument is given, we use the generator
    pathGen = False
    if len(sys.argv) > 1:
        pathGen = True

    EMD(pathGen)
