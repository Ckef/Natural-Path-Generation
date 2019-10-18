
#include <math.h>
#include <stdlib.h>

/*****************************/
int gen_white_noise(unsigned int size, float* data, void* opt)
{
	/* Make a plane with random values ranging from 0 to 1 */
	for(unsigned int i = 0; i < size * size; ++i)
		data[i] = rand() / (float)RAND_MAX;

	return 1;
}
