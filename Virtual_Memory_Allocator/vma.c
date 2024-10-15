//Duca Andrei-Rares
//311CA
#include "vma.h"

//this function reads the row and checks if it is a valid command,
//a free row, or an invalid command
int functions(char line[LMAX])
{
	char aux[LMAX], comm[LMAX], *p;
	strcpy(aux, line);
	int nr = 0;
	p = strtok(aux, " ");
	if (!p)
		return 9;
	while (p) {
		nr++;
		p = strtok(NULL, " ");
	}
	sscanf(line, "%s", comm);
	if (strcmp(comm, "ALLOC_ARENA") == 0) {
		if (nr != 2)
			return 0;
		return 1;
	} else if (strcmp(comm, "DEALLOC_ARENA") == 0) {
		if (nr != 1)
			return 0;
		return 2;
	} else if (strcmp(comm, "ALLOC_BLOCK") == 0) {
		if (nr != 3)
			return 0;
		return 3;
	} else if (strcmp(comm, "FREE_BLOCK") == 0) {
		if (nr != 2)
			return 0;
		return 4;
	} else if (strcmp(comm, "READ") == 0) {
		if (nr != 3)
			return 0;
		return 5;
	} else if (strcmp(comm, "WRITE") == 0) {
		if (nr < 3)
			return 0;
		return 6;
	} else if (strcmp(comm, "PMAP") == 0) {
		if (nr != 1)
			return 0;
		return 7;
	} else if (strcmp(comm, "MPROTECT") == 0) {
		if (nr < 3 || nr > 7)
			return 0;
		return 8;
	}
	return 0;
}

//the function reads the permissions we want to enter at the specific address
int8_t get_perms(char line[LMAX])
{
	char aux[LMAX], comm[LMAX], *p;
	strcpy(aux, line);
	int8_t nr = 0;
	p = strtok(aux, " |");
	p = strtok(NULL, " |");
	p = strtok(NULL, " |");
	while (p) {
		if (strcmp(p, "PROT_NONE") == 0) {
			nr = 0;
			break;
		}
		if (strcmp(p, "PROT_READ") == 0)
			nr |= 1 << 2;
		else if (strcmp(p, "PROT_WRITE") == 0)
			nr |= 1 << 1;
		else if (strcmp(p, "PROT_EXEC") == 0)
			nr |= 1;
		p = strtok(NULL, " |");
	}
	return nr;
}

//the function is used as a help to be able to read the buffer
//from the correct address
size_t get_strlen(char line[LMAX])
{
	size_t size = 0;
	char *p;
	p = strtok(line, " ");
	size += strlen(p);
	p = strtok(NULL, " ");
	size += strlen(p);
	p = strtok(NULL, " ");
	size += strlen(p);
	size += 3;
	return size;
}

//the function calculates how many words we have in an invalid command
int get_nr_words(char line[LMAX])
{
	int nr = 0;
	char *p;
	p = strtok(line, " ");
	while (p) {
		nr++;
		p = strtok(NULL, " ");
	}
	return nr;
}

//the function creates a miniblock, whose address is returned
//initial permissions are rw-
dll_node_t *create_mnblk(uint64_t start, size_t size)
{
	dll_node_t *new_mnb = malloc(sizeof(struct dll_node_t));
	DIE(!new_mnb, "node miniblock malloc");
	new_mnb->next = NULL;
	new_mnb->prev = NULL;

	miniblock_t *mnb = malloc(sizeof(miniblock_t));
	DIE(!mnb, "miniblock malloc");
	mnb->start_address = start;
	mnb->size = size;

	mnb->perm = (1 << 2) + (1 << 1);
	mnb->rw_buffer = calloc((size + 1), sizeof(char));
	DIE(!mnb->rw_buffer, "miniblock buffer calloc");
	char c = '\0';
	memcpy(mnb->rw_buffer + size, &c, 1);
	new_mnb->data = mnb;

	return new_mnb;
}

