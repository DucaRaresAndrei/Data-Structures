/* Copyright 2023 Duca Andrei-Rares, 311CA */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef UTILS_H_
#define UTILS_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/* useful macro for handling error codes */
#define DIE(assertion, call_description)                                       \
	do {                                                                       \
		if (assertion) {                                                       \
			fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                 \
			perror(call_description);                                          \
			exit(errno);                                                       \
		}                                                                      \
	} while (0)

#endif

#define LENGTH 256
#define ALPHABET_SIZE 26
#define ALPHABET "abcdefghijklmnopqrstuvwxyz"

typedef struct trie_node_t trie_node_t;
struct trie_node_t {
	//Value associated with key (set if end_of_word = 1)
	char value[LENGTH];

	//1 if current node marks the end of a word, 0 otherwise
	int end_of_word;

	//the number of occurrences of the word
	int attemps;

	trie_node_t **children;
	int n_children;
};

typedef struct trie_t trie_t;
struct trie_t {
	trie_node_t *root;

	// Number of keys
	int size;

	// Generic Data Structure
	int data_size;

	// Trie-Specific, alphabet properties
	int alphabet_size;
	char *alphabet;
};

trie_node_t *trie_create_node(trie_t *trie)
{
	trie_node_t *node = malloc(sizeof(trie_node_t));
	DIE(!node, "Error in malloc() for node\n");

	//the node does not represent a word
	node->end_of_word = false;
	node->n_children = 0;
	node->attemps = 0;

	//allocate the number of nodes, equal to the number of letters
	//of the alphabet
	node->children = malloc(trie->alphabet_size * sizeof(trie_node_t *));
	DIE(!node->children, "Error in malloc() for node's children\n");

	for (int i = 0; i < trie->alphabet_size; i++)
		node->children[i] = NULL;
	return node;
}

trie_t *trie_create(int data_size, int alphabet_size, char *alphabet)
{
	trie_t *trie = malloc(sizeof(trie_t));
	DIE(!trie, "Error in malloc() for trie\n");

	trie->data_size = data_size;
	trie->alphabet_size = alphabet_size;

	trie->alphabet = malloc(alphabet_size * sizeof(char));
	DIE(!trie->alphabet, "Error in malloc() for alphabet\n");

	//the alphabet is copied to my trie
	memcpy(trie->alphabet, alphabet, alphabet_size * sizeof(char));

	trie->size = 1;
	trie->root = trie_create_node(trie);

	return trie;
}

void trie_insert(trie_t *trie, char *key, char *value)
{
	trie_node_t *aux = trie->root;

	//we add the necessary nodes (representing letters) to form our word
	for (unsigned int i = 0; i < strlen(key); i++) {
		if (!aux->children[key[i] - 'a']) {
			aux->children[key[i] - 'a'] = trie_create_node(trie);
			(aux->n_children)++;
		}

		aux = aux->children[key[i] - 'a'];
	}

	strcpy(aux->value, value);

	//the word appears for the first time
	aux->attemps = 1;
	aux->end_of_word = 1;
}

//the function checks if we have included the word in the dictionary
int trie_search(trie_t *trie, char *key)
{
	trie_node_t *aux = trie->root;

	for (unsigned int i = 0; i < strlen(key); i++) {
		if (!aux->children[key[i] - 'a'])
			return 0;
		aux = aux->children[key[i] - 'a'];
	}

	if (aux->end_of_word == 0)
		return 0;

	aux->attemps++;

	return 1;
}

void rec_autocorrect(trie_t *trie, char *word,
					 trie_node_t *node, int len, int k, int *ok)
{
	if (!len) {
		if (node->end_of_word) {
			char compare[LENGTH];
			strcpy(compare, node->value);

			//check how many letters differ
			for (int i = 0; i < strlen(word); i++)
				if (word[i] != compare[i])
					k--;

			//if a word is found, we display it
			if (k >= 0) {
				printf("%s\n", compare);
				*ok = 1;
			}
		}
		return;
	}

	//we recursively go through the entire dictionary
	for (int i = 0; i < trie->alphabet_size; i++) {
		if (node->children[i])
			rec_autocorrect(trie, word, node->children[i], len - 1, k, ok);
	}
}

void autocorrect(trie_t *trie, char *word, int k)
{
	int len = strlen(word);
	//"ok" checks if we have words to display for the autocorrect operation
	int ok = 0;

	rec_autocorrect(trie, word, trie->root, len, k, &ok);

	if (!ok)
		printf("No words found\n");
}

int rec_remove(trie_node_t *node, trie_t *trie, char *key)
{
	//it is checked if we have reached the end of the word that needs
	//to be eliminated
	if (*key == '\0') {
		//it is checked whether we have that word or not in the dictionary
		if (node->end_of_word == 1) {
			(trie->size)--;
			node->end_of_word = 0;
			node->attemps = 0;

			return node->n_children == 0;
		}
		return 0;
	}

	//the algorithm is run recursively on the existing letters of the word
	if (node->children[*key - 'a'] &&
		rec_remove(node->children[*key - 'a'], trie, key + 1)) {
		free(node->children[*key - 'a']->children);
		free(node->children[*key - 'a']);

		(node->n_children)--;
		node->children[*key - 'a'] = NULL;

		if (node->n_children == 0 && node->end_of_word == 0)
			return 1;
	}

	return 0;
}

