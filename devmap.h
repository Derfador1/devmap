#ifndef DEVMAP_H
	#define DEVMAP_H

#include "decoder.h"

bool is_vendor_recommended(graph *g, struct llist *l);
void parse_args(int argc, char *argv[], int *tmp_file_count, double *battery_life);
bool removing(graph *g, struct llist *l);
double haversine(double lat1, double lat2, double lon1, double lon2, float alt1, float alt2);
graph *ll_to_graph(graph *g, struct llist *l);
struct llist *count(graph *g, struct llist *l);
struct device *find_device(struct llist *l, unsigned int min);

#endif