//function that deletes a miniblock from memory
void free_mnblk(dll_node_t *mnblk)
{
	free(((miniblock_t *)mnblk->data)->rw_buffer);
	free(mnblk->data);
	free(mnblk);
}

//the function removes the miniblock on the nth position from the list
//and returns its address
dll_node_t *remove_nth_mnblk(block_t *block, unsigned int n)
{
	if (((list_t *)block->miniblock_list)->size == 0)
		return NULL;
	dll_node_t *aux;
	if (((list_t *)block->miniblock_list)->size == 1) {
		aux = ((list_t *)block->miniblock_list)->head;
		((list_t *)block->miniblock_list)->head = NULL;
		((list_t *)block->miniblock_list)->tail = NULL;
		((list_t *)block->miniblock_list)->size--;
		aux->next = NULL;
		aux->prev = NULL;
	} else if (n == 0) {
		aux = ((list_t *)block->miniblock_list)->head;
		((list_t *)block->miniblock_list)->head->next->prev = NULL;
		((list_t *)block->miniblock_list)->head =
		((list_t *)block->miniblock_list)->head->next;
		((list_t *)block->miniblock_list)->size--;
		aux->next = NULL;
		aux->prev = NULL;
	} else if (n >= ((list_t *)block->miniblock_list)->size - 1) {
		aux = ((list_t *)block->miniblock_list)->tail;
		aux->prev->next = NULL;
		((list_t *)block->miniblock_list)->tail = aux->prev;
		((list_t *)block->miniblock_list)->size--;
		aux->next = NULL;
		aux->prev = NULL;
	} else {
		dll_node_t *curr = ((list_t *)block->miniblock_list)->head;
		for (int i = 1; i < n; i++)
			curr = curr->next;
		aux = curr->next;
		curr->next = aux->next;
		aux->next->prev = curr;
		((list_t *)block->miniblock_list)->size--;
		aux->next = NULL;
		aux->prev = NULL;
	}
	return aux;
}

//the function adds the initial miniblock as the only element of the list,
//and depending on preferences, we can add it to the beginning
//or end of the list
void add_miniblk(block_t *block, dll_node_t *new_mnb, int direction)
{
	if (direction == 0) {
		((list_t *)block->miniblock_list)->size = 1;
		((list_t *)block->miniblock_list)->head = new_mnb;
		((list_t *)block->miniblock_list)->tail = new_mnb;
	} else if (direction > 0) {
		((list_t *)block->miniblock_list)->size++;
		((list_t *)block->miniblock_list)->tail->next = new_mnb;
		new_mnb->prev = ((list_t *)block->miniblock_list)->tail;
		((list_t *)block->miniblock_list)->tail = new_mnb;
	} else {
		((list_t *)block->miniblock_list)->size++;
		new_mnb->next = ((list_t *)block->miniblock_list)->head;
		((list_t *)block->miniblock_list)->head->prev = new_mnb;
		((list_t *)block->miniblock_list)->head = new_mnb;
	}
}

//the function creates a block and initializes the list
//of miniblocks of the block
//then the block is added to position n in the list
void add_nth_block(list_t *list, const uint64_t address,
				   int n, const uint64_t size)
{
	dll_node_t *new_node = malloc(sizeof(struct dll_node_t));
	DIE(!new_node, "node block malloc");//

	block_t *block = malloc(sizeof(block_t));
	DIE(!block, "block malloc");//
	block->start_address = address;
	block->size = size;
	block->miniblock_list = malloc(sizeof(list_t));
	DIE(!block->miniblock_list, "list of miniblocks malloc");//

	((list_t *)block->miniblock_list)->data_size = sizeof(miniblock_t);
	((list_t *)block->miniblock_list)->size = 0;
	((list_t *)block->miniblock_list)->head = NULL;
	((list_t *)block->miniblock_list)->tail = NULL;
	new_node->data = block;
	new_node->next = NULL;
	new_node->prev = NULL;

	if (list->size == 0) {
		list->size = 1;
		list->head = new_node;
		list->tail = new_node;
	} else if (n == 0) {
		new_node->next = list->head;
		list->head->prev = new_node;
		list->head = new_node;
		list->size++;
	} else if (n >= list->size) {
		dll_node_t *aux = list->tail;
		aux->next = new_node;
		new_node->prev = aux;
		list->tail = new_node;
		list->size++;
	} else {
		dll_node_t *aux = list->head;
		for (int i = 1; i < n; i++)
			aux = aux->next;
		aux->next->prev = new_node;
		new_node->next = aux->next;
		aux->next = new_node;
		new_node->prev = aux;
		list->size++;
	}
}

