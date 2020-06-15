#!/usr/bin/env python3
from ortools.linear_solver import pywraplp
import json
import math

# Hardcoded input files for now
L_FILE = 'terrain_L.json'
H_FILE = 'terrain_H.json'

# T_SIZE would be the width and height of the terrains
# T_SCALE would be the distance between two adjacent vertices
T_SIZE = 5;
T_SCALE = (129-1)/(T_SIZE-1); # See the C project for this rule


# Reads a terrain from filename into a list
def readTerrain(filename):
    f = open(filename)
    terr = json.loads(f.read())
    f.close()
    return terr


# Implementation of the EMD problem, taking input from files
def EMD():
    # Read terrain
    L = readTerrain(L_FILE)
    H = readTerrain(H_FILE)

    # Create a LP solver
    solver = pywraplp.Solver('EMD', pywraplp.Solver.GLOP_LINEAR_PROGRAMMING)
    objective = solver.Objective()

    # Define the flow matrix we want to find and its objective
    # Yes this is a 4-dimensional list
    F = [[[[0] * T_SIZE] * T_SIZE] * T_SIZE] * T_SIZE
    for ic in range(0, T_SIZE):
        for ir in range(0, T_SIZE):
            for jc in range(0, T_SIZE):
                for jr in range(0, T_SIZE):
                    name = 'F_'+str(ic)+'_'+str(ir)+'_'+str(jc)+'_'+str(jr)
                    F[ic][ir][jc][jr] = solver.NumVar(0.0, solver.infinity(), name)

                    # They also contribute to the objective
                    dist = math.sqrt((ic-jc)*(ic-jc) + (ir-jr)*(ir-jr)) * T_SCALE
                    objective.SetCoefficient(F[ic][ir][jc][jr], dist)

    objective.SetMinimization()

    # Define the outgoing and ingoing flow constraints
    cOuts = [[0] * T_SIZE] * T_SIZE
    cIns = [[0] * T_SIZE] * T_SIZE
    for ic in range(0, T_SIZE):
        for ir in range(0, T_SIZE):
            cOuts[ic][ir] = solver.Constraint(-solver.infinity(), L[ic][ir])
            cIns[ic][ir] = solver.Constraint(-solver.infinity(), H[ic][ir])

            for jc in range(0, T_SIZE):
                for jr in range(0, T_SIZE):
                    cOuts[ic][ir].SetCoefficient(F[ic][ir][jc][jr], 1)
                    cIns[ic][ir].SetCoefficient(F[jc][jr][ic][ir], 1)

    # Define the total flow constraint
    Ltot = sum(sum(L, []))
    Htot = sum(sum(H, []))
    Tflow = min(Ltot, Htot)
    cFlow = solver.Constraint(Tflow, Tflow)

    for ic in range(0, T_SIZE):
        for ir in range(0, T_SIZE):
            for jc in range(0, T_SIZE):
                for jr in range(0, T_SIZE):
                    cFlow.SetCoefficient(F[ic][ir][jc][jr], 1)

    # Solve dat problem
    status = solver.Solve()
    print('-- Number of variables =', solver.NumVariables())
    print('-- Number of constraints =', solver.NumConstraints())

    # Calculate the EMD
    if status == solver.OPTIMAL:
        cost = 0.0
        flow = 0.0
        for ic in range(0, T_SIZE):
            for ir in range(0, T_SIZE):
                for jc in range(0, T_SIZE):
                    for jr in range(0, T_SIZE):
                        dist = math.sqrt((ic-jc)*(ic-jc) + (ir-jr)*(ir-jr)) * T_SCALE
                        cost += F[ic][ir][jc][jr].solution_value() * dist
                        flow += F[ic][ir][jc][jr].solution_value()

        print('-- Cost =', cost)
        print('-- Flow =', flow)

    # No optimal solution
    else:
        if status == solver.FEASIBLE:
            print('-- A potentially suboptimal solution was found.')
        else:
            print('-- The solver could not solve the problem.')


# Entry point, more or less
if __name__ == '__main__':
    EMD()
