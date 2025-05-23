#include <iostream>
#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include "cv.h"


using std::cout;
using std::endl;

mutex m;
cv c;
cv c2;

bool parent_done = false;
bool baby_done = false;
int baby_counter = 1;




void baby(uintptr_t arg){
    m.lock();
    while(!parent_done){
        c.wait(m);
    }
    ++baby_counter;
    if(baby_counter == 9){
        baby_counter = 1;
        baby_done = true;
        c2.broadcast();
    }
    m.unlock();
    cout<<"baby: "<<arg<<" ending"<<endl;

}


void child(uintptr_t arg){
    std::cout<<"child: "<<arg<<" was swapped to!"<<endl;
    for(uintptr_t j = 0; j < 9; ++j){
        thread t2(baby, j);
    }
    parent_done = true;
    
    while(!baby_done){
        m.lock();
        c.signal();
        c2.wait(m);
        m.unlock();
    }

    cout<<"child: "<<arg<<" ending"<<endl;

}



void aunt(uintptr_t arg){

    for(uintptr_t i = 0; i < 5; ++i){
        thread t1(child, i);
        cout<<"child: "<<i<<" beginning"<<endl;
        t1.join();
    }

    cout<<"aunt: "<<arg<<" ending"<<endl;
}


void parent(uintptr_t arg)
{
    for(uintptr_t i = 0; i < 2; ++i){
        thread a (aunt, i);
        cout<<"aunt: "<<i<<" beginning"<<endl;
        a.join();
    }
    cout<<"parent ending"<<endl;
}



int main()
{
    cpu::boot(parent, 0, 0);
}