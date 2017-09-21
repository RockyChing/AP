LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

define all-cpp-files-under
	$(patsubst ./%,%, $(shell find $(LOCAL_PATH) -name "platform" -prune -o -name "*.cpp" -and -not -name ".*"))
endef
  
define all-subdir-cpp-files
$(call all-cpp-files-under,.)
endef

CPP_FILE_LIST := $(call all-subdir-cpp-files)


LOCAL_SRC_FILES := $(CPP_FILE_LIST:$(LOCAL_PATH)/%=%)


LOCAL_C_INCLUDES := /home/hao/workspace/rk3288-android6.0/external/AP/include
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := mt_ap


include $(BUILD_EXECUTABLE)

