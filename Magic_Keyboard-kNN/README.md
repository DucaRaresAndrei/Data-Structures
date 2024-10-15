Duca Andrei-Rares
311CA

			Magic Keyboard problem
________________________________________________________________________________

	Through the trie_create_node function we allocate memory for a node that contains a letter.

	Through the trie_create function we create our trie and add the root.
	
	Through the trie_insert function, we go through the letters or add them, if there are none, until we get the desired word.

	The trie_search function is used to know if we add a new word to the dictionary, or if we increase the number of uses of the already existing word.

	Through the autocorrect and rec_autocorrect functions, we go through the words in the dictionary and check how many letters are distinct from the searched word. If it falls within the limits, we display it.

	Through the trie_remove and rec_remove functions, we go through the dictionary and delete the desired word, respectively the letters that make it up, if necessary.

	The autocomplete function calls the functions: lowest, smallest and most_used to display the desired words, according to the required criteria.

	The functions trie_free and rec_free free the memory of the dictionary.
	
   The implemented data structure behaves well in the case of dictionary traversals, but behaves badly in the memory used.
________________________________________________________________________________

			kNN problem
________________________________________________________________________________

	The create_tree function reads the data from the file and calls the tree creation function. The address of the root and the values of n and k read are returned.

	The kdtree function recursively sorts the vector of points, based on the current axis, calculates the median and saves in the current node the point whose index is the median and then recursively traverses the 2 halves, upper and lower, of the vector of points.

	The knn and find_knn functions read the point for which the points with the smallest distance to it are searched and these are displayed, sorted according to each dimension.

	Through the functions rs, range_search and check_range, the 2 points representing the range [start, end] are read, through which all the points included in the range are searched and are displayed, after sorting according to each dimension.

   The implemented data structure behaves well to find the points that belong to the interval and behaves badly when we need to find the closest points, thus calculating the Euclidean distance for all points in the tree.

