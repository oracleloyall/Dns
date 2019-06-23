#pragma once
#include "config.h"
#include "context.h"
#include "task.h"
#include "block_object.h"
#include "co_mutex.h"
#include "timer.h"
#include "sleep_wait.h"
#include "processer.h"
#include "debugger.h"
#include "linux/io_wait.h"

namespace co {

struct ThreadLocalInfo
{
    int thread_id = -1;     // Run thread index, increment from 1.
    uint8_t sleep_ms = 0;
    Processer *proc = nullptr;
};

class ThreadPool;

class Scheduler
{
    public:
	// Run鏃舵墽琛岀殑鍐呭
        enum eRunFlags
        {
            erf_do_coroutines   = 0x1,
            erf_do_timer        = 0x2,
            erf_do_sleeper      = 0x4,
            erf_do_eventloop    = 0x8,
            erf_idle_cpu        = 0x10,
            erf_signal          = 0x20,
            erf_all             = 0x7fffffff,
        };

        typedef std::deque<Processer*> ProcList;
        typedef std::pair<uint32_t, TSQueue<Task, false>> WaitPair;
        typedef std::unordered_map<uint64_t, WaitPair> WaitZone;
        typedef std::unordered_map<int64_t, WaitZone> WaitTable;

        static Scheduler& getInstance();

	// 鑾峰彇閰嶇疆閫夐」
        CoroutineOptions& GetOptions();

	// 鍒涘缓涓�涓崗绋�
        void CreateTask(TaskF const& fn, std::size_t stack_size,
                const char* file, int lineno, int dispatch);

	// 褰撳墠鏄惁澶勪簬鍗忕▼涓�
        bool IsCoroutine();

	// 鏄惁娌℃湁鍗忕▼鍙墽琛�
        bool IsEmpty();

	// 褰撳墠鍗忕▼璁╁嚭鎵ц鏉�
        void CoYield();

	// 璋冨害鍣ㄨ皟搴﹀嚱鏁�, 鍐呴儴鎵ц鍗忕▼銆佽皟搴﹀崗绋�
        uint32_t Run(int flags = erf_all);

	// 寰幆Run鐩村埌娌℃湁鍗忕▼涓烘
	// @loop_task_count: 涓嶈鏁扮殑甯搁┗鍗忕▼.
	//    渚嬪锛歭oop_task_count == 2鏃�, 杩樺墿鏈�鍚�2涓崗绋嬬殑鏃跺�欒繖涓嚱鏁板氨浼歳eturn.
	// @remarks: 杩欎釜鎺ュ彛浼氳嚦灏戞墽琛屼竴娆un.
        void RunUntilNoTask(uint32_t loop_task_count = 0);

	// 鏃犻檺寰幆鎵цRun
        void RunLoop();

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

    public:
        /// sleep switch
        //  \timeout_ms min value is 0.
        void SleepSwitch(int timeout_ms);

        /// ------------------------------------------------------------------------
	// @{ 瀹氭椂鍣�
        template <typename DurationOrTimepoint>
        TimerId ExpireAt(DurationOrTimepoint const& dur_or_tp, CoTimer::fn_t const& fn)
        {
            TimerId id = timer_mgr_.ExpireAt(dur_or_tp, fn);
            DebugPrint(dbg_timer, "add timer id=%llu", (long long unsigned)id->GetId());
            return id;
        }

        bool CancelTimer(TimerId timer_id);
        bool BlockCancelTimer(TimerId timer_id);
        // }@
        /// ------------------------------------------------------------------------

        /// ------------------------------------------------------------------------
	// @{ 绾跨▼姹�
        ThreadPool& GetThreadPool();
        // }@
        /// ------------------------------------------------------------------------

	// iowait瀵硅薄
        IoWait& GetIoWait() { return io_wait_; }

    public:
        Task* GetCurrentTask();

    private:
        Scheduler();
        ~Scheduler();

        Scheduler(Scheduler const&) = delete;
        Scheduler(Scheduler &&) = delete;
        Scheduler& operator=(Scheduler const&) = delete;
        Scheduler& operator=(Scheduler &&) = delete;

	// 灏嗕竴涓崗绋嬪姞鍏ュ彲鎵ц闃熷垪涓�
        void AddTaskRunnable(Task* tk, int dispatch = egod_default);

	// Run鍑芥暟鐨勪竴閮ㄥ垎, 澶勭悊runnable鐘舵�佺殑鍗忕▼
        uint32_t DoRunnable(bool allow_steal = true);

	// Run鍑芥暟鐨勪竴閮ㄥ垎, 澶勭悊epoll鐩稿叧
        int DoEpoll(int wait_milliseconds);

	// Run鍑芥暟鐨勪竴閮ㄥ垎, 澶勭悊sleep鐩稿叧
	// @next_ms: 璺濈涓嬩竴涓猼imer瑙﹀彂鐨勬绉掓暟
        uint32_t DoSleep(long long &next_ms);

