
#include "include.h"
#include "output.h"
#include "scene.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

/*****************************/
static int populate(unsigned int size, float* data, void* opt)
{
	/* Make a plane with noise ranging from 0 to 1 */
	for(unsigned int i = 0; i < size * size; ++i)
		data[i] = rand() / (float)RAND_MAX;

	return 1;
}

/*****************************/
static void set_camera(Scene* scene, double dTime)
{
	glm_mat4_identity(scene->camera.view);

	vec3 pos;
	vec3 x    = {1,0,0};
	vec3 y    = {0,1,0};
	vec3 z    = {0,0,1};
	vec3 back = {0,0,-40};
	vec3 down = {0,-40,0};
	vec3 cent = {(PATCH_SIZE-1)/2.0f, (PATCH_SIZE-1)/2.0f, 0};

	/* Update angle */
	scene->cam_angle += dTime * scene->cam_rotating;

	/* Rotate camera down a bit, so it looks down at the patch */
	glm_rotate(scene->camera.view, GLM_PI_4 * 0.5f, x);
	/* Move camera back a bit */
	glm_translate(scene->camera.view, back);
	/* Move camera up a bit */
	glm_translate(scene->camera.view, down);
	/* Rotate scene so camera is looking at patch */
	glm_rotate(scene->camera.view, GLM_PI_4, y);
	/* Rotate scene so y is up */
	glm_rotate(scene->camera.view, -GLM_PI_2, x);

	/* Translate patch back to its original position */
	glm_translate(scene->camera.view, cent);
	/* Rotate patch around 0,0,0 */
	glm_rotate(scene->camera.view, scene->cam_angle, z);
	/* Translate center of patch to 0,0,0 */
	glm_vec3_negate(cent);
	glm_translate(scene->camera.view, cent);

	/* Translate to camera position */
	glm_vec3_negate_to(scene->cam_position, pos);
	glm_translate(scene->camera.view, pos);

	/* Don't forget to update the pv matrix */
	glm_mat4_mul(scene->camera.proj, scene->camera.view, scene->camera.pv);
}

/*****************************/
static int add_patch(Scene* scene, PatchGenerator generator)
{
	/* Allocate more memory */
	size_t newSize = (scene->num_patches+1) * sizeof(Patch);
	Patch* new = realloc(scene->patches, newSize);

	if(!new)
	{
		throw_error("Could not reallocate memory for a new patch.");
		return 0;
	}

	scene->patches = new;

	/* Create a new patch */
	if(!create_patch(new + scene->num_patches, PATCH_SIZE))
	{
		throw_error("Could not create a new patch for scene.");
		return 0;
	}

	/* Set its position to the current selection */
	glm_mat4_copy(scene->help_mod, new[scene->num_patches].mod);

	/* Populate the new patch */
	if(!populate_patch(new + scene->num_patches, generator, NULL))
	{
		throw_error("Population of newly created patch failed.");
		return 0;
	}

	/* Only update num patches now so we don't get confused later on */
	/* The reallocation doesn't matter, as it's based on this number */
	++scene->num_patches;

	return 1;
}

/*****************************/
int create_scene(Scene* scene)
{
	/* Load shaders */
	if(!create_shader(&scene->patch_shader, PATCH_VERT, PATCH_FRAG))
	{
		throw_error("Could not create shader for patches in scene.");
		return 0;
	}

	if(!create_shader(&scene->help_shader, HELP_VERT, HELP_FRAG))
	{
		throw_error("Could not create shader for helpers in scene.");
		destroy_shader(&scene->patch_shader);
		return 0;
	}

	/* Create the first patch */
	/* Make sure to initialize the helper model matrix here */
	glm_mat4_identity(scene->help_mod);
	scene->patches = NULL;
	scene->num_patches = 0;

	if(!add_patch(scene, populate))
	{
		destroy_shader(&scene->patch_shader);
		destroy_shader(&scene->help_shader);
		return 0;
	}

	/* Create helper geometry */
	/* Contains a square to indicate the selected patch */
	/* Plus the axes of the coordinate system */
	float help_geom[] = {
		0,            0,            0, .5f, .5f, .5f,
		PATCH_SIZE-1, 0,            0, .5f, .5f, .5f,
		PATCH_SIZE-1, PATCH_SIZE-1, 0, .5f, .5f, .5f,
		0,            PATCH_SIZE-1, 0, .5f, .5f, .5f,

		/* X axis */
		0,         0,         0,         1, 0, 0,
		AXES_SIZE, 0,         0,         1, 0, 0,
		/* Y axis */
		0,         0,         0,         0, 1, 0,
		0,         AXES_SIZE, 0,         0, 1, 0,
		/* Z axis */
		0,         0,         0,         0, 0, 1,
		0,         0,         AXES_SIZE, 0, 0, 1
	};

	glGenVertexArrays(1, &scene->help_vao);
	glGenBuffers(1, &scene->help_buffer);

	glBindBuffer(GL_ARRAY_BUFFER, scene->help_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(help_geom), help_geom, GL_STATIC_DRAW);

	GLsizei attrSize = sizeof(float) * 3;
	glBindVertexArray(scene->help_vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE, attrSize * 2, (GLvoid*)0);
	glVertexAttribPointer(
		1, 3, GL_FLOAT, GL_FALSE, attrSize * 2, (GLvoid*)(uintptr_t)attrSize);

	/* Setup camera */
	/* The set_camera takes care of the view and pv matrices */
	glm_mat4_identity(scene->camera.proj);
	glm_vec3_zero(scene->cam_position);
	scene->cam_angle = 0.0f;
	scene->cam_rotating = 0;
	scene->cam_move = 0;
	set_camera(scene, 0.0);

	return 1;
}

