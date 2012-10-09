//-----------------------------------------------------------
//    
//-----------------------------------------------------------
#ifndef SYNTH_TYPE_H
#define SYNTH_TYPE_H

#include <cstddef>
#include <type_traits>
#include <utility>
#include <functional>

#include "midi_utility.h"

namespace TSynth{
    typedef double Real;
    typedef signed char Int8;
    typedef unsigned char UInt8;
    typedef unsigned short UInt16;
    typedef unsigned int UInt32;
    
    template<typename T>
    typename std::add_rvalue_reference<T>::type declval();
    
    //-----------------------------------------------------------
    //    enum class SynthModType
    //-----------------------------------------------------------
    enum class SynthModType : int
    {
        NULLARY = 0,
        UNARY = 1,
        BINARY = 2
    };
    
    //-----------------------------------------------------------
    //    enum class EGState
    //-----------------------------------------------------------
    enum class EGState : int
    {
        ATTACK = 301,
        DECAY = 302,
        SUSTAIN = 303,
        RELEASE = 304,
        OFF = 305
    };
    
    //-----------------------------------------------------------
    //    WaveType
    //-----------------------------------------------------------
    enum class WaveType : int
    {
        SIN = 201,
        SAW = 202,
        TRI = 203,
        SQU = 204
    };
    
    //-----------------------------------------------------------
    //    struct ADSR
    //-----------------------------------------------------------
    struct ADSR
    {
	    Real attack; // on the second time scale
	    Real decay;  // on the second time scale
	    Real sustain;// 0 to 1.0
	    Real release;// on the second time scale
    };
    
    //-----------------------------------------------------------
    //    struct MidiReceivable
    //-----------------------------------------------------------
    template<typename T>
    struct MidiReceivable
    {
        static bool const value = std::decay<decltype(MidiReceivable::template check_value<T>(0))>::type::value;
    private:
        template<typename T1, typename S = decltype(declval<T1>().MidiReceive(declval<sykes::midi::message>()))>
        static std::true_type check_value(int);
        template<typename T1>
        static std::false_type check_value(long);
    };
    
    //-----------------------------------------------------------
    //    struct PortDoubleAvailable
    //-----------------------------------------------------------
    template<typename T>
    struct PortDoubleAvailable
    {
        static bool const value = std::decay<decltype(PortDoubleAvailable::template check_value<T>(0))>::type::value;
    private:
        template<typename T1,
            typename S = typename std::enable_if<
                std::is_same<typename std::decay<decltype(declval<T1>().GetPortDouble())>::type,
                    std::function<void(int, double)>>::value,
                void
            >::type
        >
        static std::true_type check_value(int);
        template<typename T1>
        static std::false_type check_value(long);
    };
    
    //-----------------------------------------------------------
    //    struct PortIntAvailable
    //-----------------------------------------------------------
    template<typename T>
    struct PortIntAvailable
    {
        static bool const value = std::decay<decltype(PortIntAvailable::template check_value<T>(0))>::type::value;
    private:
        template<typename T1,
            typename S = typename std::enable_if<
                std::is_same<typename std::decay<decltype(declval<T1>().GetPortInt())>::type,
                    std::function<void(int, int)>>::value,
                void
            >::type
        >
        static std::true_type check_value(int);
        template<typename T1>
        static std::false_type check_value(long);
    };
    
    //-----------------------------------------------------------
    //    struct HasIsActive
    //-----------------------------------------------------------
    template<typename T>
    struct HasIsActive
    {
        static bool const value = std::decay<decltype(HasIsActive::template check_value<T>(0))>::type::value;
    private:
        template<typename T1,
            typename S = typename std::enable_if<
                std::is_same<typename std::decay<decltype(declval<T1>().IsActive())>::type,
                    bool>::value,
                void
            >::type
        >
        static std::true_type check_value(int);
        template<typename T1>
        static std::false_type check_value(long);
    };
    
    //-----------------------------------------------------------
    //    struct PortIsActiveAvailable
    //-----------------------------------------------------------
    template<typename T>
    struct PortIsActiveAvailable
    {
        static bool const value = std::decay<decltype(PortIsActiveAvailable::template check_value<T>(0))>::type::value;
    private:
        template<typename T1,
            typename S = typename std::enable_if<
                std::is_same<typename std::decay<decltype(declval<T1>().GetPortIsActive())>::type,
                    std::function<bool()>>::value,
                void
            >::type
        >
        static std::true_type check_value(int);
        template<typename T1>
        static std::false_type check_value(long);
    };
}//---- namespace
#endif

