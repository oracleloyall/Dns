/************************************************
 * Wait for sleep, nanosleep or poll(NULL, 0, timeout)
*************************************************/
#pragma once
#include "config.h"
#include "task.h"
#include "debugger.h"

namespace co
{

class SleepWait
{
public:
    // 鍦ㄥ崗绋嬩腑璋冪敤鐨剆witch, 鏆傚瓨鐘舵�佸苟yield
    void CoSwitch(int timeout_ms);

    // 鍦ㄨ皟搴﹀櫒涓皟鐢ㄧ殑switch
    void SchedulerSwitch(Task* tk);

    // @next_ms: 璺濈涓嬩竴涓猼imer瑙﹀彂鐨勬绉掓暟
    uint32_t WaitLoop(long long &next_ms);

private:
    void Wakeup(Task *tk);

    CoTimerMgr timer_mgr_;

    typedef TSQueue<Task> TaskList;
    TaskList wait_tasks_;

    friend class CoDebugger;
};


} //namespace co
