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

# Builds dictionaries for all constraints
# If no constraint data is present, it just returns nothing
def buildConstrs(size, L_flags, L_constrs):
    if len(L_flags) == 0 or len(L_constrs) == 0:
        return (None, None, None, None)

    # For gradient and directional derivative, we have 4 directions
    # For roughness, we have 4 corners, 4 edges and the center
    # The position constraint is the same everywhere
    sDicts = ({}, {}, {}, {})
    dDicts = ({}, {}, {}, {})
    rDicts = ({}, {}, {}, {}, {}, {}, {}, {}, {})
    pDict = {}

    for ic in range(0,size):
        for ir in range(0,size):
            ind = ic * size + ir
            constrs = L_constrs[ic][ir]

            # Gradient constraint
            if L_flags[ic][ir] & 0b0001:
                if ic < size-1 and ir < size-1:     # Upper right quadrant
                    sDicts[0][ind] = constrs[0]
                if ic < size-1 and ir > 0:          # Lower right quadrant
                    sDicts[1][ind] = constrs[0]
                if ic > 0 and ir > 0:               # Lower left quadrant
                    sDicts[2][ind] = constrs[0]
                if ic > 0 and ir < size-1:          # Upper left quadrant
                    sDicts[3][ind] = constrs[0]

            # Directional derivative constraint
            if L_flags[ic][ir] & 0b0010:
                if ic < size-1 and ir < size-1:     # Upper right quadrant
                    dDicts[0][ind] = (constrs[0], constrs[1])
                if ic < size-1 and ir > 0:          # Lower right quadrant
                    dDicts[1][ind] = (constrs[0], constrs[1])
                if ic > 0 and ir > 0:               # Lower left quadrant
                    dDicts[2][ind] = (constrs[0], constrs[1])
                if ic > 0 and ir < size-1:          # Upper left quadrant
                    dDicts[3][ind] = (constrs[0], constrs[1])

            # Roughness constraint
            if L_flags[ic][ir] & 0b0100:
                if ic == 0 and ir == 0:             # Lower left corner
                    rDicts[0][ind] = constrs[0]
                elif ic == 0 and ir == size-1:      # Upper left corner
                    rDicts[1][ind] = constrs[0]
                elif ic == size-1 and ir == size-1: # Upper right corner
                    rDicts[2][ind] = constrs[0]
                elif ic == size-1 and ir == 0:      # Lower right corner
                    rDicts[3][ind] = constrs[0]
                elif ic == 0:                       # Left edge
                    rDicts[4][ind] = constrs[0]
                elif ir == size-1:                  # Upper edge
                    rDicts[5][ind] = constrs[0]
                elif ic == size-1:                  # Right edge
                    rDicts[6][ind] = constrs[0]
                elif ir == 0:                       # Lower edge
                    rDicts[7][ind] = constrs[0]
                else:                               # Middle
                    rDicts[8][ind] = constrs[0]

            # Position constraint
            if L_flags[ic][ir] & 0b1000:
                pDict[ind] = constrs[2]

    return (sDicts, dDicts, rDicts, pDict)


# Implementation of the EMD problem, taking input from files
# set pathGen to False if you just want to calculate the EMD
# set pathGen to True to set the output as a decision variable too
def EMD(pathGen):
    # Read terrain
    L = readTerrain(L_FILE)
    L_flags = []
    L_constrs = []
    H = []

    if not pathGen:
        H = readTerrain(H_FILE)
    else:
        L_flags = readTerrain(L_FLAGS_FILE)
        L_constrs = readTerrain(L_CONSTRS_FILE)

    # size would be the width and height of the terrain
    # scale would be the distance between two adjacent vertices
    # sigma is the cost of creating/destroying, in this case, sigma >= all d_ij
    size = len(L)
    scale = (129-1)/(size-1) # See the C project for this rule
    sigma = (size-1)*2

    # First flatten the data and build all dictionaries
    L = list(chain.from_iterable(L))
    if not pathGen:
        H = list(chain.from_iterable(H))

        # Check dimensions
        if len(L) != len(H):
            print("-- Input and output terrains are not of the same dimensions.")
            return
    else:
        if len(L_flags) != size or len(L_constrs) != size:
            print("-- Flag or constraint files are not of the correct dimensions.")
            return

    ran = range(0,len(L))
    dist = buildDist(size, scale)
    sDicts, dDicts, rDicts, pDict = buildConstrs(size, L_flags, L_constrs)

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
        print("-- EMD =", Cost/Flow)
        if pathGen:
            print("-- H =", m.getAttr('x', H).values())


# Entry point, more or less
if __name__ == '__main__':
    # So really if any argument is given, we use the generator
    pathGen = False
    if len(sys.argv) > 1:
        pathGen = True

    EMD(pathGen)
