#include "graph/graph.h"
#include "llist.h"
#include "hash.h"
#include "heap.h"
#include "queue.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>


struct pqueue_node {
	const void *data;
	int priority;
};

struct visited_node {
	int distance;
	struct pqueue_node *priority;
	const void *prev;
};

int pq_compare(const void *a, const void *b);
struct pqueue_node *__make_node(const void *data, int priority);
struct visited_node *__make_vnode(int distance, struct pqueue_node *priority, const void *prev);
struct llist *dijkstra_path(const graph *g, const void *from, const void *to);
struct llist *graph_path(const graph *g, const void *from, const void *to);
