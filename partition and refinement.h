#ifndef NAUTY_PARTITION_AND_REFINEMENT_H
#define NAUTY_PARTITION_AND_REFINEMENT_H

/*
 * partition and refinement.h
 * Purpose: Provide a solid and efficient Data Structure to work with Partitions. This is mainly done via a:
 * (i) n-element integer vector element_vec. The elements in each cell are stored in consecutive
 * entries of element array so that the cells themselves form contiguous subarrays.
 *
 * (ii) An ordered, doubly linked list of cell structures describing the cells. Each cell structure has 1. the
 * integer ﬁelds 'first' and 'length' defining the subarray element_vec[first, first+1, ..., first+length-1] and
 * 2. a field 'in_level' detailing at which level the cell was created.
 *
 * (iii) An n-element array of iterators to the cell structures in the list. For a vertex x, the element in cell[x] is
 * the iterator of the position of the cell structure in the list, the corresponding cell contains x.
 *
 * Additionally there is an ordered doubly linked list keeping track of all the non_singleton cells of the partition and
 * there is the field 'level' and a vector of refinement stacks storing information of the refinement
 * procedure to reconstruct the partition at a given level.
 *
 *
 * I willingly renounced the use of the auto identifier for the most part in this file and the .cpp because for me it
 * is more important to be very clear about which kind of iterator I am using to avoid confusion.
 *
 * The main reference for the implementation of this data structure is:
 * Junttila, Tommi A. and Petteri Kaski.
 * “Engineering an Efficient Canonical Labeling Tool for Large and Sparse Graphs.”
 * ALENEX (2007).
 */




#include <vector>
#include <numeric>
#include <iostream>
#include <list>
#include <stack>
#include <map>

#include "adjacencymatrix.h"


/*
 * The matrices are always handled as vectors of vectors containing booleans, this type is shortened to Graph and
 * InvarType is used for an invariant that comes up.
 * Vertex is used when representing what is supposed to be a vertex
 */
using Graph = std::vector<std::vector<bool>>;
using InvarType = std::vector<int>;
using Vertex = int; //unsigned

/*
 * CellStruct
 * Purpose: A set of data representing a cell of the Partition via the element_vec
 * first: the index in element_vec of the first element of the cell
 * length: CellStruct represents the elements element_vec[first, first+1, ..., first+length-1]
 * in_level: at which level the cell was created
 *
 * Has a constructor simply initialising the fields with the given values.
 * And an equal operator, returning true if all fields are equal.
 */
struct CellStruct{
    int first;
    int length;
    int in_level;
    CellStruct(int input_first, int input_length, int input_in_level);
    bool operator==(const CellStruct& rhs) const;
};


/*
 * Partition
 * Purpose: A set of data representing a Partition as described above with according functions to act on it
 *
 * Member variables:
 * element_vec: contains all the vertices of the graph in the order corresponding to the cells of the Partition
 * lcs: a list of CellStruct's to keep track of the cell of the partition
 * in_cell: for each vertex v in_cell[v] is an iterator of the list containing the cell structures
 * non_singleton: a ordered list of iterators to lcs with only the non singleton cells
 * level: level of the partition as in the level of the node in the search tree this partition belongs to
 * refinement_stacks: keeps for each partition on a previous level the necessary info in a stack to return to that level
 *                    this info is roughly the first field of each cell that was newly created
 * 
 * Simple member functions:
 * Partition(): constructs an empty partition with zero elements
 * Partition(n): initializes the unit partition on n vertices
 * Partition(other_format_partition): Transforms a partition of the format vector of vectors of ints to a class Object
 * Partition(old): Copies and constructs a given Partition
 * operator=(rhs): Implements an assignment operator
 * Print(): Simply outputs the partition in the format [[cell1][cell2]...[cellk]]
 * print_detail(): Next to what print does, this outputs to each vertex the first field of the cell it belongs to +more
 * print_non_singleton(): Only outputs the cells with more than 1 element, same format as print
 * is_discrete(): Returns a bool of whether the partition is discrete
 * get_size(): Returns the number of elements in the partition
 * number_of_cell(): Returns the number of cells of the partition
 * get_first_of_cell(element): Returns the 'first' field of the cell the element lies in
 * decode_given_cell(cell): Return the elements represented by the given CellStruct in element_vec
 *
 */
struct Partition{
private:
    std::vector<Vertex> element_vec;
    std::list<CellStruct> lcs;
    std::vector<std::list<CellStruct>::iterator> in_cell;
    std::list<std::list<CellStruct>::iterator> non_singleton;
    int level;
    std::vector<std::stack<int>> refinement_stacks;
public:
    explicit Partition();
    explicit Partition(int n);
    explicit Partition(const std::vector<std::vector<Vertex>> &other_format_partition);
    Partition(const Partition& old);
    Partition& operator=(const Partition& rhs);
    void print() const;
    void print_detail() const;
    void print_non_singleton() const;
    bool is_discrete() const;
    int get_size() const;
    int number_of_cells() const;
    int get_first_of_cell(const int& element) const;
    std::vector<Vertex> decode_given_cell(const CellStruct& cell) const;


