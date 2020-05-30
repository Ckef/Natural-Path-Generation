
#include "patch.h"
#include <stdlib.h>

/*****************************/
int gen_white_noise(unsigned int size, Vertex* data)
{
	/* Make a plane with random values ranging from 0 to 1 */
	unsigned int i;
	for(i = 0; i < size * size; ++i)
		data[i].h = rand() / (float)RAND_MAX;

	return 1;
}
