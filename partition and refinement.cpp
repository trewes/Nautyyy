#include "partition and refinement.h"


CellStruct::CellStruct(unsigned int input_first, unsigned int input_length, unsigned int input_in_level)             //simply initialise the fields
        :first(input_first), length(input_length), in_level(input_in_level){
}

bool CellStruct::operator==(const CellStruct &rhs) const {     //two CellStructs are equal if all their fields are equal
    return (first == rhs.first and length == rhs.length and in_level == rhs.in_level);
}

Partition::Partition(): element_vec(std::vector<Vertex>()), lcs(std::list<CellStruct>()),
    in_cell(std::vector<std::list<CellStruct>::iterator>()), non_singleton(std::list<std::list<CellStruct>::iterator>()),
    level(0), refinement_stacks(std::vector<std::stack<unsigned int>>()) {
                                                                                            //init everything to nothing
}


Partition::Partition(unsigned int n): element_vec(std::vector<Vertex>(n)),//unit partition, n elements, a single cell of size one
                             lcs(std::list<CellStruct>(1, CellStruct(0, n, 1))),
                             non_singleton(std::list<std::list<CellStruct>::iterator>()){  //init non_singleton as empty
    if(n<1){
        throw std::runtime_error("Partition must be of positive size");
    }
    std::iota(element_vec.begin(), element_vec.end(), 0);              //element_vec simply is 0,1,...,n
    in_cell = std::vector<std::list<CellStruct>::iterator>(n, lcs.begin());    //every element is in the same cell
    if(n>1) {
        non_singleton.push_back(lcs.begin());
    }


    level = 0;                                          //Partition before first refinement is not used in tree, level 0
    refinement_stacks = std::vector<std::stack<unsigned int>>();
}

Partition::Partition(const std::vector<std::vector<Vertex>> &other_format_partition)
                        :non_singleton(std::list<std::list<CellStruct>::iterator>()){      //init non_singleton as empty
                                                                              //sum the sizes of cells to get total size
    int n = std::accumulate(other_format_partition.begin(), other_format_partition.end(),
                            0, []( int sum, const std::vector<Vertex>& cell){return sum+cell.size();});
    element_vec = std::vector<Vertex>(n);                                                                   //n elements
    in_cell = std::vector<std::list<CellStruct>::iterator>(n);
    int temp_first = 0;
    for(const std::vector<Vertex>& cell: other_format_partition){         //construct the cells from the integer vectors
        std::copy(cell.begin(), cell.end(), element_vec.begin()+temp_first);              //and assign element_vec
        lcs.emplace_back(temp_first, cell.size(), 1);           //construct and place new CellStruct into lcs
        temp_first += cell.size();
        for(int element: cell){                                    //update in_cell for the elements of the current cell
            in_cell[element] = std::prev(lcs.end(),1);
        }
        if(cell.size()>1){
            non_singleton.push_back(std::prev(lcs.end(),1));
        }
    }


    level = 0;                                          //Partition before first refinement is not used in tree, level 0
    refinement_stacks = std::vector<std::stack<unsigned int>>();
}


Partition::Partition(const Partition& old)
                    :non_singleton(std::list<std::list<CellStruct>::iterator>()){          //init non_singleton as empty
        element_vec = old.element_vec;                                //simply copy first two fields element_vec and lcs
        lcs = old.lcs;
        in_cell = std::vector<std::list<CellStruct>::iterator>(element_vec.size());
        for(auto it=lcs.begin(); it!=lcs.end(); it++) {
            for (auto element = element_vec.begin() + it->first;
                 element != element_vec.begin() + it->first + it->length; element++) {
                                                   //the iterators of in_cell do not update to now point to the elements
                                                      //of the new element_vec, the iterators are correctly set manually
                in_cell[*element] = it;
            }
            if (it->length > 1) {                                   //also extract non-singleton cells and set iterators
                non_singleton.push_back(it);
            }
        }
        level = old.level;                                                                    //also copy backtrack data
        refinement_stacks = old.refinement_stacks;
}

Partition& Partition::operator=(const Partition& rhs){   //same as copy constructor but returns the new copied partition
    if(this!=&rhs) {
        non_singleton = std::list<std::list<CellStruct>::iterator>();                      //init non_singleton as empty
        element_vec = rhs.element_vec;
        lcs = rhs.lcs;
        in_cell = std::vector<std::list<CellStruct>::iterator>(element_vec.size());
        for(auto it=lcs.begin(); it!=lcs.end(); it++){
            for(auto element=element_vec.begin()+it->first;
                element!=element_vec.begin()+it->first+it->length; element++) {
                in_cell[*element] = it;
            }
            if(it->length>1){                                       //also extract non-singleton cells and set iterators
                non_singleton.push_back(it);
            }
        }
        level = rhs.level;                                                                    //also copy backtrack data
        refinement_stacks = rhs.refinement_stacks;
    }
    return *this;
}



