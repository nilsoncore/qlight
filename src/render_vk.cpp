#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <string.h> // memcpy

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "render_vk.h"
#include "qlight.h"

#pragma warning ( disable : 26812 )

#define ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])

#define VK_CHECK(x)                 \
    do                              \
    {                               \
        VkResult error = x;         \
        if (error != VK_SUCCESS) {  \
            printf("Vulkan ERROR:\n\tSite: %s (%s:%d)\n\tError: %d\n", #x, __FILE__, __LINE__, error); \
            abort();                \
        }                           \
    } while (0)

/* Unused for now
QRenderVk g_vk = { };
*/

static const char *g_instance_validation_layers[] = {
	"VK_LAYER_KHRONOS_validation"
};

static const char *g_device_extensions_required[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const char **glfwGetRequiredInstanceExtensions(uint32_t *);

/* Unused for now
void vk_init() {
	g_vk.instance = vk_create_instance(QLIGHT_APP_NAME, QLIGHT_APP_VERSION);
	g_vk.physical_devices = vk_get_physical_devices(g_vk.instance);
	g_vk.device.q_physical_device = vk_select_best_physical_device(g_vk.physical_devices);
	g_vk.queue_families = vk_get_physical_device_queue_families(g_vk.device.q_physical_device);
	g_vk.selected_queue_family = vk_select_best_queue_family(g_vk.queue_families);
	g_vk.device = vk_create_device(g_vk.queue_families, g_vk.selected_queue_family);
}
*/

VkInstance vk_create_instance(const char *app_name, uint32_t app_version) {
	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = NULL,
		.pApplicationName = app_name,
		.applicationVersion = app_version,
		.pEngineName = QLIGHT_ENGINE_NAME,
		.engineVersion = QLIGHT_ENGINE_VERSION,
		.apiVersion = VK_API_VERSION_1_2
	};

	uint32_t glfw_instance_extensions_required_count = 0;
	const char **glfw_instance_extensions_required_names = glfwGetRequiredInstanceExtensions(&glfw_instance_extensions_required_count);

	{
		char buffer[4096];
		const u64 buffer_size = sizeof(buffer);
		u32 cursor = 0;

		cursor += snprintf(buffer + cursor, buffer_size - cursor, "%s", "GLFW Required Instance Extensions:\n");
		for (u32 extension_idx = 0; extension_idx < glfw_instance_extensions_required_count; extension_idx += 1) {
			const char *extension_name = glfw_instance_extensions_required_names[extension_idx];
			cursor += snprintf(buffer + cursor, buffer_size - cursor, "\t- %s\n", extension_name);
		}
		printf("(cursor: %u)\n", cursor);

		fwrite(buffer, cursor, 1, stdout);
	}

	VkInstanceCreateInfo instance_create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.pApplicationInfo = &app_info,
		.enabledLayerCount = ARRAY_SIZE(g_instance_validation_layers),
		.ppEnabledLayerNames = g_instance_validation_layers,
		.enabledExtensionCount = glfw_instance_extensions_required_count,
		.ppEnabledExtensionNames = glfw_instance_extensions_required_names
	};

	VkInstance instance = NULL;
	VK_CHECK(vkCreateInstance(&instance_create_info, NULL, &instance));

	return instance;
}

QVkPhysicalDeviceList vk_get_physical_devices(VkInstance instance) {
	QVkPhysicalDeviceList list = {
		.count = 0,
		.q_devices = NULL
	};

	VK_CHECK(vkEnumeratePhysicalDevices(instance, &list.count, NULL));

	if (list.count < 1)  return list;

	QVkPhysicalDevice *q_physical_devices = (QVkPhysicalDevice *) calloc(list.count, sizeof(QVkPhysicalDevice));
	list.q_devices = q_physical_devices;

	VkPhysicalDevice *physical_devices = (VkPhysicalDevice *) calloc(list.count, sizeof(VkPhysicalDevice));
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &list.count, physical_devices));

	for (uint32_t device_idx = 0; device_idx < list.count; device_idx += 1) {
		QVkPhysicalDevice *q_device = &list.q_devices[device_idx];
		q_device->device = physical_devices[device_idx];
		q_device->instance = instance;

		vkGetPhysicalDeviceProperties(q_device->device, &q_device->properties);
		vkGetPhysicalDeviceMemoryProperties(q_device->device, &q_device->memory_properties);
		vkGetPhysicalDeviceFeatures(q_device->device, &q_device->features);

		uint32_t memory_heaps_count = q_device->memory_properties.memoryHeapCount;
		q_device->memory_total = 0;
		for (uint32_t heap_idx = 0; heap_idx < memory_heaps_count; heap_idx += 1) {
			q_device->memory_total += q_device->memory_properties.memoryHeaps[heap_idx].size;
		}
	}

	free(physical_devices);

	return list;
}

QVkPhysicalDevice *vk_select_best_physical_device(QVkPhysicalDeviceList list) {
	if (list.count < 1)  return NULL;

	// Selection is based on total memory count
	// and goes with this priority:
	//   1. Discrete (dedicated) GPU
	//   2. Integrated GPU
	//   3. Virtual GPU
	//   4. CPU

	// GPUs:
	QVkPhysicalDevice *best_dedicated = NULL;
	VkDeviceSize best_dedicated_memory = 0;
	QVkPhysicalDevice *best_integrated = NULL;
	VkDeviceSize best_integrated_memory = 0;
	QVkPhysicalDevice *best_virtual = NULL;
	VkDeviceSize best_virtual_memory = 0;

	// CPU:
	QVkPhysicalDevice *best_cpu = NULL;
	VkDeviceSize best_cpu_memory = 0;

	for (uint32_t device_idx = 0; device_idx < list.count; device_idx += 1) {
		QVkPhysicalDevice *q_device = &list.q_devices[device_idx];

		QVkPhysicalDevice **p_device = NULL;
		VkDeviceSize *p_device_memory = NULL;
		switch (q_device->properties.deviceType) {
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: {
				p_device = &best_dedicated;
				p_device_memory = &best_dedicated_memory;
				break;
			};
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: {
				p_device = &best_integrated;
				p_device_memory = &best_integrated_memory;
				break;
			};
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: {
				p_device = &best_virtual;
				p_device_memory = &best_virtual_memory;
				break;
			};
			case VK_PHYSICAL_DEVICE_TYPE_CPU: {
				p_device = &best_cpu;
				p_device_memory = &best_cpu_memory;
				break;
			};
		}

		if (p_device == NULL || p_device_memory == NULL)  continue;

		if (q_device->memory_total > *p_device_memory) {
			*p_device = q_device;
			*p_device_memory = q_device->memory_total;
		}
	}

	if      (best_dedicated  != NULL)  return best_dedicated;
	else if (best_integrated != NULL)  return best_integrated;
	else if (best_virtual    != NULL)  return best_virtual;
	else if (best_cpu        != NULL)  return best_cpu;
	else                               return NULL;
}

