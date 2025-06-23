#include <iostream>
#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include "pizza.h"
#include <fstream>
#include "cv.h"
#include "driver.h"
#include "customer.h"
#include <vector>
#include <cstdint>
#include <sstream>
#include <map>
#include <climits>


struct Driver{

    location_t location = {0,0};
    int match = 0;
    int id;

    std::string state;
};

struct Customer{
    
    location_t location;
    int match = 0;
    int id;
   
    std::string state;
    
};

std::map<int, Driver> driver_map;
std::map<int, Customer> customer_map;

std::map<int, Customer> ready_customers;
std::map<int, Driver> ready_drivers;
//THREE STATES: READY, MATCHED, DRIVING, PAID
//FOUR STATES: READY, MATCHED, DRIVENTO, PAID
int line_number = 1;




bool check_location(Driver d, Customer c){
    //cycle through all of the ready customer locations
    int64_t mdist = llabs(static_cast<int64_t>(d.location.x) - static_cast<int64_t>(c.location.x)) 
    + llabs(static_cast<int64_t>(d.location.y) - static_cast<int64_t>(c.location.y));
  
    //std::cout<<"d id = "<<d.id<<std::endl;
    for(auto &dr : ready_drivers){
        int64_t new_dist = llabs(static_cast<int64_t>(ready_drivers[dr.first].location.x) - static_cast<int64_t>(c.location.x)) 
        + llabs(static_cast<int64_t>(ready_drivers[dr.first].location.y) - static_cast<int64_t>(c.location.y));
        
        //std::cout<<new_dist<<std::endl;
        if(new_dist < mdist){
           
            std::cout<<"MATCH WAS INCORRECT. CUSTOMER SHOULD HAVE MATCHED WITH DRIVER "<<ready_drivers[dr.first].id<<std::endl;
            return false;
        }
    }

    for(auto &cu : ready_customers){
        int64_t new_dist = llabs(static_cast<int64_t>(ready_customers[cu.first].location.x) - static_cast<int64_t>(d.location.x)) 
        + llabs(static_cast<int64_t>(ready_customers[cu.first].location.y) - static_cast<int64_t>(d.location.y));

       
        if(new_dist < mdist){
            std::cout<<"MATCH WAS INCORRECT. DRIVER SHOULD HAVE MATCHED WITH CUSTOMER "<<ready_customers[cu.first].id<<std::endl;
            return false;
        }
    }

    return true;
}



