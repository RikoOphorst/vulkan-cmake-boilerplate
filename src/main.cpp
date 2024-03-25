#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include <shaderc/shaderc.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vector>
#include <array>

#define VK_CHECK(result) { auto r = (result); if (r != VK_SUCCESS) { __debugbreak(); } }

static VkBool32 DebugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT _MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT _MessageType, const VkDebugUtilsMessengerCallbackDataEXT* _Data, void* _UserData)
{
	if (_MessageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT ||
		_MessageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT ||
		_MessageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		return VK_FALSE;

	printf("%s\n", _Data->pMessage);

	return VK_FALSE;
}

int main(int argc, char** argv)
{
	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.apiVersion = VK_API_VERSION_1_3;
	appInfo.applicationVersion = 0;
	appInfo.engineVersion = 0;
	appInfo.pApplicationName = "blowbox";
	appInfo.pEngineName = "blowbox";

	VkValidationFeaturesEXT validationFeatures = { VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
	std::vector<VkValidationFeatureEnableEXT> validationEnabledFeatures;
	std::vector<VkValidationFeatureDisableEXT> validationDisabledFeatures;

	validationEnabledFeatures.push_back(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT);
	validationEnabledFeatures.push_back(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT);
	validationEnabledFeatures.push_back(VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT);
	validationDisabledFeatures.push_back(VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT);
	validationDisabledFeatures.push_back(VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT);

	validationFeatures.disabledValidationFeatureCount = validationDisabledFeatures.size();
	validationFeatures.enabledValidationFeatureCount = validationEnabledFeatures.size();

	validationFeatures.pDisabledValidationFeatures = validationDisabledFeatures.data();
	validationFeatures.pEnabledValidationFeatures = validationEnabledFeatures.data();

	const char* instanceExtensions[] = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	const char* instanceLayers[] = {
		"VK_LAYER_KHRONOS_validation"
	};

	VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instanceInfo.pNext = &validationFeatures;
	instanceInfo.enabledExtensionCount = _countof(instanceExtensions);
	instanceInfo.ppEnabledExtensionNames = instanceExtensions;
	instanceInfo.enabledLayerCount = _countof(instanceLayers);
	instanceInfo.ppEnabledLayerNames = instanceLayers;
	instanceInfo.pApplicationInfo = &appInfo;

	VkInstance instance = VK_NULL_HANDLE;
	VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &instance));

	VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };

	debugMessengerInfo.pfnUserCallback = (PFN_vkDebugUtilsMessengerCallbackEXT)DebugMessageCallback;
	debugMessengerInfo.pUserData = nullptr;
	debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	debugMessengerInfo.flags = 0;

	PFN_vkCreateDebugUtilsMessengerEXT pfnCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
	VK_CHECK(pfnCreateDebugUtilsMessenger(instance, &debugMessengerInfo, nullptr, &debugMessenger));

	uint32_t physicalDeviceCount = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));

	if (physicalDevices.empty())
	{
		printf("No physical devices found\n");
		return -1;
	}

	VkPhysicalDevice physicalDevice = physicalDevices[0];

	const char* requiredExtensions[] = {

		VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,

		VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,

		VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,

		VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,

		VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
		VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
		VK_KHR_RAY_QUERY_EXTENSION_NAME,

		VK_KHR_MAINTENANCE3_EXTENSION_NAME,
		VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME,

		VK_KHR_SWAPCHAIN_EXTENSION_NAME,

		VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
		//VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
		VK_KHR_SPIRV_1_4_EXTENSION_NAME,
		VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
		VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME
	};

	VkDeviceCreateInfo deviceInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };

	deviceInfo.enabledExtensionCount = _countof(requiredExtensions);
	deviceInfo.ppEnabledExtensionNames = requiredExtensions;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties2> queueFamilies(queueFamilyCount, { VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 });
	vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, queueFamilies.data());

	auto GetQueueFamilyIndex = [&](VkQueueFlags _Flags, VkQueueFlags _IgnoredFlags) {
		for (uint32_t i = 0; i < queueFamilies.size(); i++)
		{
			VkQueueFlags queueFamilyFlags = queueFamilies[i].queueFamilyProperties.queueFlags & ~_IgnoredFlags;

			if (queueFamilyFlags == _Flags)
				return i;
		}

		return ~0u;
		};

	uint32_t uniqueQueueFamilies[3] = { ~0u, ~0u, ~0u };

	enum QueueType : uint32_t {
		QueueType_Graphics = 0,
		QueueType_Compute = 1,
		QueueType_Transfer = 2
	};

	uniqueQueueFamilies[QueueType_Graphics] = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_PROTECTED_BIT);
	uniqueQueueFamilies[QueueType_Compute] = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_PROTECTED_BIT);
	uniqueQueueFamilies[QueueType_Transfer] = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT, VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_PROTECTED_BIT);

	if (uniqueQueueFamilies[QueueType_Graphics] == ~0u)
	{
		printf("No graphics capable queue found.\n");
		return -1;
	}

	if (uniqueQueueFamilies[QueueType_Compute] == ~0u)
	{
		printf("No compute capable queue found.\n");
		return -1;
	}

	if (uniqueQueueFamilies[QueueType_Transfer] == ~0u)
	{
		printf("No transfer capable queue found.");
		return -1;
	}

	float one = 1.0f;

	VkDeviceQueueCreateInfo queueInfos[3] = {};
	queueInfos[0] = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueInfos[0].queueFamilyIndex = uniqueQueueFamilies[QueueType_Graphics];
	queueInfos[0].queueCount = 1;
	queueInfos[0].pQueuePriorities = &one;

	queueInfos[1] = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueInfos[1].queueFamilyIndex = uniqueQueueFamilies[QueueType_Compute];
	queueInfos[1].queueCount = 1;
	queueInfos[1].pQueuePriorities = &one;

	queueInfos[2] = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueInfos[2].queueFamilyIndex = uniqueQueueFamilies[QueueType_Transfer];
	queueInfos[2].queueCount = 1;
	queueInfos[2].pQueuePriorities = &one;

	deviceInfo.queueCreateInfoCount = _countof(queueInfos);
	deviceInfo.pQueueCreateInfos = queueInfos;

	VkPhysicalDeviceSynchronization2Features	synchronizationFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES };
	VkPhysicalDeviceTimelineSemaphoreFeatures	timelineSemaphoreFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES };
	VkPhysicalDeviceFeatures2					physicalDeviceFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };

	synchronizationFeatures.pNext = nullptr;
	timelineSemaphoreFeatures.pNext = &synchronizationFeatures;
	physicalDeviceFeatures.pNext = &timelineSemaphoreFeatures;

	vkGetPhysicalDeviceFeatures2(physicalDevice, &physicalDeviceFeatures);

	deviceInfo.pEnabledFeatures = nullptr;
	deviceInfo.pNext = &physicalDeviceFeatures;

	VkDevice device = VK_NULL_HANDLE;
	VK_CHECK(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device));

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWwindow* window = glfwCreateWindow(1920, 1080, "blowbox", nullptr, nullptr);

	VkWin32SurfaceCreateInfoKHR win32SurfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	win32SurfaceInfo.hwnd = glfwGetWin32Window(window);
	win32SurfaceInfo.hinstance = GetModuleHandle(nullptr);

	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VK_CHECK(vkCreateWin32SurfaceKHR(instance, &win32SurfaceInfo, nullptr, &surface));

	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities));

	uint32_t surfaceFormatCount = 0;
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr));
	std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data()));

	VkSwapchainCreateInfoKHR swapchainInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	swapchainInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	swapchainInfo.surface = surface;
	swapchainInfo.minImageCount = 2;
	swapchainInfo.imageFormat = surfaceFormats[0].format;
	swapchainInfo.imageColorSpace = surfaceFormats[0].colorSpace;
	swapchainInfo.imageExtent = surfaceCapabilities.currentExtent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = surfaceCapabilities.supportedUsageFlags;
	swapchainInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
	swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	swapchainInfo.queueFamilyIndexCount = 3;
	swapchainInfo.pQueueFamilyIndices = uniqueQueueFamilies;

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VK_CHECK(vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain));

	uint32_t swapchainImageCount = 0;
	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr));
	std::vector<VkImage> swapchainImages(swapchainImageCount, VK_NULL_HANDLE);
	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data()));

	std::vector<VkImageView> swapchainImageViews(swapchainImageCount);

	for (auto& swapchainImage : swapchainImages)
	{
		VkImageViewCreateInfo imageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		imageViewInfo.image = swapchainImage;
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.format = swapchainInfo.imageFormat;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; 
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

		VkImageView imageView = VK_NULL_HANDLE;
		VK_CHECK(vkCreateImageView(device, &imageViewInfo, nullptr, &imageView));
		swapchainImageViews.push_back(imageView);
	}

	VkQueue graphicsQueue, computeQueue, transferQueue;
	vkGetDeviceQueue(device, uniqueQueueFamilies[QueueType_Graphics], 0, &graphicsQueue);
	vkGetDeviceQueue(device, uniqueQueueFamilies[QueueType_Compute], 0, &computeQueue);
	vkGetDeviceQueue(device, uniqueQueueFamilies[QueueType_Transfer], 0, &transferQueue);

	VkCommandPoolCreateInfo commandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	commandPoolInfo.queueFamilyIndex = uniqueQueueFamilies[QueueType_Graphics];
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkCommandPool commandPool = VK_NULL_HANDLE;
	VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool));

	VkCommandBufferAllocateInfo cmdBufferAllocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	cmdBufferAllocInfo.commandPool = commandPool;
	cmdBufferAllocInfo.commandBufferCount = 1;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
	VK_CHECK(vkAllocateCommandBuffers(device, &cmdBufferAllocInfo, &cmdBuffer));

	VkCommandBufferBeginInfo cmdBufferInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	cmdBufferInfo.pInheritanceInfo = nullptr;
	cmdBufferInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &cmdBufferInfo));

	uint32_t timestamp = 1;

	VkSemaphoreTypeCreateInfo semaphoreType = { VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
	semaphoreType.initialValue = timestamp;
	semaphoreType.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

	VkSemaphoreCreateInfo semaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	semaphoreInfo.pNext = &semaphoreType;

	VkSemaphore timelineSemaphore = VK_NULL_HANDLE;
	VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &timelineSemaphore));

	semaphoreInfo.pNext = nullptr;
	VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
	VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore));
	VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
	VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore));

	bool shutdown = false;
	while (!shutdown)
	{
		glfwPollEvents();
	
		if (glfwWindowShouldClose(window))
		{
			shutdown = true;
			break;
		}

		VkSemaphoreWaitInfo semaphoreWaitInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
		semaphoreWaitInfo.pSemaphores = &timelineSemaphore;
		uint64_t targetTimestamp = timestamp;
		semaphoreWaitInfo.pValues = &targetTimestamp;
		semaphoreWaitInfo.semaphoreCount = 1;

		VK_CHECK(vkWaitSemaphores(device, &semaphoreWaitInfo, UINT64_MAX));
	
		VK_CHECK(vkResetCommandBuffer(cmdBuffer, 0));

		VkCommandBufferBeginInfo cmdBufferInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		cmdBufferInfo.pInheritanceInfo = nullptr;
		cmdBufferInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &cmdBufferInfo));
	
		uint32_t swapchainImageIndex = ~0u;
		VK_CHECK(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &swapchainImageIndex));

		VkClearColorValue clearColor = {};

		{
			VkImageMemoryBarrier2 imageBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
			imageBarrier.image = swapchainImages[swapchainImageIndex];
			imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier.srcAccessMask = VK_ACCESS_2_NONE;
			imageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
			imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
			imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_CLEAR_BIT;
			imageBarrier.srcQueueFamilyIndex = 0;
			imageBarrier.dstQueueFamilyIndex = 0;
			imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBarrier.subresourceRange.baseArrayLayer = 0;
			imageBarrier.subresourceRange.baseMipLevel = 0;
			imageBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
			imageBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

			VkDependencyInfo dependencyInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
			dependencyInfo.pImageMemoryBarriers = &imageBarrier;
			dependencyInfo.imageMemoryBarrierCount = 1;

			vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);
		}

		clearColor.float32[0] = 1.0f;
		clearColor.float32[1] = 0.0f;
		clearColor.float32[2] = 1.0f;
		clearColor.float32[3] = 0.0f;
	
		VkImageSubresourceRange clearRange;
		clearRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		clearRange.baseArrayLayer = 0;
		clearRange.baseMipLevel = 0;
		clearRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		clearRange.levelCount = VK_REMAINING_MIP_LEVELS;

		vkCmdClearColorImage(cmdBuffer, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &clearRange);
		
		{
			VkImageMemoryBarrier2 imageBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
			imageBarrier.image = swapchainImages[swapchainImageIndex];
			imageBarrier.srcQueueFamilyIndex = 0;
			imageBarrier.dstQueueFamilyIndex = 0;
			
			imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
			imageBarrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

			imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			imageBarrier.dstAccessMask = VK_ACCESS_NONE;
			imageBarrier.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

			imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBarrier.subresourceRange.baseArrayLayer = 0;
			imageBarrier.subresourceRange.baseMipLevel = 0;
			imageBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
			imageBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

			VkDependencyInfo dependencyInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
			dependencyInfo.pImageMemoryBarriers = &imageBarrier;
			dependencyInfo.imageMemoryBarrierCount = 1;

			vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);
		}

		VK_CHECK(vkEndCommandBuffer(cmdBuffer));

		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

		submitInfo.pCommandBuffers		= &cmdBuffer;
		submitInfo.commandBufferCount	= 1;

		VkSemaphore waitSemaphores[]	= { imageAvailableSemaphore };
		uint64_t waitSemaphoreValues[]	= { 0 };

		submitInfo.pWaitSemaphores		= waitSemaphores;
		submitInfo.waitSemaphoreCount	= _countof(waitSemaphores);
		
		VkSemaphore signalSemaphores[]	= { renderFinishedSemaphore,	timelineSemaphore };
		uint64_t signalSemaphoreValues[]= { 0,							++timestamp };

		submitInfo.pSignalSemaphores	= signalSemaphores;
		submitInfo.signalSemaphoreCount = _countof(signalSemaphores);

		VkTimelineSemaphoreSubmitInfo timelineSubmitInfo = { VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO };
		timelineSubmitInfo.pWaitSemaphoreValues		= waitSemaphoreValues;
		timelineSubmitInfo.waitSemaphoreValueCount	= _countof(waitSemaphoreValues);
		timelineSubmitInfo.pSignalSemaphoreValues	= signalSemaphoreValues;
		timelineSubmitInfo.signalSemaphoreValueCount= _countof(signalSemaphoreValues);
		submitInfo.pNext = &timelineSubmitInfo;

		VkPipelineStageFlags v = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		submitInfo.pWaitDstStageMask = &v;

		VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
		
		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.pImageIndices = &swapchainImageIndex;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pResults = nullptr;

		VK_CHECK(vkQueuePresentKHR(graphicsQueue, &presentInfo));
	}
	

	return 0;
}