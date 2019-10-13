
#include "include.h"
#include "output.h"
#include "scene.h"

/*****************************/
static int populate(unsigned int size, float* data, void* opt)
{
	/* Make a flat plane */
	for(unsigned i = 0; i < size * size; ++i)
		data[i] = 0.0f;

	return 1;
}

/*****************************/
int create_scene(Scene* scene)
{
	if(!create_patch(&scene->patch, PATCH_SIZE))
	{
		throw_error("Could not create patch for scene.");
		return 0;
	}

	if(!create_shader(&scene->shader, "shaders/col.vert", "shaders/col.frag"))
	{
		throw_error("Could not create shader for scene.");
		destroy_patch(&scene->patch);
		return 0;
	}

	populate_patch(&scene->patch, populate, NULL);

	/* Setup camera */
	vec3 eye    = {-30,-30,40};
	vec3 center = {(PATCH_SIZE-1)/2.0f,(PATCH_SIZE-1)/2.0f,0};
	vec3 up     = {0,0,1};

	glm_lookat(eye, center, up, scene->camera.view);
	scene_set_aspect(scene, 1.0f);

	return 1;
}

/*****************************/
void destroy_scene(Scene* scene)
{
	destroy_patch(&scene->patch);
	destroy_shader(&scene->shader);
}

/*****************************/
void scene_set_aspect(Scene* scene, float aspect)
{
	/* Calculate new projection matrix and update pv */
	glm_perspective(CAM_FOV, aspect, CAM_NEAR, CAM_FAR, scene->camera.proj);
	glm_mat4_mul(scene->camera.proj, scene->camera.view, scene->camera.pv);
}

/*****************************/
void draw_scene(Scene* scene)
{
	/* Setup shader */
	glUseProgram(scene->shader.program);

	mat4 mvp;
	glm_mat4_mul(scene->camera.pv, scene->patch.mod, mvp);

	GLint loc = glGetUniformLocation(scene->shader.program, "MVP");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)mvp);

	/* Draw */
	glBindVertexArray(scene->patch.vao);
	size_t elems = 6 * (scene->patch.size-1) * (scene->patch.size-1);
	glDrawElements(GL_TRIANGLES, elems, GL_UNSIGNED_INT, (GLvoid*)0);
}
