//-----------------------------------------------------------
//    MonoSynth
//-----------------------------------------------------------
#ifndef MONO_SYNTH_H
#define MONO_SYNTH_H

#include "type.h"
#include "tree.h"
#include "midi_utility.h"
#include "synth_mod_base.h"
#include <map>

namespace TSynth{
    class SynthModBase;
    //-----------------------------------------------------------
    //    class MonoSynth
    //-----------------------------------------------------------
    class MonoSynth
    {
    public:
        typedef std::shared_ptr<SynthModBase> SynthModBasePtr;
        typedef creek::tree<SynthModBasePtr>::preorder_iterator Iterator;
        typedef creek::tree<SynthModBasePtr>::const_preorder_iterator ConstIterator;
        
        MonoSynth();
        ~MonoSynth();
        
        Real operator()() const;
        
        void MidiReceive(sykes::midi::message _m);
        
        Iterator Insert(Iterator _it, SynthModBasePtr _mod);
        
        Iterator AppendChild(Iterator _it, SynthModBasePtr _mod);
        
        inline Iterator Begin()
        { return m_mod_tree.preorder_begin(); }
        
        inline Iterator End()
        { return m_mod_tree.preorder_end(); }
        
        inline ConstIterator Begin() const
        { return m_mod_tree.preorder_begin(); }
        
        inline ConstIterator End() const
        { return m_mod_tree.preorder_end(); }
        
        bool CheckTree() const;
        
        inline void Clear();
                
        void Swap(MonoSynth& _other);
        
        bool IsActive() const;
        
    private:
        creek::tree<SynthModBasePtr> m_mod_tree;
        std::map<std::size_t, SynthModBasePtr> m_mod_map;
        std::function<Real()> m_function;
        SynthModBasePtr m_root_vca;
        std::function<bool()> m_is_active;
        
        
    public:
        void BindModsInTree();
        //void ComposeMonoSynthFromString(std::string const& _str);
        MonoSynth Clone() const;
    };
}//---- namespace

#endif

