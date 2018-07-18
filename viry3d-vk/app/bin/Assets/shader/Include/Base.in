#ifdef VR_VULKAN
#define vulkan_convert() \
gl_Position.y = -gl_Position.y;\
gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
#else
#define vulkan_convert()
#endif
