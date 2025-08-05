#include <vector>
#include "fiber.h"

using namespace sylar;

class Scheduler {
private:
    // 任务队列
    std::vector<std::shared_ptr<Fiber>> m_tasks;
public:
    // 添加协程调度任务
    void schedule(std::shared_ptr<Fiber> task) {
        m_tasks.push_back(task);
    }

    // 执行调度任务
    void run() {
        std::cout << "number " << m_tasks.size() << std::endl;

        std::shared_ptr<Fiber> task;
        auto it = m_tasks.begin();

        while (it != m_tasks.end()) {
            task = *it;
            // 由主协程切换到子协程，子协程函数运行完毕后自动切换到主协程
            task->resume();
            it++;
        }
        m_tasks.clear();
    }
};

void test_fiber(int i) {
    std::cout << "hello world " << i << std::endl;
}

int main(int argc, char const* argv[]) {
    // 初始化当前线程的主协程
    Fiber::GetThis();

    // 创建调度器
    Scheduler sc;

    for (int i = 0; i < 20; ++i) {
        std::shared_ptr<Fiber> fiber = std::make_shared<Fiber>(std::bind(test_fiber, i), 0, false);
		sc.schedule(fiber);
    }
    // 执行调度任务
    sc.run();

    return 0;
}