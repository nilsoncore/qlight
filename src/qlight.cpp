#define _CRT_SECURE_NO_WARNINGS

// #define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "qlight.h"
#include "render_vk.h"

// glm::perspective, glm::lookAt
#include <GLM/gtc/matrix_transform.hpp>

#define ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])

// Sleep()
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOGDI
// #include <Windows.h>
#undef max
#undef min

#pragma warning ( disable : 26812 )

#include "allocator.h"
#include "math.h"

#define RUN_TESTS

#ifdef RUN_TESTS
#include "../tests/tests.cpp"
#endif

int main(int arguments_count, char *arguments[]) {

#ifdef RUN_TESTS
	run_tests();
#endif

	glfwInit();

	////////////////////
	//      NEW
	////////////////////

#if 1

	VkInstance vk_instance = vk_create_instance(QLIGHT_APP_NAME, QLIGHT_APP_VERSION);
	QVkPhysicalDeviceList phys_devices = vk_get_physical_devices(vk_instance);
	QVkPhysicalDevice *selected_phys_device = vk_select_best_physical_device(phys_devices);
	QVkQueueFamilyPropertiesList queue_families = vk_get_physical_device_queue_families(selected_phys_device);
	QVkQueueFamily queue_family = vk_select_best_queue_family(queue_families);
	QVkDevice q_device = vk_create_device(queue_families, queue_family);

	vk_print_physical_device_memory_properties(selected_phys_device);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	int window_width = 860;
	int window_height = 720;
	const char *window_name = QLIGHT_APP_NAME;
	GLFWwindow *window = glfwCreateWindow(window_width, window_height, window_name, NULL, NULL);

	QVkSurface q_surface = vk_get_glfw_window_surface(queue_family, window);
	QVkSwapchain q_swapchain = vk_create_swapchain(&q_device, &q_surface, 0, false, VK_PRESENT_MODE_MAILBOX_KHR);
	QVkRenderPass q_render_pass = vk_create_render_pass(&q_device, q_swapchain.info.imageFormat, q_swapchain.q_surface->window_framebuffer_size);

	FILE *vertex_shader_file = fopen("resources/shaders/pbr_vert.spv", "rb+");
	FILE *fragment_shader_file = fopen("resources/shaders/pbr_frag.spv", "rb+");

	fseek(vertex_shader_file, 0, SEEK_END);
	fseek(fragment_shader_file, 0, SEEK_END);

	uint32_t vertex_shader_file_size = ftell(vertex_shader_file);
	uint32_t fragment_shader_file_size = ftell(fragment_shader_file);

	char *vertex_shader_binary = (char *) calloc(vertex_shader_file_size, sizeof(char));
	char *fragment_shader_binary = (char *) calloc(fragment_shader_file_size, sizeof(char));

	rewind(vertex_shader_file);
	rewind(fragment_shader_file);

	fread(vertex_shader_binary, 1, vertex_shader_file_size, vertex_shader_file);
	fread(fragment_shader_binary, 1, fragment_shader_file_size, fragment_shader_file);

	fclose(vertex_shader_file);
	fclose(fragment_shader_file);

	VkShaderModule vertex_shader_module = vk_create_shader_module(&q_device, vertex_shader_binary, vertex_shader_file_size);
	VkShaderModule fragment_shader_module = vk_create_shader_module(&q_device, fragment_shader_binary, fragment_shader_file_size);
	QVkGraphicsPipeline q_graphics_pipeline = vk_create_graphics_pipeline(&q_device, &q_render_pass, vertex_shader_module, fragment_shader_module);

	printf("Vulkan graphics pipeline created.\n");

	// Free shader resources
	vk_destroy_shader_module(&q_device, vertex_shader_module);
	vk_destroy_shader_module(&q_device, fragment_shader_module);

	QVkFramebuffersList q_framebuffers = vk_create_framebuffers(&q_device, &q_render_pass, &q_swapchain);
	QVkCommandBuffersList q_command_buffers = vk_create_command_buffers(&q_device, queue_family, q_swapchain.images_count);

	vk_prepare_to_draw(&q_graphics_pipeline, &q_framebuffers, &q_command_buffers);

	uint32_t max_frames_in_flight = 2;
	QVkSynchronizationPrimitivesList sync_primitives = vk_create_synchronization_primitives(&q_device, max_frames_in_flight, q_swapchain.images_count);

	uint32_t current_frame_idx = 0;

/*
	const Vertex vertices[] = {
		{ .position = {  0.0f, -0.5f },  .color = { 1.0f, 0.0f, 0.0f } },
		{ .position = {  0.5f,  0.5f },  .color = { 0.0f, 1.0f, 0.0f } },
		{ .position = { -0.5f,  0.5f },  .color = { 0.0f, 0.0f, 1.0f } }
	};

	const uint64_t vertices_size = sizeof(vertices);

	// QVkBuffer vertex_buffer = vk_create_vertex_buffer(&q_device, vertices_size);
	QVkBuffer vertex_buffer = vk_create_buffer(
		&q_device,
		vertices_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	vk_write_to_buffer(&vertex_buffer, (void *)vertices, vertices_size);
*/

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		/* Vulkan: submit */

		vkWaitForFences(q_device.device, 1, &sync_primitives.submit_fences[current_frame_idx], VK_TRUE, UINT64_MAX);

		uint32_t image_idx = 0;
		vkAcquireNextImageKHR(q_device.device, q_swapchain.swapchain, UINT64_MAX, sync_primitives.image_acquire_semaphores[current_frame_idx], VK_NULL_HANDLE, &image_idx);

		if (sync_primitives.image_fences[image_idx] != VK_NULL_HANDLE) {
			vkWaitForFences(q_device.device, 1, &sync_primitives.image_fences[image_idx], VK_TRUE, UINT64_MAX);
		}

		VkSemaphore wait_semaphores[1] =  { sync_primitives.image_acquire_semaphores[current_frame_idx] };
		VkPipelineStageFlags wait_stages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signal_semaphores[1] = { sync_primitives.render_end_semaphores[current_frame_idx] };

		VkSubmitInfo submit_info = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = NULL,
			.waitSemaphoreCount = ARRAY_SIZE(wait_semaphores),
			.pWaitSemaphores = wait_semaphores,
			.pWaitDstStageMask = wait_stages,
			.commandBufferCount = 1,
			.pCommandBuffers = &q_command_buffers.command_buffers[image_idx],
			.signalSemaphoreCount = ARRAY_SIZE(signal_semaphores),
			.pSignalSemaphores = signal_semaphores
		};

		vkResetFences(q_device.device, 1, &sync_primitives.submit_fences[current_frame_idx]);
		vkQueueSubmit(q_device.graphics_queue, 1, &submit_info, sync_primitives.submit_fences[current_frame_idx]);

		/* Vulkan: present */

		VkPresentInfoKHR present_info = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = NULL,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = signal_semaphores,
			.swapchainCount = 1,
			.pSwapchains = &q_swapchain.swapchain,
			.pImageIndices = &image_idx,
			.pResults = NULL
		};

		vkQueuePresentKHR(q_device.present_queue, &present_info);

		current_frame_idx = (current_frame_idx + 1) % max_frames_in_flight;
	}

	vkDeviceWaitIdle(q_device.device);

	/* Free Vulkan resources */

	/*
	vkFreeCommandBuffers(device, command_pool, swapchain_images_count, command_buffers);

	for (int frame_idx = 0; frame_idx < max_frames_in_flight; frame_idx += 1) {
		vkDestroySemaphore(device, image_acquire_semaphores[frame_idx], NULL);
		vkDestroySemaphore(device, render_finalize_semaphores[frame_idx], NULL);
		vkDestroyFence(device, fences[frame_idx], NULL);
	}

	vkDestroyCommandPool(device, command_pool, NULL);

	for (int framebuffer_idx = 0; framebuffer_idx < swapchain_images_count; framebuffer_idx += 1) {
		vkDestroyFramebuffer(device, framebuffers[framebuffer_idx], NULL);
	}

	vkDestroyPipeline(device, pipeline, NULL);
	vkDestroyPipelineLayout(device, pipeline_layout, NULL);
	vkDestroyRenderPass(device, renderpass, NULL);

	for (int image_idx = 0; image_idx < swapchain_images_count; image_idx += 1) {
		vkDestroyImageView(device, image_views[image_idx], NULL);
	}

	vkDestroySwapchainKHR(device, swapchain, NULL);
	vkDestroySurfaceKHR(instance, surface, NULL);

	glfwDestroyWindow(window);

	vkDestroyDevice(device, NULL);
	vkDestroyInstance(instance, NULL);
	*/

	glfwTerminate();

