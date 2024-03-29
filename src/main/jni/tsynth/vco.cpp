//-----------------------------------------------------------
//    vco.cpp
//-----------------------------------------------------------

//#include <iostream>
#include <cmath>
#include <android/log.h>
#include "constants.h"
#include "type.h"
#include "synth_mod.h"
#include "eg.h"

namespace TSynth{
    //-----------------------------------------------------------
    //    enum class WaveType
    //-----------------------------------------------------------
    inline WaveType StringToWaveType(std::string const& _str)
    {
        if(_str == "SIN") return WaveType::SIN;
        if(_str == "SAW") return WaveType::SAW;
        if(_str == "TRI") return WaveType::TRI;
        if(_str == "SQU") return WaveType::SQU;
        return WaveType::SIN;
    }
    
    //-----------------------------------------------------------
    //    class SynthVCO
    //-----------------------------------------------------------
    class SynthVCO
    {
    public:
	    typedef Real result_type;
        SynthVCO();
        SynthVCO(WaveType _w, Real _a, Real _d, Real _s, Real _r);
        
        Real operator()();
        
        void SetFrequency(Real _f);
        
        Real GetLastVal() const
        { return m_last_val; }
        
        WaveType GetWaveType() const
        { return m_wtype; }
        
        void SetWaveType(WaveType _wt)
        { m_wtype = _wt; }
        
        void ResetPhase()
        { m_phase_position = 0.0; }
        
        static std::vector<Real> const& GetWaveTable(WaveType _wp)
        {
            if(!initialized_wave_table){
                initialized_wave_table = false;
                InitializeWaveTable();
            }
            
            switch(_wp){
                case WaveType::SIN:
                    return sin_wave;
                case WaveType::TRI:
                    return tri_wave;
                case WaveType::SAW:
                    return saw_wave;
                case WaveType::SQU:
                    return squ_wave;
                default:
                    return sin_wave;
            }
        }
        
        void MidiReceive(sykes::midi::message _m)
        {
            //std::cout << "SynthVCO::MidiReceive" << std::endl;
            m_freq_function.MidiReceive(_m);
            switch(sykes::midi::message::status(_m))
            {
                case sykes::midi::channel_voice_message_type::NOTE_ON:
                {
                    UInt8 note = sykes::midi::message::data1(_m);
                    //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "vco noteOn note: %d", note);
                    if(note < sykes::midi::note_table_size)
                        SetFrequency(sykes::midi::note_table[note]);
                    return;
                }
                default:
                    return;
            }
        }
        
        void SetParameterDouble(int port, double value)
        {
            switch(port){
                case 1:
                    m_freq_function.SetAttack(value);
                    break;
                case 2:
                    m_freq_function.SetDecay(value);
                    break;
                case 3:
                    m_freq_function.SetSustain(value);
                    break;
                case 4:
                    m_freq_function.SetRelease(value);
                    break;
                default:
                    break;
            }
        }
        
        void SetParameterInt(int port, int value)
        {
            if(port == 1){
                switch(value){
                    case 1:
                        SetWaveType(WaveType::SIN);
                        break;
                    case 2:
                        SetWaveType(WaveType::SAW);
                        break;
                    case 3:
                        SetWaveType(WaveType::TRI);
                        break;
                    case 4:
                        SetWaveType(WaveType::SQU);
                        break;
                    default:
                        break;
                }
            }
        }
        
        std::function<void(int, double)> GetPortDouble()
        {
            return std::bind(&SynthVCO::SetParameterDouble, this, std::placeholders::_1, std::placeholders::_2);
        }
        
        std::function<void(int, int)> GetPortInt()
        {
            return std::bind(&SynthVCO::SetParameterInt, this, std::placeholders::_1, std::placeholders::_2);
        }
        
    private:
        WaveType m_wtype;
        Real m_frequency;
        Real m_last_val;
        Real m_phase_position;
        Real m_delta_phase;
        SynthEG m_freq_function;
        
        static bool initialized_wave_table;
        static std::vector<Real> sin_wave;
        static std::vector<Real> tri_wave;
        static std::vector<Real> saw_wave;
        static std::vector<Real> squ_wave;
        
        static void InitializeWaveTable();
        
        TSYNTH_USE_AS_MOD
    };
    
