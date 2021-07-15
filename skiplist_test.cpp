#include<iostream>
#include"skip_table.h"

int main(){
    SkipList<std::string, std::string> skiplist(13);
    std::cout << "create skip list Success " << std::endl;
    
    // skiplist.insert_element(2, " two two ");
    // skiplist.insert_element(5, " five two ");
    // skiplist.insert_element(6, " six two ");
    // skiplist.insert_element(7, " seven two ");
    // skiplist.insert_element(3, " three two ");
    // skiplist.insert_element(4, " four two ");
    // skiplist.insert_element(9, " nine two ");
    // skiplist.insert_element(1, " one two ");
    // skiplist.insert_element(0, " zero two ");
    // skiplist.insert_element(8, " eight two ");

    skiplist.load_data_file();

    skiplist.showhole_list();

    std::cout << "Size of the Skip List : " << skiplist.getSize() << std::endl;

    // skiplist.write_data_file();


    return 0;
}