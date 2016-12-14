/*
When a data variable needs to be synchronized, in addition to the data variable, added synchronization variable (like mutex, critiocal section).
Class Protected combines these 2 variables and doesn't allow to use the data variable without synchronization.

For instance:

// Methods of data cannot accessed since Protected uses private inheritance.
ProtectedByCriticalSection<vector<int>> protectedData = {1, 2, 3};

// In order to access methods, should be called 'Lock()', it returns 'Locker' object, which allow to access data via it's pointer.
// Constructor of 'Locker' locks synchronization object and it's destructor unlocks it.
auto pData = protectedData.Lock();

pDta->push_back(1);

// Or could be used dereference to data
auto& data = *pData;

data.push_back(1);
*/


#pragma once

#include <mutex>

template <typename T, typename Protector>
class Protected: private T
{
    Protector protector_;

private:
    template <typename T>
    class ProtectedPublic : public T
    {
        Protector protector_;

    public:
        template <typename Arg>
        ProtectedPublic& operator = (const Arg& arg)
        {
            (T&)(*this) = arg;

            return *this;
        }

        template <typename Arg>
        ProtectedPublic& operator = (const std::initializer_list<Arg>& list)
        {
            (T&)(*this) = list;

            return *this;
        }
    };

public:
    class Locker
    {
        Protected* pProtected_ = nullptr;
        mutable Protector* pProtector_ = nullptr;

    public:
        Locker(Protected* pProtected, Protector* pProtector)
            : pProtected_(pProtected), pProtector_(pProtector)
        {
            printf("1 Locker\n");

            pProtector_->lock();
        }

        ~Locker()
        {
            if (pProtector_)
            {
                pProtector_->unlock();
            }
        }

        // 'Locker' can only be created via 'Protected::Lock()'. Return value optimization (RVO) is supported on most compilers, then copy constructor won't be called, 
        // but just in case implement copy constructor.
        Locker(const Locker& rhs)
            :pProtected_(rhs.pProtected_), pProtector_(rhs.pProtector_)
        {
            // 'unlock' of 'rhs' destructor won't be called
            rhs.pProtector_ = nullptr;
        }

        // Prevent copy operator
        Locker& operator = (const Locker& rhs) = delete;

        ProtectedPublic<T>* operator ->()
        {
            // Can cast since Protected and ProtectedPublic have the same layout
            return (ProtectedPublic<T>*)pProtected_;
        }

        ProtectedPublic<T>& operator*()
        {
            return (ProtectedPublic<T>&)*pProtected_;
        }

        operator T&()
        {
            return *pProtected_;
        }
    };


    template<class Arg>
    Arg&& CheckAndForward(typename std::remove_reference<Arg>::type& arg)
    {
        static_assert(!std::is_base_of<Protected, typename std::remove_reference<Arg>::type>::value, "'Protected' constructor's parameter type cannot be 'Protected'");

        return static_cast<Arg&&>(arg);
    }

    template <typename... Args>
    Protected(Args&&... args)
        :T(CheckAndForward<Args>(args)...)
    {}

    template <typename Arg>
    Protected(const std::initializer_list<Arg>& list)
        :T(list)
    {}

    Locker Lock()
    {
        return Locker(this, &protector_);
    }

    // Prevent assignments
    template <typename Arg>
    Protected& operator = (const Arg& arg) = delete;

    Protected& operator = (const Protected& arg) = delete;
};

template <typename T>
using ProtectedByMutex = Protected<T, std::mutex>;

template <typename T>
using ProtectedByRecursiveMutex = Protected<T, std::recursive_mutex>;


#if defined (_WIN32)
    struct ProtectorCriticalSection
    {
        ProtectorCriticalSection()
        {
            InitializeCriticalSection(&cs_);
        };

        ~ProtectorCriticalSection()
        {
            DeleteCriticalSection(&cs_);
        };

        void lock()
        {
            EnterCriticalSection(&cs_);
        }

        void unlock()
        {
            LeaveCriticalSection(&cs_);
        }

        CRITICAL_SECTION cs_;
    };

    template <typename T>
    using ProtectedByCriticalSection = Protected<T, ProtectorCriticalSection>;
#endif

