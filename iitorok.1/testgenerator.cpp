#include <iostream>
#include <fstream> 
#include <string>
#include <random>
#include <climits>

int main() {

    // Create and open a file named "example.txt" in write mode
    

    for(int i = 0; i < 10; ++i){

        std::string file_nm = "test." + std::to_string(i);
        

        std::ofstream outfile(file_nm);
        for(int j = 0; j < 10000; ++j){
            std::random_device rd;                      
            std::mt19937 gen(rd());                   
            std::uniform_int_distribution<unsigned int> dist(1, 4294967295);

            
            unsigned int random_number = dist(gen);
            unsigned int random_num2 = dist(gen);
            
            outfile<<random_number<<" "<<random_num2<<std::endl;
        }
       
    }
    
   
    return 0;
}
