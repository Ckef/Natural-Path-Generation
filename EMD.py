#!/usr/bin/env python3
import gurobipy as gp
import json
import math
import sys
from gurobipy import GRB
from itertools import chain

# Hardcoded input files for now
L_FILE         = 'terrain_out_l.json'
L_FLAGS_FILE   = 'terrain_out_f.json'
L_CONSTRS_FILE = 'terrain_out_c.json'
H_FILE         = 'terrain_out_h.json'
H_OPT_FILE     = 'terrain_out_h_opt.json'
EMD_FILE       = 'emd_out.txt'


#######################
# Reads a terrain from filename into a list
def readTerrain(filename):
    f = open(filename)
    terr = json.loads(f.read())
    f.close()
    return terr

# Writes a terrain from list
def writeTerrain(filename, data, size):
    # First make it into a 2D list again
    terr = [[0] * size] * size
    for c in range(0,size):
        for r in range(0,size):
            terr[c][r] = data[c*size+r]

    f = open(filename, 'w')
    f.write(json.dumps(terr))
    f.close()


#######################
# Builds a distance dictionary
def buildDist(size, scale):
    dist = {}
    for ic in range(0,size):
        for ir in range(0,size):
            for jc in range(0,size):
                for jr in range(0,size):
                    hypot = math.hypot(ic-jc, ir-jr)
                    dist[(ic*size+ir, jc*size+jr)] = hypot * scale

    return dist


#######################
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


