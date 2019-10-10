
#define GLFW_INCLUDE_GLCOREARB
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <output.h>
#include <stdio.h>

/*****************************/
void error_callback(int error, const char* description)
{
	throw_error(description);
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
	GLFWwindow* win = glfwCreateWindow(640, 480, "Terrainz", NULL, NULL);

	if(!win)
	{
		throw_error("GLFW could not open a window.");
		goto terminate;
	}

	/* Load context */
	glfwMakeContextCurrent(win);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	/* Main loop */
	while(!glfwWindowShouldClose(win))
	{
		glfwSwapBuffers(win);
		glfwPollEvents();
	}

terminate:
	/* Terminate GLFW and exit */
	glfwDestroyWindow(win);
	glfwTerminate();

	output("GLFW Terminated.");

	return 0;
}
