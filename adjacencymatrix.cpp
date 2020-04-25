#include "adjacencymatrix.h"


Graph adjacency_matrix(char const* filename){
    std::ifstream file(filename);
    if (not file) {                                                                                   //wrong file input
        throw std::runtime_error("Cannot open file.");
    }

    std::string line;
    std::getline(file, line);                                                     //read in the file line by line

    if(isalpha(line[0])){
        return dimacs(filename);
    }

    std::stringstream ss(line);
    int num_nodes = 0;
    ss >> num_nodes;
    if (not ss) {                            //first line must contain a single number, the number of nodes of the graph
        throw std::runtime_error("No number of vertices given, invalid file format.");
    }
                                                           //matrix with n row and columns is initialised, every entry 0
    Graph graph(num_nodes, std::vector<bool>(num_nodes, false));
    line = std::string();
    std::getline(file, line);
    if(line.empty()){
        return graph;
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
            if (tail != head or graph[tail][head] != true) {                        //graph is supposed to be undirected
                graph[tail][head] = true;
                graph[head][tail] = true;
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
                    graph[tail][head] = true;
                }
            }
            std::getline(file, line);
        }
    }
    return graph;
}


Graph dimacs(char const* filename){
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
                                                           //matrix with n row and columns is initialised, every entry 0
    Graph graph(n, std::vector<bool>(n, false));
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
        graph[head - offset][tail - offset] = true;
        graph[tail - offset][head - offset] = true;
    }
    while (std::getline(file, line));
    return graph;
}






void print_matrix(const Graph& adjacency_matrix){
    for(int i=0; i<adjacency_matrix.size(); i++){                //simply print out one row, newline, print out next row
        for(int j=0; j<adjacency_matrix.size(); j++){
            std::cout<<adjacency_matrix[i][j];
        }std::cout<<"\n";
    }
}

std::vector<bool> value_of_graph(const Graph& graph) {
    int n = graph.size();
    std::vector<bool> result(n*n, false);
    for(int i=0; i<n; i++){
        for(int j=0; j<n; j++){
            if(graph[i][j]){                                 //index (i,j) of matrix corresponds to position n*(n-i)-j-1
                result[n * (n - i) - j-1] = true;       //of the string if matrix is written down in a row by row fashion
            }
        }
    }
    return result;
}


int degree(const Graph& graph, const int& vertex, const std::vector<int>& cell){
    int count = 0;
    for(int i : cell){
        if(graph[vertex][i]){                         //there is an edge going from vertex into the cell, increase count
            count++;
        }
    }
    return count;
}








