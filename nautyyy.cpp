#include "nautyyy.h"


void Statistics::print() const {
    std::cout<<"Total leaves visited: "<<leaves_visited<<" and automorphisms found: "<<automorphisms_found
             <<". Times pruned by invar: "<<num_pruned_by_invar<<" by automorphisms: "<<num_pruned_by_auto;
             if(num_pruned_implicitly) {
                 std::cout<<" by implicit automorphisms: "<< num_pruned_implicitly;
             }
             std::cout<<".\nRefined " << refinements_made << " times."
             <<" Canonical updates: " << best_leaf_updates<<". Backtracks: "<<times_backtracked
             << ". Reached level: "<<max_level<<", total tc's selected: "<<total_target_cells<<std::endl;
}

void Statistics::pretty_time() const{

    auto duration = execution_time;

    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    duration -= hours;
    if(hours.count()){
        std::cout << hours.count() << "h ";
    }

    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
    duration -= minutes;
    if(minutes.count()){
        std::cout << minutes.count() << "m ";
    }

    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    std::cout << seconds.count() << "s ";

    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    duration -= milliseconds;
    std::cout << milliseconds.count() << "ms";

    if(not milliseconds.count()) {
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);
        duration -= microseconds;
        std::cout <<" "<< microseconds.count() << "us";
    }

    std::cout<<"."<<std::endl;
}

Leaf::Leaf(): vertex_sequence(std::vector<unsigned int>()), leaf_perm(std::vector<unsigned int>()), hash_of_perm_graph(){

}



Leaf::Leaf(std::vector<unsigned int> in_vertex_sequence, Permutation in_leaf_perm, std::vector<bool>  hash_val)
                         : vertex_sequence(std::move(in_vertex_sequence)), leaf_perm(std::move(in_leaf_perm)),
                           hash_of_perm_graph(std::move(hash_val)){

}

bool Leaf::undiscovered() const{
    return leaf_perm.empty();            //if vertex has no assigned permutation, it means it hasn't been discovered yet
}


unsigned int Nautyyy::get_gca_level(const std::vector<Vertex> &first_sequence, const std::vector<Vertex> &second_sequence) {

    for(size_t same_until = 0, max = std::min(first_sequence.size(), second_sequence.size()); same_until < max; same_until++){
        if(first_sequence[same_until] != second_sequence[same_until]){
            return same_until + 1;                                     //the level up to which the vertex sequences agree
        }
    }
    throw std::runtime_error("Error, the two given leaves are the same.");

}

//Little helper function to be able to use the ternary operator during the member initialization
//Necessary since we want the graph, once initialized, to be a const member of Nautyyy
Graph random_perm_of(char const* filename){
    //Create a random permutation the size of number of vertices of the graph
    Sparse g = Sparse(filename);
    Permutation perm(g.nof_vertices());
    std::iota(perm.begin(), perm.end(), 0);
    std::random_device rd;
    std::mt19937 mt(rd());
    //Shuffle the vector on numbers 0...n-1, results in a random permutation
    std::shuffle(perm.begin(), perm.end(), mt);
    return perm_graph(g, perm);
}

                                                               //simple/empty initialization of most fields of the class
Nautyyy::Nautyyy(char const* filename, Options options)
    : stats(Statistics()), opt(std::move(options)),
    graph(options.use_random_perm_of_graph ? random_perm_of(filename): Sparse(filename)),
      found_automorphisms(std::vector<Permutation>()), unbranched(std::vector<std::vector<Vertex>>()),
      current_vertex_sequence(std::vector<Vertex>()), first_leaf(Leaf()), best_leaf(Leaf()),
      max_invar_at_level(std::vector<InvarType>()){

    stats.start_time = std::chrono::steady_clock::now();

    if(opt.use_unit_partition){
        current_partition = Partition(graph.nof_vertices());                              //begin with unit partition
    }
    else{
        //current_partition = opt.input_partition;                                              //or user passes partition
        if(graph.initial_partition.empty()){
            throw std::runtime_error("No initial partition was given.");
        }
        current_partition = Partition(graph.initial_partition);
        if(graph.nof_vertices() != current_partition.get_size()){
            throw std::runtime_error("No complete initial partition was given.");
        }
    }
    current_level = 1;
    stats.max_level = 1;
    current_partition.refinement(graph);                                                       //refine to get root node
    stats.refinements_made++;
    if(opt.invarmethod == Options::refinement){
        current_partition.use_ref_invar = true;            //let partition know to create an invariant during refinement
    }

    search_tree_traversal();                                                           //main function call of algorithm
}


