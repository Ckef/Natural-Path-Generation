
#include "include.h"
#include "output.h"
#include "scene.h"

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
	/* Set viewport and aspect of the active camera */
	glViewport(0, 0, width, height);
	if(active_scene != NULL)
		scene_set_aspect(active_scene, (float)width/(float)height);
}

/*****************************/
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/* Close window on escape */
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

/*****************************/
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
}

/*****************************/
static void mouse_pos_callback(GLFWwindow* window, double x, double y)
{
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

	/* Create a scene */
	Scene scene;
	if(!create_scene(&scene))
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
	active_scene = NULL;

terminate:
	/* Terminate GLFW and exit */
	glfwDestroyWindow(win);
	glfwTerminate();

	output("GLFW Terminated.");

	return 0;
}