QVkQueueFamilyPropertiesList vk_get_physical_device_queue_families(QVkPhysicalDevice *q_physical_device) {
	QVkQueueFamilyPropertiesList families_list = {
		.q_physical_device = q_physical_device,
		.properties_count = 0,
		.properties = NULL
	};

	vkGetPhysicalDeviceQueueFamilyProperties(q_physical_device->device, &families_list.properties_count, NULL);

	families_list.properties = (VkQueueFamilyProperties *) calloc(families_list.properties_count, sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(q_physical_device->device, &families_list.properties_count, families_list.properties);

	return families_list;
}

QVkQueueFamily vk_select_best_queue_family(QVkQueueFamilyPropertiesList families_list) {
	QVkQueueFamily best_queue_family = {
		.q_physical_device = families_list.q_physical_device,
		.index = UINT32_MAX,
		.properties = NULL
	};

	if (families_list.properties_count < 1)  return best_queue_family;

	uint32_t max_queue_count = 0;
	for (uint32_t family_idx = 0; family_idx < families_list.properties_count; family_idx += 1) {
		VkQueueFamilyProperties *properties = &families_list.properties[family_idx];
		bool has_graphics_queue = properties->queueFlags & VK_QUEUE_GRAPHICS_BIT;
		if (has_graphics_queue) {
			if (properties->queueCount > max_queue_count) {
				max_queue_count = properties->queueCount;
				best_queue_family.index = family_idx;
				best_queue_family.properties = properties;
			}
		}
	}

	return best_queue_family;
}

QVkDevice vk_create_device(QVkQueueFamilyPropertiesList families_list, QVkQueueFamily selected_family) {
	if (selected_family.index < 0)
		selected_family = vk_select_best_queue_family(families_list);

	VkDeviceQueueCreateInfo *device_queue_create_infos = (VkDeviceQueueCreateInfo *) calloc(families_list.properties_count, sizeof(VkDeviceQueueCreateInfo));
	float **queue_priorities = (float **) calloc(families_list.properties_count, sizeof(float *));
	for (uint32_t queue_family_idx = 0; queue_family_idx < families_list.properties_count; queue_family_idx += 1) {
		uint32_t queue_count = families_list.properties[queue_family_idx].queueCount;
		queue_priorities[queue_family_idx] = (float *) calloc(queue_count, sizeof(float));
		for (uint32_t queue_idx = 0; queue_idx < queue_count; queue_idx += 1) {
			queue_priorities[queue_family_idx][queue_idx] = 1.0f;
		}

		VkDeviceQueueCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.queueFamilyIndex = queue_family_idx,
			.queueCount = queue_count,
			.pQueuePriorities = queue_priorities[queue_family_idx]
		};
		device_queue_create_infos[queue_family_idx] = info;
	}

	VkDeviceCreateInfo device_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.queueCreateInfoCount = families_list.properties_count,
		.pQueueCreateInfos = device_queue_create_infos,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = NULL,
		.enabledExtensionCount = 1,
		.ppEnabledExtensionNames = g_device_extensions_required,
		.pEnabledFeatures = &families_list.q_physical_device->features
	};

	QVkDevice q_device = {
		.q_physical_device = families_list.q_physical_device,
		.device = VK_NULL_HANDLE,
		.graphics_queue = VK_NULL_HANDLE,
		.present_queue = VK_NULL_HANDLE,
		.single_queue = false
	};

	VK_CHECK(vkCreateDevice(families_list.q_physical_device->device, &device_create_info, NULL, &q_device.device));

	vkGetDeviceQueue(q_device.device, selected_family.index, 0, &q_device.graphics_queue);

	q_device.single_queue = (selected_family.properties->queueCount < 2);
	uint32_t present_queue_idx = (q_device.single_queue) ? 0 : 1;
	vkGetDeviceQueue(q_device.device, selected_family.index, present_queue_idx, &q_device.present_queue);

	return q_device;
}