//the function removes the block at position n from the list
//and frees its data from memory
void remove_nth_block(list_t *list, int n)
{
	dll_node_t *aux = NULL;
	if (list->size == 0)
		return;
	if (list->size == 1) {
		aux = list->head;
		list->head = NULL;
		list->tail = NULL;
		list->size--;
		aux->next = NULL;
		aux->prev = NULL;
	} else if (n == 0) {
		aux = list->head;
		list->head->next->prev = NULL;
		list->head = list->head->next;
		list->size--;
		aux->next = NULL;
		aux->prev = NULL;
	} else if (n >= list->size - 1) {
		aux = list->tail;
		aux->prev->next = NULL;
		list->tail = aux->prev;
		list->size--;
		aux->next = NULL;
		aux->prev = NULL;
	} else {
		dll_node_t *curr = list->head;
		for (int i = 1; i < n; i++)
			curr = curr->next;
		aux = curr->next;
		curr->next = aux->next;
		aux->next->prev = curr;
		list->size--;
		aux->next = NULL;
		aux->prev = NULL;
	}
	block_t *block = aux->data;
	list_t *mini = block->miniblock_list;
	while (mini->size) {
		dll_node_t *mnblk = remove_nth_mnblk(block, 0);
		free_mnblk(mnblk);
	}

	free(mini);
	free(block);
	free(aux);
}

//the function writes in the buffer of each miniblock that belongs to
//the interval [address, address + size) the data given from stdin
void write_text(dll_node_t *node, uint64_t address, uint64_t size, int8_t *data)
{
	dll_node_t *mini =
	((list_t *)((block_t *)node->data)->miniblock_list)->head;
	uint64_t dst = address + size;
	miniblock_t *miniblk;
	while (mini) {
		miniblk = mini->data;
		uint64_t mini_dst = miniblk->start_address + miniblk->size;
		if (mini_dst <= address) {
			mini = mini->next;
			continue;
		}
		break;
	}
	int8_t i = address - miniblk->start_address, j = 0;
	uint64_t size_check = size;
	uint64_t sizee = miniblk->size - i;

	if (sizee > size_check)
		sizee = size_check;

	memcpy(miniblk->rw_buffer, data + j, sizee);
	j = sizee;
	size_check -= sizee;
	mini = mini->next;
	while (mini) {
		miniblk = mini->data;
		if ((address + size) < miniblk->start_address)
			break;
		if (miniblk->size > size_check) {
			sizee = size_check;
			memcpy(miniblk->rw_buffer, data + j, sizee);
			break;
		}
		sizee = miniblk->size;
		memcpy(miniblk->rw_buffer, data + j, sizee);
		j += sizee;
		size_check -= sizee;
		mini = mini->next;
	}
}