void Partition::print() const{
    std::cout<<"[";
    for(auto cell: lcs){                              //for each CellStruct print the elements of the cell in a nice way
        std::cout<<"[";
        for(unsigned int i=0; i<cell.length-1; i++){
            std::cout << element_vec[cell.first+i] << ",";
        }
        std::cout << element_vec[cell.first+cell.length-1] << "]";
    }std::cout<<"]"<<std::endl;
}

void Partition::print_detail() const{
    std::cout<<"[";
    for(auto cell: lcs){                              //for each CellStruct print the elements of the cell in a nice way
        std::cout<<"[";
        for(unsigned int i=0; i<cell.length-1; i++){
            std::cout << element_vec[cell.first+i] << ",";
        }
        std::cout << element_vec[cell.first+cell.length-1] << "]";
    }std::cout<<"]"<<std::endl;
    for(int i: element_vec){               //additionally output to each element the in_level value of the cell it is in
        std::cout<<in_cell[i]->in_level<<", ";
    }std::cout<<"\nLevel of partition: "<<level<<std::endl;
    std::cout<<"Stack size: "<<refinement_stacks.size()<<std::endl;
    for(unsigned int i: element_vec){               //additionally output to each element the in_level value of the cell it is in
        std::cout<<in_cell[i]->first<<", ";
    }std::cout<<" as first values"<<std::endl;
}

void Partition::print_non_singleton() const{
    std::cout<<"[";
    for(auto cell: non_singleton){      //for each CellStruct larger than 1 print the elements of the cell in a nice way
        std::cout<<"[";
        for(unsigned int i=0; i<cell->length-1; i++){
            std::cout << element_vec[cell->first+i] << ",";
        }
        std::cout << element_vec[cell->first+cell->length-1] << "]";
    }std::cout<<"]"<<std::endl;
}

bool Partition::is_discrete() const{
    return non_singleton.empty();
}

unsigned int Partition::get_size() const {
    return element_vec.size();
}

unsigned int Partition::number_of_cells() const{
    return lcs.size();
}

unsigned int Partition::get_first_of_cell(const unsigned int& element) const{
    return in_cell[element]->first;
}

std::vector<Vertex> Partition::decode_given_cell(const CellStruct& cell) const{
    return std::vector<Vertex>(element_vec.begin()+cell.first,
                     element_vec.begin()+cell.first+cell.length);
}


std::vector<std::vector<unsigned int>> Partition::decomposition(
        const Graph& graph, const std::vector<unsigned int> &cell_v, const std::vector<unsigned int> &cell_w)  {
    if(cell_v.size() == 1){
        throw std::runtime_error("A cell of size 1 cannot be decomposed.");
    }
    std::map<std::pair<unsigned  int, unsigned int>, std::vector<unsigned int>> temp{};
    for(const unsigned int& element: cell_v){
        temp[degree(graph, element, cell_w)].push_back(element);       //put each element into the vector of it's degree
    }
    std::vector<std::vector<unsigned int>> decomposition;
    for(const auto& element: temp){
        decomposition.push_back(element.second);
    }
    return decomposition;
}

