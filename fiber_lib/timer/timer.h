#ifndef _SYLAR_TIMER_H_
#define _SYALR_TIMER_H_

#include <memory>
#include <vector>
#include <set>
#include <shared_mutex>
#include <assert.h>
#include <functional>
#include <mutex>

namespace sylar {
class TimerManager;

class Timer : public std::enable_shared_from_this<Timer> {
friend class TimerManager;

public:
    // 从时间堆中删除timer
    bool cancel();
    // 刷新timer
    bool refresh();
    // 重设timer的超时时间
    bool reset(uint64_t ms, bool from_now);

private:
    Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimerManager* manager);

    // 是否循环
    bool m_recurring = false;
    // 超时时间
    uint64_t m_ms = 0;
    // 绝对超时时间
    std::chrono::time_point<std::chrono::system_clock> m_next;
    // 超时触发回调函数
    std::function<void()> m_cb;
    // 管理此timer的管理器
    TimerManager* m_manager = nullptr;

    // 最小堆比较函数
    struct Comparator {
        bool operator()(const std::shared_ptr<Timer>& lhs, const std::shared_ptr<Timer>& rhs) const;
    };
};

class TimerManager {
friend class Timer;

public:
    TimerManager();
    virtual ~TimerManager();

    // 添加timer
    std::shared_ptr<Timer> addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);

    // 添加条件timer
    // std::shared_ptr<Timer>
};
}

#endif