//the function displays the data belonging to
//the interval [address, address + size) from miniblocks
void read_text(dll_node_t *node, uint64_t address, uint64_t size)
{
	dll_node_t *mini =
	((list_t *)((block_t *)node->data)->miniblock_list)->head;
	uint64_t dst = address + size;
	miniblock_t *miniblk;
	while (mini) {
		miniblk = mini->data;
		uint64_t mini_dst = miniblk->start_address + miniblk->size;
		if (mini_dst <= address) {
			mini = mini->next;
			continue;
		}
		break;
	}
	uint64_t i = address - miniblk->start_address, j = 0, size_check = size;
	uint64_t sizee = miniblk->size;
	char *sir = miniblk->rw_buffer;

	uint64_t nr_check = 0;
	for (j = 0; j < sizee; j++) {
		if (sir[j] != '\0')
			break;
		nr_check++;
	}

	for (j = i + nr_check; j < sizee; j++) {
		if (!size_check)
			break;
		if (sir[j] == '\0')
			break;
		printf("%c", sir[j]);
		size_check--;
	}
	mini = mini->next;

	while (mini) {
		miniblk = mini->data;
		if ((address + size) < miniblk->start_address)
			break;
		sir = miniblk->rw_buffer;
		if (miniblk->size > size_check) {
			sizee = size_check;
			for (j = 0; j < sizee; j++)
				printf("%c", sir[j]);
			break;
		}
		sizee = miniblk->size;
		for (j = 0; j < sizee; j++)
			printf("%c", sir[j]);
		size_check -= sizee;
		mini = mini->next;
	}
	printf("\n");
}

//the arena and block list are allocated
arena_t *alloc_arena(const uint64_t size)
{
	arena_t *arena = malloc(sizeof(arena_t));
	DIE(!arena, "arena malloc");

	arena->arena_size = size;

	arena->alloc_list = malloc(sizeof(list_t));
	DIE(!arena->alloc_list, "list of blocks malloc");//

	arena->alloc_list->head = NULL;
	arena->alloc_list->tail = NULL;
	arena->alloc_list->data_size = sizeof(block_t);
	arena->alloc_list->size = 0;
	return arena;
}

//the function that stops the execution of the program
//all data is released from memory
void dealloc_arena(arena_t *arena)
{
	while (arena->alloc_list->size)
		remove_nth_block(arena->alloc_list, 0);
	arena->alloc_list->head = NULL;
	arena->alloc_list->tail = NULL;
	free(arena->alloc_list);
	free(arena);
}

//the function inserts a block in the list if the memory is
//not contiguous with the neighbor blocks
//the other 2 cases are:
//1 joins the one on the right
//2 joins with the one on the left and checks if
//it will also join with the one on the right
void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	if (address >= arena->arena_size) {
		printf("The allocated address is outside the size of arena\n");
	} else if (address + size > arena->arena_size) {
		printf("The end address is past the size of the arena\n");
	} else {
		int ok = 2, i = 0;
		uint64_t destination = address + size;
		dll_node_t *curr_block = arena->alloc_list->head;
		while (curr_block) {
			uint64_t blk_dst = ((block_t *)curr_block->data)->start_address
			 + ((block_t *)curr_block->data)->size;
			if (blk_dst < address) {
				curr_block = curr_block->next;
				i++;
				continue;
			}
			if ((((block_t *)curr_block->data)->start_address <= address) &&
				blk_dst > address) {
				ok = 0;
				break;
			} else if ((((block_t *)curr_block->data)->start_address
				< destination) && (blk_dst >= destination)) {
				ok = 0;
				break;
			} else if ((((block_t *)curr_block->data)->start_address >=
				address) && (blk_dst <= destination)) {
				ok = 0;
				break;
			}
			ok = 1;
			size_t sizee = size;
			if (((block_t *)curr_block->data)->start_address == destination) {
				sizee += ((block_t *)curr_block->data)->size;
				((block_t *)curr_block->data)->size = sizee;
				((block_t *)curr_block->data)->start_address = address;
				dll_node_t *mnblk = create_mnblk(address, size);
				add_miniblk(curr_block->data, mnblk, -1);
				break;
			} else if (((block_t *)curr_block->data)->start_address
				> destination) {
				add_nth_block(arena->alloc_list, address, i, size);
				dll_node_t *mnblk = create_mnblk(address, size);
				curr_block = curr_block->prev;
				add_miniblk(curr_block->data, mnblk, 0);
				break;
			}
			sizee += ((block_t *)curr_block->data)->size;
			((block_t *)curr_block->data)->size = sizee;
			dll_node_t *mnblk = create_mnblk(address, size);
			add_miniblk(curr_block->data, mnblk, 1);
			dll_node_t *compare = curr_block->next;
			if (!compare)
				break;
			if (((block_t *)compare->data)->start_address == destination) {
				sizee += ((block_t *)compare->data)->size;
				((block_t *)curr_block->data)->size = sizee;
				block_t *mini = compare->data;
				while (((list_t *)mini->miniblock_list)->size) {
					dll_node_t *mnblk = remove_nth_mnblk(mini, 0);
					add_miniblk(curr_block->data, mnblk, 1);
				}
				remove_nth_block(arena->alloc_list, i + 1);
			}
			break;
		}
		if (!ok) {
			printf("This zone was already allocated.\n");
			return;
		}
		if (ok == 2) {
			add_nth_block(arena->alloc_list, address, i, size);
			dll_node_t *mnblk = create_mnblk(address, size);
			curr_block = arena->alloc_list->tail;
			add_miniblk(curr_block->data, mnblk, 0);
		}
	}
}

