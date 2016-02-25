#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "decoder.h"
#include "llist.h"

#define PI 3.14159265
#define EARTH_RAD 6356.752


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
	
	/*
	graph stuff
	graph *final_g = graph_create();
	ll_to_graph(final_g, final_ll);
	//graph_print(g, print_item);
	*/

	ll_print(final_ll);
	ll_destroy(final_ll);
	
}

//this code was taken from http://stackoverflow.com/questions/26446308/issues-with-a-result-from-calculating-latitude-longitude-from-haversine-formula
double haversine(double lat1, double lat2, double lon1, double lon2) 
{
	//conversion to radians
	//change to m_pi with _xopen_source=500
	lat1 *= PI/180;
	lat2 *= PI/180;
	lon1 *= PI/180;
	lon2 *= PI/180;

	//getting the difference for lat and lon
	double dlat = (lat2 - lat1);
	double dlon = (lon2 - lon1);

	//math
	double a = pow(sin(dlat/2), 2) + cos(lat1) * cos(lat2) * pow(sin(dlon/2), 2);
	double b = 2 * atan2(sqrt(a), sqrt(1-a));
	return EARTH_RAD * b;
}
