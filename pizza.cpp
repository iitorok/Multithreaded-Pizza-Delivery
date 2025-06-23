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

//DRIVER AND CUSTOMER STRUCTS: USED FOR KEEPING
//TRACK OF LOCATIONS AND MATCHES.
struct Driver{

    location_t location = {0,0};
    uintptr_t match = 0;
    
};

struct Customer{
    
    location_t location;
    uintptr_t match = 0;
    
};


mutex m1;

//MATCH THREAD'S CV
cv cv1;

//VECTOR OF INTS: INDEX IS THE CUSTOMER NUMBER, VALUE IS THE LINE THEY ARE IN THEIR RESPECTIVE FILE
std::vector<int> file_place;

std::vector<std::vector<std::string>> customer_locations;

std::vector<Driver> drivers;
std::vector<Customer> customers;


std::vector<cv> customercvs;
std::vector<cv> drivercvs;


//KEEPING CUSTOMER AND DRIVER THREADS IN ORDER TO JOIN THEM LATER
std::vector<thread> customer_threads;
std::vector<thread> driver_threads;


//DRIVERS_READY AND CUSTOMERS_READY INT VECTOR FOR MATCH FUNCTION
std::vector<int> ready_drivers;
std::vector<int> ready_customers;

std::vector<int> driver_awaiting_pay;
std::vector<int> customer_awaiting_pizza;
std::vector<int> driver_awaiting_match;

std::vector<std::string> file_names;
int active_customers = 0;
uintptr_t num_drivers = 0;


int cs_ongoing = 1;
int ds_ongoing = 1;


/*DRIVER SEQUENCE
-----------------------------------------------------------------------------
DRIVER THREADS ARE CREATED WITH A FUNCTION POINTER TO DRIVER SEQUENCE. THREADS
THAT ENTER DRIVER_SEQUENCE WILL ONLY BREAK THE WHILE LOOP WHEN THERE ARE NO ACTIVE
CUSTOMERS; ACTIVE_CUSTOMERS IS ONLY MODIFIED IN CUSTOMER_SEQUENCE. WHEN A CUSTOMER
RUNS OUT OF LOCATIONS, THE COUNTER WILL DECREMENT UNTIL NO CUSTOMERS ARE LEFT.
THE DRIVERS READY UP IMMEDIATELY AFTER OBTAINING THE LOCK, AND THE DRIVER NUMBER
IS PUSHED INTO THE READY DRIVERS VECTOR, MAKING THE DRIVER ELIGIBLE FOR MATCHING.

IF BOTH THE READY DRIVER AND CUSTOMER VECTORS ARE NONEMPTY, THE THREAD WILL SIGNAL
THE MATCHING FUNCTION, PROMPTING IT TO WAKE UP.

DRIVER_AWAITING_MATCH IS ONLY MODIFIED IN THE MATCHING FUNCTION; IF A DRIVER MATCHES,
THE VALUE IN THE VECTOR INDEXED AT THE DRIVER'S NUMBER WILL BE SET TO 0 AND THE DRIVER
WILL BE SIGNALED.

THE THREAD THEN UNLOCKS BEFORE DRIVING, AS DRIVE() SHOULD BE ABLE TO BE DONE CONCURRENTLY BETWEEN
MULTIPLE THREADS.

AFTER DRIVE(), THE DRIVER FINDS THE LOCATION OF THE NEWFOUND MATCH AND SETS ITS LOCATION TO ITS MATCH'S
IT SETS CUSTOMER_AWAITING_PIZZA[MATCH_NUMBER] TO 0 AND SIGNALS THE CUSTOMER, THEN ENTERS A WAITING CONDITION
UNTIL THE CUSTOMER PAYS THE DRIVER. AFTER WAITING, THE DRIVER THREAD UNLOCKS THE MUTEX AND GOES TO THE
TOP OF THE WHILE LOOP.
-----------------------------------------------------------------------------*/


