#include "decoder.h"

struct meditrik *make_meditrik(void) //used to initialize meditrik structure when invoked
{
	struct meditrik *meditrik = malloc(sizeof(struct meditrik));
	if(!meditrik) { //makes sure there is something to malloc
		return NULL;
	}

	return meditrik;
}

struct ipv4 *make_ip(void) //used to initialize meditrik structure when invoked
{
	struct ipv4 *ipv4 = malloc(sizeof(struct ipv4));
	if(!ipv4) { //makes sure there is something to malloc
		return NULL;
	}

	return ipv4;
}

int start(int argc, char * argv[])
{
	size_t file_count = 1;
	int descrip = 0;

	//while(file_count < (size_t)argc) {
	printf("\n");
	if (argc == 1)
	{
		printf("Please retry with a valid file to open.\n");
		exit(1);
	}
	else if (argc >= 2)
	{
		descrip = open(argv[file_count], O_RDONLY); //gives an integer if open works successfully 
		if (descrip == -1)
		{
			fprintf(stderr, "Error could not open file\n");
			exit(1);
		}
		else
		{
			//printf("You successfully opened %s\n", argv[1]);
		}
	}


	int global_header = 24;
	int packet = 16;
	int ethernet = 14;
	int ipv4 = 20;
	int ipv6 = 40;
	int udp = 8;

	int count = 0;
	//int excess_headers = 0;
	//int udp_start = 0;

	//int ipv_start = global_header + packet + ethernet;

	unsigned int *type_pt = malloc(sizeof(*type_pt));

	unsigned int *total_length = malloc(sizeof(*total_length));

	int *start = malloc(sizeof(*start));

	int verse = 0;

	int src_port = 0;

	unsigned char *buf = malloc(SIZE);

	memset(buf, '\0', SIZE);

	count = read(descrip, buf, SIZE); //sets count to integer value returned by read, which is number of things read

	struct meditrik *stuff = make_meditrik(); //make meditrik to malloc space

	struct ipv4 *ver = make_ip();

	*start = global_header;

	while(*start < count) {
		*start += packet + ethernet;
		
		verse = extract_ver(ver, start, buf);

		printf("Ip verision: %d\n", verse);

		if(verse == 4 ) {
			*start += ipv4;
		}
		else {
			*start += ipv6;
		}

		/*
		if(!udp_check(udp_start, buf)) { //change to 57005
			printf("This is a malformed packet\n");
			free(buf);
			free(stuff);
			free(ver);
			free(type_pt);
			free(total_length);
			free(start);
			close(descrip);
			exit(1);
		}
		*/

		src_port = udp_check(start, buf);

		printf("SRC Port: %d\n", src_port);

		*start += udp;

		bit_seperation(stuff, buf, type_pt, total_length, start);

		/*
		if(*type_pt != 2) {
			//printf("um\n");
			continue;
		}
		*/

		fprintf(stdout, "Version: %d\n", stuff->version);
		fprintf(stdout, "Sequence: %d\n", stuff->seq_id);
		fprintf(stdout, "Type: %d\n", stuff->type);
		fprintf(stdout, "Source Device: %d\n", stuff->source_device_id);
		fprintf(stdout, "Destination Device: %d\n", stuff->dest_device_id);

		if (field_check(type_pt, buf, start, total_length) != 1)
		{
			fprintf(stderr, "Error has occured in field checking\n");
			break;
		}
	}

	free(buf);
	free(stuff);
	free(ver);
	free(type_pt);
	free(total_length);
	free(start);
	close(descrip);

	return 1;
}

int extract_ver(struct ipv4 *ver, int *start, unsigned char *buf)
{
	//int ipv_start = *start;
	unsigned int vers = buf[*start];
	vers >>= 4;
	ver->version = vers;
	//printf("ver : %d\n", vers);
	//(*start)--;
	return ver->version;
}

int udp_check(int *start, unsigned char *buf)
{
	unsigned int port_start = buf[*start];
	port_start <<= 8;
	port_start += buf[++(*start)];
	//printf("port : %d\n", port_start); //remove this later
	(*start)--;
	return port_start;
}

int bit_seperation(struct meditrik *medi, unsigned char *buf, unsigned int *type_pt, unsigned int *total_length, int *start)
{
	//version bitmath
	unsigned int byte_start = buf[*start]; //used to increment position in buffer
	byte_start >>= 4;
	medi->version = byte_start;

	//sequence_id bitmath
	byte_start = buf[*start];
	byte_start &= 15;
	byte_start <<= 5;
	unsigned int byte_start2 = buf[++(*start)];
	byte_start2 >>= 3;
	byte_start += byte_start2;
	medi->seq_id = byte_start;

	//type bitmath
	unsigned char byte_starter = buf[*start];
	byte_starter &= 7;
	medi->type = byte_starter;
	*type_pt = medi-> type;

	//total length
	unsigned char byte_length_starter = buf[++(*start)];
	byte_length_starter <<= 8;
	byte_length_starter += buf[++(*start)];
	medi->total_length = byte_length_starter;
	*total_length = medi-> total_length;

	//source device id bitmath
	unsigned int byte_start_source = buf[++(*start)];
	byte_start_source <<= 8;
	byte_start_source += buf[++(*start)];
	byte_start_source <<= 8;
	byte_start_source += buf[++(*start)];
	byte_start_source <<= 8;
	byte_start_source += buf[++(*start)];
	medi->source_device_id = byte_start_source;

	//dest device id bitmath
	unsigned int byte_start_dest = buf[++(*start)];
	byte_start_dest <<= 8;
	byte_start_dest += buf[++(*start)];
	byte_start_dest <<= 8;
	byte_start_dest += buf[++(*start)];
	byte_start_dest <<= 8;
	byte_start_dest += buf[++(*start)];
	medi->dest_device_id = byte_start_dest;

	(*start)++;

	return 0;
}


