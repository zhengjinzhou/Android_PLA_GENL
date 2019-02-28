LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := avcodec
LOCAL_SRC_FILES := ./libffmpeg/libavcodec.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := avformat
LOCAL_SRC_FILES := ./libffmpeg/libavformat.a
include $(PREBUILT_STATIC_LIBRARY) 

include $(CLEAR_VARS)
LOCAL_MODULE    := avutil
LOCAL_SRC_FILES := ./libffmpeg/libavutil.a
include $(PREBUILT_STATIC_LIBRARY) 

include $(CLEAR_VARS)
LOCAL_MODULE    := swscale
LOCAL_SRC_FILES := ./libffmpeg/libswscale.a
include $(PREBUILT_STATIC_LIBRARY) 

include $(CLEAR_VARS)
LOCAL_MODULE    := mp4v2
LOCAL_SRC_FILES := ./libffmpeg/libMp4v2.a
include $(PREBUILT_STATIC_LIBRARY) 

include $(CLEAR_VARS)

LOCAL_CFLAGS := -D__STDC_CONSTANT_MACROS -Wno-sign-compare -Wno-switch -Wno-pointer-sign -DHAVE_NEON=1 \
      -mfpu=neon -mfloat-abi=softfp -fPIC -DANDROID \
      -DLINUX -finline-functions -fexceptions -fno-strict-aliasing \
      -g -static -Wall -Wextra # -fvisibility=hidden 
      
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
					$(LOCAL_PATH)/include/mp4v2 \
					$(LOCAL_PATH)/xml

LOCAL_LDLIBS := -L$(NDK_PLATFORMS_ROOT)/$(TARGET_PLATFORM)/arch-arm/usr/lib \
				-llog -ljnigraphics -lz -ldl -lstdc++ -lm
#	-lagent -ludtc -ludt -lstdc++ -lgio-2.0 \
	-lgobject-2.0 -lgthread-2.0 -lgmodule-2.0 \
	-lglib-2.0 -lnice -lffi -lrt -lz -ldl -lresolv -lintl -lm -lpthread -static

LOCAL_WHOLE_STATIC_LIBRARIES := libstlport_static

LOCAL_STATIC_LIBRARIES := avformat avcodec swscale avutil mp4v2

LOCAL_SRC_FILES :=	xml/mxml-attr.c xml/mxml-entity.c xml/mxml-file.c xml/mxml-index.c xml/mxml-node.c \
					xml/mxml-private.c xml/mxml-search.c xml/mxml-set.c xml/mxml-string.c \
					interface.c clientlink.c DecodeSip.c gb2312.c httpAPI.c netglobal.c playlist.c playlib.c \
					st_cuapi.c tcpsock.c VideoInstance.c AudioInstance.c mp4v2export.c

LOCAL_MODULE    := nvsplayer

include $(BUILD_SHARED_LIBRARY)