//as given in (2013) with my chosen data structure for Partitions
void Partition::refinement(const Graph& graph, std::list<CellStruct> subsequence) {
    if(subsequence.empty()){                              //when no subsequence is given, use all cells of the partition
        subsequence = lcs;
    }
    while((not is_discrete()) and (not subsequence.empty())){
                                                        //get (and later remove) the first cell of the given subsequence
                                                        //get the vertices that are represented by the chosen CellStruct
        std::vector<unsigned int> decode_cell_w = decode_given_cell(subsequence.front());
        subsequence.erase(subsequence.begin());
                                                                     //iterate over non singleton cells of the partition
        for(auto cell_it = non_singleton.begin();cell_it!=non_singleton.end(); cell_it++){

              std::list<CellStruct>::iterator cell = *cell_it;       //get the iterator in lcs of the non-singleton cell
                                                                //as above, get vertices represented by the current cell
            std::vector<unsigned int> decode_cell = decode_given_cell(*cell);
                                                                  //Now decompose the cell by relation to the other cell
             std::vector<std::vector<unsigned int>> vk_decomposition = decomposition(graph, decode_cell, decode_cell_w);
            if (vk_decomposition.size() == 1) {continue;}                     //if there is no decomposition, do nothing
                                                                                      //otherwise: check some conditions
                                                     //check if current cell of the partition is also in the subsequence
            bool cell_in_subsequence = false;
            auto pos_in_subsequence = std::find(subsequence.begin(), subsequence.end(), *cell);
            std::vector<unsigned int> first_largest_splitter{};
            if ((not subsequence.empty()) and pos_in_subsequence != subsequence.end()) {
                cell_in_subsequence = true;
            } else {
                first_largest_splitter = *std::max_element(vk_decomposition.begin(), vk_decomposition.end(),
                        [](const std::vector<unsigned int> &cell_a,const std::vector<unsigned int> &cell_b) {
                                                               return cell_a.size() < cell_b.size();});
            }

            int first = cell->first;                                                     //update pi and the subsequence
            for (const std::vector<unsigned int> &splitter: vk_decomposition) {
                                      //create a new cell at level+1, size of the splitter and corresponding first field
                                                                  //and emplace it into position before the current cell
                auto new_cell_it = lcs.emplace(cell, first, splitter.size(), level+1);
                std::copy(splitter.begin(), splitter.end(), element_vec.begin() + first); //update the element_vec
                for (auto x: splitter) {
                    in_cell[x] = std::prev(cell,1);                                                  //update in_cell
                }
                if(splitter.size()>1){                                                            //update non_singleton
                    non_singleton.insert(cell_it, new_cell_it);
                }
                first += splitter.size();
                                                                                                    //update subsequence
                if (cell_in_subsequence) {                                //replace the cell by the splitters one by one
                    subsequence.insert(pos_in_subsequence,*new_cell_it);
                }
                else if (splitter !=first_largest_splitter) {              //or add all but one of the largest splitters
                    subsequence.push_back(*new_cell_it);
                }
                                             //store info about process to use as an invariant, the refinement invariant
                if(level and use_ref_invar) {
                    ref_invar.push_back(splitter.size());
                }

            }
                                                  //store some info of the refinement making later backtracking possible
                                                 //if in_level is already level+1 then cell has been created during this
                                                                  //refinement and is covered by previous backtrack info
            if(not refinement_stacks.empty() and (cell->in_level != level+1)) {
                std::prev(cell, 1)->in_level = cell->in_level;    //last cell is not seen as newly created, old level
                refinement_stacks.back().push(cell->first);            //cell was the cell being split, keep first value
            }

            lcs.erase(cell);                                                           //delete cell in lcs of partition
                                                //erase deletes iterator and returns the one after, so this is like a ++
            cell_it = non_singleton.erase(cell_it);
            cell_it = std::prev(cell_it, 1);                                      //reverse the ++ before end of loop
            if (cell_in_subsequence) {
                subsequence.erase(pos_in_subsequence);                          //we replace the cell, so remove old one
            }
        }
    }
    level++;                                   //refinement is done, partition now for the next (or first) level in tree
}

void Partition::split_by_and_refine(const Graph &graph, int vertex) {          //split the partition by the given vertex

    CellStruct& cell = *in_cell[vertex];                                                        //shorten name for usage
    if(cell.length == 1) {                                                               //trivial cell, cannot be split
        throw std::runtime_error("Cannot split partition by vertex in trivial cell.");
    }
                                                                       //partition the elements of Cell into v and not v
    std::stable_partition(element_vec.begin()+cell.first,
                          element_vec.begin()+cell.first+cell.length, [vertex](int a){return a==vertex;});
                                                                       //create new CellStruct for old cell minus vertex
                                                                     //and emplace that into list after now trivial cell
    auto it_new_cell = lcs.emplace(std::next(in_cell[vertex], 1),
            cell.first+1, cell.length-1, cell.in_level);
    cell.length = 1;                                                               //change first CellStruct accordingly
    for (auto it = element_vec.begin()+it_new_cell->first;
              it!=element_vec.begin()+it_new_cell->first+it_new_cell->length; it++) {
        in_cell[*it] = it_new_cell;                                                                     //update in_cell
    }
                                                                                 //get iterator in list of non-singleton
    std::list<std::list<CellStruct>::iterator>::iterator cell_it =
            std::find(non_singleton.begin(), non_singleton.end(), in_cell[vertex]);
    if(it_new_cell->length>1){
        non_singleton.insert(cell_it, it_new_cell);                       //insert iterator to new cell of non-singleton
    }
    non_singleton.erase(cell_it);                    //remove iterator to old cell since that only contains 'vertex' now
    cell.in_level = level+1;                         //the single vertex cell is considered as created at the next level
    refinement_stacks.emplace_back();          //new level, empty stack to store info about refinement on the next level
    refinement_stacks.back().push(cell.first);                                     //first info: we split at cell->first

                                             //Store info about process to use as an invariant, the refinement invariant
    if(use_ref_invar) {
        ref_invar.clear();
        ref_invar.push_back(it_new_cell->length + 1);
    }
    refinement(graph, std::list<CellStruct>{cell});                                            //then, refine
}