int field_check(unsigned int *type_pt, unsigned char *buf, int *start, unsigned int *total_length)
{
	short counter = 0;

	int excess_headers = 0;

	//checks againt functions based on type

	if (*type_pt == 0)
	{
		if (status_decode(start, buf, counter, excess_headers) != 0)
		{
			fprintf(stderr, "Error with status_decode\n");
			return 0;
		}
		
		return 1;

	}
	else if (*type_pt == 1)
	{
		if (command_decode(start, buf, excess_headers) != 1)
		{
			fprintf(stderr, "Error with command_decode\n");
			return 0;
		}


		return 1;
	}
	else if (*type_pt == 2)
	{
		if (gps_decode(start, buf, counter, excess_headers) != 2)
		{
			fprintf(stderr, "Error with gps_decode\n");
			return 0;
		}

		return 1;
	}
	else if (*type_pt == 3)
	{
		if (message_decode(start, buf, total_length, excess_headers) != 3) //set meditrik and excess header in func
		{
			fprintf(stderr, "Error with message_decode\n");
			return 0;
		}

		return 1;
	}
	else
	{
		return 0;
	}
}

int status_decode(int *start, unsigned char *buf, int counter, int excess_headers)
{
	short glucose = 0;
	short capsaicin = 0;
	short omorfine = 0;

	union battery power;

	for (counter = 0; counter < 8; counter++) //counts number of places needed for each field
	{
		power.tempbuf[counter] = buf[*start + counter]; //stores the places in temp buffer
	}

	*start = *start + counter;

	fprintf(stdout, "Battery power : %.2f%%\n", power.percent * 100);

	unsigned int glucose_start = buf[*start]; 
	glucose_start <<= 8;
	glucose_start += buf[++(*start)];
	glucose = glucose_start;
	fprintf(stdout, "Glucose: %d\n", glucose);

	unsigned int capsaicin_start = buf[++(*start)];
	capsaicin_start <<= 8;
	capsaicin_start += buf[++(*start)];
	capsaicin = capsaicin_start;
	fprintf(stdout, "Capsaicin: %d\n", capsaicin);

	unsigned int omorfine_start = buf[++(*start)];
	omorfine_start <<= 8;
	omorfine_start += buf[++(*start)];
	omorfine = omorfine_start;
	fprintf(stdout, "Omorfine: %d\n", omorfine);

	(*start)++;

	*start = *start + excess_headers; //adds 58 to start so it can check for multiple packets

	return 0;
}

int command_decode(int *start, unsigned char * buf, int excess_headers)
{
	unsigned int byte_start = buf[*start];
	byte_start <<= 8;
	byte_start += buf[++(*start)];
	fprintf(stdout, "Command: %d\n", byte_start);

	//checks to see if what field goes with the command field
	if ((byte_start % 2) == 0)
	{
		//nothing happens
	}
	else if (byte_start == 1)
	{
		unsigned int glucose = buf[++(*start)]; //used to find glucose
		glucose <<= 8;
		glucose += buf[++(*start)];
		fprintf(stdout, "Glucose: %d\n", glucose);
	}
	else if (byte_start == 3)
	{
		unsigned int capsaicin = buf[*start]; //used to find capsaicin
		capsaicin <<= 8;
		capsaicin += buf[++(*start)];
		fprintf(stdout, "Capsaicin: %d\n", capsaicin);
	}
	else if (byte_start == 5)
	{
		unsigned int omorfine = buf[*start];//used to fine omorfine
		omorfine <<= 8;
		omorfine += buf[++(*start)];
		fprintf(stdout, "Omorfine: %d\n", omorfine);
	}
	else if (byte_start == 7)
	{	
		unsigned int sequence_id = buf[*start]; //used to find seq_param
		sequence_id <<= 8;
		sequence_id += buf[++(*start)];	
		fprintf(stdout, "Seq_param: %d\n", sequence_id);
	}

	(*start)++;

	*start = *start + excess_headers;

	return 1;
	
}

int gps_decode(int *start, unsigned char *buf, int counter, int excess_headers)
{
	union gps_header gps;
	
	for (counter = 0; counter < 20; counter++)
	{
		gps.degrees[counter] = buf[*start + counter];
	}

	*start = *start + counter;

	fprintf(stdout, "Longitude: %.9f degree W\n", gps.fields.longs); //fills longitude, latitude, alt from what was stored

	fprintf(stdout, "Latitude: %.9f degree N\n", gps.fields.lat);	//in buffer and moves down based on types of the three

	fprintf(stdout, "Altitude: %f ft\n", gps.fields.alt * 6);
	
	(*start)++;

	*start = *start + excess_headers;

	return 2;
}

int message_decode(int *start, unsigned char *buf, unsigned int *total_length, int excess_headers)
{
	int meditrik_header = 12;

	*total_length = *total_length - meditrik_header; //gives total payload size

	int i = 0;

	int *counter = malloc(sizeof(*counter));

	*counter = *start + *total_length; //used for counting through the size of the message

	fprintf(stdout, "Message: ");

	for (i = *start; i < *counter; i++)
	{
		fprintf(stdout, "%c", buf[i]); //writes a each single character to stdout
	}

	fprintf(stdout, "\n");

	*start = *start + *total_length;

	*start = *start + excess_headers - 2;

	free(counter);

	return 3;
}
