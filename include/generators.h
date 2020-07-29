
#ifndef GENERATORS_H
#define GENERATORS_H

#include "patch.h"

/**
 * Generates a white noise pattern.
 */
int gen_white_noise(unsigned int size, Vertex* data);

/**
 * Generates smth using the diamond-square midpoint displacement algorithm.
 * Note: size must be of the form 2^N+1.
 */
int gen_mpd(unsigned int size, Vertex* data);

/**
 * Generator that reads terrain from file.
 */
int gen_file(unsigned int size, Vertex* data);


#endif
