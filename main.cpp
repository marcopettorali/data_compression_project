#define NAT_LANG_OPT true;

#include <iostream>
#include "sdc.hpp"
#include "dac.hpp"
#include "file_scanner.hpp"

int main(int, char**) {
    /*
    TextFileIterator tf = TextFileIterator("prova.txt");
    while(!tf.end()) {
        std::cout<<(char)*tf;
        //std::cout<<(char)*tf<<": got instead "<<*tf<<std::endl;
        ++tf;
    }
    std::cout<<std::endl;
    */
    
    /*
    TextFileIterator tf = TextFileIterator("dante.txt");
    auto sdc = tongrams::simple_dense_code_sequence();
    sdc.build(tf, -1, 2);
    for(uint64_t i = 0; i < sdc.size(); ++i) {
        std::cout<<(char)sdc[i];
    }
    std::cout<<std::endl;
    std::cout<<"SDC: total bytes: "<<sdc.bytes()<<", over a total of "<<sdc.size()<<" characters."<<std::endl;
    tf.close();
    */
    
    
    
    
    TextFileIterator tf2 = TextFileIterator("dante.txt");
    auto dac = tongrams::directly_accessable_code_sequence();
    dac.build(tf2, 1000, 4);
    for(uint64_t i = 0; i < dac.size(); ++i) {
        std::cout<<(char)dac[i];
    }
    std::cout<<std::endl;
    std::cout<<"DAC: total bytes: "<<dac.bytes()<<", over a total of "<<dac.size()<<" characters."<<std::endl;
    tf2.close();
    
    

    
   
}
