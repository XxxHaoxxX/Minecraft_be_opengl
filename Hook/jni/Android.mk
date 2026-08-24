LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := hook
LOCAL_SRC_FILES := inlineHook.c relocate.c hook.cpp
LOCAL_SRC_FILES += $(wildcard third_party/imgui/*.cpp)
LOCAL_SRC_FILES += third_party/imgui/backends/imgui_impl_opengl3.cpp third_party/imgui/backends/imgui_impl_android.cpp
#LOCAL_SRC_FILES += $(NDK_ROOT)/sources/android/native_app_glue/android_native_app_glue.c
LOCAL_CPPFLAGS := -std=c++20 -fvisibility=hidden
LOCAL_CPPFLAGS += -DMSGPACK_DISABLE_BOOST=1
LOCAL_CPPFLAGS += -DMSGPACK_NO_BOOST=1
LOCAL_CPPFLAGS += -frtti
LOCAL_CPPFLAGS += -fms-extensions
LOCAL_CPPFLAGS += -DANDROID
LOCAL_CPPFLAGS += -DNDEBUG -O2 -Os
LOCAL_CPP_FEATURES += exceptions
LOCAL_LDLIBS += -lc -lz -llog -landroid
LOCAL_LDLIBS += -lGLESv2 -lEGL -lGLESv1_CM -lGLESv2 -lGLESv3
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/Fonts \
    $(LOCAL_PATH)/third_party/imgui \
    $(LOCAL_PATH)/third_party/imgui/backends \
#    $(NDK_ROOT)/sources/android/native_app_glue \


include $(BUILD_SHARED_LIBRARY)
