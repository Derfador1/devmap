#include "dijkstra.h"
#include "graph/graph.h"

int pq_compare(const void *a, const void *b)
{
	if(!a) {
		return -(intptr_t)b;
	}
	if(!b) {
		return (intptr_t)a;
	}

	struct pqueue_node *pqa, *pqb;
	pqa = (struct pqueue_node *)a;
	pqb = (struct pqueue_node *)b;

	return pqa->priority - pqb->priority;
}

struct pqueue_node *__make_node(const void *data, int priority)
{
	struct pqueue_node *pqn = malloc(sizeof(*pqn));
	if(!pqn) {
		return NULL;
	}
	pqn->data = data;


	pqn->priority = priority;

	return pqn;
}

struct visited_node *__make_vnode(int distance,
		struct pqueue_node *priority, const void *prev)
{
	struct visited_node *vis = malloc(sizeof(*vis));
	if(!vis) {
		return NULL;
	}
	vis->prev = prev;
	vis->distance = distance;
	vis->priority = priority;

	return vis;
}

struct llist *dijkstra_path(const graph *g, const void *from, const void *to)
{
	heap *to_process = heap_create(pq_compare);
	struct pqueue_node *start =__make_node(from, 0);
	heap_add(to_process, start);

	hash *visited = hash_create();
	struct visited_node *first = __make_vnode(0, start, NULL);

	hash_insert(visited, from, first); 

	while(!heap_is_empty(to_process)) {
		struct pqueue_node *curr = heap_remove_min(to_process);

		if(curr->data == to) {
			free(curr);
			goto FOUND;
		}

		struct llist *adjacencies = graph_adjacent_to(g, curr->data);
		struct llist *check = adjacencies;
		while(check) {
			int dist = curr->priority +
				graph_edge_weight(g, curr->data, check->data);

			if(!hash_exists(visited, check->data)) {
				struct pqueue_node *pq_to_add =__make_node(check->data, dist);

				struct visited_node *next_node =
					__make_vnode(dist, pq_to_add, curr->data);

				hash_insert(visited, check->data, next_node);
				heap_add(to_process, pq_to_add);
			} else {
				struct visited_node *found = hash_fetch(visited, check->data);

				if(dist < found->distance) {
					found->distance = dist;
					found->prev = curr->data;
					found->priority->priority = dist;
					heap_rebalance(to_process);
				}
			}

			check = check->next;
		}
		free(curr);
		ll_disassemble(adjacencies);
	}
	heap_destroy(to_process);
	hash_destroy(visited);

	return NULL;

FOUND:
	heap_destroy(to_process);

	struct llist *path = ll_create(to);
	while(((struct visited_node *)hash_fetch(visited, path->data))->prev) {
		ll_add(&path,
				((struct visited_node *)hash_fetch(visited, path->data))->prev);
	}

	hash_destroy(visited);

	return path;
}

struct llist *graph_path(const graph *g, const void *from, const void *to)
{
	hash *visited = hash_create();
	queue *to_process = queue_create();

	hash_insert(visited, from, NULL);
	queue_enqueue(to_process, from);

	while(!queue_is_empty(to_process)) {
		void *curr = queue_dequeue(to_process);

		struct llist *adjacencies = graph_adjacent_to(g, curr);
		struct llist *check = adjacencies;
		while(check) {
			if(!hash_exists(visited, check->data)) {
				hash_insert(visited, check->data, curr);
				queue_enqueue(to_process, check->data);
				if(check->data == to) {
					ll_disassemble(adjacencies);
					goto FOUND;
				}
			}

			check = check->next;
		}

		ll_disassemble(adjacencies);
	}

	queue_disassemble(to_process);
	hash_disassemble(visited);
	return NULL;

FOUND:
	queue_disassemble(to_process);

	struct llist *path = ll_create(to);
	while(hash_fetch(visited, path->data)) {
		ll_add(&path, hash_fetch(visited, path->data));
	}

	hash_disassemble(visited);

	return path;
}


//something wrong ith surballes as well with no solution one
bool surballes(graph *tmp_g, const void *from, const void *to) 
{
	struct llist *path = NULL;
	int path_count = 0;

	graph *sur_tmp = graph_copy(tmp_g);

	path = dijkstra_path(sur_tmp, (struct device *)from, (struct device *)to);

	if(!path) {
		graph_disassemble(sur_tmp);
		ll_disassemble(path);
		return false;
	}

	struct llist *head = path;
	path = path->next;
	while(path && path->next) {
		graph_remove_node(sur_tmp, path->data);
		graph_remove_edge(sur_tmp, path->data, path->next->data);
		graph_remove_edge(sur_tmp, path->next->data, path->data);
		path = path->next;
	}
	path_count++;

	path = dijkstra_path(sur_tmp, (struct device *)from, (struct device *)to);

	if(!path) {
		graph_disassemble(sur_tmp);
		ll_disassemble(path);
		ll_disassemble(head);
		return false;
	}

	path_count++;

	graph_disassemble(sur_tmp);
	ll_disassemble(path);
	ll_disassemble(head);

	return true;
}

bool is_adjacent(const graph *g, const void *a, const void *b)
{
	if(!g) {
		return false;
	}

	struct node *map = g->nodes;

	if(!map) {
		return false;
	}

	while(map) {
		if(map->data == a) {
			struct edge *edges = map->edges;

			while(edges) {
				if(edges->to->data == b) {
					return true;
				}
				edges = edges->next;
			}
			//return false;
		}
		map = map->next;
	}

	return false;
}
