#include "nautyyy.h"

#include <utility>



void Statistics::print() const {
    std::cout<<"Total leaves visited: "<<leaves_visited<<" and automorphisms found: "<<automorphisms_found
    <<". Times pruned by invar: "<<num_pruned_by_invar<<" and automorphisms: "<<num_pruned_by_auto
    << ". Refined " << refinements_made << " times." << " Canonical updates: " << best_leaf_updates<<". Backtracks: "
    <<times_backtracked<< ". Reached level: "<<max_level<<", total tc's "<<total_target_cells<<std::endl;
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

Leaf::Leaf(): vertex_sequence(std::vector<int>()), leaf_perm(std::vector<int>()), hash_of_perm_graph(){

}



Leaf::Leaf(std::vector<int>  in_vertex_sequence, Permutation in_leaf_perm, std::vector<bool>  hash_val)
                         : vertex_sequence(std::move(in_vertex_sequence)), leaf_perm(std::move(in_leaf_perm)),
                           hash_of_perm_graph(std::move(hash_val)){

}

bool Leaf::undiscovered() const{
    return leaf_perm.empty();            //if vertex has no assigned permutation, it means it hasn't been discovered yet
}


int Nautyyy::get_gca_level(const std::vector<Vertex> &first_sequence, const std::vector<Vertex> &second_sequence) {

    for(size_t same_untill = 0, max = std::min(first_sequence.size(), second_sequence.size()); same_untill<max; same_untill++){
        if(first_sequence[same_untill] != second_sequence[same_untill]){
            return same_untill+1;                                     //the level up to which the vertex sequences agree
        }
    }
    throw std::runtime_error("Error, the two given leaves are the same.");

}


                                                               //simple/empty initialization of most fields of the class
Nautyyy::Nautyyy(char const* filename, Options options)
    : stats(Statistics()), opt(std::move(options)), graph(Sparse(filename)),
      found_automorphisms(std::vector<Permutation>()), unbranched(std::vector<std::vector<Vertex>>()),
      current_vertex_sequence(std::vector<Vertex>()), first_leaf(Leaf()), best_leaf(Leaf()),
      max_invar_at_level(std::vector<InvarType>()){

    stats.start_time = std::chrono::steady_clock::now();

    if(opt.use_unit_partition){
        current_partition = Partition(graph.nof_vertices());                                      //begin with unit partition
    }
    else{
        current_partition = opt.input_partition;                                              //or user passes partition
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


//same as other constructor but the graph is passed as graph and not as filename to read in a graph
Nautyyy::Nautyyy(Graph  in_graph, Options options)
        : stats(Statistics()), opt(std::move(options)), graph(std::move(in_graph)),
          found_automorphisms(std::vector<Permutation>()), unbranched(std::vector<std::vector<Vertex>>()),
          current_vertex_sequence(std::vector<Vertex>()), first_leaf(Leaf()), best_leaf(Leaf()),
          max_invar_at_level(std::vector<InvarType>()){

    stats.start_time = std::chrono::steady_clock::now();

    if(opt.use_unit_partition){
        current_partition = Partition(graph.nof_vertices());
    }
    else{
        current_partition = opt.input_partition;
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
        if(current_level > stats.max_level){
            stats.max_level = current_level;
        }
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
        stats.print();              //print optional statistics about the execution of the algorithm
    }
    if(opt.print_time)
    {
        std::cout << "Execution took: ";
        stats.pretty_time();
    }
}



void Nautyyy::process_node(){

                                                //utility code for setting of not using node variant for first node path
    if(current_level==1 and opt.explore_first_path) {
                          //when we encounter this code a second time we will have explored first path, thus update bool
        if (first_path_help) {
            first_path_explored = true;
        } else {
            first_path_help = true;
        }
    }

                 //first encounter of this node, get target cell but don't prune yet since we explore first child anyway
    if(unbranched.size() < current_level){
        CellStruct target_cell(-1, -1, -1);
        if(current_level < opt.max_level_strong_tc) {                             //decide if we use "stronger" selector
            target_cell = current_partition.target_cell_selector(graph, opt.strong_targetcellmethod);
        }
        else {
            target_cell = current_partition.target_cell_selector(graph, opt.targetcellmethod);
        }
        stats.total_target_cells++;
        unbranched.push_back(current_partition.decode_given_cell(target_cell));
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


    //invar on leaves is different (it is the perm graph for now) and is considered as always greater than
    //any node invar. This is what I think it means for pruning, so far it works best
    if(current_partition.is_discrete()){
        current_level++;
        return;
    }

    InvarType new_invar;
    switch (opt.invarmethod) {                                          //decide which kind of invar is used for pruning

        case Options::none:
            current_level++;                                                                                //no pruning
            return;
        case Options::shape:
            new_invar = current_partition.shape_invar();
            break;
        case Options::refinement:
            new_invar = current_partition.ref_invar;
            break;
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
    else if(not opt.explore_first_path or first_path_explored){
                                //smaller invariant, don't further explore this child and reconstruct previous partition
        current_partition.reconstruct_at_level(current_level);
        stats.num_pruned_by_invar++;
        return;
    }
    else{
        current_level++;
    }

}

void Nautyyy::process_leaf() {

    if(first_leaf.undiscovered()){                                                              //first encountered leaf
        Permutation leaf_perm = discrete_partition_to_perm(current_partition);
        std::vector<bool> hash_val = graph.perm_hash_value(leaf_perm);
        first_leaf = Leaf(current_vertex_sequence, leaf_perm, hash_val);

        best_leaf = first_leaf;
        backtrack_to(current_level-1);
        return;
    }
                                                               //otherwise compare leaf to first_found_leaf or best_leaf
    Permutation leaf_perm = discrete_partition_to_perm(current_partition);
    std::vector<bool> hash_val = graph.perm_hash_value(leaf_perm);

                                                             //there has been a new maximum invariant, update best guess
                                                                                                                    //!not sure about this. definitely best_leaf_outdated_due_to_invariant but idk about hash val >
    if(best_leaf_outdated_due_to_invariant or hash_val > best_leaf.hash_of_perm_graph){
        best_leaf = Leaf(current_vertex_sequence, leaf_perm, hash_val);      //update best canonical node
        stats.best_leaf_updates++;
        backtrack_to(current_level-1);
        best_leaf_outdated_due_to_invariant=false;
        return;
    }
     if(hash_val == first_leaf.hash_of_perm_graph){                  //leaves are equivalent, this gives an automorphism
        std::vector<int> automorphism = perm_composition(first_leaf.leaf_perm, perm_inverse(leaf_perm));
        found_automorphisms.push_back(automorphism);
        stats.automorphisms_found++;
                                                                        //backtrack to level of greatest common ancestor
        backtrack_to(get_gca_level(first_leaf.vertex_sequence, current_vertex_sequence));
        return;
    }
                                                      //same but for best leaf. Not explicitly mentioned to do this also
                                                   //but it makes sense to also use best guess to look for automorphisms
     else if(hash_val == best_leaf.hash_of_perm_graph){
         std::vector<int> automorphism = perm_composition(best_leaf.leaf_perm, perm_inverse(leaf_perm));
         found_automorphisms.push_back(automorphism);
         stats.automorphisms_found++;
         backtrack_to(get_gca_level(best_leaf.vertex_sequence, current_vertex_sequence));
         return;
     }
                                        //that we got here means hash_val < first or best leaf, do nothing but backtrack
     stats.num_bad_leaves++;
    backtrack_to(current_level-1);
}



void Nautyyy::backtrack_to(int level) {
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



