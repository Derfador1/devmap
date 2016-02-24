#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "decoder.h"
#include "graph/graph.h"

#define PI 3.14159265
#define EARTH_RAD 6356.752

int main(int argc, char *argv[]) 
{
	int file_count = 1;
	//int descrip = 0;
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

	graph *stuff = graph_create();

	while(file_count < argc) {
		mainish(stuff, &argv[file_count - 1]);
		file_count++;
	}

	printf("\n");

	//graph_print(stuff, print_item);

	graph_destroy(stuff); //this needs to move to the devmap
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
