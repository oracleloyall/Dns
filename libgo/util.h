#pragma once
#include "config.h"

namespace co
{

// 鍙浼樺寲鐨刲ock guard
struct fake_lock_guard
{
    template <typename Mutex>
    explicit fake_lock_guard(Mutex&) {}
};

///////////////////////////////////////
/*
* 杩欓噷鏋勫缓浜嗕竴涓崐渚靛叆寮忕殑寮曠敤璁℃暟浣撶郴, 浣跨敤shared_ptr璇剰鐨勫悓鏃�,
* 鍙堝彲浠ュ皢瀵硅薄鏀惧叆渚靛叆寮忓鍣�, 寰楀埌鏋佷匠鐨勬�ц兘.
*/

// 渚靛叆寮忓紩鐢ㄨ鏁板璞″熀绫�
struct RefObject
{
    atomic_t<long> reference_;

    RefObject() : reference_{1} {}
    virtual ~RefObject() {}

    void IncrementRef()
    {
        ++reference_;
    }

    void DecrementRef()
    {
        if (--reference_ == 0)
            delete this;
    }

    RefObject(RefObject const&) = delete;
    RefObject& operator=(RefObject const&) = delete;
};

// 瑁告寚閽� -> shared_ptr
template <typename T>
typename std::enable_if<std::is_base_of<RefObject, T>::value,
    std::shared_ptr<T>>::type
SharedFromThis(T * ptr)
{
    ptr->IncrementRef();
    return std::shared_ptr<T>(ptr, [](T * self){ self->DecrementRef(); });
}

// make_shared
template <typename T, typename ... Args>
typename std::enable_if<std::is_base_of<RefObject, T>::value,
    std::shared_ptr<T>>::type
MakeShared(Args && ... args)
{
    T * ptr = new T(std::forward<Args>(args)...);
    return std::shared_ptr<T>(ptr, [](T * self){ self->DecrementRef(); });
}

template <typename T>
typename std::enable_if<std::is_base_of<RefObject, T>::value>::type
IncrementRef(T * ptr)
{
    ptr->IncrementRef();
}
template <typename T>
typename std::enable_if<!std::is_base_of<RefObject, T>::value>::type
IncrementRef(T * ptr)
{
}
template <typename T>
typename std::enable_if<std::is_base_of<RefObject, T>::value>::type
DecrementRef(T * ptr)
{
    ptr->DecrementRef();
}
template <typename T>
typename std::enable_if<!std::is_base_of<RefObject, T>::value>::type
DecrementRef(T * ptr)
{
}

// 寮曠敤璁℃暟guard
class RefGuard
{
public:
    template <typename T>
    explicit RefGuard(T* ptr) : ptr_(static_cast<RefObject*>(ptr))
    {
        ptr_->IncrementRef();
    }
    template <typename T>
    explicit RefGuard(T& obj) : ptr_(static_cast<RefObject*>(&obj))
    {
        ptr_->IncrementRef();
    }
    ~RefGuard()
    {
        ptr_->DecrementRef();
    }

    RefGuard(RefGuard const&) = delete;
    RefGuard& operator=(RefGuard const&) = delete;

private:
    RefObject *ptr_;
};
///////////////////////////////////////

// 鍒涘缓鍗忕▼鐨勬簮鐮佹枃浠朵綅缃�
struct SourceLocation
{
    const char* file_ = nullptr;
    int lineno_ = 0;

    void Init(const char* file, int lineno)
    {
        file_ = file, lineno_ = lineno;
    }

    friend bool operator<(SourceLocation const& lhs, SourceLocation const& rhs)
    {
        if (lhs.file_ != rhs.file_)
            return lhs.file_ < rhs.file_;

        return lhs.lineno_ < rhs.lineno_;
    }

    std::string to_string() const
    {
        std::string s("{file:");
        if (file_) s += file_;
        s += ", line:";
        s += std::to_string(lineno_) + "}";
        return s;
    }
};


} //namespace co
