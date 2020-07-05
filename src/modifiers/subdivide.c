
#include "output.h"
#include "patch.h"
#include "scene.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>

/* Non-zero if we want to use the direction derivative, border, and border derivative constraints */
#define USE_DIR_SLOPE      0
#define USE_BORDER_STITCH  1
#define USE_BORDER_DERIV   0

/* Hardcoded path parameters for now */
/* The falloff is the ascend in the maximum slope the farther you get from the path boundary */
/* The influence is the distance from the path that the gradient constraint holds */
/* Maximum gradient ascends along the distance RADIUS + INFLUENCE */
#define MAX_SLOPE          0.0025f
#define MAX_SLOPE_FALLOFF  0.05f
#define PATH_RADIUS        2.2f
#define PATH_INFLUENCE     10.0f
#define COST_LIN           10000
#define COST_POW           1.8f

/* Some macros to make A* a bit easier */
/* Firstly accessing data of a node */
#define I(n)     ((n).c * size + (n).r) /* Index of a node into data */
#define PREV(n)  AND[I(n)].prev         /* The node from which we got to a node */
#define COST(n)  AND[I(n)].cost         /* Cost of the path to a node */
#define SCORE(n) AND[I(n)].score        /* Score of a node */

/* Compare two nodes */
#define EQUAL(a,b) ((a).c == (b).c && (a).r == (b).r)

/* The L2 distance (i.e. Euclidean ground distance) between two nodes */
#define D(a,b) hypotf((int)(a).c-(int)(b).c, (int)(a).r-(int)(b).r)


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
static void find_ellipse_intersect(
	float  a,
	float  b,
	float  px,
	float  py,
	float* ox,
	float* oy)
{
	/* Implementation to get the closest point on an ellipse from another point */
	/* I have no idea how it works, call it black magick */
	/* Blog post: */
	/*   https://wet-robots.ghost.io/simple-method-for-distance-to-ellipse/ */
	/* Stackoverflow (for optimizations): */
	/*   https://stackoverflow.com/a/46007540 */

	float tx = .707f;
	float ty = .707f;

	unsigned int i;
	for(i = 0; i < 3; ++i)
	{
		float ex = (a*a - b*b) * powf(tx, 3) / a;
		float ey = (b*b - a*a) * powf(ty, 3) / b;
		float rx = (a * tx) - ex;
		float ry = (b * ty) - ey;
		float qx = fabs(px) - ex;
		float qy = fabs(py) - ey;
		float r = hypotf(rx, ry);
		float q = hypotf(qx, qy);

		tx = fminf(1, fmaxf(0, (qx * r / q + ex) / a));
		ty = fminf(1, fmaxf(0, (qy * r / q + ey) / b));
		float t = hypotf(tx, ty);
		tx /= t;
		ty /= t;
	}

	*ox = copysignf(a * tx, px);
	*oy = copysignf(b * ty, py);
}

