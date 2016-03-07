#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "decoder.h"
#include "llist.h"

void parse_args(int argc, char *argv[], int *tmp_file_count, double *battery_life);

int main(int argc, char *argv[]) 
{
	int file_count = 1;
	double battery_life = 5; //a default value

	int *tmp_file_count;

	tmp_file_count = &file_count;

	parse_args(argc, argv, tmp_file_count, &battery_life);

	struct llist *final_ll = NULL;
	struct llist *tracker_ll = NULL;

	while(file_count < argc) {
		struct llist *test = extraction(&argv[file_count - 1]);
		if(final_ll) {
			ll_append(final_ll, test);
		}
		else {
			final_ll = test;
		}

		file_count++;
	}

	tracker_ll = final_ll;

	graph *final_g = graph_create();
	ll_to_graph(final_g, final_ll);

	graph *final_tmp = graph_copy(final_g);

	if(is_vendor_recommended(final_tmp, final_ll)) {
		printf("Network satisfies vendor recommendations\n");
	}
	else {
		if(attempt_removal(final_tmp, final_ll)) {
			printf("Network satisfies vendor recommendations\n");
		}
		else {
			printf("To many Network alterations needed\n");
		}
	}

	printf("Low Battery (%%%.02lf) :\n", battery_life);

	print_battery(tracker_ll, battery_life);

	printf("\n");

	ll_destroy(final_ll);
	graph_disassemble(final_tmp);
	graph_disassemble(final_g);
}

void print_battery(struct llist *l, double battery)
{
	while(l) {
		struct device *data = (struct device *)l->data;
		if(data->battery_power <= battery && data->battery_power != 0) {
			printf("Device #%d\n", data->source_dev_id);
		} 

		l = l->next;
	}
}

//code to parse through command line arguments to pull out the -p and value
void parse_args(int argc, char *argv[], int *tmp_file_count, double *battery_life)
{
	int c;
	int checker;
	char *program = argv[0];

	while ((c = getopt(argc, argv, "p:")) != -1) {
		switch (c) {
			case 'p':
				if (optarg != NULL) {
					char *ptr; 
					*battery_life = strtol(optarg, & ptr, 10);
					if (*ptr != '\0') {
						fprintf(stderr, "Error1: A number must be entered with -p usage:\n");
						fprintf(stderr, "Usage: %s -p <int value 0-100> <file.pcap>\n", program);
						exit(0);
					}
				}
				else {
					fprintf(stderr, "Error2: A number must be entered with -p usage:\n");
					fprintf(stderr, "Usage: %s -p <int value 0-100> <file.pcap>\n", program);
					exit(0);
				}

				if (*battery_life < 0 || *battery_life > 100) {
					fprintf(stderr, "Error3: Battery value out of range:\n");
					fprintf(stderr, "Usage: %s -p <int value 0-100> <file.pcap>\n", program);
					exit(0);
				}
				break;
			case '?':
				fprintf(stderr, "Error4: A number must be entered with -p usage:\n");
				fprintf(stderr, "Usage: %s -p <int value 0-100> <file.pcap>\n", program);
				exit(0);
		}
	}

	*tmp_file_count = optind;

	argc -= optind;
	argv += optind;

	if (argc < 1) {
		fprintf(stderr, "Error: At least one file argument must be provided.\n");
		fprintf(stderr, "Usage: %s <file.pcap>\n", program);
		exit(0);
	}

	for (int i = 0; i < argc; ++i) {
		if ((checker = access(argv[i], F_OK)) != 0) {
			fprintf(stderr, "ERROR: %s doesn't exist!\n", argv[i]);
			exit(0);
		}
	}
}

bool is_vendor_recommended(graph *g, struct llist *l)
{
	struct llist *test = l;

	//1 node is true and 0 nodes is false by default
	if(graph_node_count(g) == 1) {
		return true;
	}
	else if(graph_node_count(g) == 0) {
		return false;
	}

	while(l) {
		struct llist *tmp = l->next;
		while(tmp) {
			graph *tmp_g = graph_copy(g);
			const struct device *tmp1 = l->data;
			const struct device *tmp2 = tmp->data;
			if(is_adjacent(tmp_g, tmp1, tmp2)) {
			}
			else if(surballes(tmp_g, tmp1, tmp2)) {
			}
			else {
				//if no adjacency or surballes then return false
				graph_disassemble(tmp_g);
				return false;
			}

			tmp = tmp->next;
			graph_disassemble(tmp_g);
		}
		l = l->next;
	}
	
	l = test;
	return true;
}