void driver_sequence(uintptr_t driver_num){

    while(ds_ongoing == 1){
        m1.lock();

        driver_ready(driver_num, drivers[driver_num].location);

       
        if(active_customers == 0){
            
            m1.unlock();
            break; 
        }  

       
        ready_drivers.push_back(driver_num);

       
        if(!ready_customers.empty() && !ready_drivers.empty()){
                cv1.signal();
        }

        driver_awaiting_match[driver_num] = 1;
        
        while(driver_awaiting_match[driver_num] == 1){
   
            drivercvs[driver_num].wait(m1);  
        }

      

        location_t curr_drive = drivers[driver_num].location;

        location_t curr_cust = customers[drivers[driver_num].match].location;

        m1.unlock();
       
        
        drive(driver_num, curr_drive, curr_cust);
 
        m1.lock();
    

        
        drivers[driver_num].location.x = customers[drivers[driver_num].match].location.x;
        drivers[driver_num].location.y = customers[drivers[driver_num].match].location.y;
   

        customer_awaiting_pizza[drivers[driver_num].match] = 0;
        

        customercvs[drivers[driver_num].match].signal();
        

        driver_awaiting_pay[driver_num] = 1;

        while(driver_awaiting_pay[driver_num] == 1){
        
            drivercvs[driver_num].wait(m1);
        }
    
        m1.unlock();
    
   
    }
    

}



/*CUSTOMER SEQUENCE
-----------------------------------------------------------------------------
WHEN CUSTOMER THREADS ARE CREATED WITH FUNCTION POINTERS TO CUSTOMER SEQUENCE.
CUSTOMER SEQUENCE CONSISTS OF A WHILE LOOP THAT ONLY BREAKS WHEN A CUSTOMER HAS NO
LOCATIONS LEFT IN CUSTOMER_LOCATIONS. 

USING A FILE_PLACE TRACKER, CUSTOMER SEQUENCE WILL FIND THE LOCATION THE CUSTOMER IS
ON IN THE FORM OF A STRING AND READ IN COORDINATES AS UNSIGNED INTS, WHICH IS NECESSARY 
FOR MATCHING LATER TO PREVENT OVERFLOW.

WHEN THE CUSTOMER DETAILS ARE READ IN, THE CUSTOMER WILL DECLARE READY, AND THE CUSTOMER
NUMBER WILL BE PUSHED INTO THE READY_CUSTOMERS ARRAY SO THAT IT IS ELIGIBLE FOR MATCHING.

THEN, THE MATCHING PROCESS THREAD IS SIGNALED IF BOTH THE CUSTOMER AND DRIVER READY ARRAYS
ARE NONEMPTY. THIS IS TO PREVENT UNDERGOING 'BUSY MATCHING' AND INFINITE LOOPING.

THE CUSTOMER THEN ENTERS A WAIT CONDITION: WHILE CUSTOMER_AWAITING_PIZZA[CUSTOMER_NUM] == 1,
WAIT. THIS IS A VARIABLE THAT IS ONLY MODIFIED BY THE DRIVER THREAD AFTER IT FINISHES THE DRIVE()
FUNCTION. IT WILL THEN SET A SIMILAR VARIABLE, DRIVER_AWAITING_PAY, TO ZERO AND SIGNALS TO
THE DRIVER FUNCTION, PROMPTING IT TO WAKE UP.

FINALLY, THE THREAD UNLOCKS AT THE END OF THE WHILE LOOP AND RETURNS TO THE TOP.
-----------------------------------------------------------------------------*/


