
#ifndef GENERATORS_H
#define GENERATORS_H

/**
 * Generates a white noise pattern.
 */
int gen_white_noise(unsigned int size, float* data, void* opt);

/**
 * Generates smth using the diamond-square midpoint displacement algorithm.
 * Note: size must be of the form 2^N+1.
 */
int gen_mpd(unsigned int size, float* data, void* opt);


#endif
