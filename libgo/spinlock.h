#pragma once
#include "config.h"
#define zhaoxi
#include"common/lock.h"
namespace co
{

#if LIBGO_SINGLE_THREAD
struct LFLock
{
    bool locked_ = false;

    ALWAYS_INLINE void lock()
    {
        while (!locked_) locked_ = true;
        DebugPrint(dbg_spinlock, "lock");
    }

    ALWAYS_INLINE bool try_lock()
    {
        bool ret = !locked_;
        if (ret) locked_ = true;
        DebugPrint(dbg_spinlock, "trylock returns %s", ret ? "true" : "false");
        return ret;
    }

    ALWAYS_INLINE void unlock()
    {
        assert(locked_);
        locked_ = false;
        DebugPrint(dbg_spinlock, "unlock");
    }
};
#endif
#if LIBGO_SINGLE_THREAD_
struct LFLock
{
    volatile std::atomic_flag lck;

    LFLock()
    {
        lck.clear();
    }

    ALWAYS_INLINE void lock()
    {
        while (std::atomic_flag_test_and_set_explicit(&lck, std::memory_order_acquire)) ;
        DebugPrint(dbg_spinlock, "lock");
    }

    ALWAYS_INLINE bool try_lock()
    {
        bool ret = !std::atomic_flag_test_and_set_explicit(&lck, std::memory_order_acquire);
        DebugPrint(dbg_spinlock, "trylock returns %s", ret ? "true" : "false");
        return ret;
    }

    ALWAYS_INLINE void unlock()
    {
        std::atomic_flag_clear_explicit(&lck, std::memory_order_release);
        DebugPrint(dbg_spinlock, "unlock");
    }
};
#endif //LIBGO_SINGLE_THREAD
#ifdef zhaoxi
struct LFLock {
public:

	LFLock() {
		S_INIT_LOCK(&splock);
	}

	ALWAYS_INLINE void lock() {
		S_LOCK(&splock);
	}

	ALWAYS_INLINE bool try_lock() {
		return (bool) S_LOCK_FREE(&splock); //true not lock ,can be lock
	}

	ALWAYS_INLINE void unlock() {
		S_UNLOCK(&splock);
	}
private:
	volatile slock_t splock;
};
#endif
} //namespace co
