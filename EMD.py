#!/usr/bin/env python3
import gurobipy as gp
import json
import math
import sys
from gurobipy import GRB
from itertools import chain

# Hardcoded input files for now
L_FILE         = 'terrain_L.json'
L_FLAGS_FILE   = 'terrain_out_f.json'
L_CONSTRS_FILE = 'terrain_out_c.json'
H_FILE         = 'terrain_H.json'


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
    L_flags = readTerrain(L_FLAGS_FILE)
    L_constrs = readTerrain(L_CONSTRS_FILE)
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
    L = list(chain.from_iterable(L))
    L_flags = list(chain.from_iterable(L_flags))
    L_constrs = list(chain.from_iterable(L_constrs))
    if not pathGen:
        H = list(chain.from_iterable(H))

        # Check dimensions
        if len(L) != len(H):
            print("-- Input and output terrains are not of the same dimensions.")
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
        # Add slack variables S+ and S- to be able to take the absolute value |L-H|
        Sp = m.addVar(lb=0.0, vtype=GRB.CONTINUOUS)
        Sn = m.addVar(lb=0.0, vtype=GRB.CONTINUOUS)
        obj.add(Sp + Sn, sigma)
        m.addConstr(H.sum('*') - sum(L) <= Sp, name="Sp")
        m.addConstr(sum(L) - H.sum('*') <= Sn, name="Sn")

    m.setObjective(obj, GRB.MINIMIZE)

    # Flow direction constraint
    m.addConstrs((flow[i,j] >= 0 for i,j in dist.keys()), "flowdir")

    # Maximum flow
    m.addConstrs((flow.sum(i,'*') <= L[i] for i in ran), "flowout")
    if pathGen:
        # If we're generating, this is == instead of <=
        # This because H is unkown yet, we're setting it instead of comparing against it
        # TODO: However this does not allow for creation of material anymore...
        m.addConstrs((flow.sum('*',j) == H[j] for j in ran), "flowin")
    else:
        m.addConstrs((flow.sum('*',j) <= H[j] for j in ran), "flowin")

    # Total flow
    if pathGen:
        # So if we're generating, instead of min(L,H), we do <= L and <= H
        # Because the above constraint is changed as well this works
        m.addConstr(flow.sum('*','*') <= sum(L), "totflowout")
        m.addConstr(flow.sum('*','*') <= H.sum('*'), "totflowin")
    else:
        m.addConstr(flow.sum('*','*') == min(sum(L), sum(H)), "totflow")

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
