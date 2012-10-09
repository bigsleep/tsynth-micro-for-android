//-----------------------------------------------------------
//    
//-----------------------------------------------------------
#ifndef SYNTH_MOD_BASE_H
#define SYNTH_MOD_BASE_H

#include "type.h"
#include "midi_utility.h"
#include "constants.h"
#include "tree.h"

#include <string>
#include <memory>
#include <functional>
#include <type_traits>
#include <stdexcept>


namespace TSynth{
    extern void * enabler ;
    
    class SynthModBase;
    class SynthVCA;
    
    typedef std::shared_ptr<SynthModBase> SynthModBasePtr;
    typedef creek::tree<std::function<Real(void)>>::const_child_iterator TreeFunctionIterator;
    
    inline Real __Zero()
    { return 0.0; }
    
    namespace Detail
    {
        //-----------------------------------------------------------
        //    MidiReceive
        //-----------------------------------------------------------
        template<typename T,
            typename std::enable_if<MidiReceivable<T>::value>::type *& = enabler>
        void MidiReceive(T& mod, sykes::midi::message m)
        {
            mod.MidiReceive(m);
        }
        
        template<typename T,
            typename std::enable_if<!MidiReceivable<T>::value>::type *& = enabler>
        void MidiReceive(T& mod, sykes::midi::message m)
        {
        }
        
        //-----------------------------------------------------------
        //    GetMidiPort
        //-----------------------------------------------------------
        template<typename T,
            typename std::enable_if<MidiReceivable<T>::value>::type *& = enabler>
        std::function<void(sykes::midi::message)> GetMidiPort(T& mod)
        {
            return std::bind(&T::MidiReceive, &mod, std::placeholders::_1);
        }
        
        inline void MidiReceiveIgnore(sykes::midi::message){}
        
        template<typename T,
            typename std::enable_if<!MidiReceivable<T>::value>::type *& = enabler>
        std::function<void(sykes::midi::message)> GetMidiPort(T& mod)
        {
            return &MidiReceiveIgnore;
        }
        
        //-----------------------------------------------------------
        //    IsActive
        //-----------------------------------------------------------
        template<typename T,
            typename std::enable_if<HasIsActive<T>::value>::type *& = enabler>
        bool IsActive(T& mod)
        {
            return mod.IsActive();
        }
        
        template<typename T,
            typename std::enable_if<!HasIsActive<T>::value>::type *& = enabler>
        bool IsActive(T& mod)
        {
            return true;
        }
        
        
        //-----------------------------------------------------------
        //    GetPortIsActive
        //-----------------------------------------------------------
        template<typename T,
            typename std::enable_if<PortIsActiveAvailable<T>::value>::type *& = enabler>
        std::function<bool()> GetPortIsActive(T& mod)
        {
            return mod.GetPortIsActive();
        }
        
        inline bool AlwaysTrue(){return true;}
        
        template<typename T,
            typename std::enable_if<!PortIsActiveAvailable<T>::value>::type *& = enabler>
        std::function<bool()> GetPortIsActive(T& mod)
        {
            return &AlwaysTrue;
        }
        
        //-----------------------------------------------------------
        //    mm_GetPortDouble
        //-----------------------------------------------------------
        template<typename T,
            typename std::enable_if<PortDoubleAvailable<T>::value>::type *& = enabler>
        std::function<void(int, double)> GetPortDouble(T& mod)
        {
            return mod.GetPortDouble();
        }
        
        inline void PortDoubleIgnore(int, double){}
        
        template<typename T,
            typename std::enable_if<!PortDoubleAvailable<T>::value>::type *& = enabler>
        std::function<void(int, double)> GetPortDouble(T& mod)
        {
            return &PortDoubleIgnore;
        }
        
        //-----------------------------------------------------------
        //    mm_GetPortInt
        //-----------------------------------------------------------
        template<typename T,
            typename std::enable_if<PortIntAvailable<T>::value>::type *& = enabler>
        std::function<void(int, int)> GetPortInt(T& mod)
        {
            return mod.GetPortInt();
        }
        
        inline void PortIntIgnore(int, int){}
        
        template<typename T,
            typename std::enable_if<!PortIntAvailable<T>::value>::type *& = enabler>
        std::function<void(int, int)> GetPortInt(T& mod)
        {
            return &PortIntIgnore;
        }
    }
    