QVkSurface vk_get_glfw_window_surface(QVkQueueFamily queue_family, GLFWwindow *window) {
	// Get window surface from GLFW
	VkPhysicalDevice device = queue_family.q_physical_device->device;
	QVkSurface q_surface = { };
	VK_CHECK(glfwCreateWindowSurface(queue_family.q_physical_device->instance, window, NULL, &q_surface.surface));

	// Get framebuffer size of that window
	int framebuffer_width;
	int framebuffer_height;
	glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
	q_surface.window_framebuffer_size = {
		.width = (uint32_t)framebuffer_width,
		.height = (uint32_t)framebuffer_height
	};

	// Check surface support on device
	VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, queue_family.index, q_surface.surface, &q_surface.supported));
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, q_surface.surface, &q_surface.capabilities));

	// Clamp framebuffer size to one that is capable to device
	VkSurfaceCapabilitiesKHR caps = q_surface.capabilities;
	bool framebuffer_suitable = caps.currentExtent.width  == framebuffer_width
                             && caps.currentExtent.height == framebuffer_height;

	if (!framebuffer_suitable) {
		VkExtent2D *size = &q_surface.window_framebuffer_size;
		printf("Extent size (%ux%u) does not match framebuffer size, resizing...\n", size->width, size->height);

		if      (size->width > caps.maxImageExtent.width)    size->width  = caps.maxImageExtent.width;
		else if (size->width < caps.minImageExtent.width)    size->width  = caps.minImageExtent.width;

		if      (size->height > caps.maxImageExtent.height)  size->height = caps.maxImageExtent.height;
		else if (size->height < caps.minImageExtent.height)  size->height = caps.minImageExtent.height;

		printf("Extent rezised (%ux%u).\n", size->width, size->height);
	}

	q_surface.queue_family = queue_family;

	// Get available surface formats
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, q_surface.surface, &q_surface.formats_count, NULL));
	q_surface.formats = (VkSurfaceFormatKHR *) calloc(q_surface.formats_count, sizeof(VkSurfaceFormatKHR));
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, q_surface.surface, &q_surface.formats_count, q_surface.formats));

	// Get available surface present modes
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, q_surface.surface, &q_surface.present_modes_count, NULL));
	q_surface.present_modes = (VkPresentModeKHR *) calloc(q_surface.present_modes_count, sizeof(VkPresentModeKHR));
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, q_surface.surface, &q_surface.present_modes_count, q_surface.present_modes));

	printf("Present modes: %d\n", q_surface.present_modes_count);
	for (uint32_t present_mode_idx = 0; present_mode_idx < q_surface.present_modes_count; present_mode_idx += 1) {
		VkPresentModeKHR present_mode = q_surface.present_modes[present_mode_idx];
		const char *present_mode_name = vk_get_present_mode_name(present_mode);
		printf("\t[%u] - %s\n", present_mode_idx, present_mode_name);

		q_surface.supported_present_modes_flags |= present_mode;
	}

	return q_surface;
}

QVkSwapchain vk_create_swapchain(QVkDevice *q_device, QVkSurface *surface, uint32_t surface_format_index, bool force_single_queue, VkPresentModeKHR preferred_present_mode) {
	if (surface_format_index < 0 || surface_format_index >= surface->formats_count) {
		printf("WARNING: vk_create_swapchain(): Passed surface format index is out of bounds! Expected: [0, %u], given: %u.\n", surface->formats_count - 1, surface_format_index);
		printf("vk_create_swapchain(): Using surface format with index 0 instead.\n");
		surface_format_index = 0;
	}

	uint32_t queue_family_indices[] = { 0, 1 };

	bool single_queue = surface->queue_family.properties->queueCount < 2;
	if (force_single_queue)  single_queue = true;

	bool has_preferred_present_mode = (surface->supported_present_modes_flags & preferred_present_mode);
	if (!has_preferred_present_mode) {
		VkPresentModeKHR fallback_mode = VK_PRESENT_MODE_FIFO_KHR;
		printf("WARNING: vk_create_swapchain(): Passed preferred present mode '%s' is not supported, falling back to '%s'.\n", vk_get_present_mode_name(preferred_present_mode), vk_get_present_mode_name(fallback_mode));
		preferred_present_mode = fallback_mode;
	}

	QVkSwapchain q_swapchain = { .q_surface = surface };

	q_swapchain.info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = NULL,
		.flags = 0,
		.surface = surface->surface,
		.minImageCount = surface->capabilities.minImageCount + 1,
		.imageFormat = surface->formats[surface_format_index].format,
		.imageColorSpace = surface->formats[surface_format_index].colorSpace,
		.imageExtent = surface->window_framebuffer_size,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = (single_queue) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
		.queueFamilyIndexCount = (single_queue) ? 0 : (uint32_t)2,
		.pQueueFamilyIndices = (single_queue) ? NULL : queue_family_indices,
		.preTransform = surface->capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = preferred_present_mode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE
	};

	VkDevice device = q_device->device;
	VK_CHECK(vkCreateSwapchainKHR(device, &q_swapchain.info, NULL, &q_swapchain.swapchain));

	VK_CHECK(vkGetSwapchainImagesKHR(device, q_swapchain.swapchain, &q_swapchain.images_count, NULL));
	q_swapchain.images = (VkImage *) calloc(q_swapchain.images_count, sizeof(VkImage));
	VK_CHECK(vkGetSwapchainImagesKHR(device, q_swapchain.swapchain, &q_swapchain.images_count, q_swapchain.images));

	q_swapchain.image_views = (VkImageView *) calloc(q_swapchain.images_count, sizeof(VkImageView));
	VkImageViewCreateInfo *image_view_create_infos = (VkImageViewCreateInfo *) calloc(q_swapchain.images_count, sizeof(VkImageViewCreateInfo));

	VkComponentMapping image_view_rgba_component = {
		.r = VK_COMPONENT_SWIZZLE_IDENTITY,
		.g = VK_COMPONENT_SWIZZLE_IDENTITY,
		.b = VK_COMPONENT_SWIZZLE_IDENTITY,
		.a = VK_COMPONENT_SWIZZLE_IDENTITY
	};

	VkImageSubresourceRange image_view_subresources = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = q_swapchain.info.imageArrayLayers
	};

	for (uint32_t image_view_idx = 0; image_view_idx < q_swapchain.images_count; image_view_idx += 1) {
		VkImageViewCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.image = q_swapchain.images[image_view_idx],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = surface->formats[surface_format_index].format,
			.components = image_view_rgba_component,
			.subresourceRange = image_view_subresources
		};

		image_view_create_infos[image_view_idx] = info;

		VK_CHECK(vkCreateImageView(device, &image_view_create_infos[image_view_idx], NULL, &q_swapchain.image_views[image_view_idx]));
		printf("Image view #%d created.\n", image_view_idx);
	}

	return q_swapchain;
}

