
#ifndef SCENE_H
#define SCENE_H

#include "deps.h"
#include "patch.h"
#include "shader.h"

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
	Camera       camera;
	Patch*       patches;
	unsigned int grid_size; /* Width/height of each quadrant */
	unsigned int patch_size;
	Shader       patch_shader;
	ModMode      patch_mode;

	/* Selection (helper) graphics */
	ivec3  help_pos;
	GLuint help_vao;
	GLuint help_buffer;
	Shader help_shader;

	/* Camera movements */
	ivec3 cam_dest;
	vec3  cam_pos;
	int   cam_rotating; /* -1, 0, 1 */
	float cam_angle;

} Scene;


/**
 * Creates a new scene.
 *
 * @param  mode       Mode to use for calculation logic in all modifiers.
 * @param  patchSize  Width and height of all patches in vertices.
 *                    Set to DEF_PATCH_SIZE if value < 2.
 * @return            Zero if the scene creation failed.
 */
int create_scene(Scene* scene, ModMode mode, unsigned int patchSize);

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