    //-----------------------------------------------------------
    //    class SynthModBase
    //-----------------------------------------------------------
    class SynthModBase
    {
    public:
        typedef Real result_type;
        
        inline SynthModType GetType() const
        { return m_type; }
        
        inline static void SetSampleRate(std::size_t _sr)
        {
            SynthModBase::SampleRate<>::value = _sr;
        }
        
        inline static std::size_t GetSampleRate()
        {
            return SynthModBase::SampleRate<>::value;
        }
        
        inline SynthModBasePtr Clone() const
        {
            return SynthModBasePtr(m_NewAsBase());
        }
        
        inline explicit SynthModBase(SynthModType _t, std::size_t _id)
            : m_type(_t), m_id(_id)
        {}
        
        inline void MidiReceive(sykes::midi::message _message)
        {
            m_MidiReceive(_message);
        }
        
        inline bool IsActive() const
        {
            return m_IsActive();
        }
        
        std::string const& Name() const
        {
            return m_Name();
        }
        
        inline std::function<Real(void)>
        MakeFunction(TreeFunctionIterator _it, TreeFunctionIterator _end)
        {
            return m_MakeFunction(_it, _end);
        }
        
        inline std::function<void(sykes::midi::message)> GetMidiPort()
        {
            return m_GetMidiPort();
        }
        
        inline std::function<void(int, double)> GetPortDouble()
        {
            return m_GetPortDouble();
        }
        
        inline std::function<void(int, int)> GetPortInt()
        {
            return m_GetPortInt();
        }
        
        inline std::function<bool()> GetPortIsActive()
        {
            return m_GetPortIsActive();
        }
        
        inline std::size_t GetId() const
        {
            return m_id;
        }
        
    private:
        SynthModType const m_type;
        std::size_t m_id;
        
        template<bool B = true>
        struct SampleRate
        {
            static std::size_t value;
        };
        
        virtual SynthModBase* m_NewAsBase() const = 0;
        
        virtual void m_MidiReceive(sykes::midi::message _message) = 0;
        
        virtual std::function<Real(void)>
        m_MakeFunction(TreeFunctionIterator _it, TreeFunctionIterator _end) = 0;
        
        virtual std::string const& m_Name() const = 0;
        
        virtual bool m_IsActive() const = 0;
        virtual std::function<void(sykes::midi::message)> m_GetMidiPort() = 0;
        virtual std::function<void(int, double)> m_GetPortDouble() = 0;
        virtual std::function<void(int, int)> m_GetPortInt() = 0;
        virtual std::function<bool()> m_GetPortIsActive() = 0;
    };
    
    template<bool B>
    std::size_t SynthModBase::template SampleRate<B>::value = Constants::default_sample_rate;
    
    //-----------------------------------------------------------
    //    class NullaryMod
    //-----------------------------------------------------------
    template<typename ModT>
    class NullaryMod : public SynthModBase
    {
    public:
        NullaryMod(std::size_t id)
            : SynthModBase(SynthModType::NULLARY, id), m_mod()
        {}
        
        template<typename ... Ts>
        NullaryMod(std::size_t id, Ts ... args)
            : SynthModBase(SynthModType::NULLARY, id), m_mod(args...)
        {}
        
    private:
        ModT m_mod;
        
        virtual std::function<Real(void)>
        m_MakeFunction(TreeFunctionIterator _it, TreeFunctionIterator _end)
        {
            return std::ref(m_mod);
        }
        
        virtual SynthModBase* m_NewAsBase() const
        {
            return new NullaryMod(*this);
        }
        
        virtual std::string const& m_Name() const
        {
            return m_mod.ModName();
        }
        
        virtual void m_MidiReceive(sykes::midi::message m)
        {
            Detail::MidiReceive(m_mod, m);
        }
        
        virtual bool m_IsActive() const
        {
            return Detail::IsActive(m_mod);
        }
        
        virtual std::function<void(sykes::midi::message)> m_GetMidiPort()
        {
            return Detail::GetMidiPort(m_mod);
        }
        
        virtual std::function<void(int, double)> m_GetPortDouble()
        {
            return Detail::GetPortDouble(m_mod);
        }
        