//searched miniblock is deleted, and depending on the initial
//position of the miniblock, the block is broken in 2 or remains the same
void free_block(arena_t *arena, const uint64_t address)
{
	dll_node_t *node = arena->alloc_list->head;
	int ok = 0;
	unsigned int i = 0;
	while (node) {
		block_t *block = node->data;
		uint64_t blk_dst = block->start_address + block->size;
		if (blk_dst < address) {
			i++;
			node = node->next;
			continue;
		}
		if (block->start_address > address)
			break;
		ok = 1;
		dll_node_t *node_mini = ((list_t *)block->miniblock_list)->head;
		miniblock_t *miniblk = NULL;
		while (node_mini) {
			miniblk = node_mini->data;
			if (miniblk->start_address != address) {
				node_mini = node_mini->next;
				continue;
			}
			break;
		}
		if (!node_mini) {
			ok = 0;
			break;
		}
		if ((node_mini == ((list_t *)block->miniblock_list)->head) ||
			(node_mini == ((list_t *)block->miniblock_list)->tail)) {
			if (((list_t *)block->miniblock_list)->size == 1) {
				dll_node_t *removed_mnblk = remove_nth_mnblk(block, 0);
				free_mnblk(removed_mnblk);
				remove_nth_block(arena->alloc_list, i);
				break;
			}
			if (node_mini == ((list_t *)block->miniblock_list)->head) {
				dll_node_t *removed_mnblk = remove_nth_mnblk(block, 0);
				dll_node_t *aux = ((list_t *)block->miniblock_list)->head;
				block->start_address =
				((miniblock_t *)aux->data)->start_address;
				block->size = blk_dst - block->start_address;
				free_mnblk(removed_mnblk);
				break;
			}
			if (node_mini == ((list_t *)block->miniblock_list)->tail) {
				blk_dst -= ((miniblock_t *)node_mini->data)->size;
				dll_node_t *removed_mnblk = remove_nth_mnblk(block,
					((list_t *)block->miniblock_list)->size);
				block->size = blk_dst - block->start_address;
				free_mnblk(removed_mnblk);
				break;
			}
		}
		size_t size1 = address - block->start_address;
		add_nth_block(arena->alloc_list, block->start_address, i
			, size1);
		block_t *prev = node->prev->data;
		dll_node_t *mnblk = remove_nth_mnblk(block, 0);
		if (mnblk != node_mini) {
			add_miniblk(prev, mnblk, 0);
			mnblk = remove_nth_mnblk(block, 0);
		}
		while (mnblk != node_mini) {
			add_miniblk(prev, mnblk, 1);
			mnblk = remove_nth_mnblk(block, 0);
		}
		dll_node_t *aux = ((list_t *)block->miniblock_list)->head;
		block->start_address = ((miniblock_t *)aux->data)->start_address;
		block->size = blk_dst - block->start_address;
		free_mnblk(node_mini);
		break;
	}
	if (!ok)
		printf("Invalid address for free.\n");
}

