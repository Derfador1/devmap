#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

#include "decoder.h"
#include "llist.h"
#include "hash.h"
#include "heap.h"
#include "queue.h"
#include "graph/graph.h"


graph *big_graph(void);
void print_item(const void *data, bool is_node);
void print_path(const struct llist *path);

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

struct pqueue_node {
	const void *data;
	int priority;
};

int pq_compare(const void *a, const void *b)
{
	if(!a) {
		return -(intptr_t)b;
	}
	if(!b) {
		return (intptr_t)a;
	}

	struct pqueue_node *pqa, *pqb;
	pqa = (struct pqueue_node *)a;
	pqb = (struct pqueue_node *)b;

	return pqa->priority - pqb->priority;
}

struct pqueue_node *__make_node(const void *data, int priority)
{
	struct pqueue_node *pqn = malloc(sizeof(*pqn));
	if(!pqn) {
		return NULL;
	}
	pqn->data = data;
	pqn->priority = priority;

	return pqn;
}

struct visited_node {
	int distance;
	struct pqueue_node *priority;
	const void *prev;
};

struct visited_node *__make_vnode(int distance,
		struct pqueue_node *priority, const void *prev)
{
	struct visited_node *vis = malloc(sizeof(*vis));
	if(!vis) {
		return NULL;
	}
	vis->prev = prev;
	vis->distance = distance;
	vis->priority = priority;

	return vis;
}

struct llist *dijkstra_path(const graph *g, const void *from, const void *to)
{
	heap *to_process = heap_create(pq_compare);
	struct pqueue_node *start =__make_node(from, 0);
	heap_add(to_process, start);

	hash *visited = hash_create();
	struct visited_node *first = __make_vnode(0, start, NULL);
	hash_insert(visited, from, first);

	while(!heap_is_empty(to_process)) {
		struct pqueue_node *curr = heap_remove_min(to_process);

		if(curr->data == to) {
			goto FOUND;
		}

		struct llist *adjacencies = graph_adjacent_to(g, curr->data);
		struct llist *check = adjacencies;
		while(check) {

			int dist = curr->priority +
				graph_edge_weight(g, curr->data, check->data);

			if(!hash_exists(visited, check->data)) {
				struct pqueue_node *pq_to_add =__make_node(check->data, dist);

				struct visited_node *next_node =
					__make_vnode(dist, pq_to_add, curr->data);

				hash_insert(visited, check->data, next_node);
				heap_add(to_process, pq_to_add);
			} else {
				struct visited_node *found = hash_fetch(visited, check->data);

				if(dist < found->distance) {
					found->distance = dist;
					found->prev = curr->data;
					found->priority->priority = dist;
					heap_rebalance(to_process);
				}
			}

			check = check->next;
		}
	}
	heap_destroy(to_process);
	hash_destroy(visited);

	return NULL;

FOUND:
	heap_destroy(to_process);

	struct llist *path = ll_create(to);
	while(((struct visited_node *)hash_fetch(visited, path->data))->prev) {
		ll_add(&path,
				((struct visited_node *)hash_fetch(visited, path->data))->prev);
	}

	hash_destroy(visited);

	return path;
}

struct llist *graph_path(const graph *g, const void *from, const void *to)
{
	hash *visited = hash_create();
	queue *to_process = queue_create();

	hash_insert(visited, from, NULL);
	queue_enqueue(to_process, from);

	while(!queue_is_empty(to_process)) {
		void *curr = queue_dequeue(to_process);

		struct llist *adjacencies = graph_adjacent_to(g, curr);
		struct llist *check = adjacencies;
		while(check) {
			if(!hash_exists(visited, check->data)) {
				hash_insert(visited, check->data, curr);
				queue_enqueue(to_process, check->data);
				if(check->data == to) {
					ll_disassemble(adjacencies);
					goto FOUND;
				}
			}

			check = check->next;
		}

		ll_disassemble(adjacencies);
	}

	queue_disassemble(to_process);
	hash_disassemble(visited);
	return NULL;

FOUND:
	queue_disassemble(to_process);

	struct llist *path = ll_create(to);
	while(hash_fetch(visited, path->data)) {
		ll_add(&path, hash_fetch(visited, path->data));
	}

	hash_disassemble(visited);

	return path;
}

//change to graph start(int argc, char * argv[])
int start(int argc, char * argv[])
{
	size_t file_count = 1;
	int descrip = 0;

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

	unsigned int *type_pt = malloc(sizeof(*type_pt));

	unsigned int *total_length = malloc(sizeof(*total_length));

	int *start = malloc(sizeof(*start));

	int verse = 0;

	int src_port = 0;

	unsigned char *buf = malloc(SIZE);

	memset(buf, '\0', SIZE);

	count = read(descrip, buf, SIZE); //sets count to integer value returned by read, which is number of things read

	struct meditrik *stuff = make_meditrik(); //make meditrik to malloc space

	struct device *data = make_device();

	struct ipv4 *ver = make_ip();

	*start = global_header;

	while(*start < count) {
		*start += packet + ethernet;
		
		verse = extract_ver(ver, start, buf);

		printf("Ip version: %d\n", verse);

		if(verse == 4 ) {
			*start += ipv4;
		}
		else {
			*start += ipv6;
		}

		/*
		if(!udp_check(udp_start, buf)) { //change to 57005
			printf("This is a malformed packet\n");
			continure; 
			-or-
			free(buf);
			free(stuff);
			free(ver);
			free(type_pt);
			free(total_length);
			free(start);
			close(descrip);
			exit(1); goto here
		}
		*/

		src_port = udp_check(start, buf);
		//*start += 2;
		//dst_port = udp_check(start, buf);
		//*start -= 2;

		printf("SRC Port: %d\n", src_port);

		*start += udp;

		bit_seperation(stuff, buf, type_pt, total_length, start);

		/*
		if(*type_pt != 2) {
			printf("um\n");
			continue;
		}
		
		fprintf(stdout, "Version: %d\n", stuff->version);
		fprintf(stdout, "Type: %d\n", stuff->type);
		fprintf(stdout, "Source Device: %d\n", stuff->source_device_id);
		fprintf(stdout, "Destination Device: %d\n", stuff->dest_device_id);
		fprintf(stdout, "Sequence: %d\n", stuff->seq_id);
		*/

		if (field_check(data, type_pt, buf, start, total_length) != 1)
		{
			fprintf(stderr, "Error has occured in field checking\n");
			break;
		}

		if(*type_pt == 2) {
			data->source_dev_id = stuff->source_device_id;

			graph *g = graph_create();
			graph_add_node(g, data);
			graph_print(g, print_item);
		}

		free(data);

		//printf("Start is now: %d\n", *start);
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

void print_item(const void *data, bool is_node)
{
	if(is_node) {
		struct device *print = (struct device *)data;
		printf("Longitude %lf\n", print->longitude);
		printf("Latitude %lf\n", print->latitude);
		printf("Altitude %f\n", print->altitude);
		printf("Src_id %d\n", print->source_dev_id);
	} else {
		printf(u8" → %d", *(int *)data);
	}
}

void print_path(const struct llist *path)
{
	while(path) {
		printf(u8"%s → ", (char *)(path->data));
		path = path->next;
	}
	printf("\n");
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
		if (gps_decode(data, start, buf, counter, excess_headers) != 2)
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

int gps_decode(struct device *data, int *start, unsigned char *buf, int counter, int excess_headers)
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

	data->longitude = gps.fields.longs;

	data->latitude = gps.fields.lat;

	data->altitude = (gps.fields.alt * 6);
	
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
