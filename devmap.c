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

	struct llist *test = final_ll;

	while(final_ll) {
		struct llist *tmp = final_ll->next;
		while(tmp) {
			graph *tmp_g = graph_copy(final_g);
			const struct device *tmp1 = final_ll->data;
			const struct device *tmp2 = tmp->data;
			if(!is_adjacent(tmp_g, tmp1, tmp2)) {
				if(surballes(tmp_g, tmp1, tmp2)) {
					printf("valid\n");
				}
				else {
					printf("attemtping to remove\n");
				}
			}
			else {
				printf("adjacent\n");
			}
			tmp = tmp->next;
			graph_disassemble(tmp_g);
		}
		final_ll = final_ll->next;
	}
	
	final_ll = test;

	printf("\n");

	ll_destroy(final_ll);
	//ll_disassemble(test);
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
