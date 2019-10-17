
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
	for(unsigned i = 0; i < size * size; ++i)
		data[i] = rand() / (float)RAND_MAX;

	return 1;
}

/*****************************/
static void set_camera(Scene* scene, double dTime)
{
	glm_mat4_identity(scene->camera.view);

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

	/* Don't forget to update the pv matrix */
	glm_mat4_mul(scene->camera.proj, scene->camera.view, scene->camera.pv);
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

	/* Create and populate a patch */
	if(!create_patch(&scene->patch, PATCH_SIZE))
	{
		throw_error("Could not create patch for scene.");
		destroy_shader(&scene->patch_shader);
		destroy_shader(&scene->help_shader);
		return 0;
	}

	populate_patch(&scene->patch, populate, NULL);

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
	scene->cam_angle = 0.0f;
	scene->cam_rotating = 0;
	set_camera(scene, 0.0);

	/* Initialize random number generator */
	srand(time(NULL));

	return 1;
}

/*****************************/
void destroy_scene(Scene* scene)
{
	destroy_shader(&scene->patch_shader);
	destroy_shader(&scene->help_shader);
	destroy_patch(&scene->patch);

	glDeleteVertexArrays(1, &scene->help_vao);
	glDeleteBuffers(1, &scene->help_buffer);
}

/*****************************/
void draw_scene(Scene* scene)
{
	/* Setup patch shader */
	glEnable(GL_DEPTH_TEST);
	glUseProgram(scene->patch_shader.program);

	mat4 mvp;
	glm_mat4_mul(scene->camera.pv, scene->patch.mod, mvp);

	GLint loc = glGetUniformLocation(scene->patch_shader.program, "MVP");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)mvp);

	/* Draw patch */
	glBindVertexArray(scene->patch.vao);
	size_t elems = 6 * (scene->patch.size-1) * (scene->patch.size-1);
	glDrawElements(GL_TRIANGLES, elems, GL_UNSIGNED_INT, (GLvoid*)0);

	/* Setup helper geometry shader */
	glDisable(GL_DEPTH_TEST);
	glUseProgram(scene->help_shader.program);

	loc = glGetUniformLocation(scene->help_shader.program, "MVP");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)scene->camera.pv);

	/* Draw helper geometry */
	glBindVertexArray(scene->help_vao);
	glDrawArrays(GL_LINE_LOOP, 0, 4);
	glDrawArrays(GL_LINES, 4, 6);
}

/*****************************/
void update_scene(Scene* scene, double dTime)
{
	if(scene->cam_rotating)
		set_camera(scene, dTime);
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
		scene->cam_rotating = -1;
	if(key == GLFW_KEY_E && action == GLFW_PRESS)
		scene->cam_rotating = 1;
	if((key == GLFW_KEY_Q || key == GLFW_KEY_E) && action == GLFW_RELEASE)
		scene->cam_rotating = 0;
}
