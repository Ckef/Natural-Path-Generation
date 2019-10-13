
#include "include.h"
#include "io.h"
#include "scene.h"

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

	/* Initialize OpenGL context */
	glfwMakeContextCurrent(win);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glClearColor(.7f, .7f, .7f, 1.0f);
	
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
	Scene scene;
	if(!create_scene(&scene))
	{
		throw_error("Could not create a scene.");
		goto terminate;
	}

	output("Scene was created succesfully.");

	/* Main loop */
	while(!glfwWindowShouldClose(win))
	{
		/* Process events + prepare drawing buffer */
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);

		/* Draw smth */
		draw_scene(&scene);

		/* Aaaaand finish. */
		glfwSwapBuffers(win);
	}

	/* Clean up the scene */
	destroy_scene(&scene);

terminate:
	/* Terminate GLFW and exit */
	glfwDestroyWindow(win);
	glfwTerminate();

	output("GLFW Terminated.");

	return 0;
}
