#ifndef sykes_mutex_h
#define sykes_mutex_h

#include <pthread.h>
#include <stdexcept>
#include <errno.h>
#include "lock.h"

namespace sykes
{
    class mutex
    {
    public:
        typedef unique_lock<mutex> lock_type;
        
        inline mutex() : m_impl()
        {
            int r = pthread_mutex_init(&m_impl, NULL);
            if(r) throw std::runtime_error("sykes::mutex::mutex()");
        }
        
        inline ~mutex()
        {
            pthread_mutex_destroy(&m_impl);
        }
        
        inline void lock()
        {
            int r = pthread_mutex_lock(&m_impl);
            if(r) throw std::runtime_error("sykes::mutex::lock()");
        }

        inline void unlock()
        {
            pthread_mutex_unlock(&m_impl);
        }
        
        inline bool try_lock()
        {
            int r = pthread_mutex_trylock(&m_impl);
            if(r && (r!=EBUSY))
                std::runtime_error("sykes::mutex::try_lock()");
            return !r;
        }
        
    private:
        pthread_mutex_t m_impl;
        mutex(mutex const&) = delete;
        mutex& operator=(mutex const&) = delete;
    };
}
#endif