void customer_sequence(uintptr_t customer_num){

    while(cs_ongoing == 1){
    
        m1.lock();
    
        bool file_done = false;

        
        size_t fileplacet = file_place[customer_num];
        


        if((fileplacet > customer_locations[customer_num].size() - 1) || customer_locations[customer_num].size() == 0){
            
            --active_customers;
            file_done = true;
            m1.unlock();
            break;
        }


        if(!file_done){
            
          
            std::string customer_input = customer_locations[customer_num][file_place[customer_num]];


            std::istringstream coords(customer_input);
            unsigned int x_c;
            coords >> x_c;

            unsigned int y_c;
            coords >> y_c;

            location_t customer_loc;
            customer_loc.x = x_c;
            customer_loc.y = y_c;

            customers[customer_num].location = customer_loc;
        
            
    
            customer_ready(customer_num, customers[customer_num].location);
            
            ready_customers.push_back(customer_num);
        
            
            if(!ready_customers.empty() && !ready_drivers.empty()){
            
                cv1.signal();
            }
        
        
        
            ++file_place[customer_num];
        
      
            customer_awaiting_pizza[customer_num] = 1;
           

            while(customer_awaiting_pizza[customer_num] == 1){
           
                customercvs[customer_num].wait(m1);
            }

          
            pay(customer_num, customers[customer_num].match);
            

            driver_awaiting_pay[customers[customer_num].match] = 0;


            drivercvs[customers[customer_num].match].signal();
        
        
        }


        m1.unlock();

    }
   

}



/*BEGIN DELIVERY
-----------------------------------------------------------------------------
CALLED BY CPU::BOOT(). STARTS THE CONCURRENT PROGRAM BY CREATING THREADS FOR
EACH DRIVER AND EACH CUSTOMER. THESE THREADS ARE STORED IN ARRAYS IN ORDER TO PERFORM
.JOIN() ON EACH ONE LATER AND TIE UP THE PROGRAM NICELY.

AFTER CREATING THE THREADS, ENTERS THE MATCHING PROCESS VIA A WHILE LOOP WITH THE
CONDITION 'ACTIVE_CUSTOMERS'. THIS IS A GLOBAL VARIABLE THAT IS SET TO THE NUMBER
OF CUSTOMERS INITIALLY AND DECREMENTED ONLY WHEN EACH CUSTOMER RUNS OUT OF LOCATIONS.

THE MATCHING PROCESS USES DRIVER_READY AND CUSTOMER_READY ARRAYS TO CALCULATE DISTANCES;
AS SUCH, IF EITHER ARRAY IS EMPTY, THE MATCHING PROCESS WILL WAIT. UNTIL SIGNALLED BY EITHER
A CUSTOMER OR DRIVER THREAD WHEN THE CONDITION CHANGES.

USING A FOR-LOOP FOR EACH CUSTOMER, THE PROCESS WILL CHECK EACH CUSTOMER AND SEE WHICH DRIVER IS
CLOSEST TO THEM. THEN, IT WILL CHECK IF THE CHOSEN CUSTOMER IS THE CLOSEST CUSTOMER TO THE SELECTED
DRIVER. IF SO, THEY WILL MATCH. IF NOT, THE FOR-LOOP WILL CONTINUE TO THE NEXT CUSTOMER IN
CUSTOMER_READY.

UPON MATCHING, THE THREAD WILL BREAK OUT OF THE CUSTOMER LOOP AND UNLOCK THE MUTEX.
-----------------------------------------------------------------------------*/


