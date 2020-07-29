
#include "constants.h"
#include "deps.h"
#include "generators.h"
#include "modifiers.h"
#include "output.h"
#include "scene.h"
#include <stdlib.h>
#include <string.h>

/*****************************/
static unsigned int get_min_grid_size(int x, int y)
{
	/* So we get the position in the relevant quadrant */
	x = abs(x) - (x < 0 ? 1 : 0);
	y = abs(y) - (y < 0 ? 1 : 0);

	/* And check how large it must be */
	return (x > y ? x : y) + 1;
}

/*****************************/
static size_t get_grid_index(unsigned int gridSize, int x, int y)
{
	/* We're using 4 column-major quadrants, interlaced into the same data array */
	/* Each quadrant has the same dimensions and are square */
	/* The upper right quadrant is 0, we rotate clockwise up to 3 */
	/* To get an index, we first get the quadrant we're in */
	unsigned int sx = (x < 0) ? 1 : 0;
	unsigned int sy = (y < 0) ? 1 : 0;
	unsigned int quadrant = (sx && sy) ? 2 : sx ? 3 : sy ? 1 : 0;

	/* Then compute the position in the interlaced array */
	x = abs(x) - sx;
	y = abs(y) - sy;

	return (x * gridSize + y) * 4 + quadrant;
}

/*****************************/
static int add_patch(
	Scene*         scene,
	PatchGenerator generator,
	PatchModifier* mods,
	const char**   outs)
{
	/* First check if we have enough memory */
	int x = scene->help_pos[0];
	int y = scene->help_pos[1];
	unsigned int minGridSize = get_min_grid_size(x, y);

	if(minGridSize > scene->grid_size)
	{
		/* Allocate more memory, multiply by 4 for all quadrants */
		/* Again, interlaced data array :) */
		size_t newSize = minGridSize * minGridSize * 4;
		Patch* new = realloc(scene->patches, newSize * sizeof(Patch));

		if(!new)
		{
			throw_error("Could not reallocate memory for a new patch.");
			return 0;
		}

		/* Set memory to zero for the newly allocated spots so is_patch returns 0 */
		size_t oldSize = scene->grid_size * scene->grid_size * 4;
		memset(new + oldSize, 0, (newSize - oldSize) * sizeof(Patch));

		if(scene->grid_size)
		{
			/* We need to move all the current patches */
			/* Start iterating from the back so we don't override existing patches */
			/* Also note we don't need to move the first column */
			unsigned int ix, iy;
			for(ix = scene->grid_size - 1; ix > 0; --ix)
				for(iy = scene->grid_size; iy > 0; --iy)
				{
					/* Move to the new location and set the old one to 0's */
					/* Just copy all quadrants together btw */
					Patch* p = new + get_grid_index(scene->grid_size, ix, iy-1);
					Patch* np = new + get_grid_index(minGridSize, ix, iy-1);

					memcpy(np, p, sizeof(Patch) * 4);
					memset(p, 0, sizeof(Patch) * 4);
				}
		}

		scene->patches = new;
		scene->grid_size = minGridSize;
	}

	/* Get the index of the new patch */
	Patch* p = scene->patches + get_grid_index(scene->grid_size, x, y);

	/* Check if there was already a patch */
	if(is_patch(p))
	{
		output("Patch cannot be placed on top of another patch.");
		return 0;
	}

	/* Create a new patch */
	if(!create_patch(p, scene->patch_mode, scene->patch_size))
	{
		throw_error("Could not create a new patch for scene.");
		return 0;
	}

	/* Set its position to the current selection */
	p->pos[0] = x * (DEF_PATCH_SIZE-1);
	p->pos[1] = y * (DEF_PATCH_SIZE-1);
	p->pos[2] = 0;

	/* Get the local neighbourhood of the patch */
	/* We're just looping over it and checking if there is a patch */
	Patch* local[9];
	memset(local, 0, sizeof(Patch*) * 9);

	int c, r;
	for(c = -1; c <= 1; ++c)
		for(r = -1; r <= 1; ++r)
		{
			unsigned int i = get_grid_index(scene->grid_size, x + c, y + r);
			Patch* pn = scene->patches + i;

			if(i < scene->grid_size * scene->grid_size * 4)
				if(is_patch(pn)) local[(c+1)*3+(r+1)] = pn;
		}

	/* Populate the new patch */
	if(!populate_patch(p, generator, mods, outs, local))
	{
		throw_error("Population of newly created patch failed.");
		destroy_patch(p);
		return 0;
	}

	/* Yep done. */
	output("Patch was created succesfully.");

	return 1;
}