#######################
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
    sigma = (size-1)*scale*2

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

    #######################
    # Create a new model
    m = gp.Model("EMD")
    m.setParam("NonConvex", 2)

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
    else:
        # Add the constant value anyway so we can compare the EMD values
        obj.addConstant(sigma * abs(sum(L) - sum(H)))

    m.setObjective(obj, GRB.MINIMIZE)

    #######################
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

    #######################
    # Now add the extra constraints on H
    # This gives rise to the features we want in H
    if pathGen:
        SQR = lambda x : x*x
        HYP = lambda x : math.hypot(x[0], x[1])
        DIR = lambda x : (x[0]/HYP(x), x[1]/HYP(x))

        # Gradient constraint
        # Formulated as sqrt((y-x)^2/scale^2 + (z-x)^2/scale^2) <= g(x) for each quadrant
        # where y and z are the two neighbors in the relevant quadrant
        # can be written as:
        # (y-x)^2 + (z-x)^2 <= scale^2 * g(x)^2
        m.addConstrs(
            (SQR(H[i+size]-H[i]) + SQR(H[i+1]-H[i])
            <= SQR(scale*sDicts[0][i]) for i in sDicts[0].keys()), "gradient0")
        m.addConstrs(
            (SQR(H[i-1]-H[i]) + SQR(H[i+size]-H[i])
            <= SQR(scale*sDicts[1][i]) for i in sDicts[1].keys()), "gradient1")
        m.addConstrs(
            (SQR(H[i-size]-H[i]) + SQR(H[i-1]-H[i])
            <= SQR(scale*sDicts[2][i]) for i in sDicts[2].keys()), "gradient2")
        m.addConstrs(
            (SQR(H[i+1]-H[i]) + SQR(H[i-size]-H[i])
            <= SQR(scale*sDicts[3][i]) for i in sDicts[3].keys()), "gradient3")

        # Directional derivative constraint
        # Formulated as |((y-x)/scale, (z-x)/scale) dot D(x)| <= d(x) for each quadrant
        # where y and z are the two neighbors in the relevant quadrant and D(x) is the direction of x
        # can be written as:
        # |(y-x, z-x) dot D(x)| <= scale * d(x)
        # Now we have an absolute value, so instead we write it as two constraints:
        # (y-x, z-x) dot D(x) <= scale * d(x)
        # (y-x, z-x) dot D(x) >= scale * -d(x)
        deriv = lambda d,i : (H[i+size]-H[i])*DIR(dDicts[d][i])[0] + (H[i+1]-H[i])*DIR(dDicts[d][i])[1]
        m.addConstrs((deriv(0,i) <= scale*HYP(dDicts[0][i]) for i in dDicts[0].keys()), "deriv0p")
        m.addConstrs((deriv(0,i) >= -scale*HYP(dDicts[0][i]) for i in dDicts[0].keys()), "deriv0n")

        deriv = lambda d,i : (H[i-1]-H[i])*DIR(dDicts[d][i])[0] + (H[i+size]-H[i])*DIR(dDicts[d][i])[1]
        m.addConstrs((deriv(1,i) <= scale*HYP(dDicts[1][i]) for i in dDicts[1].keys()), "deriv1p")
        m.addConstrs((deriv(1,i) >= -scale*HYP(dDicts[1][i]) for i in dDicts[1].keys()), "deriv1n")

        deriv = lambda d,i : (H[i-size]-H[i])*DIR(dDicts[d][i])[0] + (H[i-1]-H[i])*DIR(dDicts[d][i])[1]
        m.addConstrs((deriv(2,i) <= scale*HYP(dDicts[2][i]) for i in dDicts[2].keys()), "deriv2p")
        m.addConstrs((deriv(2,i) >= -scale*HYP(dDicts[2][i]) for i in dDicts[2].keys()), "deriv2n")

        deriv = lambda d,i : (H[i+1]-H[i])*DIR(dDicts[d][i])[0] + (H[i-size]-H[i])*DIR(dDicts[d][i])[1]
        m.addConstrs((deriv(3,i) <= scale*HYP(dDicts[3][i]) for i in dDicts[3].keys()), "deriv3p")
        m.addConstrs((deriv(3,i) >= -scale*HYP(dDicts[3][i]) for i in dDicts[3].keys()), "deriv3n")

        # Roughness constraint
        # Formulated as sqrt(sum((xi-x)^2/scale^2)) == r(x)
        # where xi are the 8 neighbors around x, at the edges we omit neighbors that don't exist
        # can be written as:
        # sum((xi-x)^2) == scale^2 * r(x)^2
        # TODO: incorporate threshold, maybe this makes it feasible?
        m.addConstrs( # Lower left corner
            (SQR(H[i+1]-H[i]) + SQR(H[i+size]-H[i]) + SQR(H[i+size+1]-H[i])
            == SQR(scale*rDicts[0][i]) for i in rDicts[0].keys()), "roughness0")
        m.addConstrs( # Upper left corner
            (SQR(H[i-1]-H[i]) + SQR(H[i+size-1]-H[i]) + SQR(H[i+size]-H[i])
            == SQR(scale*rDicts[1][i]) for i in rDicts[1].keys()), "roughness1")
        m.addConstrs( # Upper right corner
            (SQR(H[i-size-1]-H[i]) + SQR(H[i-size]-H[i]) + SQR(H[i-1]-H[i])
            == SQR(scale*rDicts[2][i]) for i in rDicts[2].keys()), "roughness2")
        m.addConstrs( # Lower right corner
            (SQR(H[i-size]-H[i]) + SQR(H[i-size+1]-H[i]) + SQR(H[i+1]-H[i])
            == SQR(scale*rDicts[3][i]) for i in rDicts[3].keys()), "roughness3")
        m.addConstrs( # Left edge
            (SQR(H[i-1]-H[i]) + SQR(H[i+1]-H[i]) +
            SQR(H[i+size-1]-H[i]) + SQR(H[i+size]-H[i]) + SQR(H[i+size+1]-H[i])
            == SQR(scale*rDicts[4][i]) for i in rDicts[4].keys()), "roughness4")
        m.addConstrs( # Upper edge
            (SQR(H[i-size-1]-H[i]) + SQR(H[i-size]-H[i]) + SQR(H[i-1]-H[i]) +
            SQR(H[i+size-1]-H[i]) + SQR(H[i+size]-H[i])
            == SQR(scale*rDicts[5][i]) for i in rDicts[5].keys()), "roughness5")
        m.addConstrs( # Right edge
            (SQR(H[i-size-1]-H[i]) + SQR(H[i-size]-H[i]) + SQR(H[i-size+1]-H[i]) +
            SQR(H[i-1]-H[i]) + SQR(H[i+1]-H[i])
            == SQR(scale*rDicts[6][i]) for i in rDicts[6].keys()), "roughness6")
        m.addConstrs( # Lower edge
            (SQR(H[i-size]-H[i]) + SQR(H[i-size+1]-H[i]) + SQR(H[i+1]-H[i]) +
            SQR(H[i+size]-H[i]) + SQR(H[i+size+1]-H[i])
            == SQR(scale*rDicts[7][i]) for i in rDicts[7].keys()), "roughness7")
        m.addConstrs( # Middle
            (SQR(H[i-size-1]-H[i]) + SQR(H[i-size]-H[i]) + SQR(H[i-size+1]-H[i]) +
            SQR(H[i-1]-H[i]) + SQR(H[i+1]-H[i]) +
            SQR(H[i+size-1]-H[i]) + SQR(H[i+size]-H[i]) + SQR(H[i+size+1]-H[i])
            == SQR(scale*rDicts[8][i]) for i in rDicts[8].keys()), "roughness8")

        # Position constraint
        # Formulated as x == p(x)
        m.addConstrs((H[i] == pDict[i] for i in pDict.keys()), "position")

    #######################
    # Compute optimal solution
    m.optimize()

    # Print solution
    if m.status == GRB.OPTIMAL:
        Cost = m.getAttr("ObjVal")
        Flow = sum(m.getAttr('x', flow).values())
        print("-- Size =", len(L))
        print("-- Cost =", Cost)
        print("-- Flow =", Flow)

        # Also write solution to file
        if pathGen:
            writeTerrain(H_OPT_FILE, m.getAttr('x', H).values(), size)

        f = open(EMD_FILE, 'a')
        f.write("{}\n".format(Cost))
        f.close()


#######################
# Entry point, more or less
if __name__ == '__main__':
    # So really if any argument is given, we use the generator
    pathGen = False
    if len(sys.argv) > 1:
        pathGen = True

    EMD(pathGen)
