#ifndef _SYLAR_IOMANAGER_H_
#define _SYLAR_IOMANAGER_H_

#include "scheduler.h"
#include "timer.h"

namespace sylar {
// work flow
// 1 register one event -> 2 wait for it to ready -> 3 schedule the callback -> 4 unregister the event -> 5 run the callback
class IOManager : public Scheduler, public TimerManager {
public:
    enum Event {
        NONE = 0x0,
        READ = 0x1,     // READ == EPOLLIN
        WRITE = 0x4,    // WRITE == EPOLLOUT
    };

private:
    struct FdContext {
        struct EventContext {
            // scheduler
            Scheduler* scheduler = nullptr;
            // callback fiber
            std::shared_ptr<Fiber> fiber;
            // callback fubction
            std::function<void()> cb;
        };

        // read event context
        EventContext read;
        // write event context
        EventContext write;
        
        int fd = 0;
        // event registered
        Event events = NONE;
        std::mutex mutex;

        EventContext& getEventContext(Event event);
        void resetEventContext(EventContext& ctx);
        void triggerEvent(Event event);
    };

public:
    IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "IOManager");
    ~IOManager();

    // add one event at a time
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    // delete event
    bool delEvent(int fd, Event event);
    // delete the event and trigger its callback
    bool cancelEvent(int fd, Event event);
    // delete all events and trigger its callback
    bool cancelAll(int fd);

    static IOManager* GetThis();

protected:
    void tickle() override;

    bool stopping() override;

    void idle() override;

    void onTimerInsertedAtFront() override;

    void contextResize(size_t size);

private:
    int m_epfd = 0;
    // fd[0] read，fd[1] write
    int m_tickleFds[2];
    
    std::atomic<size_t> m_pendingEventCount = {0};
    std::shared_mutex m_mutex;
    // store fdcontext for each fd
    std::vector<FdContext*> m_fdContexts;
};
}

#endif