
#include "include.h"
#include "output.h"
#include "patch.h"
#include <stdlib.h>

/*****************************/
int create_patch(Patch* patch, unsigned int size)
{
	/* Allocate CPU memory */
	size_t vertSize = sizeof(float) * size * size;
	patch->size = size;
	patch->data = malloc(vertSize);

	if(patch->data == NULL)
	{
		throw_error("Failed to allocate memory for a patch.");
		return 0;
	}

	/* Allocate GPU memory and setup VAO */
	glGenVertexArrays(1, &patch->vao);
	glGenBuffers(1, &patch->vertices);
	glGenBuffers(1, &patch->indices);

	glBindBuffer(GL_ARRAY_BUFFER, patch->vertices);
	glBufferData(GL_ARRAY_BUFFER, vertSize * 3, NULL, GL_STATIC_DRAW);

	glBindVertexArray(patch->vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	/* Generate some index data */
	size_t indSize = sizeof(unsigned int) * 6 * (size-1) * (size-1);
	unsigned int* ind = malloc(indSize);

	if(ind == NULL)
	{
		throw_error("Failed to allocate memory to generate indices.");
		destroy_patch(patch);

		return 0;
	}

	/* Loop over all squares inbetween vertices, add two triangles */
	/* Assumes the vertices are stored column-major */
	unsigned int i, c, r;
	for(i = 0, c = 0; c < (size-1); ++c)
		for(r = 0; r < (size-1); ++r)
		{
			ind[i++] = c * size + r;
			ind[i++] = c * size + r + 1;
			ind[i++] = (c+1) * size + r;

			ind[i++] = c * size + r + 1;
			ind[i++] = (c+1) * size + r + 1;
			ind[i++] = (c+1) * size + r;
		}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, patch->indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize, ind, GL_STATIC_DRAW);
	free(ind);

	return 1;
}

/*****************************/
void destroy_patch(Patch* patch)
{
	glDeleteVertexArrays(1, &patch->vao);
	glDeleteBuffers(1, &patch->vertices);
	glDeleteBuffers(1, &patch->indices);

	free(patch->data);
}

/*****************************/
int populate_patch(Patch* patch, PatchGenerator generator, void* opt)
{
	/* Generate the terrain */
	if(!generator(patch->size, patch->data, opt))
	{
		throw_error("Could not populate patch due to faulty generation.");
		return 0;
	}

	/* Temporary buffer to generate vertex data */
	size_t vertSize = sizeof(float) * patch->size * patch->size * 3;
	float* data = malloc(vertSize);

	if(data == NULL)
	{
		throw_error("Failed to allocate memory to generate vertices.");
		return 0;
	}
	
	/* Now fill the vertex buffer */
	/* Again, assumes column-major */
	/* Columns = x, rows = -y, height = -z */
	unsigned int c, r;
	for(c = 0; c < patch->size; ++c)
		for(r = 0; r < patch->size; ++r)
		{
			unsigned int i = c * patch->size + r;

			/* x, y and z */
			data[i*3+0] = c;
			data[i*3+1] = patch->size - r - 1;
			data[i*3+2] = -patch->data[i];
		}

	glBindBuffer(GL_ARRAY_BUFFER, patch->vertices);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertSize, data);
	free(data);

	return 1;
}
