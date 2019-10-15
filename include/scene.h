
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
 * Draws the scene.
 */
void draw_scene(Scene* scene);

/**
 * Updates the scene.
 *
 * @param  dTime  Delta time of the current frame.
 */
void update_scene(Scene* scene, double dTime);

/**
 * Callback to be called when the framebuffer is resized.
 *
 * @param  width   New width of the buffer.
 * @param  height  New height of the buffer.
 */
void scene_framebuffer_size_callback(Scene* scene, int width, int height);

/**
 * Callback to be called when a key is pressed.
 *
 * @param  key     GLFW key pressed.
 * @param  action  GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
 * @param  mods    GLFW bitfield describing modifiers being held down.
 */
void scene_key_callback(Scene* scene, int key, int action, int mods);


#endif