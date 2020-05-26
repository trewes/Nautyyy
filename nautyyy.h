#ifndef NAUTY_NAUTYYY_H
#define NAUTY_NAUTYYY_H


/*
 * nautyyy.h
 * Purpose: Implementation of the actual search tree traversal and thus finding a canonical labeling. For this a class
 * Nautyyy is used to store all the necessary data during the algorithm and thus most functions of the algorithm are
 * made member functions of Nautyyy. So to calculate the canonical isomorph/labeling one has to construct a Nautyyy
 * object with that graph.
 * Auxiliary structs include Statistics for general runtime info of the algorithm and Options to set certain specific
 * parameters of the algorithm plus another, Leaf, to store easily store first_leaf or best_leaf and their data.
 */

#include <vector>
#include <iostream>
#include <chrono>
#include <stdexcept>

#include "sparse_graph.h"
#include "partition and refinement.h"
#include "permutation group.h"

/*
 * Self-explanatory typedefs of certain types.
 */
using Graph = Sparse;
using Permutation = std::vector<unsigned int>;
using PermGroup = std::vector<Permutation>;
using InvarType = std::vector<unsigned int>;
using Vertex = unsigned int;


/*
 * Statistics
 * Purpose: Used as a field in Nautyyy to store various info accumulated during the execution of the algorithm.
 *
 * The member variables and their purpose/represented data are obvious from their name.
 * print(): Outputs and describes most fields of the struct in a simple way.
 * pretty_time(): Outputs the time it took the algorithm to finish in a human readable format
 *                It should be mentioned that time is measured without time it takes to read in the graph or copy it
 */
struct Statistics{
    unsigned int refinements_made = 0;
    unsigned int leaves_visited = 0;
    unsigned int best_leaf_updates = 0;
    unsigned int num_bad_leaves = 0;
    unsigned int max_level = 0;
    unsigned int num_pruned_by_auto = 0;
    unsigned int num_pruned_by_invar = 0;
    unsigned int automorphisms_found = 0;
    unsigned int times_backtracked = 0;
    unsigned int total_target_cells = 0;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::duration<double> execution_time;
    void print() const;
    void pretty_time() const;
};

/*
 * Options
 * Purpose: Used as a field in Nautyyy and employed to set specifics of how the algorithm is executed. But be careful,
 * running Nautyyy with different settings may result in a different canonical isomorph/labelling.
 *
 * invarmethod: determines which node invariant is used for pruning. none means no pruning is done,
 *              shape is the invariant of a vector of the cell sizes of a partition
 *              refinement consists of info collected during refinement, here: all sizes of ocurring shattered cells
 *              num_cells is just the number of cells in the current partition
 * targetcellmethod: determines what target cell is selected, for further info see partition and refinement.h
 *                   reminder: options are first, first_smallest and joins
 * explore_first_path: A consideration is to not prune by node invariant while exploring the first path (first child of
 *                     root node). true means we do this and not prune in the beginning, false means always prune
 * use_unit_partition: If true, Nautyyy starts with the unit partition and the root node will be the refinement of that
 * input_partition: Otherwise one can supply their own starting partition
 * print_stats: Whether the statistics are printed at the end of an execution or not
 * print_time: Whether the execution time is printed at the end
 * max_level_tc: Should be used very optionally, allows one specify a level until which a second
 *               (stronger but more costly) target cell selector should be use
 * strong_targetcellmethod: specifies the optionally used stronger selector
 *
 */
struct Options{
    enum InvarMethod {none, shape, refinement, num_cells};
    InvarMethod invarmethod = shape;
    Partition::TargetcellMethod targetcellmethod = Partition::first;
    bool explore_first_path = true;
    bool use_unit_partition = true;
    Partition input_partition = Partition();
    bool print_stats = false;
    bool print_time = false;
    unsigned int max_level_strong_tc = 0;
    Partition::TargetcellMethod strong_targetcellmethod = Partition::joins;
};


/*
 * Leaf
 * Purpose: Used for field first_leaf and best_leaf of Nautyyy, store needed info about leaf
 *
 * vertex_sequence: An ordered list of vertices indicating which child was chosen at which level to get to leaf
 * leaf_perm: The partition corresponding to the discrete partition of the leaf
 * hash_of_perm_graph: The associated hash value of the graph after permuting by leaf_perm
 *
 * Leaf(): an 'empty' Leaf, has all member variables default initialized
 * Leaf(in_vertex_sequence, in_leaf_perm, hash_val): Constructs a Leaf with the given values for the variables
 * undiscovered(): Whether the Leaf is 'empty' or has been filled with values, i.e. is an actual Leaf
 */
struct Leaf{
    std::vector<unsigned int> vertex_sequence;
    Permutation leaf_perm;
    std::vector<bool> hash_of_perm_graph;
    Leaf();
    Leaf(std::vector<unsigned int> in_vertex_sequence, Permutation  in_leaf_perm, std::vector<bool>  hash_val);
    bool undiscovered() const;
};



