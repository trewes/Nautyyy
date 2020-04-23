#include <iostream>

#include "nautyyy.h"


int main(int argc, char* argv[]) {
    try{
        std::cout<<"Begin Nautyyy: "<<std::endl;
        char const* file1 = (argc>1) ? argv[1] : "../Resources/test7_1.txt";
        char const* file2 = (argc>2) ? argv[2] : "../Resources/test7_2.txt";

        Options nauty_settings{};
        nauty_settings.print_stats = true;
        nauty_settings.print_time = true;
        nauty_settings.targetcellmethod = Partition::first;

        Graph g = adjacency_matrix(file1);

        Nautyyy g_nauty(g, nauty_settings);

        bool isomorphic = (g_nauty.best_leaf.hash_of_perm_graph == Nautyyy(file2, nauty_settings).best_leaf.hash_of_perm_graph);

        std::vector<std::string> answer{"No", "Yes"};
        std::cout<<"Isomorphic: "<<answer[isomorphic]<<"."<<std::endl;
    }
    catch (const std::runtime_error& e){
        std::cout<<e.what()<<std::endl;
    }
    std::cout<<"Finished."<<std::endl;

    return 0;
}
