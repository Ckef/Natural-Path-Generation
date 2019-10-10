
#ifndef OUTPUT_H
#define OUTPUT_H

/**
 * Outputs a string to the output system.
 *
 * @param  description  Optional null terminated message string (can be NULL).
 *
 * The description will be formatted according to *printf format specification.
 *
 */
void output(const char* description, ...);

/**
 * Outputs an error to the output system.
 *
 * @param  description  Optional null terminated message string (can be NULL).
 *
 * The description will be formatted according to *printf format specification.
 *
 */
void throw_error(const char* description, ...);


#endif