    TSYNTH_DECLARE_NULLARY_MOD(SynthVCO, (StoI(), &StringToWaveType, StoD(), StoD(), StoD(), StoD()))
    
    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    bool SynthVCO::initialized_wave_table = false;
    std::vector<Real> SynthVCO::sin_wave;
    std::vector<Real> SynthVCO::tri_wave;
    std::vector<Real> SynthVCO::saw_wave;
    std::vector<Real> SynthVCO::squ_wave;
    Real const min_delta_phase = Real(Constants::vco_wave_table_size)
            * Constants::vco_min_frequency / Real(Constants::default_sample_rate);

    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    SynthVCO::SynthVCO()
        : m_wtype(Constants::vco_default_wave_type),
        m_frequency(Real(Constants::vco_default_frequency)),
        m_last_val(),
        m_phase_position(),
        m_delta_phase((Real(Constants::vco_wave_table_size) * m_frequency / (Real)SynthModBase::GetSampleRate())),
        m_freq_function()
    {
        if(!initialized_wave_table){
            initialized_wave_table = true;
            InitializeWaveTable();
        }
    }

    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    SynthVCO::SynthVCO(WaveType _w, Real _a, Real _d, Real _s, Real _r)
        : m_wtype(_w),
        m_frequency(Real(Constants::vco_default_frequency)),
        m_last_val(),
        m_phase_position(),
        m_delta_phase((Real(Constants::vco_wave_table_size) * m_frequency / (Real)SynthModBase::GetSampleRate())),
        m_freq_function(_a, _d, _s, _r)
    {
        if(!initialized_wave_table){
            initialized_wave_table = true;
            InitializeWaveTable();
        }
    }

    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    void SynthVCO::SetFrequency(Real _f)
    {
        if(_f < Constants::vco_min_frequency)
            m_frequency = Constants::vco_min_frequency;
        else if(_f > Constants::vco_max_frequency)
            m_frequency = Constants::vco_max_frequency;
        else
            m_frequency = _f;
        m_delta_phase = Real(Constants::vco_wave_table_size)
            * m_frequency / Real(SynthModBase::GetSampleRate());
    }
    
    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    Real SynthVCO::operator()()
    {
        auto const& wave = GetWaveTable(m_wtype);
        Real x = 0.2;
        {
            Real tmp = m_delta_phase * (1.0 - x + x * m_freq_function());
            if(tmp >= min_delta_phase)
                m_phase_position += tmp;
            else
                m_phase_position += min_delta_phase;
        }
        
        if(m_phase_position >= Real(Constants::vco_wave_table_size))
            m_phase_position -= Real(Constants::vco_wave_table_size);
        
        m_last_val = wave[static_cast<std::size_t>(m_phase_position)];
        
        return m_last_val;
    }
    
    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    void SynthVCO::InitializeWaveTable()
    {
        Real pi = std::atan2(1.0, 1.0) * 4.0;
        // sin
        {
            sin_wave.clear();
            sin_wave.reserve(Constants::vco_wave_table_size);
            Real c1 = 2.0 * pi / Real(Constants::vco_wave_table_size);
            for(std::size_t i1 = 0; i1 < Constants::vco_wave_table_size; i1++)
                sin_wave.push_back(sin(c1 * Real(i1)));
        }
        // tri
        {
            tri_wave.clear();
            tri_wave.reserve(Constants::vco_wave_table_size);
            Real c1 = 4.0 / Real(Constants::vco_wave_table_size);
            std::size_t mid = Constants::vco_wave_table_size / 2;
            for(std::size_t i1 = 0; i1 < Constants::vco_wave_table_size; i1++){
                if(i1 < mid)
                    tri_wave.push_back(- 1.0 + c1 * Real(i1));
                else
                    tri_wave.push_back(1.0 - c1 * Real(i1 - mid));
            }
        }
        // saw
        {
            saw_wave.clear();
            saw_wave.reserve(Constants::vco_wave_table_size);
            Real c1 = 2.0 / Real(Constants::vco_wave_table_size);
            for(std::size_t i1 = 0; i1 < Constants::vco_wave_table_size; i1++)
                saw_wave.push_back(- 1.0 + c1 * Real(i1));
        }
        // squ
        {
            squ_wave.clear();
            squ_wave.reserve(Constants::vco_wave_table_size);
            std::size_t mid = Constants::vco_wave_table_size / 2;
            for(std::size_t i1 = 0; i1 < Constants::vco_wave_table_size; i1++){
                if(i1 < mid)
                    squ_wave[i1] = 1.0;
                else
                    squ_wave[i1] = -1.0;
                
            }
        }
    }


}//---- namespace

