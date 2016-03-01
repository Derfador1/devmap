#include "decoder.h"
#include "graph/graph.h"
#include <math.h>

struct meditrik *make_meditrik(void) //used to initialize meditrik structure when invoked
{
	struct meditrik *meditrik = malloc(sizeof(struct meditrik));
	if(!meditrik) { //makes sure there is something to malloc
		return NULL;
	}

	return meditrik;
}

struct device *make_device(void) //used to initialize meditrik structure when invoked
{
	struct device *device = malloc(sizeof(struct device));
	if(!device) { //makes sure there is something to malloc
		return NULL;
	}

	return device;
}


struct ipv4 *make_ip(void) //used to initialize meditrik structure when invoked
{
	struct ipv4 *ipv4 = malloc(sizeof(struct ipv4));
	if(!ipv4) { //makes sure there is something to malloc
		return NULL;
	}

	return ipv4;
}

void ll_print(struct llist *test)
{
	struct llist *tmp = test;

	while(tmp) {
		const struct device *data = tmp->data;
		printf(u8"%d → ", data->source_dev_id);
		//printf("Battery Power %.02lf%%\n", data->battery_power);
		tmp = tmp->next;
	}
}

void something_print(int data, bool is_node)
{
	if(is_node) {
		printf("%d", data);
	} else {
		printf(u8" → %d", data);
	}
}


bool is_adjacent(const graph *g, const struct device *a, const struct device *b)
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


struct llist *extraction(char * argv[])
{
	size_t file_count = 1;
	int descrip = 0;

	printf("\n");

	struct llist *test = NULL;

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


	int global_header = 24;
	int packet = 16;
	int ethernet = 14;
	int ipv4 = 20;
	int ipv6 = 40;
	int udp = 8;
	int medi_header = 12;

	int count = 0;

	unsigned int *type_pt = malloc(sizeof(*type_pt));

	unsigned int *total_length = malloc(sizeof(*total_length));

	int *start = malloc(sizeof(*start));

	int verse = 0;

	int src_port = 0;

	int dst_port = 0;

	unsigned char *buf = malloc(SIZE);

	memset(buf, '\0', SIZE);

	count = read(descrip, buf, SIZE); //sets count to integer value returned by read, which is number of things read

	*start = global_header;

	while(*start < count) {
		struct meditrik *stuff = make_meditrik(); //make meditrik to malloc space

		struct device *data = make_device();

		struct ipv4 *ver = make_ip();

		*start += packet + ethernet;
		
		verse = extract_ver(ver, start, buf);

		printf("Ip version: %d\n", verse);

		if(verse == 4 ) {
			*start += ipv4;
		}
		else {
			*start += ipv6;
		}

		src_port = udp_check(start, buf);
		*start += 2;
		dst_port = udp_check(start, buf);
		*start -= 2;

		printf("SRC Port: %d\n", src_port);
		printf("DST Port: %d\n", dst_port);

		*start += udp;

		bit_seperation(stuff, buf, type_pt, total_length, start);

		if(src_port != 57005 || dst_port != 57005) {
			printf("This is a malformed packet\n");
			*start = *start + *total_length - medi_header;
			free(data); //the hell? WHY
			goto END;
		}

		/*
		fprintf(stdout, "Version: %d\n", stuff->version);
		fprintf(stdout, "Type: %d\n", stuff->type);
		fprintf(stdout, "Source Device: %d\n", stuff->source_device_id);
		fprintf(stdout, "Destination Device: %d\n", stuff->dest_device_id);
		fprintf(stdout, "Sequence: %d\n", stuff->seq_id);
		*/

		data->battery_power = 0;
		data->latitude = 0;
		data->longitude = 0;
		data->altitude = 0;

		if (field_check(data, type_pt, buf, start, total_length) != 1)
		{
			fprintf(stderr, "Error has occured in field checking\n");
			break;
		}

		if(*type_pt == 2 || *type_pt == 0) {
			data->source_dev_id = stuff->source_device_id;

			if(test) {
				ll_add(&test, data);
			}
			else {
				test = ll_create(data);				
			}
		}
		else {
			printf("There was not gps data to be found\n");
			data->latitude = 0;
			data->longitude = 0;
			data->altitude = 0;
		}

		//printf("Count:%d Start:%d\n", count, *start);
		//printf(" → ");

END:
		free(stuff);
		free(ver);
	}

