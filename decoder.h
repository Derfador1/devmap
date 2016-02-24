#ifndef DECODER_H
	#define DECODER_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "graph/graph.h"
#define SIZE 1500

/* Use this site as a starting point to seperate the fields to lower numbers based on fields given one this site
https://wiki.wireshark.org/Development/LibpcapFileFormat
*/

struct global{
	unsigned int magic_number : 32;
	unsigned int version_major : 16; 
	unsigned int version_minor : 16; 
	unsigned int time_zone: 32; 
	unsigned int sig_flags : 32; 
	unsigned int snap_len : 32; 
	unsigned int network : 32;  

};

struct packet_header{
	unsigned int ts_sec : 32;
	unsigned int ts_usec : 32;
	unsigned int incl_len : 32;
	unsigned int orig_len : 32;
};

struct ethernet{
	//had to split, no room to hold 48 bits in one var
	unsigned int destmac : 32;
	unsigned int destmac_split : 16;
	unsigned int sourcemac : 32;
	unsigned int sourcemac_split : 16;
	unsigned int length : 16;
};

struct ipv6{
	unsigned int version : 4;
	unsigned int traffic_class : 8;
	unsigned int flow_label : 20;
	unsigned int payload : 16;
	unsigned int next_header : 8;
	unsigned int hop_limit : 8;
	//128 source 
	//128 dest
};

struct ipv4{
	unsigned int version : 4;
	unsigned int h_length : 4;
	unsigned int s_type : 8;
	unsigned int total_length : 16;
	unsigned int identification : 16;
	unsigned int flags : 4;
	unsigned int offset : 8;
	unsigned int ttl : 4;
	unsigned int protocol : 4;
	unsigned int checksum : 16;
	unsigned int source_ip : 32;
	unsigned int dest_ip : 32;
	unsigned int option : 12;
};

struct udp{
	unsigned int s_port : 16;
	unsigned int d_port : 16;
	unsigned int length : 16;
	unsigned int checksum : 16;
};

struct meditrik{
	unsigned int version : 4;
	unsigned int seq_id : 9;
	unsigned int type : 3;
	unsigned int total_length : 16;
	unsigned int source_device_id : 32;
	unsigned int dest_device_id : 32;

};

struct gps {
	double longs;
	double lat;
	float alt;
};

union battery {
	unsigned char tempbuf[8];
	double percent;
};

union gps_header{
	struct gps fields;
	unsigned char degrees[20];
};

struct device {
	double longitude;
	double latitude;
	float altitude;
	unsigned int source_dev_id; 	
};


int bit_seperation(struct meditrik *medi, unsigned char *buf, unsigned int *type_pt, unsigned int *total_length, int *start);

int field_check(struct device *data, unsigned int *type_pt, unsigned char *buf, int *start, unsigned int *total_length);

int status_decode(int *start, unsigned char *buf, int counter, int excess_headers);

int command_decode(int *start, unsigned char * buf, int excess_headers);

int gps_decode(struct device *data, int *start, unsigned char *buf, int counter, int excess_headers);

int message_decode(int *start, unsigned char *buf, unsigned int *total_length, int excess_headers);

int extract_ver(struct ipv4 *ver, int *start, unsigned char *buf);

int udp_check(int *start, unsigned char *buf);

graph *start(graph *g, int argc, char * argv[]);

void print_item(const void *data, bool is_node);

void print_path(const struct llist *path);


#endif
