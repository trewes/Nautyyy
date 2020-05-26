#ifndef SPARSE_GRAPH_GRAPH_H
#define SPARSE_GRAPH_GRAPH_H

/*
 * sparse_graph.h
 * Purpose: functions to read in graphs into an adjacency list format
 * and implement basic functions on such data
 */



#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <set>


/*
 * As vertex type we use int and Permutations are handled as integer vector
 */
using Vtype = int;
using Permutation = std::vector<unsigned int>;


/*
 * Vertex
 * Purpose: A subclass of Sparse representing a vertex and its edges
 * edges: a set in which vertices are stored representing an edge from that vertex to the ones in the set
 * add_edge(vertex): adds the vertex to the set edges
 * nof_edges(): degree of vertex
 *
 * Sparse
 * Purpose: A set of data representing a sparse graph
 * Vertex: as described above
 * vertices: a vector storing all Vertex objects of the graph
 * add_edge(v1,v2): self explanatory
 * Sparse(n): Constructs a graph with n vertices and no edges
 * Sparse(filename):
 * print(): Outputs the graph as the adjacency list it is
 * nof_vertices(): returns the number of vertices
 */
class Sparse{
    class Vertex {
    public:
        Vertex();
        void add_edge(Vtype vertex);
        unsigned int nof_edges() const;
        std::set<Vtype> edges;
    };

public:
    std::vector<Vertex> vertices;
    void add_edge(Vtype v1, Vtype v2);
    explicit Sparse(unsigned int num_vertices);
    void print() const;
    unsigned int nof_vertices() const;

    /*
     * Sparse(filename) creates an adjacency list structure to a given input graph
     *
     * Parameters: filename Name or path to a .txt file of either of the following formats
     *      Format 1:               //adjacency list
     *      num_nodes               //number of nodes of the graph
     *      node1_1 node1_2         //meaning there is and edge between node1_1 and node1_2
     *      node2_1 node2_2
     *      ...
     *
     *      Format 2:               //adjacency matrix
     *      num_nodes               //number of nodes of the graph
     *      10010...011             //the first row of the matrix, filled with 0 or 1
     *
     * Only undirected and simple graphs are accepted, i.e. no loops or parallel edges are allowed.
     */
    explicit Sparse(const char* filename);

    /*
     * dimacs(filename)
     *
     *  For reference to the dimacs file format for graphs, see http://www.tcs.hut.fi/Software/bliss/fileformat.shtml
     */
    void dimacs(const char* filename);

    /*
     * degree(vertex, cell)
     *
     * Parameter: vertex and cell
     *
     * Returns: The degree of vertex into the set cell
     *
     * Not perfect and its use is being avoided except for the most_joins targect cell selector
     */
    int degree(const Vtype & vertex, const std::vector<Vtype>& cell) const;

    /*
     * hash_value()
     *
     * Returns: The hash value of the graph it is called on. This is simply given as n^2 bit string concatenating
     *          the rows of an adjacency matrix.
     */
    std::vector<bool> hash_value() const;

    /*
     * hash_value()
     *
     * Same as hash_value but returns the hash of the graph one would get after permuting the graph according to perm.
     * This is better since an explicit construction of the permuted graph is not needed.
     */
    std::vector<bool> perm_hash_value(const Permutation& perm) const;

};

#endif //SPARSE_GRAPH_GRAPH_H
