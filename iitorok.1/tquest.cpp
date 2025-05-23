#include <iostream>
#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include "cv.h"

using std::cout;
using std::endl;

bool child_done = false;

cv c;
mutex m1;
mutex m2;

void child(uintptr_t arg){
    m1.lock();
    m2.lock();

    child_done = true;
    c.signal();

    m1.unlock();
    m2.unlock();
    
}



void parent(uintptr_t arg)
{
    m1.lock();
    m2.lock();
    thread t (child, 0);
    while(!child_done){
        c.wait(m1);
        c.wait(m2);
    }

    m1.unlock();
    m2.unlock();

}

int main()
{
    cpu::boot(parent, 0, 0);
}