/*****************************/
static void flag_ellipse(
	unsigned int size,
	Vertex*      data,
	ANode        center,
	float        rx,
	float        ry,
	float        border)
{
	/* Extend the ellipse with an influence border */
	float rx2 = USE_DIR_SLOPE ? rx + border : rx;
	float ry2 = USE_DIR_SLOPE ? ry + border : ry;

	/* Loop over all vertices in its bounding box */
	/* Yeah we're using signed integers... uum could be bettter? */
	int c, r;
	for(c = (int)(-rx2); c <= (int)rx2; ++c)
		for(r = (int)(-ry2); r <= (int)ry2; ++r)
		{
			int cc = (int)center.c + c;
			int rr = (int)center.r + r;
			unsigned int i = cc * size + rr;

			/* Check bounds + check if a slope constraint was already assigned */
			if(cc < 0 || cc >= (int)size || rr < 0 || rr >= (int)size)
				continue;
			if(data[i].flags & SLOPE)
				continue;

			/* Now check if it's inside our first ellipse */
			float d = (c*(float)c) / (rx*rx) + (r*(float)r) / (ry*ry);
			if(d <= 1)
			{
				data[i].c[0] = MAX_SLOPE;
				data[i].flags = SLOPE;
			}

			/* If not, it might be in our second ellipse */
			else if(USE_DIR_SLOPE)
			{
				d = (c*(float)c) / (rx2*rx2) + (r*(float)r) / (ry2*ry2);
				if(d > 1)
					continue;

				/* Calculate the vector to the first ellipse */
				float tx, ty;
				find_ellipse_intersect(rx, ry, c, r, &tx, &ty);

				/* Normalize it to [0,1] */
				tx = (c - tx) / border;
				ty = (r - ty) / border;

				/* Calculate the maximum slope */
				/* The linear falloff is scaled by dist^0.5 */
				/* So it's not really linear anymore, otherwise it looks stupid... */
				float dist = hypotf(tx, ty);
				float nMaxSlope = MAX_SLOPE + MAX_SLOPE_FALLOFF * powf(dist, .5f);
				float cMaxSlope = hypotf(data[i].c[0], data[i].c[1]);

				/* If the slope is smaller than what is already stored, replace */
				/* This so use the smallest distance to the path */
				if(!(data[i].flags & DIR_SLOPE) || nMaxSlope < cMaxSlope)
				{
					data[i].c[0] = tx / dist * nMaxSlope;
					data[i].c[1] = ty / dist * nMaxSlope;
					data[i].flags = DIR_SLOPE;
				}
			}
		}
}

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
			float r = PATH_RADIUS / scale;
			float b = PATH_INFLUENCE / scale;
			while(!EQUAL(u, start))
			{
				flag_ellipse(size, data, u, r, r, b);
				u = PREV(u);
			}

			/* Don't forget to color the start */
			flag_ellipse(size, data, start, r, r, b);

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
				float slope = fabs(
					(data[v.c * size + v.r].h -
					data[u.c * size + u.r].h) / dist);

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
static void flag_borders(unsigned int size, Vertex* data, ModData* mod)
{
	/* Loop over the local neighborhoud */
	int c, r;
	for(c = -1; c <= 1; ++c)
		for(r = -1; r <= 1; ++r)
		{
			/* Check if there is a neighbour */
			Vertex* hdata = mod->local[(c+1)*3+(r+1)];
			if((c == 0 && r == 0) || !hdata)
				continue;

			/* From here on we're going to flag stuff */
			/* We use the second constraint value */
			/* The first is taken by roughness */
			unsigned int i;

			/* Handle corners */
			if(c != 0 && r != 0)
			{
				/* Index into the current vertex data */
				unsigned int id =
					(c == -1 && r == -1) ? 0 :
					(c == -1 && r ==  1) ? size-1 :
					(c ==  1 && r == -1) ? (size-1) * size :
					size * size - 1;

				/* Index into the neighboring's vertex data */
				unsigned int ih =
					(c == -1 && r == -1) ? size * size - 1 :
					(c == -1 && r ==  1) ? (size-1) * size :
					(c ==  1 && r == -1) ? size-1 :
					0;

				data[id].flags |= POSITION;
				data[id].c[1] = hdata[ih].h;
			}

			/* Handle edges */
			else for(i = 0; i < size; ++i)
			{
				/* Index into the current vertex data */
				unsigned int id =
					(c == -1) ? i :
					(c ==  1) ? (size-1) * size + i :
					(r == -1) ? i * size :
					i * size + (size-1);

				/* Index into the neighboring's vertex data */
				unsigned int ih =
					(c == -1) ? (size-1) * size + i :
					(c ==  1) ? i :
					(r == -1) ? i * size + (size-1) :
					i * size;

				data[id].flags |= POSITION;
				data[id].c[1] = hdata[ih].h;

				if(USE_BORDER_DERIV)
				{
					/* Offset to get the vertex next to the border */
					/* Used to match the first derivative at the border */
					int io =
						(c == -1) ? (int)size :
						(c ==  1) ? -(int)size :
						(r == -1) ? 1 : -1;

					/* If the next vertex was already set, take the average */
					int second = data[id+io].c[1] != 0.0f;

					/* Now set the next vertex so the derivative is kept */
					/* Well actually we take the average of its original position and the new one */
					/* Its original position being the relative height to the border */
					/* The new position being the new height based on derivative */
					/* I don't know why this is just an experiment */
					/* TODO: validate this in any way possible... */
					data[id+io].flags |= POSITION;
					data[id+io].c[1] += hdata[ih].h +
						.5f * ((hdata[ih].h - hdata[ih-io].h) + (data[id+io].h - data[id].h));

					/* So yeah that average */
					if(second) data[id+io].c[1] *= .5f;
				}
			}
		}
}

/*****************************/
int mod_subdivide(unsigned int size, Vertex* data, ModData* mod)
{
	/* Let it rain roughness! */
	unsigned int i;
	for(i = 0; i < size*size; ++i)
		data[i].flags = ROUGHNESS;

	/* Find a path from the lower left corner to the upper right corner */
	ANode s = { .c = 0,      .r = 0      };
	ANode g = { .c = size-1, .r = size-1 };

	if(!find_path(size, data, s, g))
		return 0;

	/* Lastly, constrain the borders to match the neighbors */
	/* It is important this is done last */
	/* This because this constraint should be OR'd with the other constraints */
	if(USE_BORDER_STITCH)
		flag_borders(size, data, mod);

	/* We don't need to iterate this modifier */
	mod->done = 1;
	return 1;
}
