#include <iostream>
#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include "cv.h"

using std::cout;
using std::endl;

int secondcounter = 0;
int thirdcounter = 0;

mutex m;
cv c;



void second(uintptr_t arg){
    
    m.lock();
    cout<<" thread "<<arg<<" in second loop."<<endl;
    m.unlock();
    if(arg == 2){
        c.signal();
        thread::yield();
        --secondcounter;
    }

   
    thread::yield();
    m.lock();
    ++secondcounter;
    c.broadcast();

    while(thirdcounter < 3){
        c.wait(m);
    }

    m.unlock();


}

void third(uintptr_t arg){

    thread::yield();
    m.lock();
    cout<<" thread "<<arg<<" in third loop."<<endl;
    ++thirdcounter;

    if(arg == 0){
        m.unlock();
        thread t (second, 2);
        t.join();
        m.lock();
    }

    while(secondcounter < 3){
        c.wait(m);
    }

    m.unlock();

}



void first(uintptr_t arg){
    //should do nothing
    thread::yield(); 
    thread* tptr1;
    thread* tptr2;

    for(uintptr_t i = 0; i < 3; ++i){
        
        tptr1 = new thread(second, reinterpret_cast<uintptr_t>(i));
        thread::yield();
    
        tptr2 = new thread (third, reinterpret_cast<uintptr_t>(i));
        thread::yield();
    }


    tptr1->join();
    tptr2->join();

    delete tptr1;
    tptr1 = nullptr;
    delete tptr2;
    tptr2 = nullptr;

}




int main()
{
    cpu::boot(1, first, 0, false, false, 0);
}