//-----------------------------------------------------------
//    
//-----------------------------------------------------------
#ifndef SYNTH_TSYNTH_H
#define SYNTH_TSYNTH_H
#include <array>
#include <cstddef>

#include "type.h"
#include "midi_utility.h"
#include "port_dispatcher.h"
#include "mono_synth.h"
#include "midi_utility.h"

namespace TSynth{
    class TSynth
    {
    public:
        typedef Real value_type;
        static std::size_t const Poly = 6;
        static std::size_t const VCO1 = 101;
        static std::size_t const VCO2 = 102;
        static std::size_t const MIXER = 103;
        static std::size_t const VCF = 104;
        static std::size_t const VCA = 105;
        
        TSynth();
        
        Real operator()();
        
        void MidiReceive(sykes::midi::message m);
        
        void SetParameterDouble(int mod, int port, double value);
        
        void SetParameterInt(int mod, int port, int value);
        
        void AllSoundOff();
        
    private:
        std::array<MonoSynth, Poly> m_synth;
        std::array<int, Poly> m_note_number;
        sykes::port_dispatcher<int, void(int, double)> m_parameter_dispatcher;
        sykes::port_dispatcher<int, void(int, int)> m_parameter_dispatcher_int;
        sykes::port_dispatcher<int, void(sykes::midi::message)> m_midi_dispatcher;
        void Init();
    };
}
#endif
