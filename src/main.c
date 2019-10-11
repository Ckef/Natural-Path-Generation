
/* Main header includes */
#include <include.h>

#include <input.h>
#include <output.h>
#include <stdio.h>

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
	
	/* Initialize the window */
	glfwSwapInterval(1);
	glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
	glfwSetKeyCallback(win, key_callback);
	glfwSetMouseButtonCallback(win, mouse_button_callback);
	glfwSetCursorPosCallback(win, mouse_pos_callback);

	int width;
	int height;
	glfwGetFramebufferSize(win, &width, &height);
	framebuffer_size_callback(win, width, height);

	/* Main loop */
	while(!glfwWindowShouldClose(win))
	{
		glfwPollEvents();
		glfwSwapBuffers(win);
	}

terminate:
	/* Terminate GLFW and exit */
	glfwDestroyWindow(win);
	glfwTerminate();

	output("GLFW Terminated.");

	return 0;
}
