
#ifndef SCENE_H
#define SCENE_H

#include "cglm/cglm.h"
#include "patch.h"
#include "shader.h"

/* Camera definition */
typedef struct
{
	mat4 pv; /* proj * view matrix, modify at free will */

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
 * Draws the scene.
 */
void draw_scene(Scene* scene);


#endif
