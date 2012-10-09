//-----------------------------------------------------------
//    command_dispatcher
//-----------------------------------------------------------
#ifndef SYKES_COMMAND_DISPATCHER_H
#define SYKES_COMMAND_DISPATCHER_H

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <tuple>
#include <utility>
#include <assert.h>

#include "mpl_utility.hpp"

namespace sykes{
    template<int I> struct placeholder {};
    
    //-----------------------------------------------------------
    //    command_dispatch_error
    //-----------------------------------------------------------
    class command_dispatch_error
        : public std::exception
    {
        std::string m_what;
    public:
        command_dispatch_error(std::string const& _w)
            : std::exception(), m_what(_w)
        {}
        
        ~command_dispatch_error() throw()
        {}
        
        virtual const char* what() const throw()
        {
            return m_what.c_str();
        }
    };
    
    //-----------------------------------------------------------
    //    command_dispatcher
    //-----------------------------------------------------------
    template<typename R>
    class command_dispatcher
    {
    private:
        
        typedef std::string id_type;
        template<typename Res, std::size_t N>
        struct call_type;
        
        typedef std::tuple<
            std::unordered_map<id_type, std::function<typename call_type<R, 0>::type>>,
            std::unordered_map<id_type, std::function<typename call_type<R, 1>::type>>,
            std::unordered_map<id_type, std::function<typename call_type<R, 2>::type>>,
            std::unordered_map<id_type, std::function<typename call_type<R, 3>::type>>,
            std::unordered_map<id_type, std::function<typename call_type<R, 4>::type>>,
            std::unordered_map<id_type, std::function<typename call_type<R, 5>::type>>,
            std::unordered_map<id_type, std::function<typename call_type<R, 6>::type>>
        > map_group_type;
        
        map_group_type m_maps;
        
    public:
        static std::size_t const max_argnum = 6;
        //-----------------------------------------------------------
        //    constructer
        //-----------------------------------------------------------
        command_dispatcher()
            : m_maps()
        {}
        
        //-----------------------------------------------------------
        //    register_command
        //-----------------------------------------------------------
        template<typename F, typename ... CastFuncs>
        void register_command(id_type const& id, F f, CastFuncs ... castf){
            static_assert(sizeof...(CastFuncs)  <= max_argnum,
                "class sykes::command_dispacher::register_command()");
            typedef typename sykes::mpl::make_indexed_integral_sequence<int, 1, (sizeof...(CastFuncs) + 1)>::type index_type;
            m_register_command(id, f, index_type(), castf...);
        }
        
        template<typename F, typename ... CastFuncs>
        void register_command(id_type const& id, F f, std::tuple<CastFuncs ...> const& castf){
            static_assert(sizeof...(CastFuncs)  <= max_argnum,
                "class sykes::command_dispacher::register_command()");
            typedef typename sykes::mpl::make_indexed_integral_sequence<int, 1, (sizeof...(CastFuncs) + 1)>::type index_type;
            m_register_command(id, f, index_type(), castf);
        }
        
        //-----------------------------------------------------------
        //    disregister_command
        //-----------------------------------------------------------
        void disregister_command(id_type const& _id, std::size_t _argnum)
        {
            switch(_argnum){
                case 0:
                    if(std::get<0>(m_maps).count(_id) != 0)
                        std::get<0>(m_maps).erase(std::get<0>(m_maps).find(_id));
                    break;
                case 1:
                    if(std::get<1>(m_maps).count(_id) != 0)
                        std::get<1>(m_maps).erase(std::get<1>(m_maps).find(_id));
                    break;
                case 2:
                    if(std::get<2>(m_maps).count(_id) != 0)
                        std::get<2>(m_maps).erase(std::get<2>(m_maps).find(_id));
                    break;
                case 3:
                    if(std::get<3>(m_maps).count(_id) != 0)
                        std::get<3>(m_maps).erase(std::get<3>(m_maps).find(_id));
                    break;
                case 4:
                    if(std::get<4>(m_maps).count(_id) != 0)
                        std::get<4>(m_maps).erase(std::get<4>(m_maps).find(_id));
                    break;
                case 5:
                    if(std::get<5>(m_maps).count(_id) != 0)
                        std::get<5>(m_maps).erase(std::get<5>(m_maps).find(_id));
                    break;
                case 6:
                    if(std::get<6>(m_maps).count(_id) != 0)
                        std::get<6>(m_maps).erase(std::get<6>(m_maps).find(_id));
                    break;
                default:
                    assert(1);
                    break;
            }
        }
        
        //-----------------------------------------------------------
        //    has_command
        //-----------------------------------------------------------
        bool has_command(std::string const& _c, std::size_t _argnum)
        {
            switch(_argnum){
                case 0: return (std::get<0>(m_maps).count(_c) != 0);
                case 1: return (std::get<1>(m_maps).count(_c) != 0);
                case 2: return (std::get<2>(m_maps).count(_c) != 0);
                case 3: return (std::get<3>(m_maps).count(_c) != 0);
                case 4: return (std::get<4>(m_maps).count(_c) != 0);
                case 5: return (std::get<5>(m_maps).count(_c) != 0);
                case 6: return (std::get<6>(m_maps).count(_c) != 0);
                default:
                    assert(1);
                    throw command_dispatch_error("bad argument number");
                    break;
            }
        }
        
