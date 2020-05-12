#ifndef SPARSE_GRAPH_GRAPH_H
#define SPARSE_GRAPH_GRAPH_H


#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <set>


using Vtype = int;
using Permutation = std::vector<int>;

class Sparse{
    class Vertex {
    public:
        Vertex();
        void add_edge(Vtype vertex);
        int nof_edges() const;
        std::set<Vtype> edges;
    };

public:
    void add_edge(Vtype v1, Vtype v2);
    explicit Sparse(int num_vertices);
    explicit Sparse(const char* filename);
    void dimacs(const char* filename);
    void print() const;
    int nof_vertices() const;
    std::vector<Vertex> vertices;
    int degree(const Vtype & vertex, const std::vector<Vtype>& cell) const;
    std::vector<bool> hash_value() const;
    std::vector<bool> perm_hash_value(const std::vector<int> &perm) const;



};

#endif //SPARSE_GRAPH_GRAPH_H