        virtual std::function<void(int, int)> m_GetPortInt()
        {
            return Detail::GetPortInt(m_mod);
        }
        
        virtual std::function<bool()> m_GetPortIsActive()
        {
            return Detail::GetPortIsActive(m_mod);
        }
    };
    
    //-----------------------------------------------------------
    //    class UnaryMod
    //-----------------------------------------------------------
    template<typename ModT>
    class UnaryMod : public SynthModBase
    {
    public:
        UnaryMod(std::size_t id)
            : SynthModBase(SynthModType::UNARY, id), m_mod()
        {}
        
        template<typename ... Ts>
        UnaryMod(std::size_t id, Ts ... args)
            : SynthModBase(SynthModType::UNARY, id), m_mod(args...)
        {}
        
    private:
        ModT m_mod;
        
        virtual std::function<Real(void)>
        m_MakeFunction(TreeFunctionIterator _it, TreeFunctionIterator _end)
        {
            assert(_it != _end);
            return std::bind(&ModT::operator(), std::ref(m_mod), std::bind(*_it));
        }
        
        virtual SynthModBase* m_NewAsBase() const
        {
            return new UnaryMod(*this);
        }
        
        virtual std::string const& m_Name() const
        {
            return m_mod.ModName();
        }
        
        virtual void m_MidiReceive(sykes::midi::message m)
        {
            Detail::MidiReceive(m_mod, m);
        }
        
        virtual bool m_IsActive() const
        {
            return Detail::IsActive(m_mod);
        }
        
        virtual std::function<void(sykes::midi::message)> m_GetMidiPort()
        {
            return Detail::GetMidiPort(m_mod);
        }
        
        virtual std::function<void(int, double)> m_GetPortDouble()
        {
            return Detail::GetPortDouble(m_mod);
        }
        
        virtual std::function<void(int, int)> m_GetPortInt()
        {
            return Detail::GetPortInt(m_mod);
        }
        
        virtual std::function<bool()> m_GetPortIsActive()
        {
            return Detail::GetPortIsActive(m_mod);
        }
    };
    
    //-----------------------------------------------------------
    //    class BinaryMod
    //-----------------------------------------------------------
    template<typename ModT>
    class BinaryMod : public SynthModBase
    {
    public:
        BinaryMod(std::size_t id)
            : SynthModBase(SynthModType::BINARY, id), m_mod()
        {}
        
        template<typename ... Ts>
        BinaryMod(std::size_t id, Ts ... args)
            : SynthModBase(SynthModType::BINARY, id), m_mod(args...)
        {}
        
    private:
        ModT m_mod;
        
        std::function<Real(void)>
        m_MakeFunction(TreeFunctionIterator _it, TreeFunctionIterator _end)
        {
            std::function<Real(void)> tmp;
            if(_it != _end){
                tmp = (*_it);
                ++_it;
            }else{
                tmp = &__Zero;
            }
            
            while(_it != _end){
                assert(bool(*_it));
                tmp = std::bind(
                    &ModT::operator(),
                    std::ref(m_mod),
                    std::bind(*_it),
                    std::bind(tmp)
                );
                ++_it;
            }
            return tmp;
        }
        
        virtual SynthModBase* m_NewAsBase() const
        {
            return new BinaryMod(*this);
        }
        
        virtual std::string const& m_Name() const
        {
            return m_mod.ModName();
        }
        
        virtual void m_MidiReceive(sykes::midi::message m)
        {
            Detail::MidiReceive(m_mod, m);
        }
        
        virtual bool m_IsActive() const
        {
            return Detail::IsActive(m_mod);
        }
        
        virtual std::function<void(sykes::midi::message)> m_GetMidiPort()
        {
            return Detail::GetMidiPort(m_mod);
        }
        
        virtual std::function<void(int, double)> m_GetPortDouble()
        {
            return Detail::GetPortDouble(m_mod);
        }
        
        virtual std::function<void(int, int)> m_GetPortInt()
        {
            return Detail::GetPortInt(m_mod);
        }
        
        virtual std::function<bool()> m_GetPortIsActive()
        {
            return Detail::GetPortIsActive(m_mod);
        }
    };
}//---- namespace
#endif