//Little helper function to be able to use the ternary operator during the member initialization
//Necessary since we want the graph, once initialized, to be a const member of Nautyyy
Graph random_perm_of(const Graph& g){
    //Create a random permutation the size of number of vertices of the graph
    Permutation perm(g.nof_vertices());
    std::iota(perm.begin(), perm.end(), 0);
    std::random_device rd;
    std::mt19937 mt(rd());
    //Shuffle the vector on numbers 0...n-1, results in a random permutation
    std::shuffle(perm.begin(), perm.end(), mt);
    return perm_graph(g, perm);
}

//same as other constructor but the graph is passed as graph and not as filename to read in a graph
Nautyyy::Nautyyy(const Graph&  in_graph, Options options)
        : stats(Statistics()), opt(std::move(options)),
          graph(options.use_random_perm_of_graph ? random_perm_of(in_graph): in_graph),
          found_automorphisms(std::vector<Permutation>()), unbranched(std::vector<std::vector<Vertex>>()),
          current_vertex_sequence(std::vector<Vertex>()), first_leaf(Leaf()), best_leaf(Leaf()),
          max_invar_at_level(std::vector<InvarType>()){

    stats.start_time = std::chrono::steady_clock::now();

    if(opt.use_unit_partition){
        current_partition = Partition(graph.nof_vertices());                              //begin with unit partition
    }
    else{
        //current_partition = opt.input_partition;                                              //or user passes partition
        if(graph.initial_partition.empty()){
            throw std::runtime_error("No initial partition was given.");
        }
        current_partition = Partition(graph.initial_partition);
        if(graph.nof_vertices() != current_partition.get_size()){
            throw std::runtime_error("No complete initial partition was given.");
        }
    }
    current_level = 1;
    stats.max_level = 1;
    current_partition.refinement(graph);
    stats.refinements_made++;
    if(opt.invarmethod == Options::refinement){
        current_partition.use_ref_invar = true;
    }

    search_tree_traversal();
}


void Nautyyy::search_tree_traversal() {

    while(current_level>=1) {
        if (not current_partition.is_discrete()) {
            process_node();
        } else {
            stats.leaves_visited++;
            process_leaf();
        }
    }

    std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
    stats.execution_time = (end_time-stats.start_time);

    if(opt.print_stats){
        stats.print();                                  //print optional statistics about the execution of the algorithm
    }
    if(opt.print_time)
    {
        std::cout << "Execution took: ";
        stats.pretty_time();
    }
}



void Nautyyy::process_node(){

                 //first encounter of this node, get target cell but don't prune yet since we explore first child anyway
    if(unbranched.size() < current_level){
        CellStruct target_cell(0, 0, 0);
        if(current_level < opt.max_level_strong_tc) {                             //decide if we use "stronger" selector
            target_cell = current_partition.target_cell_selector(graph, opt.strong_targetcellmethod);
        }
        else {
            target_cell = current_partition.target_cell_selector(graph, opt.targetcellmethod);
        }
        stats.total_target_cells++;
        unbranched.push_back(current_partition.decode_given_cell(target_cell));

        if(opt.use_implicit_pruning) {
            //when the partition is of a certain structure it allows us to infer implicit automorphisms
            //if this is the case, all child nodes of the current node are isomorphic,
            //so only the first one needs to be explored, remove all other children
            //For reference see Lemma 2.25 in McKay (1981)
            unsigned int n = graph.nof_vertices();
            unsigned int m = current_partition.number_of_non_singleton_cells();
            unsigned int pi = current_partition.number_of_cells();
            if ((n <= pi + 4) or (n == pi + m) or (n == pi + m + 1)) {
                stats.num_pruned_implicitly++;
                unbranched.back().erase(unbranched.back().begin()+1, unbranched.back().end());
            }
        }
    }
                             //only prune at second encounter, i.e. exists target cell and first child has been explored
    else if(not found_automorphisms.empty()){
        std::vector<Vertex>& current_unbranched = unbranched[current_level-1];
        std::vector<Vertex> unbranched_after_pruning{};
        std::vector<Vertex> current_mcrs = mcrs(found_automorphisms, current_vertex_sequence);
                                                  //prune target cell. set_intersect works since both vectors are sorted
        std::set_intersection(current_unbranched.begin(),  current_unbranched.end(),
                              current_mcrs.begin(),current_mcrs.end(), std::back_inserter(unbranched_after_pruning));
        stats.num_pruned_by_auto += (current_unbranched.size() - unbranched_after_pruning.size());
        current_unbranched = unbranched_after_pruning;
        }

    std::vector<Vertex>& current_unbranched = unbranched[current_level-1];

    if(current_unbranched.empty()){                                //all children of a node have been explored or pruned
       backtrack_to(current_level-1);
       return;
    }

    Vertex child = current_unbranched.front();  //choose first/smallest element of target cell/ first unbranched element
    current_unbranched.erase(current_unbranched.begin());                 //element will be branched upon now, remove it
    current_vertex_sequence.push_back(child);
    current_partition.split_by_and_refine(graph, child);                                //get refined partition of split
    stats.refinements_made++;

    prune_by_invar();
}