void begin_delivery(uintptr_t argc){

    //DRIVER THREAD CREATIONS
    for(uintptr_t i = 0; i < num_drivers; ++i){ 
       
        driver_threads.emplace_back(driver_sequence, i);
    }

    //CUSTOMER THREAD CREATIONS
    for(uintptr_t j = 0; j < argc - 2; ++j){
        
        customer_threads.emplace_back(customer_sequence, j);
    }
    

    while(active_customers > 0){
      
       
        m1.lock();
        
        
      
        while(ready_customers.empty() || ready_drivers.empty()){ 
            cv1.wait(m1);
        }
        
       
       
        for(size_t c = 0; c < ready_customers.size(); ++c){
            
            
            int64_t min_dist = INT64_MAX;

            
            int matched_driver = ready_drivers[0];
            int driver_ind = 0;
            
            //CHECK CUSTOMER READY_CUSTOMER[C]'S DISTANCE WITH EVERY DRIVER UNTIL ONE IS SELECTED.
            for(size_t d = 0; d < ready_drivers.size(); ++d){
              
                int64_t rect_dist = llabs(static_cast<int64_t>(drivers[ready_drivers[d]].location.x) - static_cast<int64_t>(customers[ready_customers[c]].location.x)) + 
                llabs(static_cast<int64_t>(drivers[ready_drivers[d]].location.y) - static_cast<int64_t>(customers[ready_customers[c]].location.y));

               
                if(rect_dist < min_dist){

                    min_dist = rect_dist;
                    matched_driver = ready_drivers[d];
                    driver_ind = d;
                    
                }

            }
            
        
            bool matched = true;

            //WITH THE SELECTED DRIVER, VERIFY THAT CUSTOMER READY_CUSTOMER[C] IS THE CLOSEST BY CHECKING EVERY OTHER CUSTOMER'S DISTANCE
            //IF READY_CUSTOMER[C] IS NOT THE CLOSEST, DO NOT MATCH. IF IT IS, DO MATCH.
            for(size_t k = 0; k < ready_customers.size(); ++k){

                int64_t rect_dist = llabs(static_cast<int64_t>(drivers[matched_driver].location.x) - static_cast<int64_t>(customers[ready_customers[k]].location.x)) + 
                llabs(static_cast<int64_t>(drivers[matched_driver].location.y) - static_cast<int64_t>(customers[ready_customers[k]].location.y));
                

                if(rect_dist < min_dist){
                    matched = false;
                }
            }

            if(matched == true){
              
                match(ready_customers[c], matched_driver);

                
                drivers[matched_driver].match = ready_customers[c];
                

                customers[ready_customers[c]].match = matched_driver;
                
                drivercvs[matched_driver].signal();
                driver_awaiting_match[matched_driver] = 0;
                

                ready_customers.erase(ready_customers.begin() + c);
                ready_drivers.erase(ready_drivers.begin() + driver_ind);
               
                break;
        
                }

        }

        m1.unlock();
     
    }
    
    for(auto &t : driver_threads){
        t.join();
    }

    for(auto &t : customer_threads){
        t.join();
    }

}




/*MAIN FUNCTION
-----------------------------------------------------------------------------
GIVEN THE NUMBER OF DRIVERS (ARGV[1]) AND THE NUMBER OF CUSTOMERS (ARGC - 2), RESIZES ALL VECTORS THAT UTILIZE
THESE TWO NUMBERS TO PREVENT MEMORY ALLOCATION ISSUES.

MAIN READS IN THE CUSTOMER LOCATIONS USING AN IFSTREAM FILE; READS IN EVERY LINE OF EACH FILE AND STORES IT 
INTO A 'CUSTOMER_LOCATIONS' ARRAY, WHICH IS LATER USED TO GRAB EACH X AND Y COORDINATE IN THE CUSTOMER_SEQUENCE
FUNCTION.

FINALLY, MAIN CALLS CPU::BOOT() TO CALL THE FUNCTION 'BEGIN_DELIVERY' USING A FUNCTION POINTER
-----------------------------------------------------------------------------*/

int main(int argc, char *argv[]){
    
    uintptr_t numdrivers = std::stoi(argv[1]);
    
    drivers.resize(numdrivers);
    
    customers.resize(argc - 2);

    active_customers = argc - 2;

    file_place.resize(argc - 2, 0);

    num_drivers = numdrivers;

    driver_awaiting_pay.resize(numdrivers, 0);
    driver_awaiting_match.resize(numdrivers, 0);
    customer_awaiting_pizza.resize(argc - 2, 0);

    customer_locations.resize(argc - 2);


   
    drivercvs.resize(numdrivers);
    customercvs.resize(argc - 2);

    for(uintptr_t d = 0; d < numdrivers; ++d){
        drivercvs[d] = cv();
    }

    //READ IN THE CUSTOMER LOCATIONS AND STORE IN CUSTOMER_LOCATIONS
    for(int c = 0; c < argc - 2; ++c){
        
        std::ifstream file(argv[c + 2]);
        std::string customer_input;
        customercvs[c] = cv();
        

        while(std::getline(file, customer_input)){
            //std::getline(file, customer_input);
            if(!customer_input.empty()){
                customer_locations[c].push_back(customer_input);
            }
           
         
        }

        file.close();
    
    }
    
    

    cpu::boot(begin_delivery, argc, 0);

}