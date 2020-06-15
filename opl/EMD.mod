/*********************************************
 * OPL 12.9.0.0 Model
 * Author: stef
 * Creation Date: Jun 9, 2020 at 5:45:57 PM
 *********************************************/

// TSize would be the width and height of the terrains
// TScale would be the distance between two adjacent vertices
int TSize = 5;
float TScale = (129-1)/(TSize-1); // See the C project for this rule

// Dim would be dimension, i.e. how to access the terrain
range Dim = 0..TSize-1;

// The EMD input (L) and output (H)
float L[Dim][Dim] = ...;
float H[Dim][Dim] = ...;

// The flow matrix we are trying to find
dvar float F[Dim][Dim][Dim][Dim];

// Minimization definition
minimize
  sum(ic in Dim, ir in Dim, jc in Dim, jr in Dim)
    F[ic][ir][jc][jr] * sqrt((ic-jc)*(ic-jc) + (ir-jr)*(ir-jr)) * TScale;

// Constraint definitions
subject to{
  forall(ic in Dim, ir in Dim, jc in Dim, jr in Dim)
    F[ic][ir][jc][jr] >= 0;

  forall(ic in Dim, ir in Dim)
    sum(jc in Dim, jr in Dim) F[ic][ir][jc][jr] <= L[ic][ir];
  
  forall(jc in Dim, jr in Dim)
    sum(ic in Dim, ir in Dim) F[ic][ir][jc][jr] <= H[jc][jr];
  
  sum(ic in Dim, ir in Dim, jc in Dim, jr in Dim)
    F[ic][ir][jc][jr] == minl(
      sum(ic in Dim, ir in Dim) L[ic][ir],
      sum(jc in Dim, jr in Dim) H[jc][jr]);
}

// Now calculate the actual EMD
float Cost = sum(ic in Dim, ir in Dim, jc in Dim, jr in Dim)
    F[ic][ir][jc][jr] * sqrt((ic-jc)*(ic-jc) + (ir-jr)*(ir-jr)) * TScale;
float Flow = sum(ic in Dim, ir in Dim, jc in Dim, jr in Dim)
    F[ic][ir][jc][jr];
    
execute DISPLAY_RESULTS {
  writeln("EMD=", Cost/Flow);
}
