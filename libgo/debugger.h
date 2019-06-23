#pragma once
#include <deque>
#include <mutex>
#include <map>
#include <vector>
#include "spinlock.h"
#include "util.h"

#if defined(__GNUC__)
#include <cxxabi.h>
#include <stdlib.h>
#endif

namespace co
{

struct ThreadLocalInfo;

// libgo璋冭瘯宸ュ叿
class CoDebugger
{
public:
    typedef atomic_t<uint64_t> count_t;
    typedef std::deque<std::pair<std::string, count_t>> object_counts_t;
    typedef std::deque<std::pair<std::string, uint64_t>> object_counts_result_t;

    /*
    * 璋冭瘯鐢ㄥ熀绫�
    */
    template <typename Drived>
    struct DebuggerBase
    {
#if ENABLE_DEBUGGER
        DebuggerBase()
        {
            object_creator_.do_nothing();
            ++Count();
        }
        DebuggerBase(const DebuggerBase&)
        {
            object_creator_.do_nothing();
            ++Count();
        }
        ~DebuggerBase()
        {
            --Count();
        }

        static count_t& Count();

        struct object_creator
        {
            object_creator()
            {
                DebuggerBase<Drived>::Count();
            }
            inline void do_nothing() {}
        };

        static object_creator object_creator_;
#endif
    };

public:
    static CoDebugger& getInstance();

    // 鑾峰彇褰撳墠鎵�鏈変俊鎭�
    std::string GetAllInfo();

    // 褰撳墠鍗忕▼鎬绘暟閲�
    uint32_t TaskCount();

    // 褰撳墠鍗忕▼ID, ID浠�1寮�濮嬶紙涓嶅湪鍗忕▼涓垯杩斿洖0锛�
    uint64_t GetCurrentTaskID();

    // 褰撳墠鍗忕▼鍒囨崲鐨勬鏁�
    uint64_t GetCurrentTaskYieldCount();

    // 璁剧疆褰撳墠鍗忕▼璋冭瘯淇℃伅, 鎵撳嵃璋冭瘯淇℃伅鏃跺皢鍥炴樉
    void SetCurrentTaskDebugInfo(std::string const& info);

    // 鑾峰彇褰撳墠鍗忕▼鐨勮皟璇曚俊鎭�, 杩斿洖鐨勫唴瀹瑰寘鎷敤鎴疯嚜瀹氫箟鐨勪俊鎭拰鍗忕▼ID
    const char* GetCurrentTaskDebugInfo();

    // 鑾峰彇褰撳墠绾跨▼ID.(鎸夋墽琛岃皟搴﹀櫒璋冨害鐨勯『搴忚)
    uint32_t GetCurrentThreadID();

    // 鑾峰彇褰撳墠杩涚▼ID.
    uint32_t GetCurrentProcessID();

    // 鑾峰彇褰撳墠璁℃椂鍣ㄤ腑鐨勪换鍔℃暟閲�
    uint64_t GetTimerCount();

    // 鑾峰彇褰撳墠sleep璁℃椂鍣ㄤ腑鐨勪换鍔℃暟閲�
    uint64_t GetSleepTimerCount();

    // 鑾峰彇褰撳墠鎵�鏈夊崗绋嬬殑缁熻淇℃伅
    std::map<SourceLocation, uint32_t> GetTasksInfo();
    std::vector<std::map<SourceLocation, uint32_t>> GetTasksStateInfo();

#if __linux__
    /// ------------ Linux -------------
    // 鑾峰彇Fd缁熻淇℃伅
    std::string GetFdInfo();

    // 鑾峰彇绛夊緟epoll鐨勫崗绋嬫暟閲�
    uint32_t GetEpollWaitCount();
#endif

    // 鑾峰彇瀵硅薄璁℃暟鍣ㄧ粺璁′俊鎭�
    object_counts_result_t GetDebuggerObjectCounts();

    // 绾跨▼灞�閮ㄥ璞�
    ThreadLocalInfo& GetLocalInfo();

private:
    CoDebugger() = default;
    ~CoDebugger() = default;
    CoDebugger(CoDebugger const&) = delete;
    CoDebugger& operator=(CoDebugger const&) = delete;

    template <typename Drived>
    std::size_t GetDebuggerDrivedIndex()
    {
        static std::size_t s_index = s_debugger_drived_type_index_++;
        return s_index;
    }

private:
    LFLock object_counts_spinlock_;
    object_counts_t object_counts_;
    atomic_t<std::size_t> s_debugger_drived_type_index_{0};
};

template <typename T>
struct real_typename_helper {};

template <typename T>
std::string real_typename()
{
#if defined(__GNUC__)
    /// gcc.
    int s;
    char * realname = abi::__cxa_demangle(typeid(real_typename_helper<T>).name(), 0, 0, &s);
    std::string result(realname);
    free(realname);
#else
    std::string result(typeid(real_typename_helper<T>).name());
#endif
    std::size_t start = result.find_first_of('<') + 1;
    std::size_t end = result.find_last_of('>');
    return result.substr(start, end - start);
}

#if ENABLE_DEBUGGER
template <typename Drived>
CoDebugger::count_t& CoDebugger::DebuggerBase<Drived>::Count()
{
    std::size_t index = CoDebugger::getInstance().GetDebuggerDrivedIndex<Drived>();
    auto &objs = CoDebugger::getInstance().object_counts_;
    if (objs.size() > index)
        return objs[index].second;

    std::unique_lock<LFLock> lock(CoDebugger::getInstance().object_counts_spinlock_);
    if (objs.size() > index)
        return objs[index].second;

    objs.resize(index + 1);
    objs[index].first = real_typename<Drived>();
    return objs[index].second;
}

template <typename Drived>
typename CoDebugger::DebuggerBase<Drived>::object_creator
    CoDebugger::DebuggerBase<Drived>::object_creator_;

#endif

} //namespace co
