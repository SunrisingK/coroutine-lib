#include "thread.h"
#include <iostream>
#include <thread>
#include <memory>
#include <vector>
#include <thread>

using namespace sylar;


void func() {
    std::cout << "id: " << Thread::GetThreadID() << ", name: " << Thread::GetName();
    std::cout << ", this id: " << Thread::GetThis()->get_id() << ", this name: " << Thread::GetThis()->get_name() << std::endl;
    
    std::this_thread::sleep_for(std::chrono::seconds(60));
}

int main(int argc, char const* argv[]) {
    std::vector<std::shared_ptr<Thread>> thrs;

    for (int i = 0; i <5; ++i) {
        std::shared_ptr<Thread> thr = std::make_shared<Thread>(&func, "thread_" + std::to_string(i));
        thrs.push_back(thr);
    }

    for (int i = 0; i < 5; ++i) {
        thrs[i]->join();
    }

    return 0;
}