
#include "deps.h"
#include "output.h"
#include "patch.h"
#include "scene.h"
#include <stdlib.h>

/*****************************/
int create_patch(Patch* patch, unsigned int size)
{
	/* Allocate CPU memory */
	size_t vertSize = sizeof(float) * size * size;
	glm_vec3_zero(patch->pos);
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
	glBufferData(GL_ARRAY_BUFFER, vertSize * 6, NULL, GL_STATIC_DRAW);

	/* So we have 3 floats for the position and 3 for the normal */
	GLsizei attrSize = sizeof(float) * 3;
	glBindVertexArray(patch->vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE, attrSize * 2, (GLvoid*)0);
	glVertexAttribPointer(
		1, 3, GL_FLOAT, GL_FALSE, attrSize * 2, (GLvoid*)(uintptr_t)attrSize);

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
			/* To the right being x, upwards being y: */
			/* bottom left, top left, bottom right */
			ind[i++] = c * size + r;
			ind[i++] = c * size + r + 1;
			ind[i++] = (c+1) * size + r;

			/* top left, top right, bottom right */
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
void draw_patch(Patch* patch)
{
	glBindVertexArray(patch->vao);
	size_t elems = 6 * (patch->size-1) * (patch->size-1);
	glDrawElements(GL_TRIANGLES, elems, GL_UNSIGNED_INT, (GLvoid*)0);
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
	size_t vertSize = sizeof(float) * patch->size * patch->size * 6;
	float* data = malloc(vertSize);

	if(data == NULL)
	{
		throw_error("Failed to allocate memory to generate vertices.");
		return 0;
	}

	/* Now fill the vertex buffer */
	/* Again, assumes column-major */
	/* Columns = x, rows = y, height = z */
	unsigned int c, r;
	for(c = 0; c < patch->size; ++c)
		for(r = 0; r < patch->size; ++r)
		{
			unsigned int i = c * patch->size + r;

			/* x, y and z */
			data[i*6+0] = c;
			data[i*6+1] = r;
			data[i*6+2] = patch->data[i];

			/* Zero the associated normal */
			glm_vec3_zero(data + (i*6+3));
		}

	/* Calculate us some normal data */
	/* Loop over all triangles and accumulate the normals */
	for(c = 0; c < (patch->size-1); ++c)
		for(r = 0; r < (patch->size-1); ++r)
		{
			unsigned int iBL = c * patch->size + r;
			unsigned int iTL = c * patch->size + r + 1;
			unsigned int iBR = (c+1) * patch->size + r;
			unsigned int iTR = (c+1) * patch->size + r + 1;

			/* Note we musn't forget to scale the height by the patch height */
			vec3 x1 = { 1, 0, PATCH_HEIGHT * (data[iBR*6+2] - data[iBL*6+2]) };
			vec3 y1 = { 0, 1, PATCH_HEIGHT * (data[iTL*6+2] - data[iBL*6+2]) };
			vec3 x2 = { -1, 0, PATCH_HEIGHT * (data[iTL*6+2] - data[iTR*6+2]) };
			vec3 y2 = { 0, -1, PATCH_HEIGHT * (data[iBR*6+2] - data[iTR*6+2]) };

			vec3 normal;
			glm_vec3_crossn(x1, y1, normal);
			glm_vec3_add(normal, data + (iBL*6+3), data + (iBL*6+3));
			glm_vec3_add(normal, data + (iTL*6+3), data + (iTL*6+3));
			glm_vec3_add(normal, data + (iBR*6+3), data + (iBR*6+3));

			glm_vec3_crossn(x2, y2, normal);
			glm_vec3_add(normal, data + (iTL*6+3), data + (iTL*6+3));
			glm_vec3_add(normal, data + (iBR*6+3), data + (iBR*6+3));
			glm_vec3_add(normal, data + (iTR*6+3), data + (iTR*6+3));
		}

	/* Now just normalize 'm all */
	for(c = 0; c < patch->size; ++c)
		for(r = 0; r < patch->size; ++r)
		{
			unsigned int i = c * patch->size + r;
			glm_vec3_normalize(data + (i*6+3));
		}

	glBindBuffer(GL_ARRAY_BUFFER, patch->vertices);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertSize, data);
	free(data);

	return 1;
}
