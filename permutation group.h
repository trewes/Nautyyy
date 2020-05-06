#ifndef NAUTY_PERMUTATION_GROUP_H
#define NAUTY_PERMUTATION_GROUP_H



/*
 * permutation group.h
 * Purpose: Elementary handling of permutations such as composition and inverses, also
 * application of permutations to subsets of [n] or graphs and basic operations on permutations group such as finding
 * stabilisors and orbits (more exactly, minimum cell representations).
 * */

#include <vector>
#include <algorithm>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include "adjacencymatrix.h"
#include "partition and refinement.h"

/*
 * The matrices are always handled as vectors of vectors containing booleans, this type is shortened to Matrix
 * And permutations are integer vectors sending i to permutation[i]. Shortened to Permutation.
 * PermGroup is then a representation of a group of Permutations in it's most basic form
 */
using Graph = std::vector<std::vector<bool>> ;
using Permutation = std::vector<int> ;
using PermGroup =  std::vector<Permutation>;

/*
 * discrete_partition_to_perm(partition) Discrete partitions can be identified
 *                                       with a permutation, that's what this function does
 *
 * Parameter: partition Has to be a discrete partition, throws errors otherwise
 * Returns: The corresponding permutation
 */
Permutation discrete_partition_to_perm(const Partition& partition);

/*
 * perm_inverse(perm) gets the inverse to a given permutation
 *
 * Parameter: A permutation
 * Returns: The inverse of the permutation
 */
Permutation perm_inverse(const Permutation& perm);

/*
 * perm_composition(first_perm, second_perm) Composes two compositions. This is done as first applying the first_perm
 *                                             and then the second, so 'read from left to right'
 *
 * Parameter: Two permutation of the same size
 * Returns: The composition of the two permutations
 */
Permutation perm_composition(const Permutation& first_perm, const Permutation& second_perm);

/*
 * apply_perm(vector, perm) Permutes the elements of vector by the given permutation
 *
 * Parameter: vector A subset with elements smaller than the size of the permutation
 *            perm A permutation
 * Returns: The subset with the elements permuted
 */
std::vector<int> apply_perm(std::vector<int> vector, const Permutation& perm);

/*
 * print_perm(perm) Outputs the permutation in cycle notation. That means only elements that move something are
 *                  printed and the permutation that moves nothing is 'Identity', output as such.
 *
 * Parameter: A permutation
 */
void print_perm(const Permutation& perm);

/*
 * perm_graph(graph, perm) Applies the permutation to the adjacency matrix of the graph
 *
 * Parameter: graph Graph as adjacency matrix
 *            perm The permutation that is applied
 * Returns: The adjacency matrix of graph with rows and columns permuted
 */
Graph perm_graph(const Graph& graph, const Permutation& perm);




//!nw

/*
 * is_fixed(perm, sequence)
 *
 * Parameter: A permutation and a sequence of integers
 *
 * Returns: A bool whether the given permutation fixes the sequence
 */
bool is_fixed(const Permutation& perm, const std::vector<int> &sequence);

//stabilizer
/*
 * subgroup_fixing_sequence(permutations, sequence)
 *
 * Parameter: A vector of permutations and a sequence of integers
 *
 * Returns: A subvector of permutations all of which fix the sequence
 */
PermGroup subgroup_fixing_sequence(const PermGroup &permutations, const std::vector<int> &sequence);

//minimum cell representatives, orbits
/*
 * mcrs(permutations, sequence, num_vertices)
 *
 * Parameter: A vector of permutations and a sequence of integers
 *
 * Returns: An approximate minimum cell representation
 */
std::vector<int> mcrs(const PermGroup& permutations, const std::vector<int>& sequence);

//for more efficiency, faster search for stabilizers and computation of orbits

#endif //NAUTY_PERMUTATION_GROUP_H
