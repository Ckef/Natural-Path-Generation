
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

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

	output("GLFW Initialization succesful.");

	/* Terminate GLFW and exit */
	glfwTerminate();

	output("GLFW Terminated.");

	return 0;
}
