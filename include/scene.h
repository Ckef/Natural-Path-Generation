
#ifndef SCENE_H
#define SCENE_H

#include "cglm/cglm.h"
#include "patch.h"
#include "shader.h"

/* Define some scene values */
#define PATCH_SIZE 100
#define CAM_FOV    45.0f
#define CAM_NEAR   0.1f
#define CAM_FAR    1000.0f

/* Camera definition */
typedef struct
{
	mat4 proj; /* Projection matrix */
	mat4 view; /* View matrix */
	mat4 pv;   /* proj * view */

} Camera;

/* Scene deinition */
typedef struct
{
	Camera camera;
	Patch  patch;
	Shader shader;

} Scene;

/**
 * Creates a new scene.
 *
 * @return Zero if the scene creation failed.
 */
int create_scene(Scene* scene);

/**
 * Destroys a scene.
 */
void destroy_scene(Scene* scene);

/**
 * Changes the aspect ratio of the scene's camera.
 */
void scene_set_aspect(Scene* scene, float aspect);

/**
 * Draws the scene.
 */
void draw_scene(Scene* scene);


#endif
