#ifndef SYKE_PORT_DISPATCHER_H
#define SYKE_PORT_DISPATCHER_H

#include <utility>
#include <memory>
#include <map>
#include <cstddef>
#include <boost/signals2/signal.hpp>
#include <boost/signals2/connection.hpp>

namespace sykes
{
    template<typename K, typename P>
    class port_dispatcher;
    
    template<typename K, typename R, typename ... A>
    class port_dispatcher<K, R(A...)>
    {
    public:
        typedef K key_type;
        typedef R result_type;
        typedef boost::signals2::signal<R(A...)> signal_type;
        typedef std::shared_ptr<signal_type> signal_ptr;
        typedef boost::signals2::connection connection_type;
        
        template<typename F>
        connection_type connect(key_type key, F f)
        {
            if(m_sigs.count(key)){
                return m_sigs.at(key).get()->connect(f);
            }else{
                m_sigs[key] = signal_ptr(new signal_type);
                return m_sigs.at(key).get()->connect(f);
            }
        }
        
        std::size_t count(key_type key)
        {
            return m_sigs.count(key);
        }
        
        result_type operator()(key_type k, A ... a) const
        {
            return m_sigs.at(k).get()->operator()(a...);
        }
        
    private:
        std::map<key_type, signal_ptr> m_sigs;
    };
}
#endif

