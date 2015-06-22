LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/stlport/stlport \
                    bionic/ \
                    external/dmtcp/dmtcp/src
LOCAL_CFLAGS+= -O0 -g3 -DHAVE_CONFIG_H
LOCAL_SRC_FILES:= jalib.cpp jassert.cpp jfilesystem.cpp \
                  jsocket.cpp jalloc.cpp  jbuffer.cpp \
                  jserialize.cpp jtimer.cpp

LOCAL_MODULE := libjal
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)
