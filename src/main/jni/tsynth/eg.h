//-----------------------------------------------------------
//    SynthEG
//-----------------------------------------------------------
#ifndef SYNTH_EG_H
#define SYNTH_EG_H

#include "type.h"
#include "constants.h"
#include "synth_mod_base.h"
//#include <iostream>
#include <cstddef>
#include <android/log.h>

namespace TSynth{

    extern Real const inv_exp_table[];
    extern ADSR const default_ADSR;
    //-----------------------------------------------------------
    //    class SynthEG
    //-----------------------------------------------------------
    class SynthEG
    {
    public:
        SynthEG();
        SynthEG(Real _a, Real _d, Real _s, Real _r);
        
        Real operator()();
        
        inline void SetState(EGState _egs)
        {
	        ResetPhase();
            m_state = _egs;
        }
        
        inline EGState GetState() const
        { return m_state; }
        
        inline void ResetPhase()
        { m_phase = 0; }
        
        inline void SetAttack(Real _ac)
        {
            //std::cout << "SynthEG::SetAttack" << std::endl;
            //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "eg SetAttack %f", _ac);
            Real tmp = static_cast<Real>(SynthModBase::GetSampleRate()) * _ac;
            m_attack = static_cast<std::size_t>(tmp);
            if(m_attack != 0)
                m_attack_rate = 1.0 / tmp;
            else
                m_attack_rate = 0.0;
        }
        
        inline void SetDecay(Real _dc)
        {
            //std::cout << "SynthEG::SetDecay" << std::endl;
            //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "eg SetDecay %f", _dc);
            Real tmp = static_cast<Real>(SynthModBase::GetSampleRate()) * _dc;
            m_decay = static_cast<std::size_t>(tmp);
            if(m_decay != 0)
                m_delta_phase_decay = static_cast<Real>(Constants::inv_exp_table_size) / tmp;
            else
                m_delta_phase_decay = 0.0;
        }
        
        inline void SetSustain(Real _sl)
        {
            //std::cout << "SynthEG::Sustain" << std::endl;
            //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "eg SetSustain %f", _sl);
            if((_sl >= 0.0) && (_sl <= 1.0)){
                m_sustain = _sl;
                m_release_level = m_sustain;
            }
        }
        
        inline void SetRelease(Real _rc)
        {
            //std::cout << "SynthEG::SetRelease" << std::endl;
            _rc = std::min(std::max(0.0, _rc), 6.0);
            //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "eg SetRelease %f", _rc);
            Real tmp = static_cast<Real>(SynthModBase::GetSampleRate()) * _rc;
            m_release = static_cast<std::size_t>(tmp);
            if(m_release != 0)
                m_delta_phase_release = static_cast<Real>(Constants::inv_exp_table_size) / tmp;
            else
                m_delta_phase_release = 0.0;
        }
        
        inline void SetADSR(ADSR const& _adsr)
        {
            SetAttack(_adsr.attack);
            SetDecay(_adsr.decay);
            SetSustain(_adsr.sustain);
            SetRelease(_adsr.release);
        }
    
        inline void MidiReceive(sykes::midi::message _m)
        {
            switch(sykes::midi::message::status(_m))
            {
                case sykes::midi::channel_voice_message_type::NOTE_ON:
                    //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "eg noteOn");
                    {
                        auto s = m_state;
                        SetState(EGState::ATTACK);
                        if(s != EGState::OFF){
                            m_phase = static_cast<std::size_t>(static_cast<Real>(m_attack) * m_last_val);
                            m_phase = std::min(m_phase, m_attack - 1);
                        }
                        return;
                    }
                case sykes::midi::channel_voice_message_type::NOTE_OFF:
                    //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "eg noteOff");
                    m_release_level = m_last_val;
                    SetState(EGState::RELEASE);
                    return;
                case sykes::midi::CMMT::ALL_SOUND_OFF:
                    SetState(EGState::OFF);
                    return;
                default:
                    return;
            }
        }
        
        inline bool IsActive() const
        {
            return (m_state != EGState::OFF);
        }
    private:
        std::size_t m_phase;
        EGState m_state;
        std::size_t m_attack;
        std::size_t m_decay;
        Real m_sustain;
        std::size_t m_release;
        Real m_delta_phase_decay;
        Real m_delta_phase_release;
        Real m_release_level;
        Real m_attack_rate;
        Real m_last_val;
    };
}
#endif

