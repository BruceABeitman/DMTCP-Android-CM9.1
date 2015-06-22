LOCAL_PATH:= $(call my-dir)

MTCP_LOCAL_CFLAGS:=-DHIGHEST_VA='(VA)0xffffe000' -fno-stack-protector
common_C_FLAGS :=  -DDEBUG -DTIMING -O2
ifeq ($(ARCH_ARM_HAVE_TLS_REGISTER),true)
  common_C_FLAGS += -DHAVE_ARM_TLS_REGISTER
endif
ifeq ($(TARGET_PRODUCT),full)
  common_C_FLAGS += -DANDROID_EMULATOR
endif
include $(CLEAR_VARS)
LOCAL_SRC_FILES:= mtcp.c mtcp_restart_nolibc.c \
        mtcp_maybebpt.c mtcp_printf.c mtcp_util.c \
        mtcp_safemmap.c mtcp_safe_open.c \
        mtcp_state.c mtcp_check_vdso.c mtcp_sigaction.c mtcp_fastckpt.c \
        bionic_pthread_r.c

ifeq ($(TARGET_ARCH),arm)
LOCAL_SRC_FILES += clone-arm.S libc-do-syscall-arm-eabi.S
common_C_FLAGS += -D__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
endif
ifeq ($(TARGET_ARCH),x86)
LOCAL_SRC_FILES += getcontest-x86.S setcontest-x86.S clone-x86.S
endif
#        bionic_pthread.c

LOCAL_C_INCLUDES := bionic/libc/private/ \
                    bionic/libc/bionic/

# Define ANDROID_SMP appropriately.
ifeq ($(TARGET_CPU_SMP),true)
    LOCAL_CFLAGS += -DANDROID_SMP=1
else
    LOCAL_CFLAGS += -DANDROID_SMP=0
endif
LOCAL_CFLAGS+= $(MTCP_LOCAL_CFLAGS)
LOCAL_CFLAGS+= $(common_C_FLAGS)
ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS += -DUSE_PROC_MAPS
endif
ifeq ($(TARGET_ARCH),x86)
LOCAL_LDFLAGS:= -Wl,-T,$(LOCAL_PATH)/mtcp.x86.t
endif
LOCAL_LDFLAGS+= -Wl,-Map,$(LOCAL_PATH)/mtcp.map
LOCAL_MODULE := libmtcp
LOCAL_SYSTEM_SHARED_LIBRARIES := libc libdl
#LOCAL_STATIC_LIBRARIES := liblog
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := readmtcp.c
LOCAL_CFLAGS+= $(MTCP_LOCAL_CFLAGS) $(common_C_FLAGS)
LOCAL_MODULE := readmtcp
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := mtcp_restart.c \
        mtcp_printf.c mtcp_util.c mtcp_maybebpt.c \
        mtcp_safemmap.c mtcp_state.c mtcp_safe_open.c \
        mtcp_check_vdso.c mtcp_fastckpt.c
ifeq ($(TARGET_ARCH),arm)
LOCAL_SRC_FILES += libc-do-syscall-arm-eabi.S
endif
LOCAL_CFLAGS+= $(MTCP_LOCAL_CFLAGS) $(common_C_FLAGS)
#LOCAL_STATIC_LIBRARIES := liblog
LOCAL_SYSTEM_SHARED_LIBRARIES := libc
LOCAL_MODULE := mtcp_restart
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := testmtcp.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_LDFLAGS:= -Wl,--export-dynamic
LOCAL_MODULE := testmtcp
LOCAL_SHARED_LIBRARIES := libmtcp
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := testmtcp-dmtcp.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_MODULE := testmtcp-dmtcp
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := testmtcp3.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_LDFLAGS:= -Wl,--export-dynamic
LOCAL_MODULE := testmtcp3
LOCAL_SHARED_LIBRARIES := libmtcp
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := testmtcp4.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_LDFLAGS:= -Wl,--export-dynamic
LOCAL_MODULE := testmtcp4
LOCAL_SHARED_LIBRARIES := libmtcp
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := testmtcp5.c
LOCAL_CFLAGS+= $(common_C_FLAGS)
LOCAL_LDFLAGS:= -Wl,--export-dynamic
LOCAL_MODULE := testmtcp5
LOCAL_SHARED_LIBRARIES := libmtcp
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)