#else

	////////////////////
	//       OLD
	////////////////////

	/* Create instance */

	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = NULL,
		.pApplicationName = QLIGHT_APP_NAME,
		.applicationVersion = QLIGHT_APP_VERSION,
		.pEngineName = QLIGHT_ENGINE_NAME,
		.engineVersion = QLIGHT_ENGINE_VERSION,
		.apiVersion = VK_API_VERSION_1_2
	};

	// Layers
	const uint32_t instance_layer_count = 1;
	char instance_layers[instance_layer_count][VK_MAX_EXTENSION_NAME_SIZE];
	strcpy(instance_layers[0], "VK_LAYER_KHRONOS_validation");
	char *instance_layer_names[instance_layer_count];
	for (int layer_idx = 0; layer_idx < instance_layer_count; layer_idx += 1) {
		instance_layer_names[layer_idx] = instance_layers[layer_idx];
	}

	// Extensions
	uint32_t instance_required_extension_count = 0;
	const char **instance_extension_names = glfwGetRequiredInstanceExtensions(&instance_required_extension_count);

	VkInstanceCreateInfo instance_create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.pApplicationInfo = &app_info,
		.enabledLayerCount = instance_layer_count,
		.ppEnabledLayerNames = instance_layer_names,
		.enabledExtensionCount = instance_required_extension_count,
		.ppEnabledExtensionNames = instance_extension_names
	};

	// Now create instance
	VkInstance instance;
	vkCreateInstance(&instance_create_info, NULL, &instance);
	printf("Vulkan instance created.\n");

	// ?????????????????
	uint32_t vk_extensions_count = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &vk_extensions_count, NULL);

	/* Create device */

	// Get a list of available physical devices
	uint32_t physical_device_count = 0;
	vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL);

	if (physical_device_count < 1) {
		printf("Cannot create Vulkan instance because there is no GPU present in this system.");
		exit(1);
	}

	VkPhysicalDevice *physical_devices = (VkPhysicalDevice *) calloc(physical_device_count, sizeof(VkPhysicalDevice));
	vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices);

	// Select physical device
	VkPhysicalDeviceProperties* physical_device_properties = (VkPhysicalDeviceProperties *) calloc(physical_device_count, sizeof(VkPhysicalDeviceProperties));

	uint32_t *discrete_gpus = (uint32_t *) calloc(physical_device_count, sizeof(uint32_t));
	uint32_t discrete_gpu_count = 0;
	uint32_t *integrated_gpus = (uint32_t *) calloc(physical_device_count, sizeof(uint32_t));
	uint32_t integrated_gpu_count = 0;

	VkPhysicalDeviceMemoryProperties *physical_device_memory_properties = (VkPhysicalDeviceMemoryProperties *) calloc(physical_device_count, sizeof(VkPhysicalDeviceMemoryProperties));
	uint32_t *physical_device_memory_count = (uint32_t *) calloc(physical_device_count, sizeof(uint32_t));
	VkDeviceSize *physical_device_memory_total = (VkDeviceSize *) calloc(physical_device_count, sizeof(VkDeviceSize));

	printf("List of available GPUs in this system:\n");

	for (int device_idx = 0; device_idx < physical_device_count; device_idx += 1) {
		vkGetPhysicalDeviceProperties(physical_devices[device_idx], &physical_device_properties[device_idx]);
		VkPhysicalDeviceType device_type = physical_device_properties[device_idx].deviceType;

		const char *device_type_name;
		if (device_type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			discrete_gpus[discrete_gpu_count] = device_idx;
			discrete_gpu_count += 1;
			device_type_name = "dedicated";
		} else if (device_type == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
			integrated_gpus[integrated_gpu_count] = device_idx;
			integrated_gpu_count += 1;
			device_type_name = "integrated";
		} else {
			printf("WARNING: Device #%d for some reason has unknown device type %d.\n", device_idx, (int)device_type);
			device_type_name = "(unknown)";
		}

		vkGetPhysicalDeviceMemoryProperties(physical_devices[device_idx], &physical_device_memory_properties[device_idx]);
		physical_device_memory_count[device_idx] = physical_device_memory_properties[device_idx].memoryHeapCount;

		physical_device_memory_total[device_idx] = 0;
		for (int memory_heap_idx = 0; memory_heap_idx < physical_device_memory_count[device_idx]; memory_heap_idx += 1) {
			physical_device_memory_total[device_idx] += physical_device_memory_properties[device_idx].memoryHeaps[memory_heap_idx].size;
		}

		const char *vendor_name = vk_get_vendor_name(physical_device_properties[device_idx].vendorID);

		printf("Device #%d:\n", device_idx);
		printf("\tID: %x\n", physical_device_properties[device_idx].deviceID);
		printf("\tVendor: %s\n", vendor_name);
		printf("\tName: %s\n", physical_device_properties[device_idx].deviceName);
		printf("\tType: %s\n", device_type_name);
		printf("\tMemory heaps: %d\n", (int)physical_device_memory_count[device_idx]);
		printf("\tTotal memory: %llu\n", (uint64_t)physical_device_memory_total[device_idx]);
	}


	// Select best physical device (usually the most powerful one)
	VkDeviceSize max_memory_size = 0;
	uint32_t best_physical_device_idx = 0;

	if (discrete_gpu_count > 0) {

		// First, try to pick best discrete (dedicated) gpu
		for (int discrete_gpu_idx = 0; discrete_gpu_idx < discrete_gpu_count; discrete_gpu_idx += 1) {
			uint32_t device_idx = discrete_gpus[discrete_gpu_idx];
			if (physical_device_memory_total[device_idx] > max_memory_size) {
				max_memory_size = physical_device_memory_total[device_idx];
				best_physical_device_idx = device_idx;
			}
		}

	} else if (integrated_gpu_count > 0) {

		// If no discrete gpus present, pick one from integrated ones
		for (int integrated_gpu_idx = 0; integrated_gpu_idx < integrated_gpu_count; integrated_gpu_idx += 1) {
			uint32_t device_idx = discrete_gpus[integrated_gpu_idx];
			if (physical_device_memory_total[device_idx] > max_memory_size) {
				max_memory_size = physical_device_memory_total[device_idx];
				best_physical_device_idx = device_idx;
			}
		}

	}

	printf("Selected best available GPU device: #%d.\n", best_physical_device_idx);
	VkPhysicalDevice *selected_physical_device = &physical_devices[best_physical_device_idx];

	/* Query queue families */

	uint32_t queue_family_properties_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(*selected_physical_device, &queue_family_properties_count, NULL);
	VkQueueFamilyProperties *queue_family_properties = (VkQueueFamilyProperties *) calloc(queue_family_properties_count, sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(*selected_physical_device, &queue_family_properties_count, queue_family_properties);

	printf("Found %d queue families on physical device #%d:\n", queue_family_properties_count, best_physical_device_idx);

	uint32_t *queue_family_queue_count = (uint32_t *) calloc(queue_family_properties_count, sizeof(uint32_t));
	for (int queue_family_idx = 0; queue_family_idx < queue_family_properties_count; queue_family_idx += 1) {
		VkQueueFamilyProperties properties = queue_family_properties[queue_family_idx];
		queue_family_queue_count[queue_family_idx] = properties.queueCount;
		printf("\tFamily #%d: %d queue%c -", queue_family_idx, properties.queueCount, (properties.queueCount > 1) ? 's' : '\0');
		if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) printf(" GRAPHICS");
		if (properties.queueFlags & VK_QUEUE_COMPUTE_BIT) printf(" COMPUTE");
		printf("\n");
	}

	/* Create logical device */

	VkDeviceQueueCreateInfo *device_queue_create_infos = (VkDeviceQueueCreateInfo *) calloc(queue_family_properties_count, sizeof(VkDeviceQueueCreateInfo));
	float **queue_priorities = (float **) calloc(queue_family_properties_count, sizeof(float *));
	for (uint32_t queue_family_idx = 0; queue_family_idx < queue_family_properties_count; queue_family_idx += 1) {
		uint32_t queue_count = queue_family_queue_count[queue_family_idx];
		queue_priorities[queue_family_idx] = (float *) calloc(queue_count, sizeof(float));
		for (int queue_idx = 0; queue_idx < queue_count; queue_idx += 1) {
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

	printf("Using %d queue families.\n", queue_family_properties_count);

	const uint32_t device_extension_count = 1;
	char device_extensions[device_extension_count][VK_MAX_EXTENSION_NAME_SIZE];
	strcpy(device_extensions[0], "VK_KHR_swapchain");

	char *device_extension_names[device_extension_count];
	for (int extension_idx = 0; extension_idx < device_extension_count; extension_idx += 1) {
		device_extension_names[extension_idx] = device_extensions[extension_idx];
	}

	VkPhysicalDeviceFeatures physical_device_features;
	vkGetPhysicalDeviceFeatures(*selected_physical_device, &physical_device_features);

	VkDeviceCreateInfo device_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.queueCreateInfoCount = queue_family_properties_count,
		.pQueueCreateInfos = device_queue_create_infos,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = NULL,
		.enabledExtensionCount = 1,
		.ppEnabledExtensionNames = device_extension_names,
		.pEnabledFeatures = &physical_device_features
	};

	VkDevice device;
	vkCreateDevice(*selected_physical_device, &device_create_info, NULL, &device);
	printf("Vulkan logical device created.\n");

	/* Select best queue family */

	uint32_t queue_family_graphics_count = 0;
	uint32_t *queue_family_graphics = (uint32_t *) calloc(queue_family_properties_count, sizeof(uint32_t));
	for (int family_idx = 0; family_idx < queue_family_properties_count; family_idx += 1) {
		bool has_graphics_bit = queue_family_properties[family_idx].queueFlags & VK_QUEUE_GRAPHICS_BIT;
		if (has_graphics_bit) {
			queue_family_graphics[queue_family_graphics_count] = family_idx;
			queue_family_graphics_count += 1;
		}
	}

	uint32_t max_queue_count = 0;
	uint32_t selected_queue_family_idx = -1;
	for (int queue_idx = 0; queue_idx < queue_family_graphics_count; queue_idx += 1) {
		uint32_t graphics_queue_idx = queue_family_graphics[queue_idx];
		if (queue_family_properties[graphics_queue_idx].queueCount > max_queue_count) {
			max_queue_count = queue_family_properties[graphics_queue_idx].queueCount;
			selected_queue_family_idx = graphics_queue_idx;
		}
	}

	printf("Max queue count: %d\n", (int)max_queue_count);
	printf("Selected queue family index: %d\n", (int)selected_queue_family_idx);

	VkQueue graphics_queue;
	VkQueue present_queue;
	vkGetDeviceQueue(device, selected_queue_family_idx, 0, &graphics_queue);

	bool single_queue = queue_family_properties[selected_queue_family_idx].queueCount < 2;
	if (single_queue) {
		vkGetDeviceQueue(device, selected_queue_family_idx, 0, &present_queue);
		printf("Using single queue for rendering.\n");
	} else {
		vkGetDeviceQueue(device, selected_queue_family_idx, 1, &present_queue);
		printf("Using double queues for rendering.\n");
	}

	/* Create window and surface */

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	int window_width = 860;
	int window_height = 720;
	const char *window_name = QLIGHT_APP_NAME;
	GLFWwindow *window = glfwCreateWindow(window_width, window_height, window_name, NULL, NULL);

	printf("Created new window \"%s\" (%dx%d).\n", window_name, window_width, window_height);

	VkSurfaceKHR surface;
	glfwCreateWindowSurface(instance, window, NULL, &surface);

	printf("Created surface for window \"%s\".\n", window_name);

	/* Create swapchain and image view */

	// Verify surface support
	VkBool32 physical_device_surface_supported;
	vkGetPhysicalDeviceSurfaceSupportKHR(*selected_physical_device, selected_queue_family_idx, surface, &physical_device_surface_supported);
	if (physical_device_surface_supported) {
		printf("Surface is supported on selected physical device.\n");
	} else {
		printf("WARNING: Surface is not supported on selected physical device!\n");
	}

	VkSurfaceCapabilitiesKHR surface_capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*selected_physical_device, surface, &surface_capabilities);

	int framebuffer_width;
	int framebuffer_height;
	glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
	VkExtent2D extent {
		.width = (uint32_t)framebuffer_width,
		.height = (uint32_t)framebuffer_height
	};

	bool extent_suitable = surface_capabilities.currentExtent.width  == framebuffer_width
	                    && surface_capabilities.currentExtent.height == framebuffer_height;
	if (!extent_suitable) {
		printf("Extent size (%ux%u) does not match framebuffer size, resizing...\n", extent.width, extent.height);

		if      (extent.width > surface_capabilities.maxImageExtent.width)    extent.width  = surface_capabilities.maxImageExtent.width;
		else if (extent.width < surface_capabilities.minImageExtent.width)    extent.width  = surface_capabilities.minImageExtent.width;

		if      (extent.height > surface_capabilities.maxImageExtent.height)  extent.height = surface_capabilities.maxImageExtent.height;
		else if (extent.height < surface_capabilities.minImageExtent.height)  extent.height = surface_capabilities.minImageExtent.height;

		printf("Extent rezised (%ux%u).\n", extent.width, extent.height);
	}

	uint32_t surface_formats_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(*selected_physical_device, surface, &surface_formats_count, NULL);
	VkSurfaceFormatKHR *surface_formats = (VkSurfaceFormatKHR *) calloc(surface_formats_count, sizeof(VkSurfaceFormatKHR));
	vkGetPhysicalDeviceSurfaceFormatsKHR(*selected_physical_device, surface, &surface_formats_count, surface_formats);

	printf("Found %d surface formats on physical device #%d.\n", surface_formats_count, best_physical_device_idx);

	uint32_t surface_present_modes_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(*selected_physical_device, surface, &surface_present_modes_count, NULL);
	VkPresentModeKHR *surface_present_modes = (VkPresentModeKHR *) calloc(surface_present_modes_count, sizeof(VkPresentModeKHR));
	vkGetPhysicalDeviceSurfacePresentModesKHR(*selected_physical_device, surface, &surface_present_modes_count, surface_present_modes);

	printf("Found %d surface present modes on physical device #%d:\n", surface_present_modes_count, best_physical_device_idx);

	int32_t supported_surface_present_modes = 0;

	for (int present_mode_idx = 0; present_mode_idx < surface_present_modes_count; present_mode_idx += 1) {
		VkPresentModeKHR present_mode = surface_present_modes[present_mode_idx];
		const char *present_mode_name = vk_get_present_mode_name(present_mode);
		printf("\tPresent mode #%d - %s\n", present_mode_idx, present_mode_name);

		supported_surface_present_modes |= present_mode;
	}

	bool mailbox_supported = supported_surface_present_modes & VK_PRESENT_MODE_MAILBOX_KHR;
	if (mailbox_supported) {
		printf("Mailbox surface present mode is supported.\n");
	} else {
		printf("Mailbox surface present mode is not suppoerted.\n");
	}

	/* Create swapchain */

	uint32_t queue_family_indices[] = { 0, 1 };

	VkSwapchainCreateInfoKHR swapchain_create_info {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = NULL,
		.flags = 0,
		.surface = surface,
		.minImageCount = surface_capabilities.minImageCount + 1,
		.imageFormat = surface_formats[0].format,
		.imageColorSpace = surface_formats[0].colorSpace,
		.imageExtent = (extent_suitable) ? surface_capabilities.currentExtent : extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = (single_queue) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
		.queueFamilyIndexCount = (single_queue) ? 0 : (uint32_t)2,
		.pQueueFamilyIndices = (single_queue) ? NULL : queue_family_indices,
		.preTransform = surface_capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = (mailbox_supported) ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_FIFO_KHR,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE
	};

	VkSwapchainKHR swapchain;
	vkCreateSwapchainKHR(device, &swapchain_create_info, NULL, &swapchain);

	printf("Vulkan swapchain created.\n");

	uint32_t swapchain_images_count = 0;
	vkGetSwapchainImagesKHR(device, swapchain, &swapchain_images_count, NULL);
	VkImage *swapchain_images = (VkImage *) calloc(swapchain_images_count, sizeof(VkImage));
	vkGetSwapchainImagesKHR(device, swapchain, &swapchain_images_count, swapchain_images);

	printf("Fetched %d images from swapchain.\n", swapchain_images_count);

	/* Create image view */

	VkImageView *image_views = (VkImageView *) calloc(swapchain_images_count, sizeof(VkImageView));
	VkImageViewCreateInfo *image_view_create_infos = (VkImageViewCreateInfo *) calloc(swapchain_images_count, sizeof(VkImageViewCreateInfo));

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
		.layerCount = swapchain_create_info.imageArrayLayers
	};

	for (int image_view_idx = 0; image_view_idx < swapchain_images_count; image_view_idx += 1) {
		VkImageViewCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.image = swapchain_images[image_view_idx],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = surface_formats[0].format,
			.components = image_view_rgba_component,
			.subresourceRange = image_view_subresources
		};

		image_view_create_infos[image_view_idx] = info;

		vkCreateImageView(device, &image_view_create_infos[image_view_idx], NULL, &image_views[image_view_idx]);
		printf("Image view #%d created.\n", image_view_idx);
	}

	/* Create render pass */

	VkAttachmentDescription attachment_description = {
		.flags = 0,
		.format = swapchain_create_info.imageFormat,
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

	VkSubpassDescription subpass_description = {
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

	VkSubpassDependency subpass_dependency = {
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
		.pAttachments = &attachment_description,
		.subpassCount = 1,
		.pSubpasses = &subpass_description,
		.dependencyCount = 1,
		.pDependencies = &subpass_dependency
	};

	VkRenderPass renderpass;
	vkCreateRenderPass(device, &renderpass_create_info, NULL, &renderpass);

	printf("Vulkan render pass created.\n");

	/* Create pipeline */

	// Load shader
	FILE *vertex_shader_file = fopen("resources/shaders/pbr_vert.spv", "rb+");
	FILE *fragment_shader_file = fopen("resources/shaders/pbr_frag.spv", "rb+");

	fseek(vertex_shader_file, 0, SEEK_END);
	fseek(fragment_shader_file, 0, SEEK_END);

	uint32_t vertex_shader_file_size = ftell(vertex_shader_file);
	uint32_t fragment_shader_file_size = ftell(fragment_shader_file);

	char *vertex_shader = (char *) malloc(vertex_shader_file_size * sizeof(char));
	char *fragment_shader = (char *) malloc(fragment_shader_file_size * sizeof(char));

	rewind(vertex_shader_file);
	rewind(fragment_shader_file);

	fread(vertex_shader, 1, vertex_shader_file_size, vertex_shader_file);
	fread(fragment_shader, 1, fragment_shader_file_size, fragment_shader_file);

	fclose(vertex_shader_file);
	fclose(fragment_shader_file);

	VkShaderModuleCreateInfo vertex_shader_module_create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.codeSize = vertex_shader_file_size,
		.pCode = (const uint32_t *)vertex_shader
	};

	VkShaderModuleCreateInfo fragment_shader_module_create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.codeSize = fragment_shader_file_size,
		.pCode = (const uint32_t *)fragment_shader
	};

	VkShaderModule vertex_shader_module;
	vkCreateShaderModule(device, &vertex_shader_module_create_info, NULL, &vertex_shader_module);
	printf("Vertex shader module created.\n");

	VkShaderModule fragment_shader_module;
	vkCreateShaderModule(device, &fragment_shader_module_create_info, NULL, &fragment_shader_module);
	printf("Fragment shader module created.\n");

	char shader_entry_point[VK_MAX_EXTENSION_NAME_SIZE] = { "main" };

	VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertex_shader_module,
		.pName = shader_entry_point,
		.pSpecializationInfo = NULL
	};

	VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragment_shader_module,
		.pName = shader_entry_point,
		.pSpecializationInfo = NULL
	};

	VkPipelineShaderStageCreateInfo shader_stage_create_infos[2] = {
		vertex_shader_stage_create_info,
		fragment_shader_stage_create_info
	};

	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.vertexBindingDescriptionCount = 0,
		.pVertexBindingDescriptions = NULL,
		.vertexAttributeDescriptionCount = 0,
		.pVertexAttributeDescriptions = NULL
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
		.width = (float)swapchain_create_info.imageExtent.width,
		.height = (float)swapchain_create_info.imageExtent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	VkOffset2D scissor_offset = {
		.x = 0,
		.y = 0
	};

	VkRect2D scissor = {
		.offset = scissor_offset,
		.extent = swapchain_create_info.imageExtent
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
	vkCreatePipelineLayout(device, &pipeline_layout_create_info, NULL, &pipeline_layout);

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
		.renderPass = renderpass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};

	VkPipeline pipeline;
	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &pipeline);

	printf("Vulkan graphics pipeline created.\n");

	// Free shader resources
	vkDestroyShaderModule(device, vertex_shader_module, NULL);
	vkDestroyShaderModule(device, fragment_shader_module, NULL);
	free(vertex_shader);
	free(fragment_shader);

	/* Create framebuffer */

	VkFramebufferCreateInfo *framebuffer_create_infos = (VkFramebufferCreateInfo *) calloc(swapchain_images_count, sizeof(VkFramebufferCreateInfo));
	VkFramebuffer *framebuffers = (VkFramebuffer *) calloc(swapchain_images_count, sizeof(VkFramebuffer));
	VkImageView *image_attachments = (VkImageView *) calloc(swapchain_images_count, sizeof(VkImageView));

	for (int framebuffer_idx = 0; framebuffer_idx < swapchain_images_count; framebuffer_idx += 1) {
		image_attachments[framebuffer_idx] = image_views[framebuffer_idx];
		VkFramebufferCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.renderPass = renderpass,
			.attachmentCount = 1,
			.pAttachments = &image_attachments[framebuffer_idx],
			.width = swapchain_create_info.imageExtent.width,
			.height = swapchain_create_info.imageExtent.height,
			.layers = 1
		};
		framebuffer_create_infos[framebuffer_idx] = info;

		vkCreateFramebuffer(device, &framebuffer_create_infos[framebuffer_idx], NULL, &framebuffers[framebuffer_idx]);
		printf("Framebuffer #%d created.\n", framebuffer_idx);
	}

	/* Create command buffer */

	VkCommandPoolCreateInfo command_pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.queueFamilyIndex = selected_queue_family_idx
	};

	VkCommandPool command_pool;
	vkCreateCommandPool(device, &command_pool_create_info, NULL, &command_pool);
	printf("Vulkan command pool created.\n");

	VkCommandBufferAllocateInfo command_buffer_allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = swapchain_images_count
	};

	VkCommandBuffer *command_buffers = (VkCommandBuffer *) calloc(swapchain_images_count, sizeof(VkCommandBuffer));
	vkAllocateCommandBuffers(device, &command_buffer_allocate_info, command_buffers);
	printf("Vulkan command buffers allocated.\n");

	/* Prepare to render */

	VkCommandBufferBeginInfo *command_buffer_begin_infos = (VkCommandBufferBeginInfo *) calloc(swapchain_images_count, sizeof(VkCommandBufferBeginInfo));
	VkRenderPassBeginInfo *renderpass_begin_infos = (VkRenderPassBeginInfo *) calloc(swapchain_images_count, sizeof(VkRenderPassBeginInfo));

	VkOffset2D renderpass_area_offset = {
		.x = 0,
		.y = 0
	};

	VkRect2D renderpass_area = {
		.offset = renderpass_area_offset,
		.extent = swapchain_create_info.imageExtent
	};

	VkClearValue clear_value = { 0.0f, 0.0f, 0.0f, 0.0f };

	for (int buffer_idx = 0; buffer_idx < swapchain_images_count; buffer_idx += 1) {
		VkCommandBufferBeginInfo command_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = NULL,
			.flags = 0,
			.pInheritanceInfo = NULL
		};
		command_buffer_begin_infos[buffer_idx] = command_info;

		VkRenderPassBeginInfo renderpass_info = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = NULL,
			.renderPass = renderpass,
			.framebuffer = framebuffers[buffer_idx],
			.renderArea = renderpass_area,
			.clearValueCount = 1,
			.pClearValues = &clear_value
		};
		renderpass_begin_infos[buffer_idx] = renderpass_info;

		vkBeginCommandBuffer(command_buffers[buffer_idx], &command_buffer_begin_infos[buffer_idx]);
		vkCmdBeginRenderPass(command_buffers[buffer_idx], &renderpass_begin_infos[buffer_idx], VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(command_buffers[buffer_idx], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		vkCmdDraw(command_buffers[buffer_idx], 3, 1, 0, 0);

		vkCmdEndRenderPass(command_buffers[buffer_idx]);
		vkEndCommandBuffer(command_buffers[buffer_idx]);

		printf("Vulkan command buffer #%d drawing recorded.\n", buffer_idx);
	}

	/* Create semaphores and fences */

	uint32_t max_frames_in_flight = 2;
	VkSemaphore *image_acquire_semaphores = (VkSemaphore *) calloc(max_frames_in_flight, sizeof(VkSemaphore));
	VkSemaphore *render_finalize_semaphores = (VkSemaphore *) calloc(max_frames_in_flight, sizeof(VkSemaphore));
	VkFence *fences = (VkFence *) calloc(max_frames_in_flight, sizeof(VkFence));

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

	for (int frame_idx = 0; frame_idx < max_frames_in_flight; frame_idx += 1) {
		vkCreateSemaphore(device, &semaphore_create_info, NULL, &image_acquire_semaphores[frame_idx]);
		vkCreateSemaphore(device, &semaphore_create_info, NULL, &render_finalize_semaphores[frame_idx]);
		vkCreateFence(device, &fence_create_info, NULL, &fences[frame_idx]);
	}

	printf("Vulkan semaphores and fences created.\n");

	uint32_t current_frame_idx = 0;
	VkFence *image_fences = (VkFence *) calloc(swapchain_images_count, sizeof(VkFence));
	for (int image_idx = 0; image_idx < swapchain_images_count; image_idx += 1) {
		image_fences[image_idx] = VK_NULL_HANDLE;
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		/* Vulkan: submit */

		vkWaitForFences(device, 1, &fences[current_frame_idx], VK_TRUE, UINT64_MAX);

		uint32_t image_idx = 0;
		vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, image_acquire_semaphores[current_frame_idx], VK_NULL_HANDLE, &image_idx);

		if (image_fences[image_idx] != VK_NULL_HANDLE) {
			vkWaitForFences(device, 1, &image_fences[image_idx], VK_TRUE, UINT64_MAX);
		}

		VkSemaphore wait_semaphores[1] =  { image_acquire_semaphores[current_frame_idx] };
		VkPipelineStageFlags wait_stages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signal_semaphores[1] = { render_finalize_semaphores[current_frame_idx] };

		VkSubmitInfo submit_info = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = NULL,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = wait_semaphores,
			.pWaitDstStageMask = wait_stages,
			.commandBufferCount = 1,
			.pCommandBuffers = &command_buffers[image_idx],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = signal_semaphores
		};

		vkResetFences(device, 1, &fences[current_frame_idx]);
		vkQueueSubmit(graphics_queue, 1, &submit_info, fences[current_frame_idx]);

		/* Vulkan: present */

		VkPresentInfoKHR present_info = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = NULL,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = signal_semaphores,
			.swapchainCount = 1,
			.pSwapchains = &swapchain,
			.pImageIndices = &image_idx,
			.pResults = NULL
		};

		vkQueuePresentKHR(present_queue, &present_info);

		current_frame_idx = (current_frame_idx + 1) % max_frames_in_flight;
	}

	vkDeviceWaitIdle(device);

	/* Free Vulkan resources */

	vkFreeCommandBuffers(device, command_pool, swapchain_images_count, command_buffers);

	for (int frame_idx = 0; frame_idx < max_frames_in_flight; frame_idx += 1) {
		vkDestroySemaphore(device, image_acquire_semaphores[frame_idx], NULL);
		vkDestroySemaphore(device, render_finalize_semaphores[frame_idx], NULL);
		vkDestroyFence(device, fences[frame_idx], NULL);
	}

	vkDestroyCommandPool(device, command_pool, NULL);

	for (int framebuffer_idx = 0; framebuffer_idx < swapchain_images_count; framebuffer_idx += 1) {
		vkDestroyFramebuffer(device, framebuffers[framebuffer_idx], NULL);
	}

	vkDestroyPipeline(device, pipeline, NULL);
	vkDestroyPipelineLayout(device, pipeline_layout, NULL);
	vkDestroyRenderPass(device, renderpass, NULL);

	for (int image_idx = 0; image_idx < swapchain_images_count; image_idx += 1) {
		vkDestroyImageView(device, image_views[image_idx], NULL);
	}

	vkDestroySwapchainKHR(device, swapchain, NULL);
	vkDestroySurfaceKHR(instance, surface, NULL);

	glfwDestroyWindow(window);

	vkDestroyDevice(device, NULL);
	vkDestroyInstance(instance, NULL);

	glfwTerminate();

#endif /* OLD RENDER */

}