bool attempt_removal(graph *g, struct llist *l) 
{
	size_t nodes_removed = 0;
	struct llist *print_list = NULL;

	while(l) {
		count(g, l); //gives me a count for each node based on the other valid nodes it can reach
		unsigned int min = find_min(l); //find the min value in the l list
		struct device *to_remove = find_device(l, min); //stores that source id to remove

		//either creates list of add to it
		if(print_list) {
			ll_add(&print_list, to_remove);
		}
		else {
			print_list = ll_create(to_remove);
		}


		graph_remove_node(g, to_remove);
		nodes_removed++;
		remover(&l, to_remove);


		//if more then half the of the graph are removed then it cant be fixed
		if(nodes_removed > (graph_node_count(g)/2)) {
			ll_destroy(print_list);
			return false;
		}

		if(is_vendor_recommended(g, l)) {
			ll_removed_dev(print_list); //prints the removed device
			ll_destroy(print_list);
			return true;
		}

	}

	ll_destroy(print_list);

	return false;
}

struct device *find_device(struct llist *l, unsigned int min)
{
	struct device *tmp1 = NULL;
	while(l) {
		tmp1 = (struct device *)l->data;
		//loops until we find the min device
		if(tmp1->source_dev_id == min) {
			goto FOUND;
		}
		
		l = l->next;		
	}

FOUND:
	return tmp1;
	
}

//this code was taken from http://stackoverflow.com/questions/26446308/issues-with-a-result-from-calculating-latitude-longitude-from-haversine-formula
double haversine(double lat1, double lat2, double lon1, double lon2, float alt1, float alt2) 
{
	//conversion to radians
	//change to m_pi with _xopen_source=500
	lat1 *= M_PI/180;
	lat2 *= M_PI/180;
	lon1 *= M_PI/180;
	lon2 *= M_PI/180;

	//getting the difference for lat and lon
	double dlat = (lat2 - lat1);
	double dlon = (lon2 - lon1);

	//math
	double a = pow(sin(dlat/2), 2) + cos(lat1) * cos(lat2) * pow(sin(dlon/2), 2);
	double b = 2 * atan2(sqrt(a), sqrt(1-a));
	double d = EARTH_RAD * b;

	float temp = alt1 - alt2;	
	double distance = sqrt(d*d + temp*temp);

	return distance;
}

//creates a graph based off my linked list
graph *ll_to_graph(graph *g, struct llist *l)
{
	struct llist *tmp = l;
	while(l) {
		graph_add_node(g, l->data);
		l = l->next;
	}

	l = tmp;
	double result = 0;
	while(l) {
		tmp = l->next;
		while(tmp) {
			const struct device *tmp1 = l->data;
			const struct device *tmp2 = tmp->data;
			result = haversine(tmp1->latitude, tmp2->latitude, tmp1->longitude, tmp2->longitude, tmp1->altitude, tmp2->altitude);
			if(result > 1.25 && result < 16.4042) { //if it is valid in feet then i will add the edges
				graph_add_edge(g, tmp1, tmp2, result);
				graph_add_edge(g, tmp2, tmp1, result);
			}
			tmp = tmp->next;
		}
		l = l->next;
	}
	return g;
}

//gives me a count based on valid adjacent or surballes nodes
struct llist *count(graph *g, struct llist *l)
{
	struct llist *test = l;

	count_reseter(test); //resets count to 0 so it can be calculated again

	while(l) {
		struct llist *tmp = l->next;
		while(tmp) {
			struct device *tmp1 = (struct device *)l->data;
			struct device *tmp2 = (struct device *)tmp->data;
			graph *tmp_g = graph_copy(g);
			if(is_adjacent(tmp_g, tmp1, tmp2)) {
				++(tmp1->count);
				++(tmp2->count);
			}
			else if(surballes(tmp_g, tmp1, tmp2)) {
				++(tmp1->count);
				++(tmp2->count);	
			}
			tmp = tmp->next;
			graph_disassemble(tmp_g);
		}
		l = l->next;
	}

	l = test;

	return l;
}
