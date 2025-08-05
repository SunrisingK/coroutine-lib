#include "fiber.h"

static bool debug = false;

namespace sylar {
// 重用一个协程
void Fiber::reset(std::function<void()> cb) {
    assert(m_stack != nullptr && m_state == TERM);

    m_state = READY;
    m_cb = cb;

    if (getcontext(&m_ctx)) {
        std::cerr << "reset() failed" << std::endl;
        pthread_exit(nullptr);
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stack;
    makecontext(&m_ctx, &Fiber::MainFunc, 0);
}

// 任务线程恢复执行
void Fiber::resume() {
    assert(m_state == READY);

    m_state = RUNNING;

    if (m_runInScheduler) {
        SetThis(this);
        if (swapcontext(&(t_scheduler_fiber->m_ctx), &m_ctx)) {
            std::cerr << "resume() to t_scheduler_fiber failed" << std::endl;
			pthread_exit(nullptr);
        }
    }
    else {
        SetThis(this);
        if (swapcontext(&(t_scheduler_fiber->m_ctx), &m_ctx)) {
            std::cerr << "resume() to t_scheduler_fiber failed" << std::endl;
			pthread_exit(nullptr);
        }
    }
}

// 线程让出执行权
void Fiber::yield() {
    assert(m_state == RUNNING || m_state == TERM);

    if (m_state != TERM) {
        m_state = READY;
    }

    if (m_runInScheduler) {
        SetThis(t_scheduler_fiber);
        if (swapcontext(&m_ctx, &(t_scheduler_fiber->m_ctx))) {
            std::cerr << "yield() to t_scheduler_fiber failed" << std::endl;
			pthread_exit(nullptr);
        }
    }
    else {
        SetThis(t_thread_fiber.get());
        if (swapcontext(&m_ctx, &(t_scheduler_fiber->m_ctx))) {
            std::cerr << "yield() to t_scheduler_fiber failed" << std::endl;
			pthread_exit(nullptr);
        }
    }
}

void Fiber::MainFunc() {
    std::shared_ptr<Fiber> curr = GetThis();
    assert(curr != nullptr);

    curr->m_cb();
    curr->m_cb = nullptr;
    curr->m_state = TERM;

    // 运行完毕让出执行权
    auto raw_ptr = curr.get();
    curr.reset();
    raw_ptr->yield();
}
}