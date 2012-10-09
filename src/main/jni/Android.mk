LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := tsynth
LOCAL_SRC_FILES := \
    tsynth/tree.cpp \
    tsynth/inv_exp_table.cpp \
    tsynth/midi_utility.cpp \
    tsynth/mono_synth.cpp \
    tsynth/mod_factory.cpp \
    tsynth/vco.cpp \
    tsynth/vca.cpp \
    tsynth/vcf.cpp \
    tsynth/mixer.cpp \
    tsynth/eg.cpp \
    tsynth/tsynth.cpp \
    glue.cpp
LOCAL_LDLIBS    += -lOpenSLES -llog
LOCAL_CPPFLAGS += -fexceptions -std=gnu++0x -DNDEBUG -O3
LOCAL_C_INCLUDES += $(LOCAL_PATH)/tsynth \
    /home/mt/source/boost_1_47_0

include $(BUILD_SHARED_LIBRARY)
