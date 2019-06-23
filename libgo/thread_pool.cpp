#include "thread_pool.h"
#include <unistd.h>
#include <thread>
#include "scheduler.h"
#include "error.h"

namespace co {

ThreadPool::~ThreadPool()
{
    TPElemBase *elem = nullptr;
    while ((elem = elem_list_.pop())) {
        delete elem;
    }
}

void ThreadPool::RunLoop()
{
    assert_std_thread_lib();
    for (;;)
    {
        std::unique_ptr<TPElemBase> elem(get());
        if (elem)
            elem->Do();
    }
}

TPElemBase* ThreadPool::get()
{
    TPElemBase *elem = elem_list_.pop();
    if (elem) return elem;

    std::unique_lock<std::mutex> lock(cond_mtx_);
    while (elem_list_.empty())
        cond_.wait(lock);
    return elem_list_.pop();
}

void ThreadPool::assert_std_thread_lib()
{
    static bool ass = __assert_std_thread_lib();
    (void)ass;
}
bool ThreadPool::__assert_std_thread_lib()
{
    std::mutex mtx;
    std::unique_lock<std::mutex> lock1(mtx);
    std::unique_lock<std::mutex> lock2(mtx, std::defer_lock);
    if (lock2.try_lock()) {
        // std::thread娌℃湁鐢熸晥
        // 鍦╣cc涓�, 闈欐�侀摼鎺ヨ鍦ㄩ摼鎺ユ椂浣跨敤鍙傛暟:
        //    -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -static
        //          鍔ㄦ�侀摼鎺ヨ鍦ㄧ紪璇戝拰閾炬帴鏃堕兘浣跨敤鍙傛暟锛�
        //    -pthread
        //    娉ㄦ剰锛氭槸-pthread, 鑰屼笉鏄�-lpthread.
        ThrowError(eCoErrorCode::ec_std_thread_link_error);
    }

//    try {
//        int v = 0;
//        std::thread t([&]{ v = 1; });
//        t.detach();
//        if (!v)
//            ThrowError(eCoErrorCode::ec_std_thread_link_error);
//    } catch (std::exception & e) {
//        ThrowError(eCoErrorCode::ec_std_thread_link_error);
//    }
    return true;
}

} //namespace co