void Nautyyy::process_leaf() {

    Permutation leaf_perm = discrete_partition_to_perm(current_partition);
    std::vector<bool> hash_val = graph.perm_hash_value(leaf_perm);

    if(current_level > stats.max_level){
        stats.max_level = current_level;
    }

    if(first_leaf.undiscovered()){                                                              //first encountered leaf
        first_leaf = Leaf(current_vertex_sequence, leaf_perm, hash_val);
        best_leaf = first_leaf;
        backtrack_to(current_level-1);
        return;
    }
                                                               //otherwise compare leaf to first_found_leaf or best_leaf
                                                             //there has been a new maximum invariant, update best guess
    if(best_leaf_outdated_due_to_invariant  or hash_val > best_leaf.hash_of_perm_graph){
        best_leaf = Leaf(current_vertex_sequence, leaf_perm, hash_val);      //update best canonical node
        stats.best_leaf_updates++;
        backtrack_to(current_level-1);
        best_leaf_outdated_due_to_invariant=false;
        return;
    }

     if(hash_val == first_leaf.hash_of_perm_graph){                  //leaves are equivalent, this gives an automorphism
        Permutation automorphism = perm_composition(first_leaf.leaf_perm, perm_inverse(leaf_perm));
        found_automorphisms.push_back(automorphism);
        stats.automorphisms_found++;
                                                                        //backtrack to level of greatest common ancestor
        //backtrack_to(get_gca_level(first_leaf.vertex_sequence, current_vertex_sequence));
        backtrack_to(current_level-1);
        return;
    }
     else if(hash_val == best_leaf.hash_of_perm_graph){                                         //same but for best leaf
         Permutation automorphism = perm_composition(best_leaf.leaf_perm, perm_inverse(leaf_perm));
         found_automorphisms.push_back(automorphism);
         stats.automorphisms_found++;
         //backtrack_to(get_gca_level(best_leaf.vertex_sequence, current_vertex_sequence));
         backtrack_to(current_level-1);
         return;
     }
                                        //that we got here means hash_val < first or best leaf, do nothing but backtrack
     stats.num_bad_leaves++;
     backtrack_to(current_level-1);
}



void Nautyyy::backtrack_to(unsigned int level) {
    stats.times_backtracked++;
    if(level==0){                                                         //handles the case of the algorithm being done
        current_level = level;                           //sets level to 0 so while loop in search_tree_traversal() ends
        return;
    }
    current_partition.reconstruct_at_level(level);                               //get old partition at the wanted level
    current_vertex_sequence.resize(level-1);       //return to old vertex sequence, simply remove later vertices
    unbranched.resize(level);                                     //later unbranched do not matter anymore, new path now
    current_level = level;
}



void Nautyyy::prune_by_invar() {

    InvarType new_invar{};
    if(current_partition.is_discrete() and (not (opt.invarmethod==Options::none))){
        new_invar.push_back(std::numeric_limits<int>::max());            //assert that leaves are considered as greatest
    }
    else{                                                                      //otherwise use invariant of normal nodes
        switch (opt.invarmethod) {
            case Options::none:                                                    //no pruning, simply go to next level
                current_level++;
                return;
            case Options::shape:
                new_invar = current_partition.shape_invar();
                break;
            case Options::refinement:
                new_invar = current_partition.ref_invar;
                break;
            case Options::num_cells:
                new_invar = InvarType{current_partition.number_of_cells()};
                break;
        }
    }
                           //comparing of invariant. This is pruning method Pa and assures that a canonical node remains
    if(max_invar_at_level.size() < current_level){                   //has there been an invariant on this level before?
        if(max_invar_at_level.size() != current_level-1){
            throw std::runtime_error("There are not as many invar's as there should be.");
        }
        max_invar_at_level.push_back(new_invar);             //if not, this is automatically new max invar at this level
        current_level++;
        return;
    }
    else if(new_invar == max_invar_at_level[current_level-1]) {
        current_level++;                                    //invar does not tell us anything, further explore this path
        return;
    }
    else if(new_invar > max_invar_at_level[current_level-1]){//invar found is better than any found before on this level
        max_invar_at_level[current_level-1] = new_invar;
                                     //reset the max invariant values following after, since everything is lexicographic
        max_invar_at_level.resize(current_level);
                                                             //update the next leaf encountered to be the new best guess
        best_leaf_outdated_due_to_invariant = true;
        current_level++;
        return;
    }
        //we either (do not skip pruning by invar for first path) or (are already done with first path)
    else {
        //smaller invariant, don't further explore this child and reconstruct previous partition
        current_partition.reconstruct_at_level(current_level);
        stats.num_pruned_by_invar++;
        return;
    }
}