    /*
     * Main functions needed for the key parts of the refinement and later canonical labelling
     */

private:
    /*
     * decomposition(graph, cell_v, cell_w) Decomposes elements of cell_v according to their degree to cell_w.
     *                                      Is used as a helper function for refinement.
     *
     * Parameter: graph A graph in which the vertices lie and is needed to decompose according to degree
     *            cell_v, cell_w Both vectors of int's representing vertices of the graph
     * Returns: A vector of subsets of cell_v in ascending order in regards to their elements degree to cell_w
     */
    static std::vector<std::vector<int>> decomposition(const Graph& graph, const std::vector<int>& cell_v, const std::vector<int>& cell_w) ;

public:
    /*
     * refinement(graph, pi, subsequence) Performs the refinement procedure as given in (2013) McKay on the
     *                                     partition it is called on. Stores information used for backtracking
     *                                     to previous partitions.
     *
     * Parameter: graph The procedure refines the partition according to this given graph
     *            subsequence A list of some cells of pi, used for the refinement. In most cases, that will either be
     *              all cells of pi or a single trivial one after splitting by a vertex. In these cases, the returned
     *              partition will be a coarsest equitable refinement of pi.
     *
     * Action: The partition is now the coarsest equitable refinement of the old partition
     */
    void refinement(const Graph& graph, std::list<CellStruct> subsequence = std::list<CellStruct>());


    /*
     * split_by(graph, pi, vertex) Splits a partition by a vertex and refines the thus obtained partition.
     *  (As in (2013) McKay, this is the individualization by vertex combined with then refining the obtained partition)
     *
     *
     * Parameter: graph The procedure refines the partition according to this given graph
     *            vertex The vertex which we want to be individualized. The cell of vertex mus not be a singleton.
     *
     * Action: The partition is now the coarsest equitable refinement of the old partition after it has been split by v
     */
    void split_by_and_refine(const Graph& graph, int vertex);


private:
    /*
     * merge_cell(first_first, last_first) Merges the cells from the cell of first_first to the cell of last_first.
     *                                     Used as a subroutine of reconstruct_at_level
     *
     * Parameter: first_cell The first cell in which we want all the cells till last_cell to be merged
     *            last_cell  The last cell going into the merged cell
     *
     * Action: Makes a single cell again out of all the cells in the given range
     */
    void merge_cells(std::list<CellStruct>::iterator first_cell, std::list<CellStruct>::iterator last_cell);
    //since working on the same object it is ok to pass iterators and they will stay valid

public:
    /*
     * reconstruct_at_level/return_level) Backtracks to make the partition the one it was at return_level
     *
     * Parameter: return_level The level of which we want the old partition of. Has to be positive.
     *
     * Action: The partition is now the one it was at the given level
     */
    void reconstruct_at_level(int return_level);



    /*
     * target_cell_selector(graph, pi, method) Chooses a target cell according to the given method
     *
     * Parameter: graph Information about the graph can also be used in selecting the cell
     *            pi The Partition of which we want to select a cell. It is assumed that pi is not discrete, i.e. has a
     *              non-trivial cell since otherwise it will throw an error
     *            method Decides how the cell is selected
     *
     * Returns: A non-trivial cell chooses according to the given method
     */
    enum TargetcellMethod {first, first_smallest, joins};
    CellStruct target_cell_selector(const Graph& graph, TargetcellMethod method = first_smallest) const;

private:
    /*
     * most_non_trivial_joins(graph, pi) A subroutine used by target_cell_selector
     *
     * Parameter: graph Information about the graph is in this case used in selecting the cell
     *            pi The of which we want to select a cell. It is assumed that pi is not discrete, i.e. has a non-trivial
     *              cell since otherwise it will throw an error
     *
     *
     * Returns: The first non-trivial cell which is non-trivially joined to the most other cells
     */
    CellStruct most_non_trivial_joins(const Graph& graph) const;


public:
    /*
     * shape_invar()
     *
     * Returns: A vector of all the sizes of all cells of the Partition
     */
    InvarType shape_invar();

    /*
     * use_ref_invar
     * ref_invar
     *
     * If use_ref_invar is true, we use the field ref_invar to collect some information about the refinement process,
     * in this case the size of each shattered cell
     */
    bool use_ref_invar = false;
    InvarType ref_invar = InvarType();

};


#endif //NAUTY_PARTITION_AND_REFINEMENT_H
