
#include "patch.h"
#include <string.h>

/*****************************/
int mod_flatten(unsigned int size, float* data, ModData* mod)
{
	/* Copy the center column to all other columns */
	unsigned int mid = size >> 1;
	unsigned int c;
	for(c = 0; c < size; ++c)
		if(c != mid) memcpy(
			data + (c * size),
			data + (mid * size),
			sizeof(float) * size);

	/* We don't need to iterate this modifier */
	mod->done = 1;
	return 1;
}
