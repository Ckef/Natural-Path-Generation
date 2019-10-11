
#ifndef INPUT_H
#define INPUT_H

/* Main header includes */
#include <include.h>

/**
 * Error callback.
 *
 * @param  error        An error code.
 * @param  description  UTF-8 encoded string describing the error.
 */
void error_callback(int error, const char* description);

/**
 * Framebuffer resize callback.
 *
 * @param window  Window in question.
 * @param width   New width in pixels.
 * @param height  New height in pixels.
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

/**
 * Key callback.
 *
 * @param  window    Window in question.
 * @param  key       Key being pressed.
 * @param  scancode  Scancode of the key.
 * @param  action    Press, release or repeat.
 * @param  mods      Modifier keys being held down.
 */
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

/**
 * Mouse button callback.
 *
 * @param  window  Window in question.
 * @param  button  Button being pressed.
 * @param  action  Press or release.
 * @param  mods    Modifier keys being held down.
 */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

/**
 * Mouse position callback.
 *
 * @param  window  Window in question.
 * @param  x       X position from the left.
 * @param  y       Y position from the top..
 */
void mouse_pos_callback(GLFWwindow* window, double x, double y);


#endif