void trie_remove(trie_t *trie, char *key)
{
	if (rec_remove(trie->root, trie, key))
		return;
}

void lowest(trie_node_t *node, int *ok)
{
	//when the first lexicographic word is found, "ok" becomes 1
	//and thus the execution stops
	if (node->end_of_word) {
		char word[LENGTH];
		strcpy(word, node->value);
		printf("%s\n", word);
		*ok = 1;
	}

	for (int i = 0; i < ALPHABET_SIZE; i++) {
		if (*ok)
			return;

		if (node->children[i])
			lowest(node->children[i], ok);
	}
}

void smallest(trie_node_t *node, char *word, int contor, int *mini, int *ok)
{
	//if we have a length greater than the minimum, we stop the execution
	if (contor >= *mini)
		return;

	//we save the word as the smallest
	if (node->end_of_word) {
		strcpy(word, node->value);
		*mini = contor;
		*ok = 1;
		return;
	}

	for (int i = 0; i < ALPHABET_SIZE; i++) {
		if (node->children[i])
			smallest(node->children[i], word, contor + 1, mini, ok);
	}
}

void most_used(trie_node_t *node, char *word, int *maxi, int *ok)
{
	//if we have a word, it is checked to have appeared more times
	//than the current maximum
	if (node->end_of_word) {
		if (node->attemps > *maxi) {
			strcpy(word, node->value);
			*maxi = node->attemps;
			*ok = 1;
		}
	}

	//the word dictionary is traversed recursively
	for (int i = 0; i < ALPHABET_SIZE; i++) {
		if (node->children[i])
			most_used(node->children[i], word, maxi, ok);
	}
}

void autocomplete(trie_t *trie, char *prefix, int crt)
{
	trie_node_t *aux = trie->root;

	//the prefix is searched in the dictionary
	for (unsigned int i = 0; i < strlen(prefix); i++) {
		if (!aux->children[prefix[i] - 'a']) {
			printf("No words found\n");

			if (!crt) {
				printf("No words found\n");
				printf("No words found\n");
			}

			return;
		}
		aux = aux->children[prefix[i] - 'a'];
	}

	int ok = 0, contor = 100;
	char word[LENGTH];

	//the words are displayed according to the required criteria
	if (crt == 0) {
		lowest(aux, &ok);

		if (!ok) {
			printf("No words found\n");
			printf("No words found\n");
			printf("No words found\n");
			return;
		}

		smallest(aux, word, 0, &contor, &ok);
		printf("%s\n", word);
		contor = 0;
		most_used(aux, word, &contor, &ok);
		printf("%s\n", word);

	} else if (crt == 1) {
		lowest(aux, &ok);

		if (!ok)
			printf("No words found\n");

	} else if (crt == 2) {
		smallest(aux, word, 0, &contor, &ok);

		if (ok)
			printf("%s\n", word);
		else
			printf("No words found\n");

	} else if (crt == 3) {
		contor = 0;
		most_used(aux, word, &contor, &ok);

		if (ok)
			printf("%s\n", word);
		else
			printf("No words found\n");
	}
}

//words from the dictionary are freed recursively
void rec_free(trie_node_t **node, trie_t *trie)
{
	for (int i = 0; i < trie->alphabet_size; i++) {
		if ((*node)->children[i])
			rec_free(&((*node)->children[i]), trie);
	}

	free((*node)->children);
	free(*node);
}

void trie_free(trie_t **trie)
{
	rec_free(&(*trie)->root, *trie);
	free((*trie)->alphabet);
	free(*trie);
}

int main(void)
{
	char line[LENGTH], comm[LENGTH], word[LENGTH], file_name[LENGTH];
	char *p;
	//with the "ok" variable, it is checked if the memory was freed
	//once the program was stopped
	int index, check_sscanf, ok = 0;

	trie_t *trie = trie_create(sizeof(int), ALPHABET_SIZE, ALPHABET);

	while (fgets(line, LENGTH, stdin)) {
		line[strlen(line) - 1] = 0;
		check_sscanf = sscanf(line, "%s", comm);

		if (!strcmp(comm, "INSERT")) {
			check_sscanf = sscanf(line, "%*s%s", word);

			if (!trie_search(trie, word))
				trie_insert(trie, word, word);

		} else if (!strcmp(comm, "LOAD")) {
			check_sscanf = sscanf(line, "%*s%s", file_name);
			FILE *f = fopen(file_name, "rt");
			fscanf(f, "%s", word);

			while (!feof(f)) {
				if (!trie_search(trie, word))
					trie_insert(trie, word, word);
				fscanf(f, "%s", word);
			}

			fclose(f);
		} else if (!strcmp(comm, "REMOVE")) {
			check_sscanf = sscanf(line, "%*s%s", word);
			trie_remove(trie, word);

		} else if (!strcmp(comm, "AUTOCORRECT")) {
			check_sscanf = sscanf(line, "%*s%s%d", word, &index);
			autocorrect(trie, word, index);

		} else if (!strcmp(comm, "AUTOCOMPLETE")) {
			check_sscanf = sscanf(line, "%*s%s%d", word, &index);
			autocomplete(trie, word, index);

		} else if (!strcmp(comm, "EXIT")) {
			trie_free(&trie);
			ok = 1;
			break;
		}
	}

	if (!ok)
		trie_free(&trie);

	return 0;
}