        //-----------------------------------------------------------
        //    call
        //-----------------------------------------------------------
        R call(std::string const& str) const
        {
            auto splited = split(str);
            return call(splited);
        }
        
        //-----------------------------------------------------------
        //    call
        //-----------------------------------------------------------
        R call(std::vector<std::string> const& _splited) const
        {
            if(_splited.empty()) throw command_dispatch_error("bad command");
            std::string command = _splited.front();
            switch(_splited.size() - 1){
                case 0:
                    if(std::get<0>(m_maps).count(command) == 0)
                        throw command_dispatch_error("nonexistent command");
                    return std::get<0>(m_maps).at(command)();
                    break;
                case 1:
                    if(std::get<1>(m_maps).count(command) == 0)
                        throw command_dispatch_error("nonexistent command");
                    return std::get<1>(m_maps).at(command)(_splited[1]);
                    break;
                case 2:
                    if(std::get<2>(m_maps).count(command) == 0)
                        throw command_dispatch_error("nonexistent command");
                    return std::get<2>(m_maps).at(command)(_splited[1], _splited[2]);
                    break;
                case 3:
                    if(std::get<3>(m_maps).count(command) == 0)
                        throw command_dispatch_error("nonexistent command");
                    return std::get<3>(m_maps).at(command)(_splited[1], _splited[2], _splited[3]);
                    break;
                case 4:
                    if(std::get<4>(m_maps).count(command) == 0)
                        throw command_dispatch_error("nonexistent command");
                    return std::get<4>(m_maps).at(command)(_splited[1], _splited[2], _splited[3], _splited[4]);
                    break;
                case 5:
                    if(std::get<5>(m_maps).count(command) == 0)
                        throw command_dispatch_error("nonexistent command");
                    return std::get<5>(m_maps).at(command)(_splited[1], _splited[2], _splited[3], _splited[4], _splited[5]);
                    break;
                case 6:
                    if(std::get<6>(m_maps).count(command) == 0)
                        throw command_dispatch_error("nonexistent command");
                    return std::get<6>(m_maps).at(command)(_splited[1], _splited[2], _splited[3], _splited[4], _splited[5], _splited[6]);
                    break;
                default:
                    throw command_dispatch_error("bad argument number");
                    break;
            }
        }
    
    private:
        template<typename Res> struct call_type<Res, 0>{ typedef Res type(void); };
        template<typename Res> struct call_type<Res, 1>{ typedef Res type(std::string const&); };
        template<typename Res> struct call_type<Res, 2>{ typedef Res type(std::string const&, std::string const&); };
        template<typename Res> struct call_type<Res, 3>{ typedef Res type(std::string const&, std::string const&, std::string const&); };
        template<typename Res> struct call_type<Res, 4>{ typedef Res type(std::string const&, std::string const&, std::string const&, std::string const&); };
        template<typename Res> struct call_type<Res, 5>{ typedef Res type(std::string const&, std::string const&, std::string const&, std::string const&, std::string const&); };
        template<typename Res> struct call_type<Res, 6>{ typedef Res type(std::string const&, std::string const&, std::string const&, std::string const&, std::string const&, std::string const&); };
        
        //-----------------------------------------------------------
        //    m_register_command
        //-----------------------------------------------------------
        template<typename F, int ... Idx, typename ... CastFuncs>
        void m_register_command(id_type const& id, F f, sykes::mpl::vector_c<int, Idx ...>, CastFuncs ... castf){
            auto func = std::bind(f, std::bind(castf, placeholder<Idx>())...);
            std::get<sizeof...(Idx)>(m_maps)[id] = func;
        }
        
        template<typename F, int ... Idx, typename ... CastFuncs>
        void m_register_command(id_type const& id, F f, sykes::mpl::vector_c<int, Idx ...>, std::tuple<CastFuncs ...> const& castf){
            auto func = std::bind(f, std::bind(std::get<Idx - 1>(castf), placeholder<Idx>())...);
            std::get<sizeof...(Idx)>(m_maps)[id] = func;
        }

        static std::vector<std::string> split(std::string const _str, std::string const& _delimiters = " \t\n")
        {
            std::string::const_iterator it1 = _str.begin(), end1 = _str.end();
            std::vector<std::string> splited;
            splited.reserve(10);
            
            while(it1 != end1){
                while(_delimiters.find(*it1) != std::string::npos && (it1 != end1))
                    ++it1;
                std::string strtmp;
                strtmp.reserve(10);
                
                while(_delimiters.find(*it1) == std::string::npos && (it1 != end1)){
                    strtmp.push_back(*it1);
                    ++it1;
                }
                if(strtmp != "") splited.push_back(strtmp);
            }
            return std::move(splited);
        }
    };
}

namespace std
{
    template<int I>
    struct is_placeholder<sykes::placeholder<I> > : integral_constant<int, I> {};
}
#endif

