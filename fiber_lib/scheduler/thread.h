#ifndef _THREAD_H_
#define _THREAD_H_

#include <mutex>
#include <string>
#include <condition_variable>
#include <functional>

namespace sylar {

// 线程方法同步
class Semaphore {
private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;

public:
    // 信号量初始化为0
    explicit Semaphore(int count_ = 0): count(count_) {}

    // P操作
    void wait() {
        std::unique_lock<std::mutex> lock(mtx);
        while (count == 0) {
            cv.wait(lock);
        }
        --count;
    }

    // V操作
    void signal() {
        std::unique_lock<std::mutex> lock(mtx);
        ++count;
        cv.notify_all();
    }
};

// Thread类创建线程
class Thread {
private:
    pid_t m_id = -1;
    pthread_t m_thread = 0;

    Semaphore m_semaphore;

    std::function<void()> m_cb;

    std::string m_name;

private:
    // 线程函数
    static void* run(void* args);

public:
    Thread(std::function<void()> cb, const std::string& name);
    ~Thread();

    const pid_t get_id() const {
        return m_id;
    }

    const std::string& get_name() const {
        return m_name;
    }

    void join();

public:
    // 获取系统分配的线程id
    static pid_t GetThreadID();

    // 获取当前所在的线程
    static Thread* GetThis();

    // 获取当前线程的名字
    static const std::string& GetName();

    // 设置当前线程的名字
    static void SetName(const std::string& name);
};
}

#endif