	// Run鍑芥暟鐨勪竴閮ㄥ垎, 澶勭悊瀹氭椂鍣�
	// @next_ms: 璺濈涓嬩竴涓猼imer瑙﹀彂鐨勬绉掓暟
        uint32_t DoTimer(long long &next_ms);

	// 鑾峰彇绾跨▼灞�閮ㄤ俊鎭�
        ThreadLocalInfo& GetLocalInfo();

        Processer* GetProcesser(std::size_t index);

        // List of Processer
        LFLock proc_init_lock_;
        ProcList run_proc_list_;
        atomic_t<uint32_t> dispatch_robin_index_{0};

        // io block waiter.
        IoWait io_wait_;

        // sleep block waiter.
        SleepWait sleep_wait_;

        // Timer manager.
        CoTimerMgr timer_mgr_;

        ThreadPool *thread_pool_;
        LFLock thread_pool_init_;

        atomic_t<uint32_t> task_count_{0};
        atomic_t<uint32_t> thread_id_{0};

    private:
        friend class CoMutex;
        friend class BlockObject;
        friend class IoWait;
        friend class SleepWait;
        friend class Processer;
        friend class FileDescriptorCtx;
        friend class CoDebugger;

    public:
        /**
	 * 鍗忕▼浜嬩欢鐩戝惉鍣�
	 * 娉ㄦ剰锛氬叾涓墍鏈夌殑鍥炶皟鏂规硶閮戒笉鍏佽鎶涘嚭寮傚父
         */
        class TaskListener {
        public:
            /**
		 * 鍗忕▼琚垱寤烘椂琚皟鐢�
		 * 锛堟敞鎰忔鏃跺苟鏈繍琛屽湪鍗忕▼涓級
             *
		 * @prarm task_id 鍗忕▼ID
             * @prarm eptr
             */
            virtual void onCreated(uint64_t task_id) noexcept {
            }

            /**
		 * 鍗忕▼寮�濮嬭繍琛�
		 * 锛堟湰鏂规硶杩愯鍦ㄥ崗绋嬩腑锛�
             *
		 * @prarm task_id 鍗忕▼ID
             * @prarm eptr
             */
            virtual void onStart(uint64_t task_id) noexcept {
            }

            /**
		 * 鍗忕▼姝ｅ父杩愯缁撴潫锛堟棤寮傚父鎶涘嚭锛�
		 * 锛堟湰鏂规硶杩愯鍦ㄥ崗绋嬩腑锛�
             *
		 * @prarm task_id 鍗忕▼ID
             */
            virtual void onCompleted(uint64_t task_id) noexcept {
            }

            /**
		 * 鍗忕▼鎶涘嚭鏈鎹曡幏鐨勫紓甯革紙鏈柟娉曡繍琛屽湪鍗忕▼涓級
		 * @prarm task_id 鍗忕▼ID
		 * @prarm eptr 鎶涘嚭鐨勫紓甯稿璞℃寚閽堬紝鍙鏈寚閽堣祴鍊间互淇敼寮傚父瀵硅薄锛�
		 *             寮傚父灏嗕娇鐢� CoroutineOptions.exception_handle 涓�
		 *             閰嶇疆鐨勬柟寮忓鐞嗭紱璧嬪�间负nullptr鍒欒〃绀哄拷鐣ユ寮傚父
		 *             锛侊紒娉ㄦ剰锛氬綋 exception_handle 閰嶇疆涓� immedaitely_throw 鏃舵湰浜嬩欢
		 *             锛侊紒涓� onFinished() 鍧囧け鏁堬紝寮傚父鍙戠敓鏃跺皢鐩存帴鎶涘嚭骞朵腑鏂▼搴忕殑杩愯锛屽悓鏃剁敓鎴恈oredump
             */
            virtual void onException(uint64_t task_id, std::exception_ptr& eptr) noexcept {
            }

            /**
		 * 鍗忕▼杩愯瀹屾垚锛宨f(eptr) 涓篺alse璇存槑鍗忕▼姝ｅ父缁撴潫锛屼负true璇存槑鍗忕▼鎶涘嚭浜嗕簡寮傚父
             *
		 * @prarm task_id 鍗忕▼ID
		 * @prarm eptr 鎶涘嚭鐨勫紓甯稿璞℃寚閽�
             */
            virtual void onFinished(uint64_t task_id, const std::exception_ptr eptr) noexcept {
            }

            virtual ~TaskListener() noexcept = default;

            //                      ---> onCompleted -->
            //                      |                  |
            // onCreated --> onStart                   ---> onFinished
            //                      |                  |
            //                      ---> onException -->
        };

    private:
        TaskListener* task_listener = nullptr;

    public:
        inline TaskListener* GetTaskListener() {
            return task_listener;
        }
        inline void SetTaskListener(TaskListener* listener) {
            this->task_listener = listener;
        }
};

} //namespace co

#define g_Scheduler ::co::Scheduler::getInstance()

namespace co
{
    inline Scheduler& Scheduler::getInstance()
    {
        static Scheduler obj;
        return obj;
    }

    inline CoroutineOptions& Scheduler::GetOptions()
    {
        return CoroutineOptions::getInstance();
    }

} //namespace co
