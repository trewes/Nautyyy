#include "permutation group.h"








Permutation discrete_partition_to_perm(const Partition& partition) {
    if(not partition.is_discrete()){
        throw std::runtime_error("The partition is not discrete and therefore not a permutation.");
    }
    std::vector<int> perm(partition.get_size());
    for(int i=0; i<partition.get_size(); i++){
        perm[i] = partition.get_first_of_cell(i);                          //element i is sent to cell in_cell[i]->first
    }                                                                      //i.e. to what position it has in element_vec
    return perm;
}

Permutation perm_inverse(const Permutation& perm) {
    std::vector<int> inverse(perm.size());
    for(int i=0; i<perm.size(); i++){
        inverse[perm[i]] = i;
    }
    return inverse;
}

Permutation perm_composition(const Permutation& first_perm, const Permutation& second_perm) {
    if(first_perm.size()!=second_perm.size()){
        throw std::runtime_error("Permutations are not of the same size.");
    }
    Permutation product(first_perm.size());
    for(int i=0; i<first_perm.size(); i++){
        product[i] = second_perm[first_perm[i]];
    }
    return product;
}

std::vector<int> apply_perm(std::vector<int> vector, const Permutation& perm) {
    for(int & i : vector){
        if(i>= perm.size()){
            throw std::runtime_error("Elements of the subset are out of range of the permutation.");
        }
        i = perm[i];                                             //sends each element ot its image under the permutation
    }
    return vector;
}

void print_perm(const Permutation& perm) {                           //prints a permutation in it's cycle representation

    if(all_of(perm.begin(), perm.end(), [perm](int i){return perm[i]==i;})){
        std::cout<<"Identity"<<std::endl;                                                     //the identity permutation
        return;
    }
    std::vector<int> visited(perm.size(), 0);
    int temp;
    for(int i=0; i<visited.size(); i++) {
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
    if(graph.size() != perm.size()){
        throw std::runtime_error("Size of graph and permutation do not match.");
    }
    Graph perm_graph = Graph(perm.size());

    /*for(int i=0; i<perm.size()-1; i++){
        for(int j=i+1; j<perm.size(); j++){
            if(graph.get(i,j)){
                perm_graph.set(perm[i], perm[j]);
            }
        }
    }*/
    //simply compose perms, figure out right order
    perm_graph.perm = perm_composition(graph.perm, perm_inverse(perm));
    //print_matrix(graph); std::cout<<"a"<<std::endl;
    //print_matrix(perm_graph); std::cout<<"b"<<std::endl;
    return perm_graph;
}





bool is_fixed(const Permutation& perm, const std::vector<int> &sequence){
    for(const int& i: sequence){
        if(perm[i] != i){
            return false;
        }
    }
    return true;
}

PermGroup subgroup_fixing_sequence(const PermGroup &permutations, const std::vector<int> &sequence) {
    PermGroup subgroup = PermGroup();
    for(const Permutation& perm: permutations){
        if(is_fixed(perm, sequence)){
            subgroup.push_back(perm);                                             //perm fixes sequence, add to subgroup
        }
    }
    return  subgroup;
}


//! needs comments
std::vector<int> mcrs(const PermGroup &permutations, const std::vector<int> &sequence) {
    if(permutations.empty()){
        throw std::runtime_error("There are no permutations.");
    }

    std::vector<int> result = std::vector<int>();
    std::vector<int> visited(permutations[0].size(), 0);
    PermGroup subgroup = subgroup_fixing_sequence(permutations, sequence);
    int temp;
    for(int i=0; i<visited.size(); i++) {
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

