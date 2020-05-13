#include "sparse_graph.h"


Sparse::Vertex::Vertex():edges(std::set<Vtype>()) {

}

void Sparse::Vertex::add_edge(const Vtype vertex) {
    edges.insert(vertex);
}

unsigned int Sparse::Vertex::nof_edges() const{
    return edges.size();
}

void Sparse::add_edge(Vtype v1, Vtype v2){
    vertices[v1].add_edge(v2);
    vertices[v2].add_edge(v1);
}

unsigned int Sparse::nof_vertices() const {
    return vertices.size();
}

Sparse::Sparse(int num_vertices): vertices(std::vector<Vertex>(num_vertices)){

}

Sparse::Sparse(const char *filename) {
    std::ifstream file(filename);
    if (not file) {                                                                                   //wrong file input
        throw std::runtime_error("Cannot open file.");
    }

    std::string line;
    std::getline(file, line);                                                     //read in the file line by line

    if(isalpha(line[0])){                                                                  //file is of dimacs format
        dimacs(filename);
        return;
    }

    std::stringstream ss(line);
    int num_nodes = 0;
    ss >> num_nodes;
    if (not ss) {                            //first line must contain a single number, the number of nodes of the graph
        throw std::runtime_error("No number of vertices given, invalid file format.");
    }
    //matrix with n row and columns is initialised, every entry 0
    vertices.resize(num_nodes);
    line = std::string();
    std::getline(file, line);
    if(line.empty()){
        return;
    }

    auto space_pos = std::find_if(line.begin(), line.end(), isspace);
    //when there is a space in the right place, it is of format 1. Exception that space is encountered at end is handled
    if(space_pos != line.end() and (std::next(space_pos,1) != line.end())){                     //File is of Format 1
        do{
            std::stringstream ss(line);
            int tail, head;
            ss >> tail >> head;
            if (not ss) {                                                          //each line must give a tail and head
                throw std::runtime_error("Invalid file format, not the correct edge format.");
            }
            if (tail != head) {                                                     //graph is supposed to be undirected
                add_edge(tail,head);
            }
            else {
                throw std::runtime_error("Invalid file format: loops and parallel edges not allowed.");
            }
        } while (std::getline(file, line));
    }
    else{                                                                                          //File is of Format 2
        for(int tail=0; tail<num_nodes; tail++) {
            std::stringstream ss(line);
            std::string edges;
            ss >> edges;

            if (not ss) {
                throw std::runtime_error("Invalid file format, not enough rows given.");
            }
            for(int head=0; head<num_nodes; head++){
                if(edges[tail]=='1'){
                    throw std::runtime_error("Invalid file format: loops not allowed.");
                }
                if(edges[head]=='1'){
                    add_edge(tail,head);
                }
            }
            std::getline(file, line);
        }
    }
}

void Sparse::dimacs(char const* filename){
    std::ifstream file(filename);
    if (not file) {                                                                                   //wrong file input
        throw std::runtime_error("Cannot open file.");
    }

    std::string line;
    std::getline(file, line);                                                     //read in the file line by line

    while(line[0] == 'c'){
        std::getline(file, line);                                                               //ignore comments
    }
    if(line[0] != 'p'){
        throw std::runtime_error("File is not in correct dimcs format.");                //a different case was expected
    }
    std::string p, edge;
    int n, e;
    std::stringstream ss(line);
    ss >> p >> edge >> n >> e;                                                       //read in number of nodes and edges

    vertices.resize(n);

    std::getline(file, line);
    while(line[0] == 'n'){
        std::cout<<"This program does not handle color assignment of\n"
                   "vertices so lines with n at the beginning are ignored."<<std::endl;
        std::getline(file, line);
    }

    int offset = 1;                                                  //in dimacs format, vertices start at 0 and go to n

    char first;
    int head, tail;
    do {
        std::stringstream ss(line);
        ss >> first >> head >> tail;
        add_edge(head - offset,tail - offset);
    }
    while (std::getline(file, line));
}





void Sparse::print() const{
    for(size_t v=0, max = vertices.size(); v<max; v++){
        std::cout<<v<<" :";
        for(Vtype w: vertices[v].edges){
            std::cout<<" "<<w;
        }std::cout<<"."<<std::endl;
    }
}

int Sparse::degree(const Vtype &vertex, const std::vector<Vtype> &cell) const{
    if(not is_sorted(cell.begin(), cell.end())){
        throw std::runtime_error("Given cell is not sorted, degree calculation may go wrong.");
    }
    std::vector<Vtype> intersection{};
    std::set_intersection(vertices[vertex].edges.begin(), vertices[vertex].edges.end(),
                          cell.begin(), cell.end(), std::back_inserter(intersection));
    return intersection.size();
}

std::vector<bool> Sparse::hash_value() const{
    int n = nof_vertices();
                                                           //could also reduce size here by indexing from strictly upper
                                                        //triangular matrix, result would only need to be n*(n-1)/2 then
    std::vector<bool> result(n*n, false);
    for(int i=0; i<n; i++){
        for(auto j: vertices[i].edges){
            result[n * (n - i) - j-1] = true;
        }
    }
    return result;
}


std::vector<bool> Sparse::perm_hash_value(const Permutation& perm) const{
    int n = nof_vertices();
    std::vector<bool> result(n*n, false);

    for(int i=0; i<n; i++){
        for(auto j: vertices[i].edges){
            result[n * (n - perm[i]) - perm[j]-1] = true;                    //check existence of permuted edges instead
        }
    }
    return result;
}



