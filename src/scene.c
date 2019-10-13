
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
	if(!create_patch(&scene->patch, 10))
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
	glm_mat4_identity(scene->camera.pv);

	return 1;
}

/*****************************/
void destroy_scene(Scene* scene)
{
	destroy_patch(&scene->patch);
	destroy_shader(&scene->shader);
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
