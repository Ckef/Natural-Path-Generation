
#include "output.h"
#include "patch.h"
#include <stdio.h>

/* Hardcoded output files for now */
#define OUT_FILE_HEIGHT  "terrain_out.json"
#define OUT_FILE_FLAGS   "terrain_out_f.json"
#define OUT_FILE_CONSTRS "terrain_out_c.json"

/*****************************/
static void print_height(FILE* f, int comma, Vertex* v)
{
	fprintf(f, !comma ? "%f " : "%f, ", v->h);
}

/*****************************/
static void print_flags(FILE* f, int comma, Vertex* v)
{
	fprintf(f, !comma ? "%i " : "%i, ", v->flags);
}

/*****************************/
static void print_constrs(FILE* f, int comma, Vertex* v)
{
	fprintf(f, !comma ? "[%f,%f,%f] " : "[%f,%f,%f], ", v->c[0], v->c[1], v->c[2]);
}

/*****************************/
static int output_terrain(
	unsigned int size,
	Vertex*      data,
	void       (*printer)(FILE*, int, Vertex*),
	const char*  file)
{
	/* Open the file */
	FILE* f = fopen(file, "w");
	if(f == NULL)
	{
		throw_error("Could not open file: %s", file);
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
			printer(f, r < size-1, data + (c * size + r));

		fputs(c == size-1 ? "]\n" : "],\n", f);
	}

	fputs("]", f);
	fclose(f);

	/* Print that the terrain has been written to the file correctly */
	output("Terrain has been written to file: %s", file);

	return 1;
}

/*****************************/
int mod_output(unsigned int size, Vertex* data, ModData* mod)
{
	/* We don't need to iterate this modifier */
	mod->done = 1;
	return output_terrain(size, data, print_height, OUT_FILE_HEIGHT);
}

/*****************************/
int mod_output_flags(unsigned int size, Vertex* data, ModData* mod)
{
	mod->done = 1;
	return output_terrain(size, data, print_flags, OUT_FILE_FLAGS);
}

/*****************************/
int mod_output_constrs(unsigned int size, Vertex* data, ModData* mod)
{
	mod->done = 1;
	return output_terrain(size, data, print_constrs, OUT_FILE_CONSTRS);
}
