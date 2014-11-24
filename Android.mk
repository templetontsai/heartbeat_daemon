# Copyright 2006 The Android Open Source Project
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libheartbeat

LOCAL_MODULE_TAGS := debug
#LOCAL_MODULE_TAGS := eng development

LOCAL_CFLAGS := -DLOG_TAG=\"Sensors\"

LOCAL_SRC_FILES := heartbeat_algorithm.c

LOCAL_SHARED_LIBRARIES := liblog
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libdl

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := main.c

#LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libc 
LOCAL_SHARED_LIBRARIES += libheartbeat
LOCAL_MODULE := heartbeat_daemon
include $(BUILD_EXECUTABLE)

