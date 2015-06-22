LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/stlport/stlport \
                    bionic/ \
                    external/dmtcp/dmtcp/src \
                    external/dmtcp/dmtcp/jalib \

LOCAL_CFLAGS+= -O0 -g3 -DHAVE_CONFIG_H
LOCAL_SHARED_LIBRARIES := libdmtcphijack libstlport
LOCAL_SRC_FILES:= \
                  pidvirt.cpp pidvirt_miscwrappers.cpp \
                  pidwrappers.cpp pidvirt_filewrappers.cpp \
                  pidvirt_syscallsreal.c virtualpidtable.cpp \


LOCAL_MODULE := pidvirt
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

