#include "permutation group.h"








Permutation discrete_partition_to_perm(const Partition& partition) {
    if(not partition.is_discrete()){
        throw std::runtime_error("The partition is not discrete and therefore not a permutation.");
    }
    Permutation perm(partition.get_size());
    for(unsigned int i=0; i<partition.get_size(); i++){
        perm[i] = partition.get_first_of_cell(i);                          //element i is sent to cell in_cell[i]->first
    }                                                                      //i.e. to what position it has in element_vec
    return perm;
}

Permutation perm_inverse(const Permutation& perm) {
    Permutation inverse(perm.size());
    for(size_t i=0, max = perm.size(); i<max; i++){
        inverse[perm[i]] = i;
    }
    return inverse;
}

Permutation perm_composition(const Permutation& first_perm, const Permutation& second_perm) {
    if(first_perm.size()!=second_perm.size()){
        throw std::runtime_error("Permutations are not of the same size.");
    }
    Permutation product(first_perm.size());
    for(size_t i=0, max = first_perm.size(); i<max; i++){
        product[i] = second_perm[first_perm[i]];
    }
    return product;
}

std::vector<unsigned int> apply_perm(std::vector<unsigned int> vector, const Permutation& perm) {
    for(unsigned & i : vector){
        if(i>= perm.size()){
            throw std::runtime_error("Elements of the subset are out of range of the permutation.");
        }
        i = perm[i];                                             //sends each element ot its image under the permutation
    }
    return vector;
}

void print_perm(const Permutation& perm) {                           //prints a permutation in it's cycle representation

    if(all_of(perm.begin(), perm.end(), [perm](unsigned int i){return perm[i]==i;})){
        std::cout<<"Identity"<<std::endl;                                                     //the identity permutation
        return;
    }
    std::vector<unsigned int> visited(perm.size(), 0);
    int temp;
    for(size_t i=0, max = visited.size(); i<max; i++) {
        if ((not visited[i]) and (perm[i]!=i)) {                //extra condition to exclude elements sent to themselves
            temp = i;
            visited[i]=1;
            std::cout<<"("<<i;
            while(perm[temp] != i){          //while not back at firs element of cycle, go through elements of the cycle
                temp=perm[temp];
                std::cout<<" "<<temp;
                visited[temp]=1;
            }std::cout<<")";
        }
    }
    std::cout<<std::endl;
}


Graph perm_graph(const Graph& graph, const Permutation& perm) {
    if(graph.nof_vertices() != perm.size()){
        throw std::runtime_error("Size of graph and permutation do not match.");
    }
    Graph perm_graph = Graph(graph.nof_vertices());

    for(size_t i=0, max = graph.nof_vertices(); i<max; i++){
        for(Vtype j : graph.vertices[i].edges){
            perm_graph.add_edge(perm[i], perm[j]);
        }
    }
    return perm_graph;
}





bool is_fixed(const Permutation& perm, const std::vector<unsigned int> &sequence){
    for(const unsigned int& i: sequence){
        if(perm[i] != i){
            return false;
        }
    }
    return true;
}

PermGroup subgroup_fixing_sequence(const PermGroup &permutations, const std::vector<unsigned int> &sequence) {
    PermGroup subgroup = PermGroup();
    for(const Permutation& perm: permutations){
        if(is_fixed(perm, sequence)){
            subgroup.push_back(perm);                                             //perm fixes sequence, add to subgroup
        }
    }
    return  subgroup;
}


//! needs comments
std::vector<unsigned int> mcrs(const PermGroup &permutations, const std::vector<unsigned int> &sequence) {
    if(permutations.empty()){
        throw std::runtime_error("There are no permutations.");
    }

    std::vector<unsigned int> result = std::vector<unsigned int>();
    std::vector<unsigned int> visited(permutations[0].size(), 0);
    PermGroup subgroup = subgroup_fixing_sequence(permutations, sequence);
    int temp;
    for(size_t i=0, max = visited.size(); i<max; i++) {
        if (not visited[i]){
            visited[i]=1;
            result.push_back(i);
            for(const Permutation& perm: subgroup) {
                temp = i;
                while (perm[temp] != i) {
                    temp = perm[temp];
                    visited[temp] = 1;
                }
            }
        }
    }
    return result;
}

