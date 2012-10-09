#include <cmath>
#include <limits>
#include <cstddef>
#include <android/log.h>
#include "OpenSLES_pcm_out.h"
#include "glue.h"
#include "mutex.h"
#include "mono_synth.h"
#include "mod_factory.h"
#include "tsynth.h"

namespace TSynth{

std::size_t const buffer_size = 1280;
static sykes::mutex synth_mutex;

sykes::OpenSLES_pcm_out& GetPcmOut()
{
    static sykes::OpenSLES_pcm_out pcm_out;
    return pcm_out;
}

TSynth& GetTSynth()
{
    static TSynth tsynth;
    return tsynth;
}

SLint16 Format(double a)
{
    static double const maxv = static_cast<double>(std::numeric_limits<SLint16>::max());
    return static_cast<SLint16>(a * maxv);
}

void Callback()
{
    typedef sykes::OpenSLES_pcm_out::format_type format;
    static auto& pcm = GetPcmOut();
    static auto& synth = GetTSynth();
    
    std::vector<format> b(buffer_size, 0);
    {
        sykes::mutex::lock_type lock(synth_mutex);
        for(std::size_t i = 0; i < buffer_size; ++i){
            b[i] = Format(synth());
        }
    }
    pcm.push(std::move(b));
}

void PushEmptyBuffer()
{
    static auto& pcm = GetPcmOut();
    typedef sykes::OpenSLES_pcm_out::format_type format;
    std::vector<format> b(buffer_size, 0);
    pcm.push(std::move(b));
}
}


JNIEXPORT void JNICALL Java_org_bigsleep_tsynth_1micro_Glue_midiReceive
  (JNIEnv *, jobject, jint status, jint data1, jint data2)
{
    sykes::mutex::lock_type lock(TSynth::synth_mutex);
    //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "Glue midiReceive %d %d %d", status, data1, data2);
    auto m = sykes::midi::make_message(status, data1, data2);
    TSynth::GetTSynth().MidiReceive(m);
}

JNIEXPORT void JNICALL Java_org_bigsleep_tsynth_1micro_Glue_setParameterDouble
  (JNIEnv *, jobject, jint mod, jint port, jdouble value)
{
    sykes::mutex::lock_type lock(TSynth::synth_mutex);
    //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "Glue setParameterDouble %d %d %f", mod, port, value);
    TSynth::GetTSynth().SetParameterDouble(mod, port, value);
}

JNIEXPORT void JNICALL Java_org_bigsleep_tsynth_1micro_Glue_setParameterInt
  (JNIEnv *, jobject, jint mod, jint port, jint value)
{
    sykes::mutex::lock_type lock(TSynth::synth_mutex);
    //__android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "Glue setParameterInt %d %d %d", mod, port, value);
    TSynth::GetTSynth().SetParameterInt(mod, port, value);
}

JNIEXPORT void JNICALL Java_org_bigsleep_tsynth_1micro_Glue_start
  (JNIEnv *, jobject)
{
    __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "Glue_start");
    TSynth::PushEmptyBuffer();
    TSynth::GetPcmOut().set_callback(&TSynth::Callback);
    TSynth::GetPcmOut().start();
}

JNIEXPORT void JNICALL Java_org_bigsleep_tsynth_1micro_Glue_stop
  (JNIEnv *, jobject)
{
    __android_log_print(ANDROID_LOG_DEBUG, "bigsleep", "Glue_stop");
    TSynth::GetTSynth().AllSoundOff();
    TSynth::GetPcmOut().stop();
}

