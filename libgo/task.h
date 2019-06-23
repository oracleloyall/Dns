#pragma once
#include "config.h"
#include "context.h"
#include "ts_queue.h"
#include "timer.h"
#include <string.h>
#include "util.h"
#include "linux/fd_context.h"

namespace co
{

enum class TaskState
{
    init,
    runnable,
    io_block,       // write, writev, read, select, poll, ...
    sys_block,      // co_mutex, ...
    sleep,          // sleep, nanosleep, poll(NULL, 0, timeout)
    done,
    fatal,
};

std::string GetTaskStateName(TaskState state);

typedef std::function<void()> TaskF;

class BlockObject;
class Processer;

struct Task
    : public TSQueueHook, public RefObject
{
    uint64_t id_;
    TaskState state_ = TaskState::init;
    uint64_t yield_count_ = 0;
    Processer* proc_ = NULL;
    Context ctx_;
    std::string debug_info_;
    TaskF fn_;
    SourceLocation location_;
	std::exception_ptr eptr_;           // 淇濆瓨exception鐨勬寚閽�

	// Network IO block鎵�闇�鐨勬暟鎹�
	// shared_ptr涓嶅叿鏈夌嚎绋嬪畨鍏ㄦ��, 鍙兘鍦ㄥ崗绋嬩腑鍜孲chedulerSwitch涓娇鐢�.
	IoSentryPtr io_sentry_;

	BlockObject* block_ = nullptr;      // sys_block绛夊緟鐨刡lock瀵硅薄
	uint32_t block_sequence_ = 0;       // sys_block绛夊緟搴忓彿(鐢ㄤ簬鍋氳秴鏃舵牎楠�)
	CoTimerPtr block_timer_;         // sys_block甯﹁秴鏃剁瓑寰呮墍鐢ㄧ殑timer
	MininumTimeDurationType block_timeout_ { 0 }; // sys_block瓒呮椂鏃堕棿
	bool is_block_timeout_ = false;     // sys_block鐨勭瓑寰呮槸鍚﹁秴鏃�

	int sleep_ms_ = 0;                  // 鐫＄湢鏃堕棿

    explicit Task(TaskF const& fn, std::size_t stack_size,
            const char* file, int lineno);
    ~Task();

    void InitLocation(const char* file, int lineno);

    ALWAYS_INLINE bool SwapIn()
    {
        return ctx_.SwapIn();
    }
    ALWAYS_INLINE bool SwapOut()
    {
        return ctx_.SwapOut();
    }

    void SetDebugInfo(std::string const& info);
    const char* DebugInfo();

    void Task_CB();

    static atomic_t<uint64_t> s_id;
    static atomic_t<uint64_t> s_task_count;

    static uint64_t GetTaskCount();

    static LFLock s_stat_lock;
    static std::set<Task*> s_stat_set;
    static std::map<SourceLocation, uint32_t> GetStatInfo();
    static std::vector<std::map<SourceLocation, uint32_t>> GetStateInfo();
};

} //namespace co
