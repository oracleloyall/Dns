#pragma once
#include "config.h"
#include "task.h"
#include "ts_queue.h"

namespace co {

struct ThreadLocalInfo;

// 鍗忕▼鎵ц鍣�
//   绠＄悊涓�鎵瑰崗绋嬬殑鍏变韩鏍堝拰璋冨害, 闈炵嚎绋嬪畨鍏�.
class Processer
    : public TSQueueHook
{
private:
    typedef TSQueue<Task> TaskList;

    Task* current_task_ = nullptr;
    TaskList runnable_list_;
    uint32_t id_;
    static atomic_t<uint32_t> s_id_;

public:
    explicit Processer();

    void AddTaskRunnable(Task *tk);

    uint32_t Run(uint32_t &done_count);

    void CoYield();

    uint32_t GetTaskCount();

    Task* GetCurrentTask();

    std::size_t StealHalf(Processer & other);
};

} //namespace co
