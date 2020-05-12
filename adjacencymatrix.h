#ifndef NAUTY_ADJACENCYMATRIX_H
#define NAUTY_ADJACENCYMATRIX_H


/*
 * adjacencymatrix.h
 * Purpose: functions to read in graphs into an adjacencymatrix format
 * and implement basic functions on such data
 *
 * graph and adjacency matrix are used synonymously in the context of this file
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <stdexcept>


/*
 * The matrices are always handled as vectors of vectors containing booleans, this type is shortened to Matrix
 */
using Index = unsigned int;
using Permutation = std::vector<int>;
using Graph = struct GraphStruct{
    std::vector<bool> matrix{};
    Permutation perm{};
    int size() const{
        return perm.size();
    }
    void set(Index i, Index j){     //only used when reading in the graph
        if(i<j){
            std::swap(i,j);
        }
        int n = size();
        //int index = (n*(n-1))/2 - ((n-i)*((n-i)-1))/2 + j - i - 1;
        int index = (i*(2*n-i-1))/2+j;
        matrix[index] = true;
    }
    bool get(Index i, Index j) const{
        i=perm[i];
        j=perm[j];
        if(i==j){std::cout<<"ERROR"<<std::endl; return false;}
        if(i<j){
            std::swap(i,j);
        }
        int n = size();
        //int index = (n*(n-1))/2 - ((n-i)*((n-i)-1))/2 + j - i - 1;
        //std::cout<<index<<" from "<<i<<" "<<j<<std::endl;
        int index = (i*(2*n-i-1))/2+j;
        return matrix[index];
    }
    GraphStruct() = default;
    explicit GraphStruct(int n){
        matrix= std::vector<bool>((n-1)*n/2, false);
        perm = Permutation(n);
        for(int i=0; i<n; i++){
            perm[i] = i;
        }
    }
    GraphStruct(const GraphStruct& old){
        matrix = old.matrix;
        perm = old.perm;
    }
    GraphStruct& operator=(const GraphStruct& old){
        matrix = old.matrix;
        perm = old.perm;
        return *this;
    }

};

/*
 * adjacency_matrix(filename) creates an adjacency matrix to a given input graph
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
 *Only undirected and simple graphs are accepted, i.e. no loops or parallel edges are allowed
 *
 * Returns: Matrix The adjacency matrix to the graph saved in the given file
 *          without loops or parallel edges
 */
Graph adjacency_matrix(char const* filename);

/*
 *  dimacs(filename)
 *
 *  For reference to the dimacs file format for graphs, see http://www.tcs.hut.fi/Software/bliss/fileformat.shtml
 */
Graph dimacs(char const* filename);

/*
 * print_matrix(adjacency_matrix) Prints the matrix in a simple manner
 *
 * Parameter: The adjacency matrix that should be printed
 */
void print_matrix(const Graph& graph);

/*
 * value_of_graph(graph) The rows of the matrix written down row by row and interpreted as a n^2-bit string
 *
 * Parameter: graph A graph is given in the form of an n by n adjacency matrix
 * Returns: An n^2-bit string
 */
std::vector<bool> value_of_graph(const Graph& graph);



/*
 * degree(graph, vertex, cell)
 *
 * Parameter: The graph, a vertex of it, and a subset of it's vertices
 * Returns: The count of Edges going from vertex to the cell
 */
int degree(const Graph& graph, const int& vertex, const std::vector<int>& cell);




#endif //NAUTY_ADJACENCYMATRIX_H
