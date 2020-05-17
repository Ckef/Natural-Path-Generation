
#include "deps.h"
#include "generators.h"
#include "modifiers.h"
#include "output.h"
#include "scene.h"
#include <stdlib.h>

/*****************************/
static inline float get_scale(Scene* scene)
{
	/* Returns the scale of the patches relative to the default size */
	/* Useful for scaling things when the patch size changes */
	return (float)scene->patch_size / (float)DEF_PATCH_SIZE;
}

/*****************************/
static int add_patch(
	Scene*         scene,
	PatchGenerator generator,
	PatchModifier* mods)
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
	if(!create_patch(new + scene->num_patches, scene->patch_size))
	{
		throw_error("Could not create a new patch for scene.");
		return 0;
	}

	/* Set its position to the current selection */
	glm_vec3_copy(scene->help_pos, new[scene->num_patches].pos);

	/* Populate the new patch */
	if(!populate_patch(new + scene->num_patches, generator, mods, NULL))
	{
		throw_error("Population of newly created patch failed.");
		return 0;
	}

	/* Only update num patches now so we don't get confused later on */
	/* The reallocation doesn't matter, as it's based on this number */
	++scene->num_patches;

	output("Patch was created succesfully.");

	return 1;
}

/*****************************/
static void update_camera(Scene* scene, double dTime)
{
	/* Scale camera position and speed according to patch size */
	float scale = get_scale(scene);
	glm_mat4_identity(scene->camera.view);

	vec3 mov;
	vec3 x    = {1,0,0};
	vec3 y    = {0,1,0};
	vec3 z    = {0,0,1};
	vec3 back = {0,-40 * scale, -40 * scale};
	vec3 cent = {(scene->patch_size-1)/2.0f, (scene->patch_size-1)/2.0f, 0};

	/* Update position */
	glm_vec3_copy(scene->cam_dest, mov);
	glm_vec3_sub(mov, scene->cam_pos, mov);
	float ml = glm_vec3_norm(mov);

	if(CAM_SPEED * scale * dTime >= ml)
		glm_vec3_copy(scene->cam_dest, scene->cam_pos);
	else
	{
		glm_vec3_scale(mov, CAM_SPEED * scale * dTime / ml, mov);
		glm_vec3_add(scene->cam_pos, mov, scene->cam_pos);
	}

	/* Update angle */
	scene->cam_angle += dTime * scene->cam_rotating;

	/* Rotate camera down a bit, so it looks down at the patch */
	glm_rotate(scene->camera.view, GLM_PI_4 * 0.5f, x);
	/* Move camera back and up a bit */
	glm_translate(scene->camera.view, back);
	/* Rotate scene so camera is looking at patch */
	glm_rotate(scene->camera.view, GLM_PI_4, y);
	/* Rotate scene so y is up */
	glm_rotate(scene->camera.view, -GLM_PI_2, x);

	/* Rotate around center of the patch we're looking at */
	glm_rotate_at(scene->camera.view, cent, scene->cam_angle, z);
	/* Translate to camera position */
	glm_vec3_negate_to(scene->cam_pos, mov);
	glm_translate(scene->camera.view, mov);

	/* Don't forget to update the pv matrix */
	glm_mat4_mul(scene->camera.proj, scene->camera.view, scene->camera.pv);
}