QVkRenderPass vk_create_render_pass(QVkDevice *q_device, VkFormat image_view_format, VkExtent2D render_area_size) {
	VkAttachmentDescription attachments = {
		.flags = 0,
		.format = image_view_format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	VkAttachmentReference attachment_reference = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpass_descriptions = {
		.flags = 0,
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.inputAttachmentCount = 0,
		.pInputAttachments = NULL,
		.colorAttachmentCount = 1,
		.pColorAttachments = &attachment_reference,
		.pResolveAttachments = NULL,
		.pDepthStencilAttachment = NULL,
		.preserveAttachmentCount = 0,
		.pPreserveAttachments = NULL
	};

	VkSubpassDependency subpass_dependencies = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dependencyFlags = 0
	};

	VkRenderPassCreateInfo renderpass_create_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.attachmentCount = 1,
		.pAttachments = &attachments,
		.subpassCount = 1,
		.pSubpasses = &subpass_descriptions,
		.dependencyCount = 1,
		.pDependencies = &subpass_dependencies
	};

	VkOffset2D area_offset = {
		.x = 0,
		.y = 0
	};

	VkRect2D area = {
		.offset = area_offset,
		.extent = render_area_size
	};

	QVkRenderPass q_render_pass = {
		.q_device = q_device,
		.render_pass = VK_NULL_HANDLE,
		.area = area,
		.clear_value = { 0.0f, 0.0f, 0.0f, 0.0f }
	};

	VK_CHECK(vkCreateRenderPass(q_device->device, &renderpass_create_info, NULL, &q_render_pass.render_pass));

	return q_render_pass;
}

VkShaderModule vk_create_shader_module(QVkDevice *q_device, const char *shader_spirv_binary, size_t shader_size) {
	assert(q_device != NULL);
	assert(shader_spirv_binary != NULL);
	assert(shader_size > 0);

	VkShaderModuleCreateInfo vertex_shader_module_create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.codeSize = shader_size,
		.pCode = (const uint32_t *)shader_spirv_binary
	};

	VkShaderModule vertex_shader_module;
	VK_CHECK(vkCreateShaderModule(q_device->device, &vertex_shader_module_create_info, NULL, &vertex_shader_module));

	return vertex_shader_module;
}

void vk_destroy_shader_module(QVkDevice *q_device, VkShaderModule shader_module) {
	assert(q_device != NULL);
	assert(shader_module != NULL);

	vkDestroyShaderModule(q_device->device, shader_module, NULL);
}

QVkGraphicsPipeline vk_create_graphics_pipeline(QVkDevice *q_device, QVkRenderPass *render_pass, VkShaderModule vertex_shader, VkShaderModule fragment_shader) {
	VkDevice device = q_device->device;

	const char *shader_entry_point = "main";

	VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertex_shader,
		.pName = shader_entry_point,
		.pSpecializationInfo = NULL
	};

	VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragment_shader,
		.pName = shader_entry_point,
		.pSpecializationInfo = NULL
	};

	VkPipelineShaderStageCreateInfo shader_stage_create_infos[2] = {
		vertex_shader_stage_create_info,
		fragment_shader_stage_create_info
	};

	/*
	struct Vertex {
		Vector2_f32 position;
		Vector3_f32 color;
	}
	*/

	const uint32_t position_location = 0;
	const   size_t position_size     = sizeof(Vector2_f32);
	const uint32_t position_offset   = 0;

	const uint32_t color_location = 1;
	const   size_t color_size     = sizeof(Vector3_f32);
	const uint32_t color_offset   = position_offset + position_size;

	VkVertexInputBindingDescription vertex_binding_descriptions[] = {
		{
			.binding = 0,
			.stride = sizeof(Vertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		}
	};

	VkVertexInputAttributeDescription vertex_attribute_descriptions[] = {
		{ // position
			.location = position_location,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = position_offset
		},

		{ // color
			.location = color_location,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = color_offset
		}
	};

	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.vertexBindingDescriptionCount = ARRAY_SIZE(vertex_binding_descriptions),
		.pVertexBindingDescriptions = vertex_binding_descriptions,
		.vertexAttributeDescriptionCount = ARRAY_SIZE(vertex_attribute_descriptions),
		.pVertexAttributeDescriptions = vertex_attribute_descriptions
	};

	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = (float)render_pass->area.extent.width,
		.height = (float)render_pass->area.extent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	VkOffset2D scissor_offset = {
		.x = 0,
		.y = 0
	};

	VkRect2D scissor = {
		.offset = scissor_offset,
		.extent = render_pass->area.extent
	};

	VkPipelineViewportStateCreateInfo viewport_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor
	};

	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 0.0f,
		.lineWidth = 1.0f
	};

	VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.0f,
		.pSampleMask = NULL,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE
	};

	VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
		.blendEnable = VK_FALSE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
		                  VK_COLOR_COMPONENT_G_BIT |
		                  VK_COLOR_COMPONENT_B_BIT |
		                  VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &color_blend_attachment_state
	};

	for (int blend_constant_idx = 0; blend_constant_idx < 4; blend_constant_idx += 1) {
		color_blend_state_create_info.blendConstants[blend_constant_idx] = 0.0f;
	}

	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.setLayoutCount = 0,
		.pSetLayouts = NULL,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = NULL
	};

	VkPipelineLayout pipeline_layout;
	VK_CHECK(vkCreatePipelineLayout(device, &pipeline_layout_create_info, NULL, &pipeline_layout));

	VkGraphicsPipelineCreateInfo pipeline_create_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stageCount = 2,
		.pStages = shader_stage_create_infos,
		.pVertexInputState = &vertex_input_state_create_info,
		.pInputAssemblyState = &input_assembly_create_info,
		.pTessellationState = NULL,
		.pViewportState = &viewport_state_create_info,
		.pRasterizationState = &rasterization_state_create_info,
		.pMultisampleState = &multisample_state_create_info,
		.pDepthStencilState = NULL,
		.pColorBlendState = &color_blend_state_create_info,
		.pDynamicState = NULL,
		.layout = pipeline_layout,
		.renderPass = render_pass->render_pass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};

	QVkGraphicsPipeline graphics_pipeline = {
		.q_device = q_device,
		.pipeline = VK_NULL_HANDLE,
		.pipeline_layout = pipeline_layout,
		.vertex_shader = vertex_shader,
		.fragment_shader = fragment_shader,
		.viewport = viewport,
		.scissors = scissor
	};

	VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &graphics_pipeline.pipeline));

	return graphics_pipeline;
}

