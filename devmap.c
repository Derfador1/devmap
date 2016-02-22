#include <stdio.h>
#include "decoder.h"

int main(int argc, char *argv[]) 
{
	int file_count = 1;
	int descrip = 0;
	if(argc == 1) {
		printf("Please retry with a valid file to open.\n");
		exit(1);
	}
	else if (argc >= 2) {
		descrip = open(argv[file_count], O_RDONLY); //gives an integer if open works successfully 
		if (descrip == -1)
		{
			fprintf(stderr, "Error could not open file\n");
			exit(1);
		}
		else {
			//not sure
		}
	}

	while(file_count < argc) {
		start(argc, &argv[file_count - 1]);
		file_count++;
	}
}