/*****************************/
static void update_camera(Scene* scene, double dTime)
{
	glm_mat4_identity(scene->camera.view);

	vec3 mov;
	vec3 x = {1,0,0};
	vec3 y = {0,1,0};
	vec3 z = {0,0,1};
	vec3 back = {0,-40, -40};
	vec3 cent = {(DEF_PATCH_SIZE-1)/2.0f, (DEF_PATCH_SIZE-1)/2.0f, 0};

	/* Update position */
	vec3 dest = {
		scene->cam_dest[0] * (DEF_PATCH_SIZE-1),
		scene->cam_dest[1] * (DEF_PATCH_SIZE-1),
		scene->cam_dest[2] * (DEF_PATCH_SIZE-1)};

	glm_vec3_copy(dest, mov);
	glm_vec3_sub(mov, scene->cam_pos, mov);
	float ml = glm_vec3_norm(mov);

	if(CAM_SPEED * dTime >= ml)
		glm_vec3_copy(dest, scene->cam_pos);
	else
	{
		glm_vec3_scale(mov, CAM_SPEED * dTime / ml, mov);
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
int create_scene(Scene* scene, ModMode mode, unsigned int patchSize)
{
	if(patchSize < 2) patchSize = DEF_PATCH_SIZE;
	scene->patch_size = patchSize;
	scene->patch_mode = mode;

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
	/* The update_camera takes care of the view and pv matrices */
	memset(scene->cam_dest, 0, sizeof(scene->cam_dest));
	glm_mat4_identity(scene->camera.proj);
	glm_vec3_zero(scene->cam_pos);
	scene->cam_rotating = 0;
	scene->cam_angle = 0.0f;
	update_camera(scene, 0.0);

	/* Make sure to initialize the helper position here */
	memset(scene->help_pos, 0, sizeof(scene->help_pos));
	scene->patches = NULL;
	scene->grid_size = 0;

	/* Create helper geometry */
	/* Contains a square to indicate the selected patch */
	/* Plus the axes of the coordinate system */
	float help_geom[] = {
		0,                0,                0, .5f, .5f, .5f,
		DEF_PATCH_SIZE-1, 0,                0, .5f, .5f, .5f,
		DEF_PATCH_SIZE-1, DEF_PATCH_SIZE-1, 0, .5f, .5f, .5f,
		0,                DEF_PATCH_SIZE-1, 0, .5f, .5f, .5f,

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
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE, attrSize * 2, (GLvoid*)0);
	glVertexAttribPointer(
		2, 3, GL_FLOAT, GL_FALSE, attrSize * 2, (GLvoid*)(uintptr_t)attrSize);

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
	for(p = 0; p < scene->grid_size * scene->grid_size * 4; ++p)
		if(is_patch(scene->patches + p))
			destroy_patch(scene->patches + p);

	free(scene->patches);
}

/*****************************/
int scene_add_patch(Scene* scene)
{
	/* Define the modifiers */
	PatchModifier mods[] = {
		/* This first bit is if we want a 1D slope constraint example */
		/*mod_flatten,
		mod_stats,
		mod_relax_slope_1d,
		mod_flatten,
		mod_stats,*/
		mod_subdivide,
		mod_output,
		mod_output_flags,
		mod_output_constrs,
		mod_stats,
		mod_relax,
		mod_output,
		mod_stats,
		NULL
	};

	/* And its output files */
	const char* outs[] = {
		NULL,
		OUT_FILE_L,
		OUT_FILE_FLAGS,
		OUT_FILE_CONSTRS,
		OUT_FILE_STATS_L,
		OUT_FILE_ITERS,
		OUT_FILE_H,
		OUT_FILE_STATS_H
	};

	/* First place 4 surrounding patches if necessary */
	if(AUTO_SURROUND)
	{
		/* Left */
		scene->help_pos[0] -= 1;
		add_patch(scene, gen_mpd, NULL, NULL);

		/* Right */
		scene->help_pos[0] += 2;
		add_patch(scene, gen_mpd, NULL, NULL);

		/* Down */
		scene->help_pos[0] -= 1;
		scene->help_pos[1] -= 1;
		add_patch(scene, gen_mpd, NULL, NULL);

		/* Up */
		scene->help_pos[1] += 2;
		add_patch(scene, gen_mpd, NULL, NULL);

		scene->help_pos[1] -= 1;
	}

	/* Now place the main patch */
	if(scene->patch_mode == READ_FILE)
		return add_patch(scene, gen_file, NULL, NULL);
	else
		return add_patch(scene, gen_mpd, mods, outs);
}

/*****************************/
void draw_scene(Scene* scene)
{
	/* Setup patch shader */
	glEnable(GL_DEPTH_TEST);
	glUseProgram(scene->patch_shader.program);
	GLint loc = glGetUniformLocation(scene->patch_shader.program, "MVP");

	/* Loop over all patches */
	/* Scale the patch to the default size */
	float s = GET_SCALE(scene->patch_size);
	vec3 scale = {s, s, PATCH_HEIGHT};
	mat4 mvp;

	size_t p;
	for(p = 0; p < scene->grid_size * scene->grid_size * 4; ++p)
	{
		if(!is_patch(scene->patches + p))
			continue;

		/* The height (z-coord) of all patches is roughly in [0,1] */
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

	vec3 hpos = {
		scene->help_pos[0] * (DEF_PATCH_SIZE-1),
		scene->help_pos[1] * (DEF_PATCH_SIZE-1),
		0};

	glm_translate_to(scene->camera.pv, hpos, mvp);
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
		scene->cam_dest[0] * (DEF_PATCH_SIZE-1) != scene->cam_pos[0] ||
		scene->cam_dest[1] * (DEF_PATCH_SIZE-1) != scene->cam_pos[1] ||
		scene->cam_dest[2] * (DEF_PATCH_SIZE-1) != scene->cam_pos[2] ||
		scene->cam_rotating)
	{
		update_camera(scene, dTime);
	}

	/* Loop over all patches to update them */
	size_t p;
	for(p = 0; p < scene->grid_size * scene->grid_size * 4; ++p)
		if(is_patch(scene->patches + p))
			update_patch(scene->patches + p);
}

/*****************************/
int is_scene_done(Scene* scene)
{
	int done = 1;
	size_t p;
	for(p = 0; p < scene->grid_size * scene->grid_size * 4; ++p)
		done &= is_patch_done(scene->patches + p);

	return done;
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
		scene->cam_dest[1] += 1;
	if(key == GLFW_KEY_S && action == GLFW_PRESS)
		scene->cam_dest[1] -= 1;
	if(key == GLFW_KEY_A && action == GLFW_PRESS)
		scene->cam_dest[0] -= 1;
	if(key == GLFW_KEY_D && action == GLFW_PRESS)
		scene->cam_dest[0] += 1;

	/* Move the helper geometry */
	if(key == GLFW_KEY_UP && action == GLFW_PRESS)
		scene->help_pos[1] += 1;
	if(key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		scene->help_pos[1] -= 1;
	if(key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		scene->help_pos[0] -= 1;
	if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		scene->help_pos[0] += 1;

	/* Add a new patch */
	if(key == GLFW_KEY_ENTER && action == GLFW_PRESS)
		scene_add_patch(scene);
}