//the block from which to read is searched, and it is checked
//if we have read permissions
void read(arena_t *arena, uint64_t address, uint64_t size)
{
	dll_node_t *node = arena->alloc_list->head;
	int ok = 0;
	unsigned int i = 0;
	uint64_t sizee, blk_dst;
	while (node) {
		block_t *block = node->data;
		blk_dst = block->start_address + block->size;
		if (blk_dst <= address) {
			i++;
			node = node->next;
			continue;
		}
		if (block->start_address > address)
			break;
		if (blk_dst < address + size) {
			ok = 2;
			dll_node_t *mini = ((list_t *)block->miniblock_list)->head;
			int verif = 1;
			while (mini) {
				miniblock_t *miniblk = mini->data;
				uint64_t mini_dst = miniblk->start_address + miniblk->size;
				if (mini_dst <= address) {
					mini = mini->next;
					continue;
				}
				if (!(miniblk->perm & 4)) {
					verif = 0;
					break;
				}
				mini = mini->next;
			}
			if (!verif) {
				printf("Invalid permissions for read.\n");
				ok = 3;
			}
			break;
		}
		ok = 1;
		dll_node_t *mini = ((list_t *)block->miniblock_list)->head;
		int verif = 1;
		while (mini) {
			miniblock_t *miniblk = mini->data;
			uint64_t mini_dst = miniblk->start_address + miniblk->size;
			if (mini_dst <= address) {
				mini = mini->next;
				continue;
			}
			if ((address + size) < miniblk->start_address)
				break;
			if (!(miniblk->perm & 4)) {
				verif = 0;
				break;
			}
			mini = mini->next;
		}
		if (!verif) {
			printf("Invalid permissions for read.\n");
			ok = 3;
		}
		break;
	}
	if (!ok) {
		printf("Invalid address for read.\n");
	} else if (ok == 2) {
		uint64_t sizee = blk_dst - address;
		printf("Warning: size was bigger than the block size.");
		printf(" Reading %lu characters.\n", sizee);
		read_text(node, address, sizee);
	} else if (ok == 1) {
		read_text(node, address, size);
	}
}

//the block in which it will be written is searched
//and it is checked if we have write permissions
void write(arena_t *arena, const uint64_t address,
		   const uint64_t size, int8_t *data)
{
	dll_node_t *node = arena->alloc_list->head;
	int ok = 0;
	unsigned int i = 0;
	uint64_t sizee, blk_dst;
	while (node) {
		block_t *block = node->data;
		blk_dst = block->start_address + block->size;
		if (blk_dst <= address) {
			i++;
			node = node->next;
			continue;
		}
		if (block->start_address > address)
			break;
		if (blk_dst < address + size) {
			ok = 2;
			dll_node_t *mini = ((list_t *)block->miniblock_list)->head;
			int verif = 1;
			while (mini) {
				miniblock_t *miniblk = mini->data;
				uint64_t mini_dst = miniblk->start_address + miniblk->size;
				if (mini_dst <= address) {
					mini = mini->next;
					continue;
				}
				if (!(miniblk->perm & 2)) {
					verif = 0;
					break;
				}
				mini = mini->next;
			}
			if (!verif) {
				printf("Invalid permissions for write.\n");
				ok = 3;
			}
			break;
		}
		ok = 1;
		dll_node_t *mini = ((list_t *)block->miniblock_list)->head;
		int verif = 1;
		while (mini) {
			miniblock_t *miniblk = mini->data;
			uint64_t mini_dst = miniblk->start_address + miniblk->size;
			if (mini_dst <= address) {
				mini = mini->next;
				continue;
			}
			if ((address + size) < miniblk->start_address)
				break;
			if (!(miniblk->perm & 2)) {
				verif = 0;
				break;
			}
			mini = mini->next;
		}
		if (!verif) {
			printf("Invalid permissions for write.\n");
			ok = 3;
		}
		break;
	}
	if (!ok) {
		printf("Invalid address for write.\n");
	} else if (ok == 2) {
		uint64_t sizee = blk_dst - address;
		printf("Warning: size was bigger than the block size.");
		printf(" Writing %lu characters.\n", sizee);
		write_text(node, address, sizee, data);
	} else if (ok == 1) {
		write_text(node, address, size, data);
	}
}