/*****************************/
void destroy_scene(Scene* scene)
{
	destroy_shader(&scene->patch_shader);
	destroy_shader(&scene->help_shader);

	glDeleteVertexArrays(1, &scene->help_vao);
	glDeleteBuffers(1, &scene->help_buffer);

	/* Loop over all patches to destroy them */
	size_t p;
	for(p = 0; p < scene->num_patches; ++p)
		destroy_patch(scene->patches + p);

	free(scene->patches);
}

/*****************************/
void draw_scene(Scene* scene)
{
	mat4 mvp;

	/* Setup patch shader */
	glEnable(GL_DEPTH_TEST);
	glUseProgram(scene->patch_shader.program);
	GLint loc = glGetUniformLocation(scene->patch_shader.program, "MVP");

	/* Loop over all patches */
	size_t p;
	for(p = 0; p < scene->num_patches; ++p)
	{
		glm_mat4_mul(scene->camera.pv, scene->patches[p].mod, mvp);
		glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)mvp);

		/* Draw it, this assumes the above work is done, which it is :) */
		draw_patch(scene->patches + p);
	}

	/* Setup helper geometry shader */
	glDisable(GL_DEPTH_TEST);
	glUseProgram(scene->help_shader.program);
	loc = glGetUniformLocation(scene->help_shader.program, "MVP");

	glm_mat4_mul(scene->camera.pv, scene->help_mod, mvp);
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)mvp);

	/* Draw helper geometry */
	glBindVertexArray(scene->help_vao);
	glDrawArrays(GL_LINE_LOOP, 0, 4);
	glDrawArrays(GL_LINES, 4, 6);
}

/*****************************/
void update_scene(Scene* scene, double dTime)
{
	if(scene->cam_move || scene->cam_rotating)
		set_camera(scene, dTime);

	scene->cam_move = 0;
}

/*****************************/
void scene_framebuffer_size_callback(Scene* scene, int width, int height)
{
	float aspect = (float)width/(float)height;

	/* Calculate new projection matrix and update pv */
	glm_perspective(CAM_FOV, aspect, CAM_NEAR, CAM_FAR, scene->camera.proj);
	glm_mat4_mul(scene->camera.proj, scene->camera.view, scene->camera.pv);
}

/*****************************/
void scene_key_callback(Scene* scene, int key, int action, int mods)
{
	/* Signal when the camera should be rotating */
	if(key == GLFW_KEY_Q && action == GLFW_PRESS)
		scene->cam_rotating = 1;
	if(key == GLFW_KEY_E && action == GLFW_PRESS)
		scene->cam_rotating = -1;
	if((key == GLFW_KEY_Q || key == GLFW_KEY_E) && action == GLFW_RELEASE)
		scene->cam_rotating = 0;

	/* Move the camera */
	if(key == GLFW_KEY_W && action == GLFW_PRESS)
		scene->cam_position[1] += PATCH_SIZE-1, scene->cam_move = 1;
	if(key == GLFW_KEY_S && action == GLFW_PRESS)
		scene->cam_position[1] -= PATCH_SIZE-1, scene->cam_move = 1;
	if(key == GLFW_KEY_A && action == GLFW_PRESS)
		scene->cam_position[0] -= PATCH_SIZE-1, scene->cam_move = 1;
	if(key == GLFW_KEY_D && action == GLFW_PRESS)
		scene->cam_position[0] += PATCH_SIZE-1, scene->cam_move = 1;

	/* Move the helper geometry */
	if(key == GLFW_KEY_UP && action == GLFW_PRESS)
		glm_translate_y(scene->help_mod, PATCH_SIZE-1);
	if(key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		glm_translate_y(scene->help_mod, -(PATCH_SIZE-1));
	if(key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		glm_translate_x(scene->help_mod, -(PATCH_SIZE-1));
	if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		glm_translate_x(scene->help_mod, PATCH_SIZE-1);

	/* Add a new patch */
	if(key == GLFW_KEY_ENTER && action == GLFW_PRESS)
		add_patch(scene, populate);
}