void Partition::merge_cells(std::list<CellStruct>::iterator first_cell, std::list<CellStruct>::iterator last_cell){
                                              //want all the cells in the range to be merged into first_cell, set length
    first_cell->length = (last_cell->first - first_cell->first) + last_cell->length;
    first_cell->in_level = last_cell->in_level;       //function is for backtracking, here to the level of the last cell
                                                     //merged CellStruct's into one, now reorder elements of merged cell
    std::sort(element_vec.begin()+first_cell->first, element_vec.begin()+first_cell->first+first_cell->length);
    for(auto it= element_vec.begin()+first_cell->first;
             it!= element_vec.begin()+first_cell->first+first_cell->length; it++){
        in_cell[*it] = first_cell;                                               //update in which cell the elements lie
    }

                                                 //first update non_singleton before lcs to not invalidate the iterators
    if(non_singleton.empty()){
        non_singleton.push_back(first_cell);                                         //it is the only non_singleton cell
    }
    else{
                                                          //find the position in the ordered list of non_singleton cells
                                                //for that, find first existing non_singleton having a later first_field
        std::list<std::list<CellStruct>::iterator>::iterator first_larger =
                std::find_if(non_singleton.begin(),non_singleton.end(),
                [first_cell](std::list<CellStruct>::iterator cell){return cell->first >= first_cell->first;});
                                                                                                 //insert it before that
        non_singleton.insert(first_larger, first_cell);//also works when first_larger == end(), then it's like push_back

        while(first_larger!=non_singleton.end() and (*first_larger)->first <= first_cell->first+first_cell->length-1){
            first_larger = non_singleton.erase(first_larger);              //and erase all non_singletons that it covers
        }
    }
                                                                 //remove all but the first cell, including the last one
    lcs.erase(std::next(first_cell, 1), std::next(last_cell,1));
}

void Partition::reconstruct_at_level(unsigned int return_level) {
    if(return_level<1){
        throw std::runtime_error("Cannot return to level before root.");
    }
                                                    //get the stack containing the info to backtrack to the wanted level
    std::stack<unsigned int>& bt_stack= refinement_stacks[return_level-1];
    while(not bt_stack.empty()){                                            //for each split merge again the split cells
                                                 //these are all the cells after first with a higher in_level value than
                                                    //the return_level plus the first with in_level at most return_level
        int first = bt_stack.top();
        int element_at_first = element_vec[first];
        bt_stack.pop();
        if(in_cell[element_at_first]->in_level > return_level) {
                                                                                   //find last cell up to which we merge
            auto last_cell = std::find_if(in_cell[element_at_first], lcs.end(),
                    [return_level](const CellStruct& cell) { return (cell.in_level <= return_level); });
            merge_cells(in_cell[element_at_first], last_cell);
        }
    }
    level = return_level;                                              //partition is now the one it was at return_level
    refinement_stacks.resize(return_level-1);                          //later stack info is not needed anymore
    if(use_ref_invar) {
                                                            //resetting it to be empty here is ok since a new refinement
        ref_invar.clear();                               //is made in Nautyyy before the node invariant is applied again
    }
}
//! doc of actual backtrack procedure and in_level assignment
/*
 * When a cell is split anywhere, all but the last cell assume the level of the partition the split happened in, the
 * last cell keeps its in_level value and that signifies the earliest level this and the previous cells belonged together.
 * Example [0,1,2,3] is equitable and split by 1 refines to [1|0|2,3] and split by 2 refines to [1|0|2|3]
 * in_level:1,1,1,1                                          2,2,1,1                             2,1,3,1
 *   all in same level.                 cell 1, 0 newly created, in_level 2             cell 2 newly created
 *                                       but 2,3 as last cell in_level 1                thus in_level is 3
 *  splits stored:                       split by 1, then by 0 in refinement            split by 2
 *  (as 'first')
 *
 *  Now to go back up to level k, we consult stack at level k and merge from the cell that has that firs field till the
 *  first succeeding cell with in_level value at most k.
 *  Example above: Recover partition at level 2. from stack we get first field 2, next cell has in_level 1 which is
 *  at most 2, so cell at index 2 and 3 are merged. Recover partition at level 1. from stack we get 1 (as the first
 *  field of cell 0. Next cell is 2 with in_level 2, greater than 1, go on to cell 3 with in_level value of 1 so
 *  merge cells 0,2,3 to get [1|0,2,3] and now from stack we get the first value 0, continue.
 */


