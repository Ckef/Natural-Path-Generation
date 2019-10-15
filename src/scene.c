
#include "include.h"
#include "output.h"
#include "scene.h"
#include <math.h>

/* Angle of the camera and whether it's rotating */
static float angle = 0.0f;
static int rotating = 0;

/*****************************/
static int populate(unsigned int size, float* data, void* opt)
{
	/* Make a flat plane */
	for(unsigned i = 0; i < size * size; ++i)
		data[i] = 0.0f;

	return 1;
}

/*****************************/
static void set_camera(Camera* camera, double dTime)
{
	glm_mat4_identity(camera->view);

	vec3 x    = {1,0,0};
	vec3 y    = {0,1,0};
	vec3 z    = {0,0,1};
	vec3 back = {0,0,-40};
	vec3 down = {0,-40,0};
	vec3 cent = {(PATCH_SIZE-1)/2.0f, (PATCH_SIZE-1)/2.0f, 0};

	/* Update angle */
	angle += dTime * rotating;

	/* Rotate camera down a bit, so it looks down at the patch */
	glm_rotate(camera->view, GLM_PI_4 * 0.5f, x);
	/* Move camera back a bit */
	glm_translate(camera->view, back);
	/* Move camera up a bit */
	glm_translate(camera->view, down);
	/* Rotate scene so camera is looking at patch */
	glm_rotate(camera->view, GLM_PI_4, y);
	/* Rotate scene so y is up */
	glm_rotate(camera->view, -GLM_PI_2, x);

	/* Translate patch back to its original position */
	glm_translate(camera->view, cent);
	/* Rotate patch around 0,0,0 */
	glm_rotate(camera->view, angle, z);
	/* Translate center of patch to 0,0,0 */
	glm_vec3_negate(cent);
	glm_translate(camera->view, cent);

	/* Don't forget to update the pv matrix */
	glm_mat4_mul(camera->proj, camera->view, camera->pv);
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
	/* The set_camera takes care of the view and pv matrices */
	glm_mat4_identity(scene->camera.proj);
	set_camera(&scene->camera, 0.0);

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

/*****************************/
void update_scene(Scene* scene, double dTime)
{
	if(rotating)
		set_camera(&scene->camera, dTime);
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
	if(key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		rotating = -1;
	if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		rotating = 1;
	if((key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) && action == GLFW_RELEASE)
		rotating = 0;
}