int main(int argc, char *argv[]){

   
    std::ifstream file(argv[1]);
    std::string customer_input;

    while(!file.eof()){
            std::getline(file, customer_input);

            std::istringstream stream(customer_input);
            

            //NEED TO PARSE THIS LINE APPROPRIATELY
            //GET LINE UNTIL FIRST SPACE
            std::string role;
            stream >> role;

            if(role == "driver"){
                std::string number;
                stream >> number;
                std::string verb;
                stream >> verb;
                if(verb == "ready"){
                    //IF ENTRY DOESN'T EXIST YET
                    if(driver_map.find(std::stoi(number)) == driver_map.end()){
                        //Create a Driver object with state "ready"
                        Driver new_d;
                        new_d.id = std::stoi(number);
                        new_d.state = "ready";
                        driver_map[std::stoi(number)] = new_d;
                        new_d.id = std::stoi(number);
                        //std::cout<<"this id = "<<std::stoi(number)<<std::endl;

                        ready_drivers[std::stoi(number)] = driver_map[std::stoi(number)];

                    }else{
                        if(driver_map[std::stoi(number)].state != "paid"){
                            std::cout<<"ERROR: DRIVER "<<std::stoi(number)<<" READY BEFORE GETTING PAID"<<std::endl;
                            std::cout<<"LINE = "<<line_number<<std::endl;
                            return 1;
                        }else{

                            std::string junk;
                            stream >> junk; //at

                            std::string loc;
                            stream >> loc; //(0,0)
                        
                        
                            unsigned int x_coord;
                            unsigned int y_coord;
                            char comma;
                    
                            loc = loc.substr(1, loc.size() - 2);  // Strip the parentheses
                            std::istringstream ss(loc);

                            ss >> x_coord;
                            ss >> comma;
                            ss >> y_coord;

                
                        
                            driver_map[std::stoi(number)].location.x = x_coord;
                            driver_map[std::stoi(number)].location.y = y_coord;
                            driver_map[std::stoi(number)].state = "ready";
                           
                            ready_drivers[std::stoi(number)] = driver_map[std::stoi(number)];
                        }
                    }
                }else if(verb == "driving"){
                    std::string junk;
                    stream >> junk; //from
                    //driver loc
                    //if location is not this driver's location, cout
                    //to
                    //customer loc
                    //if location is not matched customer's location, cout

                    if(driver_map.find(std::stoi(number)) == driver_map.end()){
                        std::cout<<"DRIVER "<<std::stoi(number)<<" DRIVING BEFORE DECLARING READY."<<std::endl;
                        std::cout<<"LINE = "<<line_number<<std::endl;
                        return 1;
                    }else{

                        if(driver_map[std::stoi(number)].state != "matched"){
                            std::cout<<"DRIVER "<<std::stoi(number)<<" DRIVING BEFORE MATCHED."<<std::endl;
                            std::cout<<"LINE = "<<line_number<<std::endl;
                          return 1;

                        }else{
                            driver_map[std::stoi(number)].state = "driving";
                            customer_map[driver_map[std::stoi(number)].match].state = "drivento";
                        }
                    }

                }
                

            }else if(role == "customer"){
                std::string number;
                stream >> number;
                
                int c_num = std::stoi(number);
                
                std::string verb;
                stream >> verb;

                if(verb == "requests"){
                    if(customer_map.find(c_num) == customer_map.end()){
                        Customer new_cust;
                        //read in location
                        std::string junk;
                        stream >> junk; //pizza
                        stream >> junk; //at
                        std::string loc;
                        stream >> loc; //(0,0)
                        
                        
                        unsigned int x_coord;
                        unsigned int y_coord;
                        char comma;
                    
                        loc = loc.substr(1, loc.size() - 2);  // Strip the parentheses
                        std::istringstream ss(loc);

                        ss >> x_coord;
                        ss >> comma;
                        ss >> y_coord;

                    
                        //ERROR HERE
                        
                        new_cust.location.x = x_coord;
                        new_cust.location.y = y_coord;

                        new_cust.state = "ready";
                        new_cust.id = c_num;
                        
                        customer_map[c_num] = new_cust;
                        ready_customers[c_num] = customer_map[c_num];
                        
                    }else{
                        if(customer_map[c_num].state != "paid"){
                            std::cout<<"CUSTOMER "<<c_num<<" READY BEFORE PAYING"<<std::endl;
                            std::cout<<"LINE = "<<line_number<<std::endl;
                            return 1;

                        }else{
                            customer_map[c_num].state = "ready";
                            ready_customers[std::stoi(number)] = customer_map[c_num];
                            std::string junk;
                            stream >> junk; //pizza
                            stream >> junk; //at
                            std::string loc;
                            stream >> loc; //(0,0)
                        
                        
                            unsigned int x_coord;
                            unsigned int y_coord;
                            char comma;
                    
                            loc = loc.substr(1, loc.size() - 2);  // Strip the parentheses
                            std::istringstream ss(loc);

                            ss >> x_coord;
                            ss >> comma;
                            ss >> y_coord;

                            customer_map[c_num].location.x = x_coord;
                            customer_map[c_num].location.y = y_coord;
                            ready_customers[c_num] = customer_map[c_num];

                        }
                    }

                }else if(verb == "matched"){
                    std::string junk;
                    stream >> junk; //with
                    stream >> junk; //driver
                    std::string d_num;
                    stream >> d_num;

                    if(customer_map.find(c_num) == customer_map.end()){
                        std::cout<<"CUSTOMER " <<c_num<<" MATCHING BEFORE DECLARING READY."<<std::endl;
                        std::cout<<"LINE = "<<line_number<<std::endl;
                        return 1;
                    }else{

                        if(customer_map[c_num].state != "ready"){
                            std::cout<<"CUSTOMER "<<c_num<<" MATCHED BEFORE BEING READY"<<std::endl;
                            std::cout<<"LINE = "<<line_number<<std::endl;
                            return 1;

                        }else{
                            //std::cout<<" d = "<<std::stoi(d_num)<<" and c = "<<c_num<<std::endl;
                            bool passcheck = check_location(ready_drivers[std::stoi(d_num)], ready_customers[c_num]);
                            if(!passcheck){
                                std::cout<<"LINE = "<<line_number<<std::endl;
                                return 1;
                            }

                            customer_map[c_num].match = std::stoi(d_num);
                            driver_map[customer_map[c_num].match].match = c_num;
                            customer_map[c_num].state = "matched";
                            driver_map[std::stoi(d_num)].state = "matched";
                            //erase from ready maps
                            ready_customers.erase(c_num);
                            ready_drivers.erase(std::stoi(d_num));
                        }
                    }

                }else if(verb == "pays"){
                    std::string junk;
                    stream >> junk; //driver
                    std::string d_num;
                    stream >> d_num;
                    
                    if(customer_map.find(c_num) == customer_map.end()){
                        std::cout<<"CUSTOMER " <<c_num<<" PAYING BEFORE DECLARING READY."<<std::endl;
                        std::cout<<"LINE = "<<line_number<<std::endl;
                        return 1;
                    }else{

                        if(customer_map[c_num].state != "drivento"){
                            std::cout<<"CUSTOMER "<<c_num<<" PAID BEFORE BEING DRIVEN TO / MATCHING"<<std::endl;
                            std::cout<<"LINE = "<<line_number<<std::endl;
                            return 1;

                        }else{

                            customer_map[c_num].state = "paid";
                            driver_map[std::stoi(d_num)].state = "paid";
                        }
                    }
                }
            }
            ++line_number;
        }

        //CHECK THAT ALL CUSTOMER STATES ARE "PAID" AND ALL DRIVER STATES ARE "READY"
        for(size_t i = 0; i < customer_map.size(); ++i){
            if(customer_map[i].state != "paid"){
                std::cout<<"CUSTOMER "<<i<<" HAS NOT ENDED."<<std::endl;
                return 1;
            }
        }

        for(size_t i = 0; i < driver_map.size(); ++i){
            if(driver_map[i].state != "ready"){
                std::cout<<"DRIVER "<<i<<" DID NOT END READY."<<std::endl;
                return 1;
            }
        }

    std::cout<<"all done!"<<std::endl;
    return 0;
}


