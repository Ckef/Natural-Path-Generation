
#include "include.h"
#include "input.h"
#include "output.h"
#include "patch.h"
#include "shader.h"

/*****************************/
int populate(unsigned int size, float* data, void* opt)
{
	/* Make a flat plane */
	for(unsigned i = 0; i < size * size; ++i)
		data[i] = 0.0f;

	return 1;
}

/*****************************/
int main(int argc, char* argv[])
{
	/* Initialize GLFW */
	if(!glfwInit())
	{
		throw_error("GLFW Initialization failed.");
		return 0;
	}

	glfwSetErrorCallback(error_callback);
	output("GLFW Initialization succesful.");

	/* Create a window */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	GLFWwindow* win = glfwCreateWindow(900, 500, "Terrainz", NULL, NULL);

	if(!win)
	{
		throw_error("GLFW could not open a window.");
		goto terminate;
	}

	output("GLFW opened a window succesfully.");

	/* Load OpenGL context */
	glfwMakeContextCurrent(win);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	
	/* Initialize the window */
	glfwSwapInterval(1);
	glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
	glfwSetKeyCallback(win, key_callback);
	glfwSetMouseButtonCallback(win, mouse_button_callback);
	glfwSetCursorPosCallback(win, mouse_pos_callback);

	int width, height;
	glfwGetFramebufferSize(win, &width, &height);
	framebuffer_size_callback(win, width, height);

	/* Init a scene */
	Patch patch;
	create_patch(&patch, 2);
	populate_patch(&patch, populate, NULL);

	Shader shad;
	create_shader(&shad, "shaders/col.vert", "shaders/col.frag");

	/* Main loop */
	while(!glfwWindowShouldClose(win))
	{
		/* Process events + prepare drawing buffer */
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);

		/* Draw smth */
		shader_draw(&shad, &patch);

		/* Aaaaand finish. */
		glfwSwapBuffers(win);
	}

	/* Clean up the scene */
	destroy_shader(&shad);
	destroy_patch(&patch);

terminate:
	/* Terminate GLFW and exit */
	glfwDestroyWindow(win);
	glfwTerminate();

	output("GLFW Terminated.");

	return 0;
}
