
#include "include.h"
#include "output.h"
#include <stdarg.h>
#include <stdio.h>

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

/*****************************/
void output(const char* description, ...)
{
	/* Format the description */
	va_list vl;
	va_start(vl, description);

	fprintf(stdout, "-- ");
	vfprintf(stdout, description, vl);

	va_end(vl);

	/* Append newline and flush */
	fputc('\n', stdout);
	fflush(stdout);
}

/*****************************/
void throw_error(const char* description, ...)
{
	/* Format the error */
	va_list vl;
	va_start(vl, description);

	fprintf(stderr, "ERROR -- ");
	vfprintf(stderr, description, vl);

	va_end(vl);

	/* Append newline and flush */
	fputc('\n', stderr);
	fflush(stderr);
}
