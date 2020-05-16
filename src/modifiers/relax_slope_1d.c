
/* Hardcoded slope constraint for now */
#define MAX_SLOPE   0.005f
#define ITERATIONS  5000

/*****************************/
int mod_relax_slope_1d(unsigned int size, float* data, void* opt)
{
	/* Only modify the center column */
	float* mid = data + ((size >> 1) * size);

	/* Apply the relaxation process for ITERATIONS times */
	unsigned int i = ITERATIONS;
	while(i--)
	{
		/* Loop over all pairs of vertices */
		unsigned int r;
		for(r = 0; r < size-1; ++r)
		{
			/* The current slope and the indices */
			/* a is the lowest vertex, b the highest */
			float s = mid[r+1] - mid[r];
			int b = (s > 0);
			int a = 1-b;
			s = b ? s : -s;

			if(s > MAX_SLOPE)
			{
				/* If the slope is too great, move a and b closer to each other */
				/* Weigh the added difference by 1/2 */
				/* This weight is so all vertex pairs can be done in parallel */
				float move = (s - MAX_SLOPE) * .5f;
				mid[r+a] += move * .5f;
				mid[r+b] -= move * .5f;
			}
		}
	}

	return 1;
}
