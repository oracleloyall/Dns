#pragma once
#include "task.h"

namespace co
{

// 淇″彿绠＄悊瀵硅薄
// @绾跨▼瀹夊叏

class BlockObject
{
protected:
    friend class Processer;
    std::size_t wakeup_;        // 褰撳墠淇″彿鏁伴噺
    std::size_t max_wakeup_;    // 鍙互绉疮鐨勪俊鍙锋暟閲忎笂闄�
    TSQueue<Task, false> wait_queue_;   // 绛夊緟淇″彿鐨勫崗绋嬮槦鍒�
    LFLock lock_;

public:
    explicit BlockObject(std::size_t init_wakeup = 0, std::size_t max_wakeup = -1);
    ~BlockObject();

    // 闃诲寮忕瓑寰呬俊鍙�
    void CoBlockWait();

    // 甯﹁秴鏃剁殑闃诲寮忕瓑寰呬俊鍙�
    // @returns: 鏄惁鎴愬姛绛夊埌淇″彿
	bool CoBlockWaitTimed(MininumTimeDurationType timeo);

    template <typename R, typename P>
    bool CoBlockWaitTimed(std::chrono::duration<R, P> duration)
    {
        return CoBlockWaitTimed(std::chrono::duration_cast<MininumTimeDurationType>(duration));
    }

    template <typename Clock, typename Dur>
    bool CoBlockWaitTimed(std::chrono::time_point<Clock, Dur> const& deadline)
    {
        auto now = Clock::now();
        if (deadline < now)
            return CoBlockWaitTimed(MininumTimeDurationType(0));

        return CoBlockWaitTimed(std::chrono::duration_cast<MininumTimeDurationType>
                (deadline - Clock::now()));
    }

    bool TryBlockWait();

    bool Wakeup();

    bool IsWakeup();

private:
    void CancelWait(Task* tk, uint32_t block_sequence, bool in_timer = false);

    bool AddWaitTask(Task* tk);
};

} //namespace co
