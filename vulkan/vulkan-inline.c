
#include <kklib.h>
#include <kklib/maybe.h>
#include <kklib/vector.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// TODO(zeph): preprocessor logic for wsi defines?
#include <volk.h>

#include "vulkan_vulkan.h"

static void kkfree_pnext_chain(const void *first, kk_context_t *ctx) {
	const VkBaseInStructure *p = first;
	while (p != NULL) {
		const VkBaseInStructure *n = p->pNext;
		kk_free(p, ctx);
		p = n;
	}
}

struct vulkan_instance_params {
	VkInstanceCreateInfo info;
	VkApplicationInfo app;
};

static kk_box_t kk_vulkan_create_instance_params(bool enumerate_portability, kk_context_t *ctx) {
	struct vulkan_instance_params *p = kk_zalloc(kk_ssizeof(struct vulkan_instance_params), ctx);
	VkInstanceCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
	};
	if (enumerate_portability) {
		info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	}
	p->info = info;
	return kk_cptr_box(p, ctx);
}

static void kk_vulkan_create_instance_set_application_info(kk_box_t kparams, kk_string_t kname, int32_t appver, kk_string_t kengine, int32_t engver, int32_t apiver, kk_context_t *ctx) {
	struct vulkan_instance_params *p = kk_cptr_unbox_borrowed(kparams, ctx);
	VkApplicationInfo app = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = kk_string_cbuf_borrow(kname, NULL, ctx),
		.applicationVersion = (uint32_t)appver,
		.pEngineName = kk_string_cbuf_borrow(kengine, NULL, ctx),
		.engineVersion = (uint32_t)engver,
		.apiVersion = (uint32_t)apiver,
	};
	p->app = app;
	p->info.pApplicationInfo = &p->app;
}

static void string_vec_to_alloc_array(kk_vector_t strings, kk_context_t *ctx, const char * const **out, uint32_t *out_len) {
	kk_ssize_t len;
	kk_box_t *arr = kk_vector_buf_borrow(strings, &len, ctx);
	const char **l = kk_malloc(len * sizeof(char *), ctx);
	for (kk_ssize_t i = 0; i < len; i++) {
		kk_string_t s = kk_string_unbox(arr[i]);
		l[i] = kk_string_cbuf_borrow(s, NULL, ctx);
	}
	*out = l;
	*out_len = (uint32_t)len;
}

static void kk_vulkan_create_instance_set_layers(kk_box_t kparams, kk_vector_t layers, kk_context_t *ctx) {
	struct vulkan_instance_params *p = kk_cptr_unbox_borrowed(kparams, ctx);
	string_vec_to_alloc_array(layers, ctx, &(p->info.ppEnabledLayerNames), &(p->info.enabledLayerCount));
}

static void kk_vulkan_create_instance_set_extensions(kk_box_t kparams, kk_vector_t exts, kk_context_t *ctx) {
	struct vulkan_instance_params *p = kk_cptr_unbox_borrowed(kparams, ctx);
	string_vec_to_alloc_array(exts, ctx, &(p->info.ppEnabledExtensionNames), &(p->info.enabledExtensionCount));
}

static void vulkan_instance_free(void *p, kk_block_t *b, kk_context_t *ctx) {
	VkInstance *instance = (VkInstance *)p;
	if (instance != NULL) {
		kk_info_message("destroy instance %p\n", instance);
		vkDestroyInstance(*instance, NULL);
	}
	kk_free(instance, ctx);
}

// TODO(zeph): pNext for both instance info and app info

static kk_box_t kk_vulkan_create_instance_and_free_info(kk_box_t kparams, kk_context_t *ctx) {
	struct vulkan_instance_params *p = kk_cptr_unbox_borrowed(kparams, ctx);
	if (p->info.pApplicationInfo == &p->app) {
		kk_info_message("application name: %s\n", p->app.pApplicationName);
		kk_info_message("application version: %u\n", (unsigned)p->app.applicationVersion);
		kk_info_message("engine name: %s\n", p->app.pEngineName);
		kk_info_message("engine version: %u\n", p->app.engineVersion);
		kk_info_message("api version: %x\n", p->app.apiVersion);
	}
	VkInstance *instance = kk_malloc(sizeof(VkInstance), ctx);
	kk_info_message("create vulkan instance instance=%p\n", instance);
	VkResult result = vkCreateInstance(&p->info, NULL, instance);
	kk_info_message("created instance, result %d\n", result);
	// TODO(zeph): handle result
	if (result != VK_SUCCESS) {
		kk_fatal_error(EINVAL, "result %d\n", result);
	}
	// Free the instance create info.
	{
		kkfree_pnext_chain(p->info.pNext, ctx);
		kkfree_pnext_chain(p->app.pNext, ctx);
		kk_free(p->info.ppEnabledLayerNames, ctx);
		kk_free(p->info.ppEnabledExtensionNames, ctx);
		kk_free(p, ctx);
	}
	volkLoadInstanceOnly(*instance);
	return kk_cptr_raw_box(&vulkan_instance_free, instance, ctx);
}
