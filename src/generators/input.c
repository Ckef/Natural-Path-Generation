
#include "constants.h"
#include "output.h"
#include "patch.h"
#include <stdio.h>

/* Hardcoded input files for now... */
static const char* read_files[] = {
	OUT_FILE_L,
	OUT_FILE_H,
	IN_FILE_H_OPT
};

/* Index of the next file to read */
static unsigned int read_index = 0;


/*****************************/
int gen_file(unsigned int size, Vertex* data)
{
	/* Check if there are any files left to open */
	if(read_index >= sizeof(read_files) / sizeof(char*))
	{
		throw_error("No more known terrain files to read.");
		return 0;
	}

	/* Open the file */
	FILE* f = fopen(read_files[read_index], "r");
	if(f == NULL)
	{
		throw_error("Could not open file: %s", read_files[read_index]);
		return 0;
	}

	/* Also pretend we know what file to read for the flags */
	/* Do this so we have pretty colors :) */
	FILE* ff = fopen(OUT_FILE_FLAGS, "r");
	if(ff == NULL)
	{
		throw_error("Could not open file: %s", OUT_FILE_FLAGS);
		fclose(f);
		return 0;
	}

	/* Loop over all data points and try to read something for it */
	/* Yeah we could check for errors here but whatever */
	unsigned int i;
	for(i = 0; i < size*size; ++i)
	{
		fscanf(f, " %*[][, \n] %f", &(data[i].h));
		fscanf(ff, " %*[][, \n] %i", &(data[i].flags));
	}

	/* Go to next file */
	fclose(f);
	fclose(ff);
	++read_index;

	return 1;
}
