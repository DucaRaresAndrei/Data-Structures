//Duca Andrei-Rares
//311CA
#include "vma.h"

//the main function reads the commands line by line and
//calls the functions using a switch case type instruction
int main(void)
{
	char line[LMAX], aux[LMAX], date[LMAX], *p;
	arena_t *arena = NULL;
	int ok = 0, nr_words = 0;
	while (fgets(line, sizeof(line), stdin)) {
		strcpy(aux, line);
		if (aux[strlen(aux) - 1] == '\n')
			aux[strlen(aux) - 1] = '\0';
		ok = functions(aux);
		uint64_t param, size_char, aux_size;
		size_t size;
		int8_t perms;
		char *sir;
		switch (ok) {
		case 0:
			nr_words = get_nr_words(aux);
			for (int i = 0; i < nr_words; i++)
				printf("Invalid command. Please try again.\n");
			break;
		case 1:
			sscanf(line, "%*s%lu", &param);
			arena = alloc_arena(param);
			break;
		case 2:
			dealloc_arena(arena);
			break;
		case 3:
			sscanf(line, "%*s%lu%lu", &param, &size);
			alloc_block(arena, param, size);
			break;
		case 4:
			sscanf(line, "%*s%lu", &param);
			free_block(arena, param);
			break;
		case 5:
			sscanf(line, "%*s%lu%lu", &param, &size_char);
			read(arena, param, size_char);
			break;
		case 6:
			sscanf(line, "%*s%lu%lu", &param, &size_char);
			size = get_strlen(aux);
			strcpy(date, line + size);
			sir = calloc((size_char + 1), sizeof(char));
			int i = 0, j = 0;
			aux_size = size_char;
			while (aux_size) {
				sir[i] = date[j];
				aux_size--;
				i++;
				j++;
				if (aux_size && j == strlen(date)) {
					fgets(date, sizeof(date), stdin);
					j = 0;
				}
			}
			write(arena, param, size_char, sir);
			free(sir);
			break;
		case 7:
			pmap(arena);
			break;
		case 8:
			sscanf(line, "%*s%lu", &param);
			perms = get_perms(aux);
			mprotect(arena, param, &perms);
		}
	}
	return 0;
}