/*****************************/
int create_scene(Scene* scene, unsigned int patchSize)
{
	if(patchSize < 2) patchSize = DEF_PATCH_SIZE;
	scene->patch_size = patchSize;

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

	/* Setup camera */
	/* The set_camera takes care of the view and pv matrices */
	glm_mat4_identity(scene->camera.proj);
	glm_vec3_zero(scene->cam_dest);
	glm_vec3_zero(scene->cam_pos);
	scene->cam_rotating = 0;
	scene->cam_angle = 0.0f;
	update_camera(scene, 0.0);

	/* Make sure to initialize the helper position here */
	glm_vec3_zero(scene->help_pos);
	scene->patches = NULL;
	scene->num_patches = 0;

	/* Create helper geometry */
	/* Contains a square to indicate the selected patch */
	/* Plus the axes of the coordinate system */
	/* Scale axes size according to patch size */
	float aSize = AXES_SIZE * get_scale(scene);
	float help_geom[] = {
		0,           0,           0, .5f, .5f, .5f,
		patchSize-1, 0,           0, .5f, .5f, .5f,
		patchSize-1, patchSize-1, 0, .5f, .5f, .5f,
		0,           patchSize-1, 0, .5f, .5f, .5f,

		/* X axis */
		0,     0,     0,     1, 0, 0,
		aSize, 0,     0,     1, 0, 0,
		/* Y axis */
		0,     0,     0,     0, 1, 0,
		0,     aSize, 0,     0, 1, 0,
		/* Z axis */
		0,     0,     0,     0, 0, 1,
		0,     0,     aSize, 0, 0, 1
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
	/* Setup patch shader */
	glEnable(GL_DEPTH_TEST);
	glUseProgram(scene->patch_shader.program);
	GLint loc = glGetUniformLocation(scene->patch_shader.program, "MVP");

	/* Loop over all patches */
	vec3 scale = {1, 1, PATCH_HEIGHT * get_scale(scene)};
	mat4 mvp;

	size_t p;
	for(p = 0; p < scene->num_patches; ++p)
	{
		/* The height (z-coord) of all patches is in [0,1] */
		/* So move it down 0.5 and scale it */
		glm_translate_to(scene->camera.pv, scene->patches[p].pos, mvp);
		glm_scale(mvp, scale);
		glm_translate_z(mvp, -.5f);
		glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)mvp);

		/* Draw it, this assumes the above work is done, which it is :) */
		draw_patch(scene->patches + p);
	}

	/* Setup helper geometry shader */
	glDisable(GL_DEPTH_TEST);
	glUseProgram(scene->help_shader.program);
	loc = glGetUniformLocation(scene->help_shader.program, "MVP");

	glm_translate_to(scene->camera.pv, scene->help_pos, mvp);
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)mvp);

	/* Draw helper geometry */
	glBindVertexArray(scene->help_vao);
	glDrawArrays(GL_LINE_LOOP, 0, 4);
	glDrawArrays(GL_LINES, 4, 6);
}

/*****************************/
void update_scene(Scene* scene, double dTime)
{
	if(
		/* If the camera is moving or rotating, update it */
		scene->cam_dest[0] != scene->cam_pos[0] ||
		scene->cam_dest[1] != scene->cam_pos[1] ||
		scene->cam_dest[2] != scene->cam_pos[2] ||
		scene->cam_rotating)
	{
		update_camera(scene, dTime);
	}
}

/*****************************/
void scene_framebuffer_size_callback(Scene* scene, int width, int height)
{
	float aspect = (float)width / (float)height;

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
		scene->cam_dest[1] += scene->patch_size-1;
	if(key == GLFW_KEY_S && action == GLFW_PRESS)
		scene->cam_dest[1] -= scene->patch_size-1;
	if(key == GLFW_KEY_A && action == GLFW_PRESS)
		scene->cam_dest[0] -= scene->patch_size-1;
	if(key == GLFW_KEY_D && action == GLFW_PRESS)
		scene->cam_dest[0] += scene->patch_size-1;

	/* Move the helper geometry */
	if(key == GLFW_KEY_UP && action == GLFW_PRESS)
		scene->help_pos[1] += scene->patch_size-1;
	if(key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		scene->help_pos[1] -= scene->patch_size-1;
	if(key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		scene->help_pos[0] -= scene->patch_size-1;
	if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		scene->help_pos[0] += scene->patch_size-1;

	/* Add a new patch */
	if(key == GLFW_KEY_ENTER && action == GLFW_PRESS)
	{
		PatchModifier mods[] = {
			//mod_flatten,
			//mod_stats,
			//mod_relax_slope_1d,
			//mod_flatten,
			//mod_stats,
			mod_stats,
			mod_relax_slope,
			mod_stats,
			NULL
		};

		add_patch(scene, gen_mpd, mods);
	}
}