	free(buf);
	free(type_pt);
	free(total_length);
	free(start);
	close(descrip);
	return test;
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

bool surballes(graph *g, struct llist *l) 
{
	struct llist *path = NULL;
	struct llist *path2 = NULL;
	//double weight = 0;

	while(l) {
		struct llist *tmp = l->next;
		while(tmp) {
			const struct device *tmp1 = l->data;
			const struct device *tmp2 = tmp->data;
			//if they arent adjacent then run dijkstra
			if(!is_adjacent(g, tmp1, tmp2)) {
				printf("Is not Adjacent\n");
				graph *tmp_g = graph_copy(g);
				//weight = graph_edge_weight(tmp_g, tmp1, tmp2);
				//printf("%d : %d - Weight: %lf\n", tmp1->source_dev_id, tmp2->source_dev_id, weight); //doesnt seem right
				path = dijkstra_path(tmp_g, tmp1, tmp2);
				if(path) {
					ll_print(path);
				}
				else {
					return false;
				}


				graph_remove_edge(tmp_g, tmp2, tmp1);
				/*
				while(path->next) {
					path = path->next;
					graph_remove_node(tmp_g, path->data);
				}
				*/


				path2 = dijkstra_path(tmp_g, tmp1, tmp2);
				printf("\nPath\n");
				ll_print(path2);
				printf("\nhere\n");

				graph_disassemble(tmp_g);
				ll_disassemble(path);
				ll_disassemble(path2);
				return true;

			}
			tmp = tmp->next;
		}
		l = l->next;
	}
	printf("\n");
	size_t count = graph_node_count(g);
	printf("Node Count : %zd\n", count);
	return true;
}



void print_item(const void *data, bool is_node)
{
	struct device *print = (struct device *)data;
	if(is_node) {
		printf("\n%d", print->source_dev_id);
	} 
	else {
		printf(" → %d", print->source_dev_id);
	}
}

int extract_ver(struct ipv4 *ver, int *start, unsigned char *buf)
{
	unsigned int vers = buf[*start];
	vers >>= 4;
	ver->version = vers;
	return ver->version;
}

int udp_check(int *start, unsigned char *buf)
{
	unsigned int port_start = buf[*start];
	port_start <<= 8;
	port_start += buf[++(*start)];
	(*start)--; //returns start to correct value
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


int field_check(struct device *data, unsigned int *type_pt, unsigned char *buf, int *start, unsigned int *total_length)
{
	short counter = 0;


	//checks againt functions based on type

	if (*type_pt == 0)
	{
		if (status_decode(data, start, buf, counter) != 0)
		{
			fprintf(stderr, "Error with status_decode\n");
			return 0;
		}
		
		return 1;

	}
	else if (*type_pt == 1)
	{
		if (command_decode(start, buf) != 1)
		{
			fprintf(stderr, "Error with command_decode\n");
			return 0;
		}


		return 1;
	}
	else if (*type_pt == 2)
	{
		if (gps_decode(data, start, buf, counter) != 2)
		{
			fprintf(stderr, "Error with gps_decode\n");
			return 0;
		}

		return 1;
	}
	else if (*type_pt == 3)
	{
		if (message_decode(start, buf, total_length) != 3) //set meditrik and excess header in func
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

int status_decode(struct device *data, int *start, unsigned char *buf, int counter)
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

	data->battery_power = (power.percent * 100);

	printf("Battery_power %.02lf%%\n", data->battery_power);

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

	return 0;
}

int command_decode(int *start, unsigned char * buf)
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

	*start = *start;

	return 1;
}

int gps_decode(struct device *data, int *start, unsigned char *buf, int counter)
{
	union gps_header gps;
	
	for (counter = 0; counter < 20; counter++)
	{
		gps.degrees[counter] = buf[*start + counter];
	}

	*start = *start + counter;

	//fprintf(stdout, "Longitude: %.9f degree W\n", gps.fields.longs); //fills longitude, latitude, alt from what was stored

	//fprintf(stdout, "Latitude: %.9f degree N\n", gps.fields.lat);	//in buffer and moves down based on types of the three

	//fprintf(stdout, "Altitude: %f ft\n", gps.fields.alt * 6);

	data->latitude = gps.fields.lat;

	data->longitude = gps.fields.longs;

	data->altitude = (gps.fields.alt * 6);
	
	printf("%lf\n", data->latitude);
	printf("%lf\n", data->longitude);
	printf("%f\n", data->altitude);
	//(*start)++; //remove this to make new solutions work with proper byte count

	//*start = *start + 3; //remove + 3

	return 2;
}

int message_decode(int *start, unsigned char *buf, unsigned int *total_length)
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

	*start = *start - 2;

	free(counter);

	return 3;
}
