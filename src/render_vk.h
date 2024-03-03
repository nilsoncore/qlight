#ifndef QLIGHT_RENDER_VK_H
#define QLIGHT_RENDER_VK_H

#include <vulkan/vulkan.h>

#include "math.h"

struct QVkPhysicalDevice {
	VkInstance instance;
	VkPhysicalDevice device;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceMemoryProperties memory_properties;
	VkPhysicalDeviceFeatures features;
	VkDeviceSize memory_total;
};

struct QVkQueueFamily {
	QVkPhysicalDevice *q_physical_device;
	uint32_t index;
	VkQueueFamilyProperties *properties;
};

struct QVkQueueFamilyPropertiesList {
	QVkPhysicalDevice *q_physical_device;
	uint32_t properties_count;
	VkQueueFamilyProperties *properties;
};

struct QVkPhysicalDeviceList {
	uint32_t count;
	QVkPhysicalDevice *q_devices;
};

struct QVkDevice {
	QVkPhysicalDevice *q_physical_device;
	VkDevice device;
	VkQueue graphics_queue;
	VkQueue present_queue;
	bool single_queue;
};

struct QVkSurface {
	VkSurfaceKHR surface;
	QVkQueueFamily queue_family;
	VkBool32 supported;
	VkSurfaceCapabilitiesKHR capabilities;
	VkExtent2D window_framebuffer_size;
	uint32_t formats_count;
	VkSurfaceFormatKHR *formats;
	uint32_t present_modes_count;
	VkPresentModeKHR *present_modes;
	uint32_t supported_present_modes_flags;
};

struct QVkSwapchain {
	QVkSurface *q_surface;
	VkSwapchainKHR swapchain;
	VkSwapchainCreateInfoKHR info;
	uint32_t images_count;
	VkImage *images;
	VkImageView *image_views;
};

struct QVkRenderPass {
	QVkDevice *q_device;
	VkRenderPass render_pass;
	VkRect2D area;
	VkClearValue clear_value;
};

struct QVkGraphicsPipeline {
	QVkDevice *q_device;
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
	VkShaderModule vertex_shader;
	VkShaderModule fragment_shader;
	VkViewport viewport;
	VkRect2D scissors;
};

struct QVkFramebuffersList {
	QVkDevice *q_device;
	QVkRenderPass *q_render_pass;
	uint32_t framebuffers_count;
	VkFramebuffer *framebuffers;
};

struct QVkCommandBuffersList {
	QVkDevice *q_device;
	QVkQueueFamily queue_family;
	VkCommandPool command_pool;
	uint32_t command_buffers_count;
	VkCommandBuffer *command_buffers;
};

struct QVkSynchronizationPrimitivesList {
	uint32_t frames_count;
	VkSemaphore *image_acquire_semaphores;
	VkSemaphore *render_end_semaphores;
	VkFence *submit_fences;
	uint32_t image_fences_count;
	VkFence *image_fences;
};

// TODO?
struct QRenderVk {
	VkInstance instance;
	QVkQueueFamilyPropertiesList queue_families;
	QVkQueueFamily selected_queue_family;
	QVkPhysicalDeviceList physical_devices;
	QVkDevice device;
	QVkSurface surface;
	QVkSwapchain swapchain;
	QVkRenderPass render_pass;
	QVkGraphicsPipeline graphics_pipeline;
	QVkFramebuffersList framebuffers;
	QVkCommandBuffersList command_buffers;
	uint32_t max_frames_in_fligt;
};

struct Vertex {
	Vector2_f32 position;
	Vector3_f32 color;
};

const char *vk_get_vendor_name(uint32_t vendor_id);
const char *vk_get_physical_device_type_name(VkPhysicalDeviceType device_type);
const char *vk_get_present_mode_name(VkPresentModeKHR present_mode);

// void vk_init();

VkInstance vk_create_instance(const char *app_name, uint32_t app_version);
QVkPhysicalDeviceList vk_get_physical_devices(VkInstance instance);
QVkPhysicalDevice *vk_select_best_physical_device(QVkPhysicalDeviceList list);
QVkQueueFamilyPropertiesList vk_get_physical_device_queue_families(QVkPhysicalDevice *q_physical_device);
QVkQueueFamily vk_select_best_queue_family(QVkQueueFamilyPropertiesList families_list);
QVkDevice vk_create_device(QVkQueueFamilyPropertiesList families_list, QVkQueueFamily selected_family);
QVkSurface vk_get_glfw_window_surface(QVkQueueFamily queue_family, GLFWwindow *window);
QVkSwapchain vk_create_swapchain(QVkDevice *q_device, QVkSurface *surface, uint32_t surface_format_index, bool force_single_queue, VkPresentModeKHR preferred_present_mode);
QVkRenderPass vk_create_render_pass(QVkDevice *q_device, VkFormat image_view_format, VkExtent2D render_area_size);
VkShaderModule vk_create_shader_module(QVkDevice *q_device, const char *shader_spirv_binary, size_t shader_size);
void vk_destroy_shader_module(QVkDevice *q_device, VkShaderModule shader_module);
QVkGraphicsPipeline vk_create_graphics_pipeline(QVkDevice *q_device, QVkRenderPass *render_pass, VkShaderModule vertex_shader, VkShaderModule fragment_shader);
QVkFramebuffersList vk_create_framebuffers(QVkDevice *q_device, QVkRenderPass *q_render_pass, QVkSwapchain *q_swapchain);
QVkCommandBuffersList vk_create_command_buffers(QVkDevice *q_device, QVkQueueFamily queue_family, uint32_t swapchain_images_count);
void vk_prepare_to_draw(QVkGraphicsPipeline *q_graphics_pipeline, QVkFramebuffersList *q_framebuffers, QVkCommandBuffersList *q_command_buffers);
QVkSynchronizationPrimitivesList vk_create_synchronization_primitives(QVkDevice *q_device, uint32_t max_frames_in_fligt, uint32_t image_fences_count);

struct QVkBuffer {
	QVkDevice *q_device;
	VkBuffer buffer;
	VkDeviceMemory device_memory;
	VkMemoryRequirements requirements;
	VkMemoryPropertyFlags memory_flags;
};

QVkBuffer vk_create_vertex_buffer(QVkDevice *q_device, uint64_t buffer_size);
QVkBuffer vk_create_buffer(QVkDevice *q_device, uint64_t buffer_size, VkBufferUsageFlags buffer_usage, VkMemoryPropertyFlags memory_properties);
void vk_destroy_buffer(QVkBuffer *buffer);

#define QVK_INVALID_MEMORY_TYPE (uint32_t)0xFFFFFFF

void vk_print_physical_device_memory_properties(QVkPhysicalDevice *physical_device);
uint32_t vk_find_suitable_memory_type(VkPhysicalDeviceMemoryProperties *memory_properties, uint32_t required_memory_type_bits, VkMemoryPropertyFlags required_properties);

void vk_write_to_buffer(QVkBuffer *buffer, void *write_data, uint64_t write_data_size);
void vk_copy_buffer_to(VkCommandPool command_pool, QVkBuffer *source_buffer, QVkBuffer *destination_buffer, uint64_t copy_size);

#endif /* QLIGHT_RENDER_VK_H */