CellStruct Partition::target_cell_selector(const Graph& graph, TargetcellMethod method) const {
                                                         //there are three methods of choosing a non-trivial cell so far
                                                               //"first smallest", "first", and "most non-trivial joins"
    if(non_singleton.empty()){
        throw std::runtime_error("All cells are trivial, no target cell can be selected.");
    }
    if(method == first or (non_singleton.size() == 1)){                  //chose and return the first non-singleton cell
        return *non_singleton.front();
    }
    else if(method == first_smallest) {                                 //find first non trivial cell of smallest size
        int min = std::numeric_limits<int>::max();                                                            //infinity
        std::list<CellStruct>::const_iterator index = lcs.end();
        int size;
        for (auto it: non_singleton) {
            size = it->length;
            if(size == 2){                      //consideration: can immediately stop if a cell of size 2 has been found
                return *it;
            }
            else if (size < min) {
                min = size;
                index = it;
            }
        }
        if (index != lcs.end()) {
            return *index;
        }
    }
    else if(method == joins){
        return most_non_trivial_joins(graph);    //first non-trivial cell that is non-trivially joined to the most cells
    }
                                                   //at this point, the chosen method should have returned a target cell
    throw std::runtime_error("No target cell could be selected, something went wrong.");
}


CellStruct Partition::most_non_trivial_joins(const Graph& graph) const {
                                   //returns the number of cells to which the cell of vertex is non_trivially joined to.
                      //Assuming the given partition to be equitable, we only need to check a single vertex of the class
    if(non_singleton.empty()){
        throw std::runtime_error("All cells are trivial, no target cell can be selected.");
    }
    std::vector<unsigned int> count = std::vector<unsigned int>(non_singleton.size(), 0);          //count the non-trivial joins
    std::vector<unsigned int> decode_cell_2 = std::vector<unsigned int>();
    std::list<std::list<CellStruct>::iterator>::const_iterator it_cell_1 = non_singleton.begin();
    std::list<std::list<CellStruct>::iterator>::const_iterator it_cell_2;
    std::pair<unsigned  int, unsigned int> current_degree = {0,0};
    for (unsigned int cell_1=0; cell_1<non_singleton.size(); cell_1++) {            //check for each pair if non-trivially joined
        it_cell_2 = std::next(it_cell_1,1);                     //excludes checking if non-trivially joined to itself
        for (unsigned int cell_2=cell_1+1; cell_2<non_singleton.size(); cell_2++) {

            decode_cell_2 = std::vector<unsigned int>(element_vec.begin()+(*it_cell_2)->first,
                                             element_vec.begin()+(*it_cell_2)->first+(*it_cell_2)->length);

            current_degree = degree(graph, element_vec[(*it_cell_1)->first], decode_cell_2);
            if ((0 < (current_degree.first+current_degree.second))
                and ((current_degree.first+current_degree.second) < (*it_cell_2)->length)) {
                count[cell_1]++;
                count[cell_2]++;
            }
            it_cell_2++;                     //manually increment the iterators since for loop is to get index for count
        }
        it_cell_1++;
    }
                                                           //return the first element that achieves the maximum in count
                                        //also works if there are no non-trivial joins, returns first non-singleton cell
    int max_pos = std::distance(count.begin(), std::max_element(count.begin(), count.end()));
    std::list<std::list<CellStruct>::iterator>::const_iterator max_cell = std::next(non_singleton.begin(), max_pos);
    return **max_cell;                                                                 //double iterator needs double **
}


InvarType Partition::shape_invar() {
    InvarType result{};
    for(auto cell: lcs){
        result.push_back(cell.length);                              //simply encodes the sizes of cells of the partition
    }
    return result;
}
























