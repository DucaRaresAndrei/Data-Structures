/* Copyright 2023 Duca Andrei-Rares, 311CA */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

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

typedef struct point_struct point_struct;
struct point_struct {
	int *dimensions;

	int size;
};

typedef struct node_struct node_struct;
struct node_struct {
	point_struct *point;

	struct node_struct *l_child;
	struct node_struct *r_child;
};

node_struct *kdtree(point_struct **points, int left, int right,
					int level, int k)
{
	if (left > right)
		return NULL;

	int axis = level % k;

	//the points are sorted according to the current axis
	for (int i = left; i < right; i++) {
		for (int j = i + 1; j <= right; j++) {
			if (points[i]->dimensions[axis] > points[j]->dimensions[axis]) {
				point_struct *aux = points[i];
				points[i] = points[j];
				points[j] = aux;
			}
		}
	}

	//the median is calculated
	int median = left + (right - left) / 2;
	node_struct *node = malloc(sizeof(node_struct));
	//the current node receives the point with the median index
	node->point = points[median];
	//we go through the lower and upper half in turn
	node->l_child = kdtree(points, left, median - 1, level + 1, k);
	node->r_child = kdtree(points, median + 1, right, level + 1, k);

	return node;
}

node_struct *create_tree(int *n, int *k)
{
	char file_name[LENGTH];
	point_struct **points;
	node_struct *root;

	scanf("%s", file_name);

	FILE *file = fopen(file_name, "rt");
	DIE(!file, "Error opening file\n");

	fscanf(file, "%d %d", n, k);
	points = malloc(*n * sizeof(point_struct *));
	DIE(!points, "Error in malloc() for points\n");

	//all points from the file are read and loaded into memory
	for (int i = 0; i < *n; i++) {
		points[i] = malloc(sizeof(point_struct));
		DIE(!points[i], "Error in malloc() for points[i]\n");

		points[i]->dimensions = malloc(*k * sizeof(int));
		DIE(!points[i]->dimensions,
			"Error in malloc() for points[i]->dimensions\n");

		for (int j = 0; j < *k; j++)
			fscanf(file, "%d", &points[i]->dimensions[j]);

		points[i]->size = *k;
	}

	fclose(file);

	//the address of the root of the tree is received
	root = kdtree(points, 0, *n - 1, 0, *k);
	free(points);
	return root;
}

//the function returns the Euclidean distance between the 2 points
double euclid_distance(point_struct *a, point_struct *b)
{
	int k = a->size;
	int distance = 0;

	for (int i = 0; i < k; i++) {
		int diff = b->dimensions[i] - a->dimensions[i];
		distance = distance + diff * diff;
	}

	return sqrt(distance);
}

//the function used by qsort to sort according to each dimension, if the
//previous ones are equal
int compare(const void *p1, const void *p2)
{
	point_struct *point1 = *(point_struct **)p1;
	point_struct *point2 = *(point_struct **)p2;

	for (int i = 0; i < point1->size; i++) {
		if (point1->dimensions[i] < point2->dimensions[i])
			return -1;
		else if (point1->dimensions[i] > point2->dimensions[i])
			return 1;
	}

	return 0;
}

void print(point_struct **points, int count)
{
	//if we have no points to display, we leave the function
	if (!count)
		return;

	int k = points[0]->size;

	//if we have more than one point, we perform the sorting using
	//our comparison function
	if (count > 1)
		qsort(points, count, sizeof(point_struct *), compare);

	//we display the points
	for (int i = 0; i < count; i++) {
		point_struct *point = points[i];

		for (int j = 0; j < k; j++)
			printf("%d ", point->dimensions[j]);

		printf("\n");
	}
}

void find_knn(node_struct *root, point_struct *target,
			  point_struct **points, int *count, int level)
{
	if (!root)
		return;

	int k = root->point->size;

	//we calculate the current and minimum distances
	double curr_dist = euclid_distance(root->point, target);
	double lowest_dist = euclid_distance(points[0], target);

	//depending on the result, we only stay with the current point or we add a
	//new point with the same distance as what we had so far in the vector
	if (curr_dist < lowest_dist) {
		for (int i = *count; i > 0; i--)
			points[i] = NULL;

		points[0] = root->point;
		*count = 1;
	} else if (curr_dist == lowest_dist) {
		if (points[0] != root->point) {
			points[*count] = root->point;
			(*count)++;
		}
	}

	find_knn(root->l_child, target, points, count, level + 1);
	find_knn(root->r_child, target, points, count, level + 1);
}