QVkFramebuffersList vk_create_framebuffers(QVkDevice *q_device, QVkRenderPass *q_render_pass, QVkSwapchain *q_swapchain) {
	assert(q_device != NULL);
	assert(q_render_pass != NULL);
	assert(q_swapchain != NULL);

	QVkFramebuffersList framebuffers_list = {
		.q_device = q_device,
		.q_render_pass = q_render_pass,
		.framebuffers_count = q_swapchain->images_count,
		.framebuffers = NULL,
	};

	VkFramebufferCreateInfo *framebuffer_create_infos = (VkFramebufferCreateInfo *) calloc(framebuffers_list.framebuffers_count, sizeof(VkFramebufferCreateInfo));
	framebuffers_list.framebuffers = (VkFramebuffer *) calloc(framebuffers_list.framebuffers_count, sizeof(VkFramebuffer));

	for (uint32_t framebuffer_idx = 0; framebuffer_idx < framebuffers_list.framebuffers_count; framebuffer_idx += 1) {
		VkFramebufferCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.renderPass = q_render_pass->render_pass,
			.attachmentCount = 1,
			.pAttachments = &q_swapchain->image_views[framebuffer_idx],
			.width = q_render_pass->area.extent.width,
			.height = q_render_pass->area.extent.height,
			.layers = 1
		};
		framebuffer_create_infos[framebuffer_idx] = info;

		VK_CHECK(vkCreateFramebuffer(q_device->device, &framebuffer_create_infos[framebuffer_idx], NULL, &framebuffers_list.framebuffers[framebuffer_idx]));
		printf("Framebuffer #%d created.\n", framebuffer_idx);
	}

	free(framebuffer_create_infos);

	return framebuffers_list;
}

QVkCommandBuffersList vk_create_command_buffers(QVkDevice *q_device, QVkQueueFamily queue_family, uint32_t swapchain_images_count) {
	assert(q_device != NULL);
	assert(queue_family.index != UINT32_MAX);
	assert(swapchain_images_count != UINT32_MAX);

	QVkCommandBuffersList buffers_list = {
		.q_device = q_device,
		.queue_family = queue_family,
		.command_pool = VK_NULL_HANDLE,
		.command_buffers_count = swapchain_images_count,
		.command_buffers = NULL
	};

	VkCommandPoolCreateInfo command_pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.queueFamilyIndex = queue_family.index
	};

	VK_CHECK(vkCreateCommandPool(q_device->device, &command_pool_create_info, NULL, &buffers_list.command_pool));

	VkCommandBufferAllocateInfo command_buffer_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = buffers_list.command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = buffers_list.command_buffers_count
	};

	buffers_list.command_buffers = (VkCommandBuffer *) calloc(buffers_list.command_buffers_count, sizeof(VkCommandBuffer));
	VK_CHECK(vkAllocateCommandBuffers(q_device->device, &command_buffer_allocate_info, buffers_list.command_buffers));

	return buffers_list;
}

