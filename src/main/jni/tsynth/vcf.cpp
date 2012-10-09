//-----------------------------------------------------------
//    vcf.cpp
//-----------------------------------------------------------
#include "constants.h"
#include "synth_mod.h"
#include "eg.h"
//#include <iostream>
#include <cmath>
#include <numeric>
#include <array>
#include <android/log.h>


namespace TSynth{

    //-----------------------------------------------------------
    //    class SynthVCF
    //-----------------------------------------------------------
    class SynthVCF
    {
    public:
        typedef Real result_type;
        static std::size_t const filter_order = 4;
        
        SynthVCF();
        SynthVCF(Real _a, Real _d, Real _s, Real _r);
        
        Real operator()(Real _in);
        
        Real GetLastVal() const
        { return m_last_val; }
        
        void ResetCBuffers()
        {
            m_c_buffer_input.fill(0.0);
            m_c_buffer_output.fill(0.0);
        }
        
        void SetCutOffFrequency(Real _f)
        { m_cutoff = _f; }
        
        void MidiReceive(sykes::midi::message _m)
        {
            //std::cout << "SynthVCF::MidiReceive" << std::endl;
            m_cutoff_function.MidiReceive(_m);
            switch(sykes::midi::message::status(_m))
            {
                case sykes::midi::channel_voice_message_type::NOTE_ON:
                {
                    UInt8 note = sykes::midi::message::data1(_m);
                    //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "vcf noteOn note: %d", note);
                    if(note < sykes::midi::note_table_size)
                        SetCutOffFrequency(sykes::midi::note_table[note] * 2.0);
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
                    m_cutoff_function.SetAttack(value);
                    break;
                case 2:
                    m_cutoff_function.SetDecay(value);
                    break;
                case 3:
                    m_cutoff_function.SetSustain(value);
                    break;
                case 4:
                    m_cutoff_function.SetRelease(value);
                    break;
                default:
                    break;
            }
        }
        
        std::function<void(int, double)> GetPortDouble()
        {
            return std::bind(&SynthVCF::SetParameterDouble, this, std::placeholders::_1, std::placeholders::_2);
        }
        
    private:
        Real m_cutoff;
        Real m_last_val;
        std::array<Real,filter_order> m_c_buffer_input;
        std::array<Real, filter_order> m_c_buffer_output;
        SynthEG m_cutoff_function;
        
        static void GetFilterConstants(Real _f, std::array<Real, filter_order + 1>& _a, std::array<Real, filter_order>& _b);
        static std::tuple<std::array<Real, filter_order + 1> const&, std::array<Real, filter_order> const&>
        GetFilterConstants(Real _f);
        
        static void CalcButterWorthConstants(Real _f, std::array<Real, filter_order + 1>& _a, std::array<Real, filter_order>& _b);
        static void SetFilterTable();
        static bool initialized_filter_table;
        static std::vector<std::array<Real, filter_order + 1>> butterworth4_ai;
        static std::vector<std::array<Real, filter_order>> butterworth4_bi;
        
        TSYNTH_USE_AS_MOD
    };
    
