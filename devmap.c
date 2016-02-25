#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "decoder.h"
#include "llist.h"

int main(int argc, char *argv[]) 
{
	int file_count = 1;
	double battery_life;
	char *ptr;

	if(argc == 1) {
		printf("Please retry with a valid file to open.\n");
		exit(1);
	}
	else if (argc >= 2) {
		if(strncmp(argv[file_count], "-p", 10) == 0) {
			file_count++;
			printf("Num: %d\n", file_count);
			char *tester = argv[file_count];
			for(unsigned int d = 0; d < strlen(argv[2]); ++d){
				if(isdigit(tester[d]) != 0){
					//
				}
				else{
					//
				}
			}

			battery_life = strtol(argv[file_count], &ptr, 10);
			if(battery_life <= 0){

			}
			else {
				file_count++;
			}
		}
	}

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
	printf("Graph print: \n");
	graph_print(final_g, print_item);

	//print_path(final_ll);

	//ll_print(final_ll);
	ll_destroy(final_ll);

	graph_disassemble(final_g);
}
