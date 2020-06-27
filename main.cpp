#include <iostream>
#include <getopt.h>

#include "nautyyy.h"

void print_help(){
    std::cout<<"Usage: Nautyyy.exe [options] graph1.txt graph2.txt"<<std::endl;
    std::cout<<"Options:"<<std::endl;
    std::cout<<"-h|--help               :Prints this help message."<<std::endl;
    std::cout<<"-s|--stats              :Enables output of statistics gathered during the algorithm."<<std::endl;
    std::cout<<"-t|--time               :Enables output of execution time."<<std::endl;
    std::cout<<"-i|--invarmethod  arg   :Change invariant used during algorithm."<<std::endl;
    std::cout<<"                         n for none, s for shape, r for refinement, i for number of cells."<<std::endl;
    std::cout<<"-c|--tcmethod           :Change targetcell selector used during algorithm."<<std::endl;
    std::cout<<"                         f for first, s for first_smallest, j for joins"<<std::endl;
    std::cout<<"-u|--use_implicit       :Enables use of implicit automorphisms for pruning."<<std::endl;
    std::cout<<"-p|--partition          :Enables possibility of using an initial partition instead of unit partition."<<std::endl;
    std::cout<<"-r|--random             :Runs the algorithm on a random permutation of the given graph."<<std::endl;
}





int main(int argc, char* argv[]) {

    Options nauty_settings{};

    int opt;
    int option_index = 0;
    static struct option long_options[] = {
            {"help", no_argument, nullptr, 'h'},
            {"stats", no_argument, nullptr, 's'},
            {"time", no_argument, nullptr, 't'},
            {"invarmethod", required_argument, nullptr, 'i'},
            {"tcmethod", required_argument, nullptr, 'c'},
            {"use_implicit", no_argument, nullptr, 'u'},
            {"partition", no_argument, nullptr, 'p'},
            {"random", no_argument, nullptr, 'r'},
    };

    while ((opt = getopt_long(argc, argv, "hsti:c:nupr", long_options, &option_index)) != -1){
        switch (opt) {

            default:
            case '?':                                                       //getopt itself will return an error message
                return -1;
            case 'h':
                print_help();
                return -1;

            case 's':
                nauty_settings.print_stats = true;
                break;
            case 't':
                nauty_settings.print_time = true;
                break;
            case 'i':
                if(*optarg=='n'){
                    nauty_settings.invarmethod = Options::none;
                }
                else if(*optarg=='s'){
                    nauty_settings.invarmethod = Options::shape;
                }
                else if(*optarg=='r'){
                    nauty_settings.invarmethod = Options::refinement;
                }
                else if(*optarg=='c'){
                    nauty_settings.invarmethod = Options::num_cells;
                }
                else{
                    std::cout<<"The invarmethod was not correctly specified."<<std::endl;
                    std::cout<<"Program failed."<<std::endl;
                    return -1;
                }
                break;
            case 'c':
                if(*optarg=='f'){
                    nauty_settings.targetcellmethod = Partition::first;
                }
                else if(*optarg=='s'){
                    nauty_settings.targetcellmethod = Partition::first_smallest;
                }
                else if(*optarg=='j'){
                    nauty_settings.targetcellmethod = Partition::joins;
                }
                else{
                    std::cout<<"The targetcell selection method was not correctly specified."<<std::endl;
                    std::cout<<"Program failed."<<std::endl;
                    return -1;
                }
                break;
            case 'u':
                nauty_settings.use_implicit_pruning = true;
                break;
            case 'p':
                nauty_settings.use_unit_partition = false;
                break;
            case 'r':
                nauty_settings.use_random_perm_of_graph = true;
                break;
        }
    }
    if((not nauty_settings.use_unit_partition) and nauty_settings.use_random_perm_of_graph){
        std::cout<<"Currently it is not supported to use a random permutation graph and keep the given initial "
                   "partition intact, error."<<std::endl;
        std::cout<<"Program failed."<<std::endl;
        return -1;
    }

    //optind is the index in argv after going through all the options, now the arguments are given
    char const* file1 = (argc>1) ? argv[optind] : "../Graphs/test12_1.txt";
    char const* file2 = (argc>2) ? argv[optind+1] : "../Graphs/test12_2.txt";

    try{
        //I'm using this format of the main function to showcase the different methods of calling the Nautyyy algorithm
        std::cout<<"Begin Nautyyy: "<<std::endl;
        Graph g = Sparse(file1);
        Nautyyy g_nautyyy(g, nauty_settings);

        bool isomorphic = (g_nautyyy.best_leaf.hash_of_perm_graph == Nautyyy(file2, nauty_settings).best_leaf.hash_of_perm_graph);

        std::vector<std::string> answer{"No", "Yes"};
        std::cout<<"Isomorphic: "<<answer[isomorphic]<<"."<<std::endl;
    }
    catch (const std::runtime_error& e){
        std::cout<<e.what()<<std::endl;
        std::cout<<"Program failed."<<std::endl;
        return -1;
    }
    std::cout<<"Finished."<<std::endl;
    return 0;
}