    TSYNTH_DECLARE_UNARY_MOD(SynthVCF, (StoI(), StoD(), StoD(), StoD(), StoD()))
    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    bool SynthVCF::initialized_filter_table = false;
    std::vector<std::array<Real, SynthVCF::filter_order + 1>> SynthVCF::butterworth4_ai;
    std::vector<std::array<Real, SynthVCF::filter_order>> SynthVCF::butterworth4_bi;
    Real const delta_freq =
        (Real(Constants::vcf_max_cutoff - Constants::vcf_min_cutoff)
        / Real(Constants::vcf_filter_table_size));
    Real const inv_delta_freq = 1.0 / delta_freq;
    
    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    SynthVCF::SynthVCF()
        : m_cutoff(Constants::vcf_default_cutoff),
        m_last_val(),
        m_c_buffer_input(std::array<Real, filter_order>{{0.0}}),
        m_c_buffer_output(std::array<Real, filter_order>{{0.0}}),
        m_cutoff_function()
    {
        if(!initialized_filter_table){
            initialized_filter_table = false;
            SetFilterTable();
        }
    }

    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    SynthVCF::SynthVCF(Real _a, Real _d, Real _s, Real _r)
        : m_cutoff(Constants::vcf_default_cutoff),
        m_last_val(),
        m_c_buffer_input(std::array<Real, filter_order>{{0.0}}),
        m_c_buffer_output(std::array<Real, filter_order>{{0.0}}),
        m_cutoff_function(_a, _d, _s, _r)
    {
        if(!initialized_filter_table){
            initialized_filter_table = false;
            SetFilterTable();
        }
    }
    
    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    Real SynthVCF::operator()(Real _in)
    {
        Real x = 0.2;
        Real cutoff = m_cutoff * (1.0 - x + x * m_cutoff_function());
        
        std::array<Real, filter_order + 1> itmp = {{0.0}};
        std::array<Real, filter_order> otmp = m_c_buffer_output;
        
        std::copy(m_c_buffer_input.begin(),
            m_c_buffer_input.end(), itmp.begin());
        
        itmp.back() = _in;
        
        auto filt_consts = GetFilterConstants(cutoff);
        auto const& a = std::get<0>(filt_consts);
        auto const& b = std::get<1>(filt_consts);
        
        Real tmp = std::inner_product(m_c_buffer_input.begin(), m_c_buffer_input.end(), a.begin(), Real(0.0));
        tmp -= std::inner_product(otmp.begin(), otmp.end(), b.begin(), Real(0.0));
        
        std::copy(itmp.begin() + 1, itmp.end(), m_c_buffer_input.begin());
        
        std::copy_backward(m_c_buffer_output.begin() + 1,
            m_c_buffer_output.end(),
            m_c_buffer_output.end() - 1);
        
        m_c_buffer_output.back() = tmp;
        
        m_last_val = tmp;
        return tmp;
    }

    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    void SynthVCF::GetFilterConstants(Real _f, std::array<Real, filter_order + 1>& _a, std::array<Real, filter_order>& _b)
    {
        std::size_t n1 = 0;
        
        if(_f < Real(Constants::vcf_min_cutoff))
            n1 = 0;
        else if(_f > Real(Constants::vcf_max_cutoff))
            n1 = Constants::vcf_filter_table_size - 1;
        else{
            Real c1 = (_f - Real(Constants::vcf_min_cutoff)) * inv_delta_freq;
            n1 = std::size_t(c1);
        }
        
        _a = butterworth4_ai[n1];
        _b = butterworth4_bi[n1];
    }

    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    std::tuple<std::array<Real, SynthVCF::filter_order + 1> const&, std::array<Real, SynthVCF::filter_order> const&>
    SynthVCF::GetFilterConstants(Real _f)
    {
        std::size_t n1 = 0;
        
        if(_f < Real(Constants::vcf_min_cutoff))
            n1 = 0;
        else if(_f > Real(Constants::vcf_max_cutoff))
            n1 = Constants::vcf_filter_table_size - 1;
        else{
            Real c1 = (_f - Real(Constants::vcf_min_cutoff)) * inv_delta_freq;
            n1 = std::size_t(c1);
        }
        
        return std::tuple<std::array<Real, SynthVCF::filter_order + 1> const&, std::array<Real, SynthVCF::filter_order> const&>
            (butterworth4_ai[n1], butterworth4_bi[n1]);
    }

    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    void SynthVCF::SetFilterTable()
    {
        butterworth4_ai.reserve(Constants::vcf_filter_table_size);
        butterworth4_bi.reserve(Constants::vcf_filter_table_size);
        
        Real freq = Real(Constants::vcf_min_cutoff);
        for(std::size_t i1 = 0; i1 < Constants::vcf_filter_table_size; i1++){
            std::array<Real, filter_order + 1> atmp{{0.0}};
            std::array<Real, filter_order> btmp{{0.0}};
            
            CalcButterWorthConstants(freq, atmp, btmp);
            
            butterworth4_ai.push_back(atmp);
            butterworth4_bi.push_back(btmp);
            freq += delta_freq;
        }
    }

    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    void SynthVCF::CalcButterWorthConstants(Real _f, std::array<Real, filter_order + 1>& _a, std::array<Real, filter_order>& _b)
    {
        Real pi = std::atan2(1.0, 1.0) * 4.0;
        Real srate = (Real)SynthModBase::GetSampleRate();
        Real wd = 2.0 * srate * tan(_f * pi / srate);
        Real dt = wd / srate * 0.5;
        Real dt2 = dt * dt;
        Real dt3 = dt2 * dt;
        Real dt4 = dt2 * dt2;
        Real ci[3] = {2.613126, 3.4142136, 2.613126};
        
        Real b0 = 1.0 / (dt4 + dt3 * ci[2]
            + dt2 * ci[1] + dt * ci[0] + 1.0);
        
        _b[0] = (dt4 - dt3 * ci[2]
            + dt2 * ci[1] - dt * ci[0] + 1.0) * b0;
        _b[1] = (4.0 * dt4 - 2.0 * dt3 * ci[2]
            + 2.0 * dt * ci[0] - 4.0) * b0;
        _b[2] = (6.0 * dt4 - 2.0 * dt2 * ci[1] + 6.0) * b0;
        _b[3] = (4.0 * dt4 + 2.0 * dt3 * ci[2]
            - 2.0 * dt * ci[0] - 4.0) * b0;
        
        Real a0 = dt4 * b0;
        
        _a[0] = a0;
        _a[1] = 4.0 * a0;
        _a[2] = 6.0 * a0;
        _a[3] = 4.0 * a0;
        _a[4] = a0;
    }
}//---- namespace



