//Duca Andrei-Rares
//311CA
#pragma once
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}							\
	} while (0)

#define LMAX 10000

typedef struct dll_node_t dll_node_t;
struct dll_node_t {
	void *data;
	dll_node_t *prev, *next;
};

typedef struct {
	dll_node_t *head;
	unsigned int data_size;
	unsigned int size;
	dll_node_t *tail;
} list_t;

typedef struct {
	uint64_t start_address;
	size_t size;
	void *miniblock_list;
} block_t;

typedef struct {
	uint64_t start_address;
	size_t size;
	uint8_t perm;
	void *rw_buffer;
} miniblock_t;

typedef struct {
	uint64_t arena_size;
	list_t *alloc_list;
} arena_t;

int functions(char line[LMAX]);
int8_t get_perms(char line[LMAX]);
size_t get_strlen(char line[LMAX]);
int get_nr_words(char line[LMAX]);

dll_node_t *create_mnblk(uint64_t start, size_t size);
void free_mnblk(dll_node_t *mnblk);
dll_node_t *remove_nth_mnblk(block_t *block, unsigned int n);
void add_miniblk(block_t *block, dll_node_t *new_mnb, int direction);

void add_nth_block(list_t *list, const uint64_t address,
				   int n, const uint64_t size);
void remove_nth_block(list_t *list, int n);

void write_text(dll_node_t *node, uint64_t address,
				uint64_t size, int8_t *data);
void read_text(dll_node_t *node, uint64_t address, uint64_t size);

arena_t *alloc_arena(const uint64_t size);
void dealloc_arena(arena_t *arena);

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size);
void free_block(arena_t *arena, const uint64_t address);

void read(arena_t *arena, uint64_t address, uint64_t size);
void write(arena_t *arena, const uint64_t address,
		   const uint64_t size, int8_t *data);
void pmap(const arena_t *arena);
void mprotect(arena_t *arena, uint64_t address, int8_t *permission);
