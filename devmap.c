#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "decoder.h"
#include "llist.h"

void parse_args(int argc, char *argv[], int *tmp_file_count, double *battery_life);

int main(int argc, char *argv[]) 
{
	int file_count = 1;
	double battery_life = 5;

	int *tmp_file_count;

	tmp_file_count = &file_count;

	parse_args(argc, argv, tmp_file_count, &battery_life);

	struct llist *final_ll = NULL;

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

	printf("\n");

	graph *final_g = graph_create();
	ll_to_graph(final_g, final_ll);
	printf("\nGraph print: \n");
	graph_print(final_g, print_item);
	printf("\n");

	if(is_vendor_recommended(final_g, final_ll)) {
		printf("Network satisfies vendor recommendations\n");
	}
	else {
		printf("\nattempt removing\n");

		if(removing(final_g, final_ll)) {
			printf("Network satisfies vendor recommendations\n");
		}
		else {
			printf("Cant be made right\n");
		}
	}

	printf("\n");

	ll_destroy(final_ll);
	graph_disassemble(final_g);
}

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
	if(graph_node_count(g) == 2) {
		printf("true\n");
		return true;
	}

	struct llist *test = l;

	while(l) {
		struct llist *tmp = l->next;
		while(tmp) {
			graph *tmp_g = graph_copy(g);
			const struct device *tmp1 = l->data;
			const struct device *tmp2 = tmp->data;
			if(is_adjacent(tmp_g, tmp1, tmp2)) {
				printf("valid adjacent\n");
			}
			else if(surballes(tmp_g, tmp1, tmp2)) {
				printf("valid surballes\n");
			}
			else {
				printf("returning false\n");
				graph_disassemble(tmp_g);
				return false; //this is the fucking problem! I dont need it to print false
			}

			tmp = tmp->next;
			graph_disassemble(tmp_g);
		}
		l = l->next;
	}
	
	l = test;
	return true;
}

//should attempt to remove all the nodes until only 2 but doesnt
//something wrong with my is vendor recommended
bool removing(graph *g, struct llist *l) 
{
	struct llist *tracker = l;

	struct llist *count_ll = count(g, l); //warning at this line

	unsigned int min = find_min(l);

	printf("MIN: %d\n", min);

	while(l) {
		struct device *tmp1 = (struct device *)l->data;
		graph *tmp_g = graph_copy(g);
		if(tmp1->source_dev_id == min) {
			graph_remove_node(tmp_g, tmp1);

			bool double_head = tracker == l;
			printf("removing node in ll\n");
			remover(&tracker, tmp1);
			count_ll = count(tmp_g, tracker);

			min = find_min(tracker);
			printf("MIN: %d\n", min);

			if(is_vendor_recommended(tmp_g, tracker)) {
				graph_disassemble(tmp_g);
				//l = tracker;
				return true;
			}

			if(double_head) {
				l = tracker;
				continue;
			}

		}

		l = l->next;
		graph_disassemble(tmp_g);
	}

	return false;
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
			if(result > 1.25 && result < 16.4042) {
				printf("Result: %f\n", result);
				graph_add_edge(g, tmp1, tmp2, result);
				graph_add_edge(g, tmp2, tmp1, result);
			}
			tmp = tmp->next;
		}
		l = l->next;
	}
	printf("\n");	

	printf("Edgecount: %zu\n", graph_edge_count(g));
	return g;
}

struct llist *count(graph *g, struct llist *l)
{
	struct llist *test = l;

	count_reseter(test);

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
