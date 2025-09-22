#include "axpch.h"
#include "VulkanAdapter.h"
#include "VulkanInstance.h"

namespace Axiom {
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		switch (messageSeverity) {
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				AX_CORE_LOG_TRACE("Vulkan validation layer: {0}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				AX_CORE_LOG_INFO("Vulkan validation layer: {0}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				AX_CORE_LOG_WARN("Vulkan validation layer: {0}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				AX_CORE_LOG_ERROR("Vulkan validation layer: {0}", pCallbackData->pMessage);
				break;
			default:
				break;
		}
		return VK_FALSE;
	}

	static VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	VulkanInstance::~VulkanInstance() {
		AX_CORE_LOG_INFO("Destroying Vulkan Instance...");
		if (debugMessenger != VK_NULL_HANDLE) {
			destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		vkDestroyInstance(instance, nullptr);
	}

	void VulkanInstance::init(const InstaceInfo& info) {
		AX_CORE_LOG_INFO("Initializing Vulkan Instance...");

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = info.appName.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(info.appVersionMajor, info.appVersionMinor, info.appVersionPatch);
		appInfo.pEngineName = info.engineName.c_str();
		appInfo.engineVersion = VK_MAKE_VERSION(info.engineVersionMajor, info.engineVersionMinor, info.engineVersionPatch);
		appInfo.apiVersion = VK_API_VERSION_1_4;

		std::vector<const char*> extensions = getRequiredExtensions();
		std::vector<const char*> layers = getValidationLayers();

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		createInfo.enabledLayerCount = layers.size();
		createInfo.ppEnabledLayerNames = layers.data();
		AX_CORE_ASSERT(vkCreateInstance(&createInfo, nullptr, &instance) == VK_SUCCESS, "Failed to create Vulkan instance!");

#ifdef AX_DEBUG
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = debugCallback;
		AX_CORE_ASSERT(createDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) == VK_SUCCESS, "Failed to set up debug messenger!");
#endif // AX_DEBUG
	}

	std::unique_ptr<Adapter> VulkanInstance::getAdapter() {
		std::vector<VulkanAdapter> adapters = VulkanAdapter::getAvailableAdapters(*this);
		for (const auto& adapter : adapters) {
			AX_CORE_LOG_DEBUG("Found Vulkan Adapter: {} (API Version: {}.{}.{} | Driver Version: {} | Vendor ID: {} | Device ID: {})", adapter.getProperties().deviceName,
							  VK_VERSION_MAJOR(adapter.getProperties().apiVersion),
							  VK_VERSION_MINOR(adapter.getProperties().apiVersion),
							  VK_VERSION_PATCH(adapter.getProperties().apiVersion),
							  adapter.getProperties().driverVersion,
							  adapter.getProperties().vendorID,
							  adapter.getProperties().deviceID);
			if (adapter.checkAvailableDeviceExtensions()) {
				AX_CORE_LOG_DEBUG("Selected Vulkan Adapter: {}", adapter.getProperties().deviceName);
				return std::make_unique<VulkanAdapter>(adapter);
			}
		}
		return nullptr;
	}

	VkSurfaceKHR VulkanInstance::createSurface(void* windowHandle) const  {
		VkSurfaceKHR surface = VK_NULL_HANDLE;
#if defined(AX_PLATFORM_WINDOWS)
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		Win32Window* window = reinterpret_cast<Win32Window*>(windowHandle);
		createInfo.hwnd = reinterpret_cast<HWND>(window->getNativeWindow());
		createInfo.hinstance = GetModuleHandle(nullptr);
		AX_CORE_ASSERT(vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) == VK_SUCCESS, "Failed to create window surface!");
#elif defined(AX_PLATFORM_LINUX)
		VkXlibSurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		XLibWindow* window = reinterpret_cast<XLibWindow*>(windowHandle);
		createInfo.dpy = static_cast<Display*>(window->getNativeDisplay());
		createInfo.window = *static_cast<::Window*>(window->getNativeWindow());
		AX_CORE_ASSERT(vkCreateXlibSurfaceKHR(instance, &createInfo, nullptr, &surface) == VK_SUCCESS, "Failed to create window surface!");
#endif
		return surface;
	}

	void VulkanInstance::destroySurface(VkSurfaceKHR surface) const {
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}

	std::vector<const char*> VulkanInstance::getRequiredExtensions() const {
		std::vector<const char*> extensions;
		extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef AX_DEBUG
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // AX_DEBUG
#if defined(AX_PLATFORM_WINDOWS)
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(AX_PLATFORM_LINUX)
		extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
		for (const char* ext : extensions) {
			AX_CORE_LOG_DEBUG("Required Vulkan Instance Extension: {}", ext);
		}
		return extensions;
	}

	std::vector<const char*> VulkanInstance::getValidationLayers() const {
		std::vector<const char*> layers{};
#ifdef AX_DEBUG
		layers.push_back("VK_LAYER_KHRONOS_validation");
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (uint32_t i = 0; i < layers.size(); i++) {
			bool layerFound = false;
			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layers[i], layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
			AX_CORE_ASSERT(layerFound, "Validation layer not found: {}", layers[i]);
		}
#endif // AX_DEBUG
		return layers;
	}
}