void vk_prepare_to_draw(QVkGraphicsPipeline *q_graphics_pipeline, QVkFramebuffersList *q_framebuffers, QVkCommandBuffersList *q_command_buffers) {
	assert(q_graphics_pipeline != NULL);
	assert(q_framebuffers != NULL);
	assert(q_command_buffers != NULL);

	QVkRenderPass *q_render_pass = q_framebuffers->q_render_pass;
	const uint32_t command_buffers_count = q_command_buffers->command_buffers_count;

	VkCommandBufferBeginInfo *command_buffer_begin_infos = (VkCommandBufferBeginInfo *) calloc(command_buffers_count, sizeof(VkCommandBufferBeginInfo));
	VkRenderPassBeginInfo *renderpass_begin_infos = (VkRenderPassBeginInfo *) calloc(command_buffers_count, sizeof(VkRenderPassBeginInfo));

	for (uint32_t command_buffer_idx = 0; command_buffer_idx < command_buffers_count; command_buffer_idx += 1) {
		VkCommandBuffer command_buffer = q_command_buffers->command_buffers[command_buffer_idx];
		VkCommandBufferBeginInfo command_info_data = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = NULL,
			.flags = 0,
			.pInheritanceInfo = NULL
		};
		command_buffer_begin_infos[command_buffer_idx] = command_info_data;

		VkRenderPassBeginInfo renderpass_info_data = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = NULL,
			.renderPass = q_render_pass->render_pass,
			.framebuffer = q_framebuffers->framebuffers[command_buffer_idx],
			.renderArea = q_render_pass->area,
			.clearValueCount = 1,
			.pClearValues = &q_render_pass->clear_value
		};
		renderpass_begin_infos[command_buffer_idx] = renderpass_info_data;

		VkCommandBufferBeginInfo *command_buffer_begin_info = &command_buffer_begin_infos[command_buffer_idx];
		VK_CHECK(vkBeginCommandBuffer(
			/* VkCommandBuffer           commandBuffer */ command_buffer,
			/* VkCommandBufferBeginInfo    *pBeginInfo */ command_buffer_begin_info
		));

		VkRenderPassBeginInfo *renderpass_begin_info = &renderpass_begin_infos[command_buffer_idx];
		vkCmdBeginRenderPass(
			/* VkCommandBuffer            commandBuffer */ command_buffer,
			/* VkRenderPassBeginInfo  *pRenderPassBegin */ renderpass_begin_info,
			/* VkSubpassContents               contents */ VK_SUBPASS_CONTENTS_INLINE
		);

		vkCmdBindPipeline(
			/* VkCommandBuffer          commandBuffer */ command_buffer,
			/* VkPipelineBindPoint  pipelineBindPoint */ VK_PIPELINE_BIND_POINT_GRAPHICS,
			/* VkPipeline                    pipeline */ q_graphics_pipeline->pipeline
		);

		const Vertex vertices[] = {
			{ .position = { -0.5f, -0.5f },  .color = { 1.0f, 0.0f, 0.0f } },
			{ .position = {  0.5f, -0.5f },  .color = { 0.0f, 1.0f, 0.0f } },
			{ .position = {  0.5f,  0.5f },  .color = { 0.0f, 1.0f, 0.0f } },
			{ .position = { -0.5f,  0.5f },  .color = { 0.0f, 0.0f, 1.0f } }
		};

		const uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

		const uint64_t vertices_size = sizeof(vertices);
		const uint32_t vertices_count = ARRAY_SIZE(vertices);

		const uint64_t indices_size = sizeof(indices);
		const uint32_t indices_count = ARRAY_SIZE(indices);

		// TODO(nilsoncore): @DevicePointerRedundancy: Move q_device out of all the `QVk*` structs. - 18 February 2024
		QVkBuffer vertex_staging_buffer = vk_create_buffer(
			q_graphics_pipeline->q_device,
			vertices_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		vk_write_to_buffer(&vertex_staging_buffer, (void *)vertices, vertices_size);

		QVkBuffer vertex_buffer = vk_create_buffer(
			q_graphics_pipeline->q_device,
			vertices_size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		vk_copy_buffer_to(q_command_buffers->command_pool, &vertex_staging_buffer, &vertex_buffer, vertices_size);

		QVkBuffer index_staging_buffer = vk_create_buffer(
			q_graphics_pipeline->q_device,
			indices_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		vk_write_to_buffer(&index_staging_buffer, (void *)indices, indices_size);

		QVkBuffer index_buffer = vk_create_buffer(
			q_graphics_pipeline->q_device,
			indices_size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		vk_copy_buffer_to(q_command_buffers->command_pool, &index_staging_buffer, &index_buffer, indices_size);

		VkBuffer vertex_buffers[] = { vertex_buffer.buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(
			/* VkCommandBuffer  commandBuffer */ command_buffer,   // Command buffer into which the command is recorded.
			/* uint32_t          firstBinding */ 0,                // The index of the first vertex input binding whose state is updated by the command.
			/* uint32_t          bindingCount */ 1,                // The number of vertex input bindings whose state is updated by the command.
			/* VkBuffer             *pBuffers */ vertex_buffers,   // A pointer to an array of buffer handles.
			/* VkDeviceSize         *pOffsets */ offsets           // A pointer to an array of buffer offsets.
		);

		// Possible index types:
		// Provided by VK_VERSION_1_0
		// typedef enum VkIndexType {
		// 		VK_INDEX_TYPE_UINT16 = 0,
		// 		VK_INDEX_TYPE_UINT32 = 1,
		// 		// Provided by VK_KHR_acceleration_structure
		// 		VK_INDEX_TYPE_NONE_KHR = 1000165000, // no indices are provided
		// } VkIndexType;
		vkCmdBindIndexBuffer(
			/* VkCommandBuffer commandBuffer */ command_buffer,
			/* VkBuffer               buffer */ index_buffer.buffer,
			/* VkDeviceSize           offset */ 0,
			/* VkIndexType         indexType */ VK_INDEX_TYPE_UINT32
		);

		// Draw without index buffer:
		// vkCmdDraw(
		// 	/* VkCommandBuffer  commandBuffer */ command_buffer,   // Command buffer into which the command is recorded.
		// 	/* uint32_t           vertexCount */ vertices_count,   // The number of vertices to draw.
		// 	/* uint32_t         instanceCount */ 1,                // The number of instances to draw.
		// 	/* uint32_t           firstVertex */ 0,                // The index of the first vertex to draw.
		// 	/* uint32_t         firstInstance */ 0                 // The instance ID of the first instance to draw.
		// );

		// Draw with index buffer:
		vkCmdDrawIndexed(
			/* VkCommandBuffer commandBuffer */ command_buffer,
			/* uint32_t           indexCount */ indices_count,
			/* uint32_t        instanceCount */ 1,
			/* uint32_t           firstIndex */ 0,
			/* int32_t          vertexOffset */ 0,
			/* uint32_t        firstInstance */ 0
		);

		vkCmdEndRenderPass(command_buffer);
		VK_CHECK(vkEndCommandBuffer(command_buffer));

		vk_destroy_buffer(&vertex_staging_buffer);
		vk_destroy_buffer(&index_staging_buffer);
	}

	free(command_buffer_begin_infos);
	free(renderpass_begin_infos);
}

QVkSynchronizationPrimitivesList vk_create_synchronization_primitives(QVkDevice *q_device, uint32_t max_frames_in_fligt, uint32_t image_fences_count) {
	QVkSynchronizationPrimitivesList primitives_list = {
		.frames_count = max_frames_in_fligt,
		.image_acquire_semaphores = VK_NULL_HANDLE,
		.render_end_semaphores = VK_NULL_HANDLE,
		.submit_fences = VK_NULL_HANDLE,
		.image_fences_count = image_fences_count,
		.image_fences = VK_NULL_HANDLE
	};

	primitives_list.image_acquire_semaphores = (VkSemaphore *) calloc(primitives_list.frames_count, sizeof(VkSemaphore));
	primitives_list.render_end_semaphores = (VkSemaphore *) calloc(primitives_list.frames_count, sizeof(VkSemaphore));
	primitives_list.submit_fences = (VkFence *) calloc(primitives_list.frames_count, sizeof(VkFence));

	VkSemaphoreCreateInfo semaphore_create_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0
	};

	VkFenceCreateInfo fence_create_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = NULL,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	VkDevice device = q_device->device;

	for (uint32_t frame_idx = 0; frame_idx < primitives_list.frames_count; frame_idx += 1) {
		VK_CHECK(vkCreateSemaphore(device, &semaphore_create_info, NULL, &primitives_list.image_acquire_semaphores[frame_idx]));
		VK_CHECK(vkCreateSemaphore(device, &semaphore_create_info, NULL, &primitives_list.render_end_semaphores[frame_idx]));
		VK_CHECK(vkCreateFence(device, &fence_create_info, NULL, &primitives_list.submit_fences[frame_idx]));
	}

	primitives_list.image_fences = (VkFence *) calloc(primitives_list.image_fences_count, sizeof(VkFence));
	for (uint32_t image_idx = 0; image_idx < primitives_list.image_fences_count; image_idx += 1) {
		primitives_list.image_fences[image_idx] = VK_NULL_HANDLE;
	}

	return primitives_list;
}

QVkBuffer vk_create_vertex_buffer(QVkDevice *q_device, uint64_t buffer_size) {
	VkBufferCreateInfo buffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = buffer_size,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	QVkBuffer vk_buffer = {
		.q_device = q_device,
		.buffer = NULL,
		.device_memory = NULL,
		.requirements = { },
		.memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	VK_CHECK(vkCreateBuffer(vk_buffer.q_device->device, &buffer_create_info, VK_NULL_HANDLE, &vk_buffer.buffer));
	vkGetBufferMemoryRequirements(vk_buffer.q_device->device, vk_buffer.buffer, &vk_buffer.requirements);

	VkPhysicalDeviceMemoryProperties *memory_properties = &q_device->q_physical_device->memory_properties;
	uint32_t buffer_memory_type = vk_find_suitable_memory_type(
		memory_properties,
		vk_buffer.requirements.memoryTypeBits,
		vk_buffer.memory_flags
	);
	assert((buffer_memory_type != QVK_INVALID_MEMORY_TYPE) && "Suitable memory type not found");

	VkMemoryAllocateInfo memory_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = vk_buffer.requirements.size,
		.memoryTypeIndex = buffer_memory_type
	};

	VK_CHECK(vkAllocateMemory(
		/* VkDevice                     device */ vk_buffer.q_device->device,
		/* VkMemoryAllocateInfo *pAllocateInfo */ &memory_allocate_info,
		/* VkAllocationCallbacks   *pAllocator */ NULL,
		/* VkDeviceMemory             *pMemory */ &vk_buffer.device_memory
	));

	VK_CHECK(vkBindBufferMemory(
		/* VkDevice             device */ vk_buffer.q_device->device,
		/* VkBuffer             buffer */ vk_buffer.buffer,
		/* VkDeviceMemory       memory */ vk_buffer.device_memory,
		/* VkDeviceSize   memoryOffset */ 0
	));

	return vk_buffer;
}

QVkBuffer vk_create_buffer(QVkDevice *q_device, uint64_t buffer_size, VkBufferUsageFlags buffer_usage, VkMemoryPropertyFlags memory_properties) {
	VkBufferCreateInfo buffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = buffer_size,
		.usage = buffer_usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	QVkBuffer vk_buffer = {
		.q_device = q_device,
		.buffer = NULL,
		.device_memory = NULL,
		.requirements = { },
		.memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	VK_CHECK(vkCreateBuffer(vk_buffer.q_device->device, &buffer_create_info, VK_NULL_HANDLE, &vk_buffer.buffer));
	vkGetBufferMemoryRequirements(vk_buffer.q_device->device, vk_buffer.buffer, &vk_buffer.requirements);

	VkPhysicalDeviceMemoryProperties *device_memory_properties = &q_device->q_physical_device->memory_properties;
	uint32_t buffer_memory_type = vk_find_suitable_memory_type(
		device_memory_properties,
		vk_buffer.requirements.memoryTypeBits,
		vk_buffer.memory_flags
	);
	assert((buffer_memory_type != QVK_INVALID_MEMORY_TYPE) && "Suitable memory type not found");

	VkMemoryAllocateInfo memory_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = vk_buffer.requirements.size,
		.memoryTypeIndex = buffer_memory_type
	};

	VK_CHECK(vkAllocateMemory(
		/* VkDevice                     device */ vk_buffer.q_device->device,
		/* VkMemoryAllocateInfo *pAllocateInfo */ &memory_allocate_info,
		/* VkAllocationCallbacks   *pAllocator */ NULL,
		/* VkDeviceMemory             *pMemory */ &vk_buffer.device_memory
	));

	VK_CHECK(vkBindBufferMemory(
		/* VkDevice             device */ vk_buffer.q_device->device,
		/* VkBuffer             buffer */ vk_buffer.buffer,
		/* VkDeviceMemory       memory */ vk_buffer.device_memory,
		/* VkDeviceSize   memoryOffset */ 0
	));

	return vk_buffer;
}

void vk_destroy_buffer(QVkBuffer *buffer) {
	vkDestroyBuffer(buffer->q_device->device, buffer->buffer, NULL);
	vkFreeMemory(buffer->q_device->device, buffer->device_memory, NULL);
}

void vk_write_to_buffer(QVkBuffer *buffer, void *write_data, uint64_t write_data_size) {
	assert(buffer);
	assert(write_data);
	assert(write_data_size > 0 && "Why would you write 0 bytes of data?");

	assert((buffer->memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && "Can't write to non-host-visible memory");

	const VkDeviceSize buffer_size = buffer->requirements.size;
	void *buffer_mapped_data;
	vkMapMemory(
		/* VkDevice           device */ buffer->q_device->device,
		/* VkDeviceMemory     memory */ buffer->device_memory,
		/* VkDeviceSize       offset */ 0,
		/* VkDeviceSize         size */ buffer_size,
		/* VkMemoryMapFlags    flags */ 0,
		/* void             **ppData */ &buffer_mapped_data
	);

	uint64_t bytes_to_be_written = (write_data_size > buffer_size) ? buffer_size : write_data_size;
	if (bytes_to_be_written < write_data_size) {
		printf("WARNING: Tried to write %llu bytes to %p Vulkan buffer, but its' size is only %llu. Only %llu bytes will be written.\n",
			write_data_size, buffer->buffer, buffer_size, bytes_to_be_written);
	}

	memcpy(
		/* void   *destination */ buffer_mapped_data,
		/* void        *source */ write_data,
		/* size_t         size */ bytes_to_be_written
	);

	if ((buffer->memory_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) { // Not host coherent memory
		printf("Vulkan buffer %p memory is not Host Coherent, performing mapped memory flush.\n",
			buffer->buffer);

		VkMappedMemoryRange mapped_ranges[] = {
			{
				.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
				.pNext = NULL,
				.memory = buffer->device_memory,
				.offset = 0,
				.size = buffer_size
			}
		};

		vkFlushMappedMemoryRanges(
			/* VkDevice                     device */ buffer->q_device->device,
			/* uint32_t           memoryRangeCount */ ARRAY_SIZE(mapped_ranges),
			/* VkMappedMemoryRange  *pMemoryRanges */ mapped_ranges
		);
	}

	vkUnmapMemory(
		/* VkDevice       device */ buffer->q_device->device,
		/* VkDeviceMemory memory */ buffer->device_memory
	);
}

void vk_copy_buffer_to(VkCommandPool command_pool, QVkBuffer *source_buffer, QVkBuffer *destination_buffer, uint64_t copy_size) {
	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	VkCommandBuffer command_buffer = { };
	VK_CHECK(vkAllocateCommandBuffers(source_buffer->q_device->device, &allocate_info, &command_buffer));

	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = NULL
	};
	VK_CHECK(vkBeginCommandBuffer(command_buffer, &begin_info));

	VkBufferCopy copy_regions[] = {
		{
			.srcOffset = 0,
			.dstOffset = 0,
			.size = copy_size
		}
	};
	vkCmdCopyBuffer(
		/* VkCommandBuffer commandBuffer */ command_buffer,
		/* VkBuffer            srcBuffer */ source_buffer->buffer,
		/* VkBuffer            dstBuffer */ destination_buffer->buffer,
		/* uint32_t          regionCount */ ARRAY_SIZE(copy_regions),
		/* VkBufferCopy*        pRegions */ copy_regions
	);

	VK_CHECK(vkEndCommandBuffer(command_buffer));

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer
	};

	// Graphics Queue (and Compute Queue) implicitly support Transfer Queue,
	// but it is probably a good idea to create an explicitly separate Transfer Queue.
	VK_CHECK(vkQueueSubmit(
		/* VkQueue          queue */ source_buffer->q_device->graphics_queue,
		/* uint32_t   submitCount */ 1,
		/* VkSubmitInfo* pSubmits */ &submit_info,
		/* VkFence          fence */ NULL
	));

	// TODO(nilsoncore): Use fences to schedule multiple transfers simultaneously.
	VK_CHECK(vkQueueWaitIdle(source_buffer->q_device->graphics_queue));

	vkFreeCommandBuffers(source_buffer->q_device->device, command_pool, 1, &command_buffer);
}

#define PRINT_VKMEMHEAPFLAGS_FMT "%c%c"

#define PRINT_VKMEMHEAPFLAGS_ARG(flags) \
	(flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT  ) ? 'D' : '-', \
	(flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) ? 'M' : '-'

#define PRINT_VKMEMPROPFLAGS_FMT "%c%c%c%c%c%c"

#define PRINT_VKMEMPROPFLAGS_ARG(flags) \
	(flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT    ) ? 'D' : '-', \
	(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT    ) ? 'V' : '-', \
	(flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT   ) ? 'C' : '-', \
	(flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT     ) ? '$' : '-', \
	(flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) ? 'L' : '-', \
	(flags & VK_MEMORY_PROPERTY_PROTECTED_BIT       ) ? 'P' : '-'

void vk_print_physical_device_memory_properties(QVkPhysicalDevice *physical_device) {
	const VkPhysicalDeviceMemoryProperties memory_properties = physical_device->memory_properties;

	printf("Memory heaps: %d\n", memory_properties.memoryHeapCount);
	for (uint32_t memory_heap_idx = 0; memory_heap_idx < memory_properties.memoryHeapCount; memory_heap_idx += 1) {
		const VkMemoryHeap memory_heap = memory_properties.memoryHeaps[memory_heap_idx];
		printf("    [%u] - flags: " PRINT_VKMEMHEAPFLAGS_FMT ", size: %zu\n",
			memory_heap_idx, PRINT_VKMEMHEAPFLAGS_ARG(memory_heap.flags), memory_heap.size);
	}

	printf("Memory types: %d\n", memory_properties.memoryTypeCount);
	for (uint32_t memory_type_idx = 0; memory_type_idx < memory_properties.memoryTypeCount; memory_type_idx += 1) {
		const VkMemoryType memory_type = memory_properties.memoryTypes[memory_type_idx];
		printf("    [%u] - flags: " PRINT_VKMEMPROPFLAGS_FMT ", heap: [%u]\n",
			memory_type_idx, PRINT_VKMEMPROPFLAGS_ARG(memory_type.propertyFlags), memory_type.heapIndex);
	}
}

uint32_t vk_find_suitable_memory_type(VkPhysicalDeviceMemoryProperties *memory_properties, uint32_t required_memory_type_bits, VkMemoryPropertyFlags required_properties) {
	for (uint32_t memory_type_index = 0; memory_type_index < memory_properties->memoryTypeCount; memory_type_index += 1) {
		const int32_t memory_type_bit = (1 << memory_type_index);
		const bool is_required_memory_type = (memory_type_bit & required_memory_type_bits);

		const VkMemoryPropertyFlags memory_type_properties = memory_properties->memoryTypes[memory_type_index].propertyFlags;
		const bool has_required_properties = ((memory_type_properties & required_properties) == required_properties);

		if (is_required_memory_type && has_required_properties) {
			return memory_type_index;
		}
	}

	return QVK_INVALID_MEMORY_TYPE;
}

const char *vk_get_vendor_name(uint32_t vendor_id) {
	switch (vendor_id) {
		case 0x1002:  return "AMD";
		case 0x1010:  return "ImgTec";
		case 0x10DE:  return "NVIDIA";
		case 0x13B5:  return "ARM";
		case 0x5143:  return "Qualcomm";
		case 0x8086:  return "INTEL";

		default:      return "(unknown)";
	}
}

const char *vk_get_physical_device_type_name(VkPhysicalDeviceType device_type) {
	switch (device_type) {
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:    return "Discrete GPU";
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:  return "Integrated GPU";
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:     return "Virtual GPU";
		case VK_PHYSICAL_DEVICE_TYPE_CPU:             return "CPU";

		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
		default:                                      return "(unknown)";
	}
}

const char *vk_get_present_mode_name(VkPresentModeKHR present_mode) {
	switch (present_mode) {
		case VK_PRESENT_MODE_IMMEDIATE_KHR:     return "Immediate";
		case VK_PRESENT_MODE_MAILBOX_KHR:       return "Mailbox";
		case VK_PRESENT_MODE_FIFO_KHR:          return "FIFO";
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:  return "FIFO Relaxed";

		default:                                return "(unknown)";
	}
}