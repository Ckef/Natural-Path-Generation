
/* Main header includes */
#include <include.h>

#include <output.h>

/*****************************/
void error_callback(int error, const char* description)
{
	throw_error(description);
}

/*****************************/
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

/*****************************/
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/* All actions */
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

/*****************************/
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
}

/*****************************/
void mouse_pos_callback(GLFWwindow* window, double x, double y)
{
}
