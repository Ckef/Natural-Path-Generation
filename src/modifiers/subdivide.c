
#include "output.h"
#include "patch.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>

/* Some macros to make A* a bit easier */
/* Firstly accessing data of a node */
#define I(n)     (n.c * size + n.r) /* Index of a node into data */
#define PREV(n)  AND[I(n)].prev     /* The node from which we got to a node */
#define COST(n)  AND[I(n)].cost     /* Cost of the path to a node */
#define SCORE(n) AND[I(n)].score    /* Score of a node */

/* Compare two nodes */
#define EQUAL(a,b) (a.c == b.c && a.r == b.r)

/* And the heuristic, L2 distance */
#define H(a,b) sqrtf(((int)a.c-(int)b.c)*((int)a.c-(int)b.c) + ((int)a.r-(int)b.r)*((int)a.r-(int)b.r))

/* A* node */
typedef struct
{
	unsigned int c;
	unsigned int r;

} ANode;


/* A* node storage */
typedef struct
{
	ANode prev;
	float cost;  /* Cost to get here */
	float score; /* Cost + heuristic */

} ANodeData;


/* Min-Heap */
typedef struct
{
	ANode*       data;
	unsigned int size;

} AHeap;


/*****************************/
static void heapify(
	AHeap*       heap,
	unsigned int i,
	unsigned int size,
	ANodeData*   AND)
{
	/* So get the smallest node if i and its two children */
	unsigned int l = (i << 1) + 1;
	unsigned int r = (i << 1) + 2;
	unsigned int m = i;

	if(l < heap->size && SCORE(heap->data[l]) < SCORE(heap->data[m]))
		m = l;
	if(r < heap->size && SCORE(heap->data[r]) < SCORE(heap->data[m]))
		m = r;

	if(m != i)
	{
		/* Now swap i with the smallest child */
		ANode t = heap->data[i];
		heap->data[i] = heap->data[m];
		heap->data[m] = t;

		/* And heapify the child i's at now */
		heapify(heap, m, size, AND);
	}
}

/*****************************/
static int find_path(
	unsigned int size,
	Vertex*      data,
	ANode        start,
	ANode        goal)
{
	/* So we're just gonna A* this bitch */
	/* L2 distance is used as heuristic */
	/* First we need to keep track of a bunch of things per node */
	ANodeData* AND = malloc(sizeof(ANodeData) * size * size);
	if(AND == NULL)
	{
		throw_error("Failed to allocate memory for A* data.");
		return 0;
	}

	/* Initialize the score of all nodes to max */
	unsigned int i;
	for(i = 0; i < size * size; ++i)
		AND[i].cost = FLT_MAX;

	/* So we need a a min-heap to get nodes based on score */
	AHeap Q;
	Q.data = malloc(sizeof(ANode) * size * size);
	Q.size = 1;

	if(Q.data == NULL)
	{
		throw_error("Failed to allocate memory for a min-heap.");
		free(AND);
		return 0;
	}

	/* At first we only need the start node in the heap */
	PREV(start) = start;
	COST(start) = 0;
	SCORE(start) = H(start, goal);
	Q.data[0] = start;

	/* Now keep iterating over the frontier of discovered nodes */
	/* Each time, get the one with the lowest score */
	/* Which is the first node in the min-heap :) */
	while(Q.size > 0)
	{
		/* If we've reached the goal, hurray! */
		if(EQUAL(Q.data[0], goal))
		{
			/* TODO: Color the path from start to goal red */
			/* i.e. set their flags to 1 */
			free(Q.data);
			free(AND);
			return 1;
		}

		/* Remove the node from the heap */
		Q.data[0] = Q.data[--Q.size];
		heapify(&Q, 0, size, AND);

		/* Loop over its neighbors */
		/* TODO: well loop over its neighbors */
	}

	/* Free all the things */
	free(Q.data);
	free(AND);

	throw_error("Uh oh A* did not reach its goal.");
	return 0;
}

/*****************************/
int mod_subdivide(unsigned int size, Vertex* data, ModData* mod)
{
	/* Find a path from the lower left corner to the upper right corner */
	ANode s = { .c = 0,      .r = 0      };
	ANode g = { .c = size-1, .r = size-1 };

	if(!find_path(size, data, s, g))
		return 0;

	/* We don't need to iterate this modifier */
	mod->done = 1;
	return 1;
}