void knn(node_struct *root, int n)
{
	int k = root->point->size;

	point_struct *target = malloc(sizeof(point_struct));
	DIE(!target, "Error in malloc() for target\n");

	target->dimensions = malloc(k * sizeof(int));
	DIE(!target->dimensions, "Error in malloc() for target->dimensions\n");

	//the point whose minimum distance from it is sought is read
	for (int i = 0; i < k; i++)
		scanf("%d", &target->dimensions[i]);

	//in the vector "points" the addresses of the points will be saved,
	//which we will display later
	point_struct **points = malloc(n * sizeof(point_struct *));
	DIE(!points, "Error in malloc() for points\n");

	//we add the root to have something to compare with initially
	points[0] = root->point;
	int count = 1;

	find_knn(root, target, points, &count, 0);

	print(points, count);

	//the auxiliary memory is freed
	free(target->dimensions);
	free(target);
	free(points);
}

//it is checked if the point is included in the interval [start, end]
int check_range(point_struct *point, point_struct *start, point_struct *end)
{
	int k = point->size;

	for (int i = 0; i < k; i++) {
		if (start->dimensions[i] > point->dimensions[i] ||
			end->dimensions[i] < point->dimensions[i]) {
			return 0;
		}
	}

	return 1;
}

void range_search(node_struct *root, point_struct *start, point_struct *end,
				  point_struct **points, int *count, int level)
{
	if (!root)
		return;

	point_struct *curr = root->point;

	range_search(root->l_child, start, end, points, count, level + 1);

	int ok = check_range(curr, start, end);

	//if it is a valid point, we add it to the vector of points
	if (ok) {
		points[*count] = root->point;
		(*count)++;
	}

	range_search(root->r_child, start, end, points, count, level + 1);
}

void rs(node_struct *root, int n)
{
	int k = root->point->size;
	point_struct *start = malloc(sizeof(point_struct));
	DIE(!start, "Error in malloc() for start\n");

	start->dimensions = malloc(k * sizeof(int));
	DIE(!start->dimensions, "Error in malloc() for start->dimensions\n");

	point_struct *end = malloc(sizeof(point_struct));
	DIE(!end, "Error in malloc() for end\n");

	end->dimensions = malloc(k * sizeof(int));
	DIE(!end->dimensions, "Error in malloc() for end->dimensions\n");

	//read the points that hold the start and end dimensions
	for (int i = 0; i < k; i++) {
		scanf("%d", &start->dimensions[i]);
		scanf("%d", &end->dimensions[i]);
	}

	//in the vector "points" the addresses of the points will be saved,
	//which we will display later
	point_struct **points = malloc(n * sizeof(point_struct *));
	DIE(!points, "Error in malloc() for points\n");

	int count = 0;

	range_search(root, start, end, points, &count, 0);

	print(points, count);

	//the auxiliary memory is freed
	free(points);
	free(start->dimensions);
	free(start);
	free(end->dimensions);
	free(end);
}

//the memory of all the nodes containing the points is freed, recursively
void free_tree(node_struct *parent)
{
	if (parent->l_child)
		free_tree(parent->l_child);

	if (parent->r_child)
		free_tree(parent->r_child);

	free(parent->point->dimensions);
	free(parent->point);
	free(parent);
}

int main(void)
{
	char comm[LENGTH];
	struct node_struct *root;
	int n, k;

	while (1) {
		scanf("%s", comm);

		if (!strcmp(comm, "LOAD")) {
			//we get the address of the root and the values of n and k
			root = create_tree(&n, &k);

		} else if (!strcmp(comm, "NN")) {
			knn(root, n);

		} else if (!strcmp(comm, "RS")) {
			rs(root, n);

		} else if (!strcmp(comm, "EXIT")) {
			free_tree(root);
			break;
		}
	}

	return 0;
}
