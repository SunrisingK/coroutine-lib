#include "fiber.h"

static bool debug = false;

namespace sylar {
// 当前线程上的协程控制信息

// 正在运行的协程
static thread_local Fiber* t_fiber = nullptr;
// 主协程
static thread_local std::shared_ptr<Fiber> t_thread_fiber = nullptr;
// 调度协程
static thread_local Fiber* t_scheduler_fiber = nullptr;

// 协程id
static std::atomic<uint64_t> s_fiber_id{0};
// 协程计数器
static std::atomic<uint64_t> s_fiber_count{0};

Fiber::Fiber() {
    SetThis(this);
    m_state = RUNNING;

    if (getcontext(&m_ctx)) {
        std::cerr << "Fiber() failed" << std::endl;
        pthread_exit(nullptr);
    }

    m_id = s_fiber_id++;
    ++s_fiber_count;

    if (debug) std::cout << "Fiber(): main id = " << m_id << std::endl;
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool run_in_scheduler): m_cb(cb), m_runInScheduler(run_in_scheduler) {
    m_state = READY;

    // 分配协程栈空间
    m_stacksize = stacksize ? stacksize : 128000;
    m_stack = malloc(m_stacksize);

    if (getcontext(&m_ctx)) {
        std::cerr << "Fiber(std::function<void()> cb, size_t stacksize, bool run_in_scheduler) failed" << std::endl;
		pthread_exit(nullptr);
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    makecontext(&m_ctx, &Fiber::MainFunc, 0);

    m_id = s_fiber_id++;
    ++s_fiber_count;

    if(debug) std::cout << "Fiber(): child id = " << m_id << std::endl;
}

Fiber::~Fiber() {
    --s_fiber_count;
    if (m_stack) {
        free(m_stack);
    }
    if(debug) std::cout << "~Fiber(): id = " << m_id << std::endl;	
}

// 设置当前运行的协程
void Fiber::SetThis(Fiber* f) {
    t_fiber = f;
}

// 创建主协程
std::shared_ptr<Fiber> Fiber::GetThis() {
    if (t_fiber) {
        return t_fiber->shared_from_this();
    }

    std::shared_ptr<Fiber> main_fiber(new Fiber());
    t_thread_fiber = main_fiber;
    // 主协程默认为调度协程
    t_scheduler_fiber = main_fiber.get();
    assert(t_fiber == main_fiber.get());

    return t_fiber->shared_from_this();
}

// 设置调度协程(主协程)
void Fiber::SetSchedulerFiber(Fiber* f) {
    t_scheduler_fiber = f;
}

uint64_t Fiber::GetFiberID() {
    if (t_fiber) {
        return t_fiber->get_id();
    }
    return uint64_t(-1);
}

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
    m_ctx.uc_stack.ss_size = m_stacksize;
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