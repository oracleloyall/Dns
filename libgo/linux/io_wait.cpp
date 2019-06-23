#include "io_wait.h"
#include <sys/epoll.h>
#include "../scheduler.h"
#include <signal.h>

namespace co
{

IoWait::IoWait()
{
    epoll_event_size_ = 1024;
}

int& IoWait::EpollFdRef()
{
    thread_local static int epoll_fd = -1;
    return epoll_fd;
}
pid_t& IoWait::EpollOwnerPid()
{
    thread_local static pid_t owner_pid = -1;
    return owner_pid;
}

static uint32_t PollEvent2Epoll(short events)
{
    uint32_t e = 0;
    if (events & POLLIN)   e |= EPOLLIN;
    if (events & POLLOUT)  e |= EPOLLOUT;
    if (events & POLLHUP)  e |= EPOLLHUP;
    if (events & POLLERR)  e |= EPOLLERR;
    return e;
}

static short EpollEvent2Poll(uint32_t events)
{
    short e = 0;
    if (events & EPOLLIN)  e |= POLLIN;
    if (events & EPOLLOUT) e |= POLLOUT;
    if (events & EPOLLHUP) e |= POLLHUP;
    if (events & EPOLLERR) e |= POLLERR;
    return e;
}
/*
static std::string EpollEvent2Str(uint32_t events)
{
    std::string e("|");
    if (events & EPOLLIN)  e += "POLLIN|";
    if (events & EPOLLOUT) e += "POLLOUT|";
    if (events & EPOLLHUP) e += "POLLHUP|";
    if (events & EPOLLERR) e += "POLLERR|";
    if (events & EPOLLET) e += "EPOLLET|";
    return e;
}
static const char* EpollMod2Str(int mod)
{
    if (mod == EPOLL_CTL_ADD)
        return "EPOLL_CTL_ADD";
    else if (mod == EPOLL_CTL_DEL)
        return "EPOLL_CTL_DEL";
    else if (mod == EPOLL_CTL_MOD)
        return "EPOLL_CTL_MOD";
    else
        return "UNKOWN";
}
*/
void IoWait::CoSwitch()
{
    Task* tk = g_Scheduler.GetCurrentTask();
    if (!tk) return ;

    tk->state_ = TaskState::io_block;
    DebugPrint(dbg_ioblock, "task(%s) enter io_block", tk->DebugInfo());
    g_Scheduler.CoYield();
}

void IoWait::SchedulerSwitch(Task* tk)
{
    auto sentry = tk->io_sentry_;   // reference increment. avoid wakeup task at other thread.
    wait_io_sentries_.push(sentry.get()); // A
    if (sentry->io_state_ == IoSentry::triggered) // B
        __IOBlockTriggered(sentry);
}

void IoWait::IOBlockTriggered(IoSentryPtr io_sentry)
{
    if (io_sentry->switch_state_to_triggered()) // B
        __IOBlockTriggered(io_sentry);
}

void IoWait::__IOBlockTriggered(IoSentryPtr io_sentry)
{
    assert(io_sentry->io_state_ == IoSentry::triggered);
    if (wait_io_sentries_.erase(io_sentry.get())) { // A
        DebugPrint(dbg_ioblock, "task(%s) exit io_block",
                io_sentry->task_ptr_->DebugInfo());
        g_Scheduler.AddTaskRunnable(io_sentry->task_ptr_.get());
    }
}

int IoWait::reactor_ctl(int epollfd, int epoll_ctl_mod, int fd,
        uint32_t poll_events, bool is_support, bool et_mode)
{
    if (is_support) {
        epoll_event ev;
        ev.events = PollEvent2Epoll(poll_events);
        if (et_mode) ev.events |= EPOLLET;
        ev.data.fd = fd;
        int res = epoll_ctl(epollfd, epoll_ctl_mod, fd, &ev);
        DebugPrint(dbg_ioblock, "epoll_ctl(fd:%d, MOD:%s, events:%s) returns %d",
                fd, EpollMod2Str(epoll_ctl_mod),
                EpollEvent2Str(ev.events).c_str(), res);
        return res;
    }

    // TODO: poll妯℃嫙
    errno = EPERM;
    return -1;
}
int IoWait::WaitLoop(int wait_milliseconds)
{
    if (!IsEpollCreated())
        return -1;

    // TODO: epoll澶氱嚎绋嬭Е鍙�, poll鍗曠嚎绋嬭Е鍙�.

    thread_local static epoll_event *evs = new epoll_event[epoll_event_size_];

retry:
    int n = epoll_wait(GetEpollFd(), evs, epoll_event_size_, wait_milliseconds);
    if (n == -1) {
        if (errno == EINTR) {
            goto retry;
        }

        return 0;
    }

    DebugPrint(dbg_scheduler|dbg_scheduler_sleep, "epollwait(%d ms) returns: %d",
            wait_milliseconds, n);

    TriggerSet triggers;
    for (int i = 0; i < n; ++i)
    {
        int fd = evs[i].data.fd;
        FdCtxPtr fd_ctx = FdManager::getInstance().get_fd_ctx(fd);
        DebugPrint(dbg_ioblock, "epoll trigger fd(%d) events(%s) has_ctx(%d)",
                fd, EpollEvent2Str(evs[i].events).c_str(), !!fd_ctx);
        if (!fd_ctx) continue;

        // 鏆傚瓨, 鏈�鍚庡啀鎵цTrigger, 浠ヤ究浜巔oll鍙互寰楀埌鏇村鐨勪簨浠惰Е鍙�.
        fd_ctx->reactor_trigger(EpollEvent2Poll(evs[i].events), triggers);
    }

    // TODO: run poll

    // 瑙﹀彂浜嬩欢, 鍞ら啋绛夊緟涓殑鍗忕▼.
	// 杩囨椂鐨勫敜閱掔敱浜庡凡涓嶅湪wait鍒楄〃涓�,
    // 浼氳IOBlockTriggered涓殑鍘熷瓙鎿嶄綔switch_state_to_triggered鏍规嵁杩斿洖鍊艰繃婊ゆ帀.
    for (auto & sentry : triggers)
        IOBlockTriggered(sentry);

    return n;
}

int IoWait::GetEpollFd()
{
    CreateEpoll();
    return EpollFdRef();
}

void IoWait::CreateEpoll()
{
    pid_t pid = getpid();
    pid_t& epoll_owner_pid = EpollOwnerPid();
    if (epoll_owner_pid == pid) return ;
    std::unique_lock<LFLock> lock(epoll_create_lock_);
    if (epoll_owner_pid == pid) return ;

    epoll_owner_pid = pid;
    epoll_event_size_ = g_Scheduler.GetOptions().epoll_event_size;

    int & epoll_fd_ = EpollFdRef();
    if (epoll_fd_ >= 0)
        close(epoll_fd_);

    epoll_fd_ = epoll_create(epoll_event_size_);
    if (epoll_fd_ != -1) {
        DebugPrint(dbg_ioblock, "create epoll success. epollfd=%d", epoll_fd_);
        // 浣跨敤epoll闇�瑕佸拷鐣IGPIPE淇″彿
        IgnoreSigPipe();
    }
    else {
        fprintf(stderr,
                "CoroutineScheduler init failed. epoll create error:%s\n",
                strerror(errno));
        exit(1);
    }
}

void IoWait::IgnoreSigPipe()
{
    DebugPrint(dbg_ioblock, "Ignore signal SIGPIPE");
    sigignore(SIGPIPE);
}

bool IoWait::IsEpollCreated()
{
    return EpollFdRef() != -1 && EpollOwnerPid() == getpid();
}

} //namespace co
