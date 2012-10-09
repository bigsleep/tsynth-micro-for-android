#ifndef sykes_lock_h
#define sykes_lock_h

#include <stdexcept>

namespace sykes
{
    template<typename Mutex>
    class unique_lock
    {
    public:
        explicit unique_lock(Mutex& _m) : m_mutex(_m), m_locked(false)
        {
            lock();
        }
        
        ~unique_lock()
        {
            if(owns_lock()) m_mutex.unlock();
        }
        
        void lock()
        {
            if(owns_lock()) throw std::runtime_error("sykes::unique_lock::lock");
            m_mutex.lock();
            m_locked = true;
        }
        
        bool try_lock()
        {
            if(owns_lock()) throw std::runtime_error("sykes::unique_lock::try_lock");
            m_locked = m_mutex.try_lock();
            return m_locked;
        }
        
        void unlock()
        {
            if(!owns_lock()) throw std::runtime_error("sykes::unique_lock::unlock");
            m_mutex.unlock();
            m_locked = false;
        }
        
        bool owns_lock() const { return m_locked; }
        
    private:
        Mutex& m_mutex;
        bool m_locked;
    };
}
#endif

