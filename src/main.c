
//#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <stdio.h>

int main(int argc, char* argv[])
{
	if(!glfwInit())
	{
		printf("GLFW Initialization failed.\n");
	}

	glfwTerminate();

	return 0;
}
