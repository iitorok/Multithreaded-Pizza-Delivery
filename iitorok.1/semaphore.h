#pragma once

class semaphore {
public:

    unsigned int s_val;

    semaphore(unsigned int s_in){
        s_val = s_in;
    }

    ~semaphore();

    void down(){

        while(1){
            if(s_val > 0){
                --s_val;
                break;
            }
        }

    }

    void up(){
        ++s_val;
    }

    /*
     * Disable the copy constructor and copy assignment operator.
     */
    semaphore(const semaphore&) = delete;
    semaphore& operator=(const semaphore&) = delete;

    /*
     * Move constructor and move assignment operator.
     */
    semaphore(semaphore&&);
    semaphore& operator=(semaphore&&);
    
};
