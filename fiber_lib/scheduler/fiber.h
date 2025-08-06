#ifndef _COROUTINE_H_
#define _COROUTINE_H_

#include <iostream>
#include <memory>
#include <atomic>
#include <functional>
#include <cassert>
#include <ucontext.h>
#include <unistd.h>
#include <mutex>

namespace sylar {
class Fiber: public std::enable_shared_from_this<Fiber> {
public:
    std::mutex m_mutex;
public:
    // 协程的状态
    enum State {READY, RUNNING, TERM};

    Fiber(std::function<void()> cb, size_t stacksize = 0, bool run_in_scheduler = true);
    ~Fiber();

    // 重用一个协程
    void reset(std::function<void()> cb);

    // 任务线程恢复执行
    void resume();

    // 线程让出执行权
    void yield();

    const uint64_t get_id() const {
        return m_id;
    }

    const State get_state() const {
        return m_state;
    }

public:
    // 设置当前运行的协程
    static void SetThis(Fiber* f);

    // 获取当前运行的协程
    static std::shared_ptr<Fiber> GetThis();

    // 设置调度协程(主协程)
    static void SetSchedulerFiber(Fiber* f);

    // 获取当前运行的协程ID
    static uint64_t GetFiberID();

    // 协程函数
    static void MainFunc();

private:
    // 仅由GetThis()调用: GetThis()->Fiber();
    Fiber();

    uint64_t m_id = 0;
    uint32_t m_stacksize = 0;

    State m_state = READY;

    // 协程上下文
    ucontext_t m_ctx;

    // 协程栈指针
    void* m_stack = nullptr;

    // 协程函数
    std::function<void()> m_cb;

    // 是否让出协程执行权交给调度函数的标志
    bool m_runInScheduler;
};
}

#endif