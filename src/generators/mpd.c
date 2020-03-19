
#include <stdlib.h>

/*****************************/
int gen_mpd(unsigned int size, float* data, void* opt)
{
	/* Initialize corners */
	data[0]             = .5f;
	data[size-1]        = .5f;
	data[size*size-1]   = .5f;
	data[size*(size-1)] = .5f;

	/* Iterate over all step sizes, i.e. 'frequencies' */
	float scale = 1.0f;
	unsigned int step;
	for(step = size-1; step > 1; step /= 2, scale /= 2)
	{
		/* Iterate over all squares */
		unsigned int c, r;
		for(c = 0; c < (size-1); c += step)
			for(r = 0; r < (size-1); r += step)
			{
				unsigned int tl = c * size + r;
				unsigned int bl = c * size + r+step;
				unsigned int tr = (c+step) * size + r;
				unsigned int br = (c+step) * size + r+step;
				unsigned int cent = (c+step/2) * size + r+step/2;

				/* Set a new center point */
				float val = data[tl] + data[bl] + data[tr] + data[br];
				data[cent] = val/4 + scale * (rand() / (float)RAND_MAX - .5f);
			}

		/* Iterate over all diamonds */
		unsigned int i;
		for(i = 0, c = 0; c < size; c += step/2, i ^= 1)
			for(r = i ? 0 : step/2; r < size; r += step)
			{
				unsigned int le = (c-step/2) * size + r;
				unsigned int ri = (c+step/2) * size + r;
				unsigned int to = c * size + r-step/2;
				unsigned int bo = c * size + r+step/2;
				unsigned int cent = c * size + r;

				/* Set a new center point */
				float val = 0;
				unsigned int a = 0;

				if(c > 0) val += data[le], ++a;
				if(r > 0) val += data[to], ++a;
				if(c < size-1) val += data[ri], ++a;
				if(r < size-1) val += data[bo], ++a;

				data[cent] = val/a + scale * (rand() / (float)RAND_MAX - .5f);
			}
	}

	return 1;
}
