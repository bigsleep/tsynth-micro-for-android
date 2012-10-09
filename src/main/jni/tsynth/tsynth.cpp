//-----------------------------------------------------------
//    
//-----------------------------------------------------------
#include "tsynth.h"
#include "mod_factory.h"
//#include <iostream>
#include <android/log.h>

namespace TSynth{
    TSynth::TSynth() :
        m_synth(),
        m_note_number(),
        m_parameter_dispatcher(),
        m_parameter_dispatcher_int(),
        m_midi_dispatcher()
    {
        Init();
    }
    
    void TSynth::Init()
    {
        m_note_number.fill(-1);
        __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "TSynth::Init");
        auto vco1 = SynthModFactory::Create("SynthVCO 101 SIN 0.02 0.0 1.0 0.02");
        auto vco2 = SynthModFactory::Create("SynthVCO 102 SIN 0.02 0.0 1.0 0.02");
        auto mixer = SynthModFactory::Create("Mixer 103");
        auto vcf = SynthModFactory::Create("SynthVCF 104 0.02 0.0 1.0 0.02");
        auto vca = SynthModFactory::Create("SynthVCA 105 0.1 0.02 0.0 1.0 0.02");
        //std::cout << "create mod" << std::endl;
        
        MonoSynth synth;
        auto a = synth.Insert(synth.Begin(), vca);
        auto b = synth.AppendChild(a, vcf);
        auto c = synth.AppendChild(b, mixer);
        auto d = synth.AppendChild(c, vco1);
        auto e = synth.AppendChild(c, vco2);
        synth.BindModsInTree();
        //std::cout << "bind mods" << std::endl;
        
        for(std::size_t i = 0; i < Poly; ++i){
            auto clone = synth.Clone();
            //std::cout << "clone" << std::endl;
            m_synth[i].Swap(clone);
            //std::cout << "swap" << std::endl;
            auto it = m_synth[i].Begin();
            auto end = m_synth[i].End();
            //std::cout << "begin end" << std::endl;
            while(it != end){
                auto id = (**it).GetId();
                m_parameter_dispatcher.connect(id, (**it).GetPortDouble());
                m_parameter_dispatcher_int.connect(id, (**it).GetPortInt());
                //std::cout << "connect port" << std::endl;
                m_midi_dispatcher.connect(i, (**it).GetMidiPort());
                //std::cout << "connect midi port" << std::endl;
                ++it;
            }
            //std::cout << "connect" << std::endl;
        }
        //std::cout << "poly" << std::endl;
        __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "TSynth::Init end");
    }
    
    Real TSynth::operator()()
    {
        Real a = 0.0;
        for(std::size_t i = 0; i < Poly; ++i){
            if(m_note_number[i] != -1) a += m_synth[i]();
        }
        return a;
    }
    
    void TSynth::MidiReceive(sykes::midi::message m)
    {
        for(std::size_t i = 0; i < Poly; ++i){
            if(!m_synth[i].IsActive()){
                m_note_number[i] = -1;
            }
        }
        int note = (int)sykes::midi::message::data1(m);
        switch(sykes::midi::message::status(m))
        {
            case sykes::midi::channel_voice_message_type::NOTE_ON:
            {
                //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "TSynth::MidiReceive noteon %d", note);
                for(std::size_t i = 0; i < Poly; ++i){
                    if(m_note_number[i] == note){
                        //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "TSynth::MidiReceive noteon poly %d", i);
                        m_synth[i].MidiReceive(m);
                        return;
                    }
                }
                for(std::size_t i = 0; i < Poly; ++i){
                    if(m_note_number[i] < 0){
                        //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "TSynth::MidiReceive noteon poly %d", i);
                        m_note_number[i] = note;
                        m_synth[i].MidiReceive(m);
                        return;
                    }
                }
                return;
            }
            case sykes::midi::channel_voice_message_type::NOTE_OFF:
                //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "TSynth::MidiReceive noteoff %d", note);
                for(std::size_t i = 0; i < Poly; ++i){
                    if(m_note_number[i] == note){
                        //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "TSynth::MidiReceive noteoff poly %d", i);
                        m_synth[i].MidiReceive(m);
                    }
                }
                return;
            default:
                return;
        }
    }
    
    void TSynth::SetParameterDouble(int mod, int port, Real value)
    {
        if(m_parameter_dispatcher.count(mod))
            m_parameter_dispatcher(mod, port, value);
    }
    
    void TSynth::SetParameterInt(int mod, int port, int value)
    {
        if(m_parameter_dispatcher_int.count(mod))
            m_parameter_dispatcher_int(mod, port, value);
    }
    
    void TSynth::AllSoundOff()
    {
        for(std::size_t i = 0; i < Poly; ++i){
            m_synth[i].MidiReceive(sykes::midi::make_message(sykes::midi::CMMT::ALL_SOUND_OFF, 0, 0));
        }
    }
}