/*
 * Nautyyy
 * Purpose: Class that manages the high-level execution of the Algorithm to calculate a canonical isomorph/labelling
 *
 * Member varaibales:
 * stats: A Statistics object in which certain general execution data is kept track of
 * opt: An Option object with settings of how the algorithm is run
 * graph: The graph for/on which we perform the algorithm
 * current_level: the level of the node in the search tree we are currently processing.
 *                if this is zero, the algorithm terminates
 * current_partition: A Partition object which is the only partition we will working on, will be split and refined as
 *                    as well as restored to partitions on previous levels many times.
 * found_automorphism: In this the automorphisms discovered during the search tree traversal are stored
 * unbranched: Stores the yet unexplored children at each level even while in the process of working on later nodes.
 *             When backtracking the vector is resized so last element is target cell at level we wanted to return to
 * current_vertex_sequence: An ordered list of vertices indicating which child was chosen at each level
 *                          an split by to get to this current node.
 * first_leaf: The first ever encountered leaf (or its data) is saved here
 * best_leaf: The so far best discovered guess for a leaf giving a  canonical isomorph is saved and updated here
 * max_invar_at_level: A vector of InvarType's to store the greatest invar found at each level. Since the ordering is
 *                     lexikographic, we erase all invars after the current level if a new greatest has been found
 *
 * Auxiliary boolean variables:
 * best_leaf_outdated_due_to_invariant: found new max invariant so next encountered leaf will be next max
 * first_path_explored: we do not prune by node invariants for nodes a prefix of the first found leaf,
 *                      i.e. prune by node invariant only after second encounter of root node (if set that way in opt)
 *  first_path_help: helper bool to check if we explored first path
 *
 * Simpler member functions:
 * get_gcs_level(first_sequence, second_sequence): From two nodes an their vertex sequence calculate the level of their
 *                                                greatest common ancestor, i.e. the prefix on which the sequences agree
 *
 *
 */
class Nautyyy{
private:
    Statistics stats;
    const Options opt;
    const Graph graph;
    unsigned int current_level;
    Partition current_partition;
public:
    PermGroup found_automorphisms;                                   //public, if one is interested in the automorphisms
private:
    std::vector<std::vector<Vertex>> unbranched;
    std::vector<Vertex> current_vertex_sequence;
    Leaf first_leaf;
public:
    Leaf best_leaf;                                         //public, since this best leaf gives the canonical labelling
private:
    std::vector<InvarType> max_invar_at_level;

    bool best_leaf_outdated_due_to_invariant = false;
    bool first_path_explored = false;
    bool first_path_help = false;

    static unsigned int get_gca_level(const std::vector<Vertex> &first_sequence, const std::vector<Vertex> &second_sequence);

    /*
     * search_tree_traversal()
     *
     * At each point in the algorithm there is a node represented via current_partition and current_vertex_sequence
     * This function then, while the search tree is not completely traversed yet, either calls
     * process_node or process_node if the node is a leaf.
     */
    void search_tree_traversal();
    /*
     * process_node()
     *
     * Handles the particulars of encountering a node.
     * Specifically, gets a target cell or takes the unbranched on that level, prunes this set with the help of found
     * automorphisms if any bay taking the intersect with mcrs. When there is an unbranched und not pruned element left,
     * take the first/earliest element and split the partition by that vertex, creating the corresponding child node.
     * Finally it manages the specified node invariant of that child node and if possible pruning via the node invariant
     */
    void process_node();
    /*
     * prune_by_invar()
     *
     * Handles the pruning of a node.
     * On non-leaf nodes it gets the in opt:invarmethod specified Invar, compares it to ones already found and either
     * updates maximal invariant, simply goes to the next level or prunes the current node and backtracks to parent.
     * On leaves it only asserts that the leaf is considered greatest, the pruning or rather updating of the best_leaf
     * is handled separately in process_node().
     */
    void prune_by_invar();
    /*
     * process_leaf()
     *
     * Handles the particulars of encountering a leaf
     * Specifically, if it is the first leaf encountered, save it. Then, if we know that leaf is a better guess for the
     * canonical isomorph/labelling, update that. If the leaf is equivalent to either first_leaf or best_leaf that
     * gives us an automorphism of the graph which we add to found_automorphisms
     */
    void process_leaf();
    /*
     * backtrack_to(level)
     *
     * At several point of the algorithm backtracking happens. Either we encountered a leaf that has no children so we
     * once again go back to the parent node (or some ancestor) or we explored all unbranched vertices on a level or
     * the node invariant tells us a the current node will not yield a better best_leaf.
     * If that happens, we want to go back to a previous node, i.e. get that partition (how is detailed in
     * "partition and refinement.h") and reset both unbranched and vertex_sequence up to the return level.
     * Thus we return to a previous node.
     */
    void backtrack_to(unsigned int level);


public:
    /*
     * Nautyyy(filename, option)
     * Nautyyy(graph, option)
     *
     * Constructs a class object, mainly with most things default initialised but handles root node and employs some
     * settings given in opt. Depending on which constructor is used, the graph is copied to Nautyyy or is read in
     * for that purpose via adjacency_matrix(filename).
     * Then it calls search_tree_traversal() and the main algorithm begins.
     */
    explicit Nautyyy(char const* filename, Options options = Options{});
    explicit Nautyyy(Graph  in_graph, Options options = Options{});
};


#endif //NAUTY_NAUTYYY_H
