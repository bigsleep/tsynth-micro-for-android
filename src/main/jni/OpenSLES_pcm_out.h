#ifndef sykes_OpenSLES_pcm_out_h
#define sykes_OpenSLES_pcm_out_h

#include <vector>
#include <cstddef>
#include <functional>
#include <assert.h>
#include <limits>

#include <android/log.h>
#include <unistd.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "mutex.h"

namespace sykes
{
    class OpenSLES_pcm_out
    {
    public:
        typedef SLint16 format_type;
        typedef OpenSLES_pcm_out self_type;
        static SLuint32 const sampling_rate = SL_SAMPLINGRATE_32 / 1000;
        static std::size_t const queue_max_size = 8;
        
        inline OpenSLES_pcm_out() :
            m_mutex(),
            m_engine(),
            m_output_mix(),
            m_player(),
            m_player_interface(),
            m_bufferqueue_interface(),
            m_volume_interface(),
            m_callback_function(),
            m_queue(),
            m_stop_flag(false)
        {
            SLresult r;
            // create engine
            r = slCreateEngine(&m_engine, 0, NULL, 0, NULL, NULL);
            __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out create engine : %d", r);
            assert(r == SL_RESULT_SUCCESS);
            
            // realize engine
            r = (*m_engine)->Realize(m_engine, SL_BOOLEAN_FALSE);
            __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out realize engine : %d", r);
            assert(r == SL_RESULT_SUCCESS);
            
            // get engine interface
            SLEngineItf engine_interface;
            r = (*m_engine)->GetInterface(m_engine, SL_IID_ENGINE, static_cast<void*>(&engine_interface));
            __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out get engine interface : %d", r);
            assert(r == SL_RESULT_SUCCESS);
            
            // create output mix
            r = (*engine_interface)->CreateOutputMix(engine_interface, &m_output_mix, 0, 0, 0);
            __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out create output mix : %d", r);
            assert(r == SL_RESULT_SUCCESS);
            
            // realize output mix
            r = (*m_output_mix)->Realize(m_output_mix, SL_BOOLEAN_FALSE);
            __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out realize output mix : %d", r);
            assert(r == SL_RESULT_SUCCESS);
            
            // create player
            SLDataLocator_AndroidSimpleBufferQueue datalocator = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, queue_max_size};
            SLDataFormat_PCM dataformat_pcm = {SL_DATAFORMAT_PCM, 1, sampling_rate * 1000,
                SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
            SLDataSource datasource = {&datalocator, &dataformat_pcm};
            SLDataLocator_OutputMix datalocator_outputmix = {SL_DATALOCATOR_OUTPUTMIX, m_output_mix};
            SLDataSink datasink = {&datalocator_outputmix, NULL};
            
            SLInterfaceID id2[2] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_VOLUME};
            SLboolean req2[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_FALSE};
            r = (*engine_interface)->CreateAudioPlayer(engine_interface, &m_player, &datasource, &datasink, 1, &id2[0], &req2[0]);
            __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out create player : %d", r);
            assert(r == SL_RESULT_SUCCESS);
            
            // realize player
            r = (*m_player)->Realize(m_player, SL_BOOLEAN_FALSE);
            __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out realize player : %d", r);
            assert(r == SL_RESULT_SUCCESS);
            
            r = (*m_player)->GetInterface(m_player, SL_IID_PLAY, &m_player_interface);
            __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out get player interface : %d", r);
            assert(r == SL_RESULT_SUCCESS);
            
            // get bufferqueue interface
            r = (*m_player)->GetInterface(m_player, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, static_cast<void*>(&m_bufferqueue_interface));
            __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out get bufferqueue interface : %d", r);
            assert(r == SL_RESULT_SUCCESS);
            
            // register call back
            r = (*m_bufferqueue_interface)->RegisterCallback(m_bufferqueue_interface, &m_callback, this);
            __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out register callback : %d", r);
            assert(r == SL_RESULT_SUCCESS);
            
            /*
            // get volume interface
            r = (*m_player)->GetInterface(m_player, SL_IID_VOLUME, &m_volume_interface);
            assert(r == SL_RESULT_SUCCESS);
            
            SLEnvironmentalReverbItf reverb;
            SLEnvironmentalReverbSettings reverb_setting = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
            r = (*m_output_mix)->GetInterface(m_output_mix, SL_IID_ENVIRONMENTALREVERB, &reverb);
            r = (*reverb)->SetEnvironmentalReverbProperties(reverb, &reverb_setting);
            */
        }
        
