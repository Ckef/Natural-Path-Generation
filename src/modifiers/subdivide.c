
#include "patch.h"

/*****************************/
int mod_subdivide(unsigned int size, Vertex* data, ModData* mod)
{
	/* Set half of the terrain to have a gradient constraint */
	/* TODO: Do something useful here */
	unsigned int i;
	for(i = 0; i < (size * size >> 1); ++i)
		data[i].flags = 1;

	/* We don't need to iterate this modifier */
	mod->done = 1;
	return 1;
}
