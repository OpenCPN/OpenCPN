LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := assetbridge

# LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(TCL_PATH)/include

# Add your application source files here...
LOCAL_SRC_FILES := assetbridge.c

# LOCAL_SHARED_LIBRARIES := tclkit860

# LOCAL_LDLIBS := -landroid -llog

include $(BUILD_SHARED_LIBRARY)
