//-----------------------------------------------------------
//    SynthVCA
//-----------------------------------------------------------
#ifndef SYNTH_VCA_H
#define SYNTH_VCA_H

#include "type.h"
#include "constants.h"
#include "midi_utility.h"
#include "eg.h"
#include "synth_mod.h"
//#include <iostream>
#include <functional>
#include <android/log.h>

namespace TSynth{
    
    //-----------------------------------------------------------
    //    class SynthVCA
    //-----------------------------------------------------------
    class SynthVCA
    {
    public:
	    typedef Real result_type;
        SynthVCA()
            : m_level(Constants::vca_default_level), m_level_function()
        {}
        
        SynthVCA(double _l, double _a, double _d, double _s, double _r)
            : m_level(_l), m_level_function(_a, _d, _s, _r)
        {}
        
        Real operator()(Real _in)
        {
            return _in * m_level * m_level_function();
        }
        
        void SetLevel(Real _l)
        {
            if(_l >= 0.0 && _l <= 1.0) m_level = _l;
        }
        
        Real GetLevel() const
        { return m_level; }
        
        bool IsActive() const
        {
            return m_level_function.IsActive();
        }
        
        void MidiReceive(sykes::midi::message _m)
        {
            //std::cout << "SynthVCA::MidiReceive" << std::endl;
            m_level_function.MidiReceive(_m);
            switch(sykes::midi::message::status(_m))
            {
                case sykes::midi::channel_voice_message_type::NOTE_ON:
                    //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "vca noteOn");
                    return;
                case sykes::midi::channel_voice_message_type::NOTE_OFF:
                    //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "vca noteOff");
                    return;
                default:
                    return;
            }
        }
        
        void SetParameterDouble(int port, double value)
        {
            switch(port){
                case 1:
                    m_level_function.SetAttack(value);
                    break;
                case 2:
                    m_level_function.SetDecay(value);
                    break;
                case 3:
                    m_level_function.SetSustain(value);
                    break;
                case 4:
                    m_level_function.SetRelease(value);
                    break;
                case 5:
                    m_level = std::min(std::max(0.0, value), 2.0);
                    break;
                default:
                    break;
            }
        }
        
        std::function<void(int, double)> GetPortDouble()
        {
            return std::bind(&SynthVCA::SetParameterDouble, this, std::placeholders::_1, std::placeholders::_2);
        }
        
        std::function<bool()> GetPortIsActive() const
        {
            return std::bind(&SynthVCA::IsActive, this);
        }
        
    private:
        Real m_level;
        SynthEG m_level_function;
        
        TSYNTH_USE_AS_MOD
    };
    
    TSYNTH_DECLARE_UNARY_MOD(SynthVCA, (StoI(), StoD(), StoD(), StoD(), StoD(), StoD()))
}

#endif


