
#include "deps.h"
#include "output.h"
#include "scene.h"
#include <stdlib.h>

/* Window dimensions */
#define WINDOW_WIDTH   1200
#define WINDOW_HEIGHT  600

/* Scene to forward input callbacks to */
static Scene* active_scene = NULL;

/*****************************/
static void error_callback(int error, const char* description)
{
	throw_error(description);
}

/*****************************/
static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	/* Set viewport */
	glViewport(0, 0, width, height);

	/* Pass it to the active scene */
	if(active_scene != NULL)
		scene_framebuffer_size_callback(active_scene, width, height);
}

/*****************************/
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/* Close window on escape */
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	/* Pass it to the active scene */
	if(active_scene != NULL)
		scene_key_callback(active_scene, key, action, mods);
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

	GLFWwindow* win = glfwCreateWindow(
		WINDOW_WIDTH, WINDOW_HEIGHT, "Terrainz", NULL, NULL);

	if(!win)
	{
		throw_error("GLFW could not open a window.");
		goto terminate;
	}

	output("GLFW opened a window succesfully.");

	/* Initialize OpenGL context */
	glfwMakeContextCurrent(win);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	/* Initialize the window */
	glfwSwapInterval(1);
	glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
	glfwSetKeyCallback(win, key_callback);

	/* Create a scene */
	/* First see if an input mode and patch size was given */
	/* The first parameter determines the mode */
	/* s = sequential */
	/* p = parallel */
	/* g = gpu (parallel) */
	/* The second parameter is a number for the grid size */
	Scene scene;
	ModMode mode = PARALLEL;
	unsigned int pSize = 0;

	if(argc > 1) mode =
		argv[1][0] == 's' ? SEQUENTIAL :
		argv[1][0] == 'p' ? PARALLEL :
		argv[1][0] == 'g' ? GPU :
		mode;
	if(argc > 2)
		pSize = atoi(argv[2]);

	if(!create_scene(&scene, mode, pSize))
	{
		throw_error("Could not create a scene.");
		goto terminate;
	}

	output("Scene was created succesfully.");

	/* Set scene as active so it receives input callbacks */
	active_scene = &scene;

	/* Now let the aspect ratio be triggered by a forced framebuffer resize */
	int width, height;
	glfwGetFramebufferSize(win, &width, &height);
	framebuffer_size_callback(win, width, height);

	/* Main loop */
	double time = glfwGetTime();

	while(!glfwWindowShouldClose(win))
	{
		/* Draw smth */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw_scene(&scene);
		glfwSwapBuffers(win);

		/* Update scene */
		double newTime = glfwGetTime();
		update_scene(&scene, newTime - time);
		time = newTime;

		/* Process events */
		glfwPollEvents();
	}

	/* Clean up the scene */
	destroy_scene(&scene);
	active_scene = NULL;

terminate:
	/* Terminate GLFW and exit */
	glfwDestroyWindow(win);
	glfwTerminate();

	output("GLFW Terminated.");

	return 0;
}
