#include "sparse_graph.h"


Sparse::Vertex::Vertex():edges(std::set<Vtype>()) {

}

void Sparse::Vertex::add_edge(const Vtype vertex) {
    edges.insert(vertex);
}

int Sparse::Vertex::nof_edges() const{
    return edges.size();
}

void Sparse::add_edge(Vtype v1, Vtype v2){
    vertices[v1].add_edge(v2);
    vertices[v2].add_edge(v1);
}

int Sparse::nof_vertices() const {
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

    if(isalpha(line[0])){
        std::cout<<"Later."<<std::endl; throw std::runtime_error("Ah!");
        //return dimacs(filename);
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
            if (tail != head) {                        //graph is supposed to be undirected
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

void Sparse::print() const{
    for(int v=0; v<vertices.size(); v++){
        std::cout<<v<<" :";
        for(Vtype w: vertices[v].edges){
            std::cout<<" "<<w;
        }std::cout<<"."<<std::endl;
    }
}

int Sparse::degree(const Vtype &vertex, const std::vector<Vtype> &cell) const{
    if(not is_sorted(cell.begin(), cell.end())){
        //throw std::runtime_error("Given cell is not sorted, degree calculation may go wrong.");
    }
    std::vector<Vtype> intersection{};
    std::set_intersection(vertices[vertex].edges.begin(), vertices[vertex].edges.end(),
                          cell.begin(), cell.end(), std::back_inserter(intersection));
    return intersection.size();
}

std::vector<bool> Sparse::hash_value() const{
    int n = nof_vertices();
    std::vector<bool> result(n*n, false);
    for(int i=0; i<n; i++){
        for(auto j: vertices[i].edges){
            result[n * (n - i) - j-1] = true;
        }
    }
    return result;
}


