#include <iostream>
#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include "cv.h"

using std::cout;
using std::endl;

bool third_done = false;
int counter = 0;

cv c;
mutex m;


//SHOULD DEADLOCK
void first(uintptr_t arg)
{
    m.lock();
    m.lock();

}

//SHOULD THROW AN STD::RUNTIME ERROR
void second(uintptr_t arg){
    
    try {
        m.unlock();
    }
    catch(const std::runtime_error& e){
    
        cout<<"Exception occurred = "<<e.what()<<endl;
        exit(1);
    }
    
}


int main()
{
    cpu::boot(1, second, 0, false, false, 0);
}