/*
**
**This program implements a cache simulator  that takes a valgrind
**memory grace as input, simulates the hit/miss behavior of a cache
**memory, and output the total number of hits, misses and evictions
*/

#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

typedef struct{
    long tag;
    int set_index;
    int block_offset;
}address;

//tag, valid, priority field of a cache line
typedef struct{
    long tag;
    int valid;
    int priority;    //eviction when priority equals 1
}line;


//read the command line input and obtain s, E, b, trace_name
int inputCmd(int argc, char **argv, int *s, int *E, int *b, char *trace_name);
//take the operation address as input
//return the tag, set, block value of operation address
address getAddr(long opt_addr, int s, int b);
//judge whether the operation results in a cache hit, miss or eviction
//cache is a 2D array of line
int cacheReaction(line **cache, int s, int E, int b, char *trace_line);
//update the LRU bit
//eviction occurs when priority equals 1
void updatePriority(line **cache, int set_num, int line_num, int E);

//global vars
int hit_count = 0;
int miss_count = 0;
int eviction_count = 0;

int main(int argc, char **argv){
    FILE *fp;
    int s, E, b;
    char trace_name[20];
    char trace_line[50];    //store a line from the trace file
    int set_total;    //total number of sets in the cache
    line **cache;    //the simulated cache is a 2D array of line

    s = E = b = 0;
    inputCmd(argc, argv, &s, &E, &b, trace_name);

    //init cache
    set_total = 1 << s;
    cache = malloc(set_total * sizeof(line *));
    for(int i = 0; i < set_total; i++){
        cache[i] = malloc(E * sizeof(line));
    }

    for(int i = 0; i < set_total; i++){
        for(int j = 0; j < E; j++){
            cache[i][j].tag = 0;
            cache[i][j].valid = 0;
            cache[i][j].priority = 0;
        }
    }

    //read trace file
    if((fp = fopen(trace_name, "r")) == NULL){
        printf("cannot open file %s\n", trace_name);
        exit(-1);
    }

    while(fgets(trace_line, 50, fp) != NULL){
        //skip if is instruction
        if(trace_line[0] != ' ')
            continue;

        //verify cache hit, miss or eviction
        cacheReaction(cache, s, E, b, trace_line);
    }
    fclose(fp);

    //print the total count of hits, misses and evictions
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}

//get command line opts and save input s, E, b in corresponding vars
int inputCmd(int argc, char **argv, int *s, int *E, int *b, char *trace_name){
    int input;
    opterr = 0;

    while((input = getopt(argc, argv, "s:E:b:t:")) != -1){
        switch(input){
            case 's':
                *s = atoi(optarg);
                break;
            case 'E':
                *E = atoi(optarg);
                break;
            case 'b':
                *b = atoi(optarg);
                break;
            case 't':
                strcpy(trace_name, optarg);
                break;
            case '?':
                exit(-1);
                break;
            default:
                abort();
        }
    }

    return 0;
}

//split the opt address into three parts: tag, set index and block offset
address getAddr(long opt_addr, int s, int b){
    address addr = {0, 0, 0};
    long mask = 0x7fffffffffffffff;

    addr.block_offset = opt_addr & (mask >> (63 - b));
    opt_addr >>= b;
    addr.set_index = opt_addr & (mask >> (63 - s));
    addr.tag = opt_addr >> s;
    return addr;
}

//verify cache hit, miss or eviction
int cacheReaction(line **cache, int s, int E, int b, char *trace_line){
    char opt;
    long opt_addr;
    line temp_line;
    address input;

    //obtain the operation charactor and address from a single line in trace file
    sscanf(trace_line, " %c %lx", &opt, &opt_addr);
    //split the address into tag bits, set bits and block bits
    input = getAddr(opt_addr, s, b);

    //if the operation type is (M)odify, there is always a hit
    if(opt == 'M')
        hit_count++;

    for(int line_num = 0; line_num < E; line_num++){
        temp_line = cache[input.set_index][line_num];
   
        //cache hit if both valid and match
        if((temp_line.valid != 0) && (temp_line.tag == input.tag)){
            hit_count++;

            //update priority
            updatePriority(cache, input.set_index, line_num, E); 
            return 0;
        }
    }

    //cache miss
    miss_count++;
    for(int line_num = 0; line_num < E; line_num++){
        if(cache[input.set_index][line_num].valid == 0){
            //no eviction needed if empty line exists
            cache[input.set_index][line_num].valid = 1;
            cache[input.set_index][line_num].tag = input.tag;

            //update priority
            updatePriority(cache, input.set_index, line_num, E);
            return 0;
        }
    }

    //need eviciton
    eviction_count++;
    for(int line_num = 0; line_num < E; line_num++){
        if(cache[input.set_index][line_num].priority == 1){
            //evict the least recently used line
            cache[input.set_index][line_num].tag = input.tag;

            //update priority
            updatePriority(cache, input.set_index, line_num, E);
            return 0;
        }
    }

    return -1; 
}

//priority = 1 means the least recently used line
//priority = 0 means invalid line
void updatePriority(line **cache, int set, int line, int E){
    int temp = cache[set][line].priority;

    for(int i = 0; i < E; i++){
        if((cache[set][i].priority > temp) && (cache[set][i].valid != 0)){
            //update priority
            cache[set][i].priority--;
        }
    }
    //the current accessed line is given the lowest priority
    cache[set][line].priority = E;
}
