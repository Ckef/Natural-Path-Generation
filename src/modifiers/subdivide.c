
#include "output.h"
#include "patch.h"
#include "scene.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>

/* Hardcoded path parameters for now */
#define PATH_RADIUS  2.2f
#define COST_LIN     10000
#define COST_POW     1.8f

/* Some macros to make A* a bit easier */
/* Firstly accessing data of a node */
#define I(n)     (n.c * size + n.r) /* Index of a node into data */
#define PREV(n)  AND[I(n)].prev     /* The node from which we got to a node */
#define COST(n)  AND[I(n)].cost     /* Cost of the path to a node */
#define SCORE(n) AND[I(n)].score    /* Score of a node */

/* Compare two nodes */
#define EQUAL(a,b) (a.c == b.c && a.r == b.r)

/* The L2 distance (i.e. Euclidean ground distance) from one node to another */
#define D(a,b) sqrtf(((int)a.c-(int)b.c)*((int)a.c-(int)b.c) + ((int)a.r-(int)b.r)*((int)a.r-(int)b.r))

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
static void heapify_up(
	AHeap*       heap,
	unsigned int i,
	unsigned int size,
	ANodeData*   AND)
{
	/* Check if its smaller than its parent */
	unsigned int p = (i-1) >> 1;

	if(i > 0 && SCORE(heap->data[i]) < SCORE(heap->data[p]))
	{
		/* Now swap i with its parent */
		ANode t = heap->data[i];
		heap->data[i] = heap->data[p];
		heap->data[p] = t;

		/* And heapify the parent i's now at */
		heapify_up(heap, p, size, AND);
	}
}

/*****************************/
static void heapify_down(
	AHeap*       heap,
	unsigned int i,
	unsigned int size,
	ANodeData*   AND)
{
	/* So get the smallest node of i and its two children */
	unsigned int s = i;
	unsigned int l = (i << 1) + 1;
	unsigned int r = (i << 1) + 2;

	if(l < heap->size && SCORE(heap->data[l]) < SCORE(heap->data[s]))
		s = l;
	if(r < heap->size && SCORE(heap->data[r]) < SCORE(heap->data[s]))
		s = r;

	if(s != i)
	{
		/* Now swap i with the smallest child */
		ANode t = heap->data[i];
		heap->data[i] = heap->data[s];
		heap->data[s] = t;

		/* And heapify the child i's at now */
		heapify_down(heap, s, size, AND);
	}
}

/*****************************/
static void flag_ellipse(
	unsigned int size,
	Vertex*      data,
	ANode        center,
	float        rx,
	float        ry,
	int          flags)
{
	/* Loop over all vertices in its bounding box */
	/* Yeah we're using signed integers... uum could be bettter? */
	int c, r;
	for(c = (int)(-rx); c <= (int)rx; ++c)
		for(r = (int)(-ry); r <= (int)ry; ++r)
		{
			/* Validate the node */
			int cc = (int)center.c + c;
			int rr = (int)center.r + r;

			if(cc < 0 || cc >= (int)size || rr < 0 || rr >= (int)size)
				continue;

			/* Now check if it's inside our ellipse */
			float d = (c*(float)c) / (rx*rx) + (r*(float)r) / (ry*ry);
			if(d <= 1)
				data[cc * size + rr].flags |= flags;
		}
}

/*****************************/
static int find_path(
	unsigned int size,
	Vertex*      data,
	ANode        start,
	ANode        goal)
{
	float scale = GET_SCALE(size);

	/* So we're just gonna A* this bitch */
	/* L2 distance is used as heuristic */
	/* First we need to keep track of a bunch of things per node */
	ANodeData* AND = malloc(sizeof(ANodeData) * size * size);
	if(AND == NULL)
	{
		throw_error("Failed to allocate memory for A* data.");
		return 0;
	}

	/* Initialize the cost of all nodes to max */
	/* Also set the previous node to undefined (i.e. >= size of terrain) */
	unsigned int i;
	for(i = 0; i < size * size; ++i)
	{
		AND[i].prev.c = size;
		AND[i].prev.r = size;
		AND[i].cost = FLT_MAX;
	}

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
	Q.data[0] = start;
	PREV(start) = start;
	COST(start) = 0;
	SCORE(start) = D(start, goal) * scale;

	/* Now keep iterating over the frontier of discovered nodes */
	/* Each time, get the one with the lowest score */
	/* Which is the first node in the min-heap :) */
	while(Q.size > 0)
	{
		ANode u = Q.data[0];

		/* If we've reached the goal, hurray! */
		if(EQUAL(u, goal))
		{
			/* Flag the path from start to goal */
			while(!EQUAL(u, start))
			{
				flag_ellipse(size, data, u, PATH_RADIUS, PATH_RADIUS, F_SLOPE);
				u = PREV(u);
			}

			/* Don't forget to color the start */
			flag_ellipse(size, data, start, PATH_RADIUS, PATH_RADIUS, F_SLOPE);

			free(Q.data);
			free(AND);
			return 1;
		}

		/* Remove the node from the heap */
		Q.data[0] = Q.data[--Q.size];
		heapify_down(&Q, 0, size, AND);

		/* Loop over its neighbors */
		/* Again signed integers... ? */
		int c, r;
		for(c = (int)u.c-1; c <= (int)u.c+1; ++c)
			for(r = (int)u.r-1; r <= (int)u.r+1; ++r)
			{
				ANode v = { .c = c, .r = r};

				/* Skip if its outside the terrain */
				if(c < 0 || c >= (int)size || r < 0 || r >= (int)size)
					continue;

				/* Skip if its equal to its parent */
				if(EQUAL(u,v))
					continue;

				/* Calculate the cost for this neighbor */
				/* So basically the cost from u to v is: */
				/* distance + slope cost * distance */
				/* Where the slope cost has a power and linear component */
				float dist = D(u,v) * scale;
				float slope =
					(data[v.c * size + v.r].h -
					data[u.c * size + u.r].h) / dist;

				slope = slope > 0 ? slope : -slope;
				float alt = COST(u) + dist * (1 + powf(slope, COST_POW) * COST_LIN);

				/* If the alternative cost is smaller, set new path */
				if(alt < COST(v))
				{
					COST(v) = alt;
					SCORE(v) = alt + D(v, goal) * scale;

					/* Add the neighbor to the heap if it hasn't been in there yet */
					/* We check this by the previous node we came from */
					/* If it is undefined, it was apparently never reached yet */
					if(PREV(v).c >= size || PREV(v).r >= size)
					{
						Q.data[Q.size++] = v;
						heapify_up(&Q, Q.size-1, size, AND);
					}

					/* Now we can update the path */
					PREV(v) = u;
				}
			}
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