//the function displays all the current data about what
//we have in the arena's memory
void pmap(const arena_t *arena)
{
	uint64_t total_mem = arena->arena_size;
	uint64_t adr_tot_occupied = 0;
	unsigned long nr_miniblk = 0, nr_blk = arena->alloc_list->size;
	dll_node_t *block = ((list_t *)arena->alloc_list)->head;
	while (block) {
		adr_tot_occupied += ((block_t *)block->data)->size;
		list_t *aux = ((block_t *)block->data)->miniblock_list;
		nr_miniblk += aux->size;
		block = block->next;
	}
	uint64_t free_mem = total_mem - adr_tot_occupied;
	printf("Total memory: 0x%lX bytes\n", total_mem);
	printf("Free memory: 0x%lX bytes\n", free_mem);
	printf("Number of allocated blocks: %lu\n", nr_blk);
	printf("Number of allocated miniblocks: %lu\n", nr_miniblk);
	block = ((list_t *)arena->alloc_list)->head;
	int i = 1;
	while (block) {
		printf("\nBlock %d begin\n", i);
		uint64_t blk_dst = ((block_t *)block->data)->start_address +
		((block_t *)block->data)->size;
		printf("Zone: 0x%lX - 0x%lX\n",
			   ((block_t *)block->data)->start_address, blk_dst);
		int j = 1;
		list_t *aux = ((block_t *)block->data)->miniblock_list;
		dll_node_t *miniblk = aux->head;
		while (miniblk) {
			uint64_t miniblk_dst = ((miniblock_t *)miniblk->data)->start_address
			+ ((miniblock_t *)miniblk->data)->size;
			uint8_t perm = ((miniblock_t *)miniblk->data)->perm;
			char r = '-', w = '-', x = '-';
			if (perm & 1)
				x = 'X';
			if (perm & 2)
				w = 'W';
			if (perm & 4)
				r = 'R';
			printf("Miniblock %d:\t\t0x%lX\t\t-\t\t0x%lX\t\t| %c%c%c\n",
				   j, ((miniblock_t *)miniblk->data)->start_address,
				 miniblk_dst, r, w, x);
			j++;
			miniblk = miniblk->next;
		}

		printf("Block %d end\n", i);
		i++;
		block = block->next;
	}
}

//the function applies wanted permissions to the miniblock
//at the specified address
void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{
	dll_node_t *node = arena->alloc_list->head;
	int ok = 0;
	unsigned int i = 0;
	while (node) {
		block_t *block = node->data;
		uint64_t blk_dst = block->start_address + block->size;
		if (blk_dst < address) {
			i++;
			node = node->next;
			continue;
		}
		if (block->start_address > address)
			break;
		ok = 1;
		dll_node_t *node_mini = ((list_t *)block->miniblock_list)->head;
		miniblock_t *miniblk = NULL;
		while (node_mini) {
			miniblk = node_mini->data;
			if (miniblk->start_address != address) {
				node_mini = node_mini->next;
				continue;
			}
			break;
		}
		if (!node_mini) {
			ok = 0;
			break;
		}
		miniblk->perm = (uint64_t)(*permission);
		break;
	}
	if (!ok)
		printf("Invalid address for mprotect.\n");
}
