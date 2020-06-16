/*********************************************
 * OPL 12.9.0.0 Model
 * Author: stef
 * Creation Date: Jun 9, 2020 at 5:45:57 PM
 *********************************************/

int TC = 5; // Terrain width (number of columns)
int TR = 5; // Terrain height (number of rows)

// TScale would be the distance between two adjacent vertices
float TScale = (129-1)/(maxl(TC,TR)-1); // See the C project for this rule

// Dim would be dimension, i.e. how to access the terrain
range DimC = 0..TC-1;
range DimR = 0..TR-1;

// The EMD input (L) and output (H)
float L[DimC][DimR] = ...;
float H[DimC][DimR] = ...;

// The flow matrix we are trying to find
dvar float F[DimC][DimR][DimC][DimR];

// Minimization definition
minimize
  sum(ic in DimC, ir in DimR, jc in DimC, jr in DimR)
    F[ic][ir][jc][jr] * sqrt((ic-jc)*(ic-jc) + (ir-jr)*(ir-jr)) * TScale;

// Constraint definitions
subject to{
  forall(ic in DimC, ir in DimR, jc in DimC, jr in DimR)
    F[ic][ir][jc][jr] >= 0;

  forall(ic in DimC, ir in DimR)
    sum(jc in DimC, jr in DimR) F[ic][ir][jc][jr] <= L[ic][ir];
  
  forall(jc in DimC, jr in DimR)
    sum(ic in DimC, ir in DimR) F[ic][ir][jc][jr] <= H[jc][jr];
  
  sum(ic in DimC, ir in DimR, jc in DimC, jr in DimR)
    F[ic][ir][jc][jr] == minl(
      sum(ic in DimC, ir in DimR) L[ic][ir],
      sum(jc in DimC, jr in DimR) H[jc][jr]);
}

// Now calculate the actual EMD
float Cost = sum(ic in DimC, ir in DimR, jc in DimC, jr in DimR)
    F[ic][ir][jc][jr] * sqrt((ic-jc)*(ic-jc) + (ir-jr)*(ir-jr)) * TScale;
float Flow = sum(ic in DimC, ir in DimR, jc in DimC, jr in DimR)
    F[ic][ir][jc][jr];
    
execute DISPLAY_RESULTS {
  writeln("Cost=", Cost);
  writeln("Flow=", Flow);
  writeln("EMD=", Cost/Flow);
}
