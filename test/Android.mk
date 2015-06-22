LOCAL_PATH:= $(call my-dir)

common_C_FLAGS := -O0 -g3 -DDEBUG -DTIMING

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/dmtcp/dmtcp/src
LOCAL_SRC_FILES := dmtcp-pre_user_thread_hook.cpp
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_MODULE := dmtcp-pre_user_thread_hook
LOCAL_SHARED_LIBRARIES := libdmtcpaware
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := dmtcp-tls.cpp
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_MODULE := dmtcp-tls
LOCAL_SHARED_LIBRARIES := libdl
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := testdmtcp-ashmem.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_MODULE := testdmtcp-ashmem
LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := dmtcp-ashmem-2.cpp
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_MODULE := dmtcp-ashmem-2
LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := dmtcp-fp.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_MODULE := dmtcp-fp
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := dmtcp1.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_MODULE := dmtcp1
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := dmtcp2.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_MODULE := dmtcp2
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := dmtcp3.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_MODULE := dmtcp3
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := dmtcp4.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_MODULE := dmtcp4
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := dmtcp5.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_MODULE := dmtcp5
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := dmtcp_fork.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_MODULE := dmtcp_fork
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/dmtcp/dmtcp/src
LOCAL_SRC_FILES := dmtcpaware1.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_SHARED_LIBRARIES := libdmtcpaware
LOCAL_MODULE := dmtcpaware1
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/dmtcp/dmtcp/src
LOCAL_SRC_FILES := dmtcpaware2.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_SHARED_LIBRARIES := libdmtcpaware
LOCAL_MODULE := dmtcpaware2
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/dmtcp/dmtcp/src
LOCAL_SRC_FILES := dmtcpaware3.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_SHARED_LIBRARIES := libdmtcpaware
LOCAL_MODULE := dmtcpaware3
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := shared-memory.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_MODULE := dmtcp-shared-memory
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := shared-fd.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_MODULE := dmtcp-shared-fd
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := stale-fd.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_MODULE := dmtcp-stale-fd
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := dmtcp-android-log.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_MODULE := dmtcp-android-log
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := forkexec.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_MODULE := dmtcp-forkexec
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := dmtcp-binder.cpp
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_SHARED_LIBRARIES := libbinder
LOCAL_MODULE := dmtcp-binder
LOCAL_MODULE_TAGS := optional
#include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := dmtcp-binder-server.cpp
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_C_INCLUDES += \
    bionic \
    bionic/libstdc++/include \
    external/stlport/stlport \
    external/gtest/include \
    system/extras/tests/include \
    frameworks/base/include
LOCAL_STATIC_LIBRARIES += \
    libgtest \
    libgtest_main \
    libtestUtil
LOCAL_SHARED_LIBRARIES += \
    libutils \
    libstlport \
    libbinder
LOCAL_MODULE := dmtcp-binder-server
LOCAL_MODULE_TAGS := optional
#include $(BUILD_EXECUTABLE)
