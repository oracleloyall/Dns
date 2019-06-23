/************************************************
 * 澶勭悊IO鍗忕▼鍒囨崲銆乪poll绛夊緟銆佺瓑寰呮垚鍔熴�佽秴鏃跺彇娑堢瓑寰咃紝
 *     鍙婂叾澶氱嚎绋嬪苟琛屽叧绯汇��
*************************************************/
#pragma once
#include <vector>
#include <list>
#include <set>
#include "../task.h"
#include "fd_context.h"
#include "../debugger.h"

namespace co
{

class IoWait
{
public:
    IoWait();

    int GetEpollFd();

    // 鍦ㄥ崗绋嬩腑璋冪敤鐨剆witch, 鏆傚瓨鐘舵�佸苟yield
    void CoSwitch();

    // --------------------------------------
    /*
    * 浠ヤ笅涓や釜鎺ュ彛瀹炵幇浜咥BBA寮忕殑骞惰
    */
    // 鍦ㄨ皟搴﹀櫒涓皟鐢ㄧ殑switch, 濡傛灉鎴愬姛鍒欒繘鍏ョ瓑寰呴槦鍒楋紝濡傛灉澶辫触鍒欓噸鏂板姞鍥瀝unnable闃熷垪
    void SchedulerSwitch(Task* tk);

    // trigger by timer or epoll or poll.
    void IOBlockTriggered(IoSentryPtr io_sentry);
    void __IOBlockTriggered(IoSentryPtr io_sentry);
    // --------------------------------------

    // --------------------------------------
    /*
    * reactor鐩稿叧鎿嶄綔, 浣跨敤绫讳技epoll鐨勬帴鍙ｅ睆钄絜poll/poll鐨勫尯鍒�
    * TODO: 鍚屾椂鏀寔socket-io鍜屾枃浠秈o.
    */
    int reactor_ctl(int epollfd, int epoll_ctl_mod, int fd,
            uint32_t poll_events, bool is_support, bool et_mode);
    // --------------------------------------

    int WaitLoop(int wait_milliseconds);

    bool IsEpollCreated();

private:
    void CreateEpoll();

    void IgnoreSigPipe();

    int& EpollFdRef();
    pid_t& EpollOwnerPid();

    LFLock epoll_create_lock_;
    int epoll_event_size_;

    typedef TSQueue<IoSentry> IoSentryList;
    IoSentryList wait_io_sentries_;

    friend class CoDebugger;

    // TODO: poll to support (file-fd, other-fd)
};

} //namespace co
