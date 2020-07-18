
#ifndef CONSTANTS_H
#define CONSTANTS_H

/*****************************/
/* Hardcoded output files for now */
#define OUT_FILE_L        "terrain_out_l.json"
#define OUT_FILE_FLAGS    "terrain_out_f.json"
#define OUT_FILE_CONSTRS  "terrain_out_c.json"
#define OUT_FILE_H        "terrain_out_h.json"
#define OUT_FILE_ITERS    "iter_out.txt"
#define OUT_FILE_STATS    "stats_out.txt"


/*****************************/
/* Define some scene values */
#define SHADER_DIR  "shaders/"
#define PATCH_VERT  SHADER_DIR "normal.vert"
#define PATCH_FRAG  SHADER_DIR "lighting.frag"
#define HELP_VERT   SHADER_DIR "color.vert"
#define HELP_FRAG   SHADER_DIR "color.frag"

#define DEF_PATCH_SIZE  129 /* 2^N+1 with N=7, that's 128 tiles */
#define PATCH_HEIGHT    25
#define AXES_SIZE       16
#define CAM_FOV         45.0f
#define CAM_NEAR        0.1f
#define CAM_FAR         1000.0f
#define CAM_SPEED       120

/* Scale a patch to the default size */
/* This is equivalent to the ground distance between two neighbouring vertices */
#define GET_SCALE(size) ((float)(DEF_PATCH_SIZE-1) / (float)(size-1))


/*****************************/
/* Non-zero if we want to use certain features */
/* USE_DIR_SLOPE indicates to use directional derivative for path borders */
/* USE_ROUGHNESS indicates to use roughness constraints */
/* USE_BORDER_STITCH indicates to set position constraints at patch borders */
/* USE_BORDER_DERIV extends the patch borders with derivative constraints (more position constraints) */
/* When AUTO_SURROUND is non-zero, any patch will first be surrounded by 4 unconstrained patches */
#define USE_DIR_SLOPE      1
#define USE_ROUGHNESS      0
#define USE_BORDER_STITCH  1
#define USE_BORDER_DERIV   0
#define AUTO_SURROUND      0

/* Hardcoded path parameters for now */
/* The falloff is the ascend in the maximum slope the farther you get from the path boundary */
/* The influence is the distance from the path that the gradient constraint holds */
/* Maximum gradient ascends along the distance RADIUS + INFLUENCE */
#define MAX_SLOPE          0.0035f
#define MAX_SLOPE_FALLOFF  0.05f
#define PATH_RADIUS        2.2f
#define PATH_INFLUENCE     10.0f
#define COST_LIN           10000
#define COST_POW           1.8f


/*****************************/
/* Hardcoded thresholds for now */
#define S_THRESHOLD     0.00001f /* Convergence threshold of slope error */
#define R_THRESHOLD     0.04f /* Convergence threshold of roughness error */
#define MAX_ITERATIONS  100000
#define STEP_SIZE       10
#define ITER_PRINT      1000


#endif
