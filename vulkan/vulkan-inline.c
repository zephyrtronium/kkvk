
#include <kklib.h>
#include <kklib/maybe.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// TODO(zeph): preprocessor logic for wsi defines?
#include <volk.h>

#include "vulkan_vulkan.h"

static void vulkan_instance_free(void *p, kk_block_t *b, kk_context_t *ctx) {
	kk_unused(ctx);
	VkInstance *instance = (VkInstance *)p;
	if (instance != NULL) {
		vkDestroyInstance(*instance, NULL);
	}
}

static kk_box_t kk_vulkan_create_instance(kk_vulkan_vulkan__instance_create kinfo, kk_std_core_types__maybe kapp, kk_context_t *ctx) {
    VkInstanceCreateFlags flags = 0;
    if (kk_vulkan_vulkan_instance_create_fs_enumerate_portability(kinfo, ctx)) {
        flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
	VkInstanceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = flags,
    };
    VkApplicationInfo app;
    if (kk_std_core_types__is_Just(kapp, ctx)) {
        kk_vulkan_vulkan__application t = kk_vulkan_vulkan__application_unbox(kapp._cons.Just.value, KK_OWNED, ctx);
        kk_string_t name = kk_vulkan_vulkan_application_fs_application_name(t, ctx);
        kk_string_t engine = kk_vulkan_vulkan_application_fs_engine_name(t, ctx);
        kk_vulkan_vulkan__vk_version apiver = kk_vulkan_vulkan_application_fs_api_version(t, ctx);
        VkApplicationInfo u = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            // TODO(zeph): pNext
            .pApplicationName = kk_string_cbuf_borrow(name, NULL, ctx),
            .applicationVersion = (uint32_t)kk_vulkan_vulkan_application_fs_application_version(t, ctx),
            .pEngineName = kk_string_cbuf_borrow(engine, NULL, ctx),
            .engineVersion = (uint32_t)kk_vulkan_vulkan_application_fs_engine_version(t, ctx),
            .apiVersion = (uint32_t)kk_vulkan_vulkan_vk_version_fs_v(apiver, ctx),
        };
        app = u;
        info.pApplicationInfo = &app;
		kk_info_message("application name: %s\n", u.pApplicationName);
		kk_info_message("application version: %u\n", (unsigned)u.applicationVersion);
		kk_info_message("engine name: %s\n", u.pEngineName);
		kk_info_message("engine version: %u\n", u.engineVersion);
		kk_info_message("api version: %x\n", u.apiVersion);
    }
    // TODO(zeph): pNext, enabled layers, enabled extensions
    VkInstance *instance = kk_malloc(sizeof(VkInstance), ctx);
	kk_info_message("create vulkan instance instance=%p\n", instance);
    VkResult result = vkCreateInstance(&info, NULL, instance);
    // TODO(zeph): handle result
    if (result != VK_SUCCESS) {
		kk_fatal_error(EINVAL, "result %d\n", result);
	}
	return kk_cptr_raw_box(&vulkan_instance_free, instance, ctx);
}