        inline ~OpenSLES_pcm_out()
        {
            stop();
            if(m_player != 0)
                (*m_player)->Destroy(m_player);
            
            if(m_output_mix != 0)
                (*m_output_mix)->Destroy(m_output_mix);
            
            if(m_engine != 0)
                (*m_engine)->Destroy(m_engine);
        }
        
        inline bool is_playing() const
        {
            mutex::lock_type lock(m_mutex);
            SLuint32 state;
            SLresult r = (*m_player_interface)->GetPlayState(m_player_interface, &state);
            assert(r == SL_RESULT_SUCCESS);
            return (state == SL_PLAYSTATE_PLAYING);
        }
        
        inline bool is_stopped() const
        {
            mutex::lock_type lock(m_mutex);
            SLuint32 state;
            SLresult r = (*m_player_interface)->GetPlayState(m_player_interface, &state);
            assert(r == SL_RESULT_SUCCESS);
            return (state == SL_PLAYSTATE_STOPPED);
        }
        
        inline void start()
        {
            if(is_playing()) return;
            mutex::lock_type lock(m_mutex);
            
            SLresult r = (*m_player_interface)->SetPlayState(m_player_interface, SL_PLAYSTATE_PLAYING);
            __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out start : %d", r);
            assert(r == SL_RESULT_SUCCESS);
        }
        
        inline void pause()
        {
            mutex::lock_type lock(m_mutex);
            
            SLresult r = (*m_player_interface)->SetPlayState(m_player_interface, SL_PLAYSTATE_PAUSED);
            __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out stop : %d", r);
            r = (*m_bufferqueue_interface)->Clear(m_bufferqueue_interface);
            __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out clear queue : %d", r);
            assert(r == SL_RESULT_SUCCESS);
            m_queue.clear();
        }
        
        inline void stop()
        {
            {
                mutex::lock_type lock(m_mutex);
                m_stop_flag = true;
                SLresult r = (*m_bufferqueue_interface)->Clear(m_bufferqueue_interface);
            }
            usleep(200000);
            {
                mutex::lock_type lock(m_mutex);
                SLresult r = (*m_player_interface)->SetPlayState(m_player_interface, SL_PLAYSTATE_STOPPED);
                __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out stop : %d", r);
                __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out clear queue : %d", r);
                assert(r == SL_RESULT_SUCCESS);
                m_queue.clear();
                m_stop_flag = false;
            }
        }
        
        inline void push(std::vector<format_type>&& _b)
        {
            mutex::lock_type lock(m_mutex);
            if(!m_stop_flag){
                // check number of buffers in the queue
                SLAndroidSimpleBufferQueueState qstate;
                SLresult r = (*m_bufferqueue_interface)->GetState(
                    m_bufferqueue_interface, &qstate);
                assert(r == SL_RESULT_SUCCESS);
                
                if(qstate.count + 1 <= queue_max_size){
                    m_queue.push_back(std::forward<std::vector<format_type>&&>(_b));
                    r = (*m_bufferqueue_interface)->Enqueue(
                        m_bufferqueue_interface,
                        m_queue.back().data(),
                        m_queue.back().size() * sizeof(format_type) / sizeof(char));
                    assert(r == SL_RESULT_SUCCESS);
                }else{
                    __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out::push : too many buffers in the queue");
                }
            }
        }
        
        template<typename F>
        inline void set_callback(F _f)
        {
            mutex::lock_type lock(m_mutex);
            m_callback_function = _f;
        }
        
    private:
        mutex mutable m_mutex;
        SLObjectItf m_engine;
        SLObjectItf m_output_mix;
        SLObjectItf m_player;
        SLPlayItf m_player_interface;
        SLAndroidSimpleBufferQueueItf m_bufferqueue_interface;
        SLVolumeItf m_volume_interface;
        std::function<void()> m_callback_function;
        std::vector<std::vector<format_type>> m_queue;
        bool m_stop_flag;
        
        inline static void m_callback(SLAndroidSimpleBufferQueueItf _bq, void* _context)
        {
            //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "pcm_out on callback thread id %d", gettid());
            self_type& self = *(static_cast<self_type*>(_context));
            mutex::lock_type lock(self.m_mutex);
            std::function<void()> cb = self.m_callback_function;
            lock.unlock();
            if(cb) cb();
            
            {
                mutex::lock_type lock(self.m_mutex);
                if(!self.m_queue.empty()) self.m_queue.erase(self.m_queue.begin());
            }
        }
        
        OpenSLES_pcm_out(OpenSLES_pcm_out const&) = delete;
        OpenSLES_pcm_out& operator=(OpenSLES_pcm_out const&) = delete;
    };
}
#endif


















