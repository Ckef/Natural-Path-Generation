
#include "output.h"
#include "patch.h"
#include <stdio.h>

/* Hardcoded output file for now */
#define OUT_FILE "terrain_out.json"

/*****************************/
int mod_output(unsigned int size, Vertex* data, ModData* mod)
{
	/* Open the file */
	FILE* f = fopen(OUT_FILE, "w");
	if(f == NULL)
	{
		throw_error("Could not open file: %s", OUT_FILE);
		return 0;
	}

	/* Output the terrain to the file */
	/* We output it as a matrix, so [ [...],[...],...,[...] ] */
	/* Note this is just a JSON array :) */
	fputs("[\n", f);

	/* The output is column-major */
	/* In practice this means the first row is actually the first column of the terrain */
	unsigned int c, r;
	for(c = 0; c < size; ++c)
	{
		fputs("[ ", f);

		for(r = 0; r < size; ++r)
			fprintf(f, r == size-1 ? "%f " : "%f, ", data[c * size + r].h);

		fputs(c == size-1 ? "]\n" : "],\n", f);
	}

	fputs("]", f);
	fclose(f);

	/* Print that the terrain has been written to the file correctly */
	output("Terrain has been written to file: %s", OUT_FILE);

	/* We don't need to iterate this modifier */
	mod->done = 1;
	return 1;
}
