#include "axpch.h"
#include "VulkanAdapter.h"
#include "VulkanInstance.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Axiom {
	static VKAPI_ATTR Vk::Bool32 VKAPI_CALL debugCallback(Vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, Vk::DebugUtilsMessageTypeFlagsEXT messageType, const Vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		switch (messageSeverity) {
			case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
				AX_CORE_LOG_TRACE("Vulkan validation layer: {0}", pCallbackData->pMessage);
				break;
			case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
				AX_CORE_LOG_INFO("Vulkan validation layer: {0}", pCallbackData->pMessage);
				break;
			case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
				AX_CORE_LOG_WARN("Vulkan validation layer: {0}", pCallbackData->pMessage);
				break;
			case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
				AX_CORE_LOG_ERROR("Vulkan validation layer: {0}", pCallbackData->pMessage);
				break;
			default:
				break;
		}
		return Vk::False;
	}

	VulkanInstance::~VulkanInstance() {
		AX_CORE_LOG_INFO("Destroying Vulkan Instance...");
		if (debugMessenger) {
			instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr);
		}
		instance.destroy();
	}

	bool VulkanInstance::linkVulkan() {
#if defined(AX_PLATFORM_WINDOWS) 
		HMODULE vulkanLib = LoadLibraryA("vulkan-1.dll");
		if (vulkanLib == nullptr) {
			AX_CORE_LOG_ERROR("Failed to load Vulkan library!");
			return false;
		}
		vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(vulkanLib, "vkGetInstanceProcAddr"));
#elif defined(AX_PLATFORM_LINUX)
		void* vulkanLib = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
		if (vulkanLib == nullptr) {
			AX_CORE_LOG_ERROR("Failed to load Vulkan library: {}", dlerror());
			return false;
		}
		vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(vulkanLib, "vkGetInstanceProcAddr"));
#endif
		if (vkGetInstanceProcAddr == nullptr) {
			AX_CORE_LOG_ERROR("Failed to load vkGetInstanceProcAddr!");
			return false;
		}
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
		return true;
	}

	void VulkanInstance::init(const InstaceInfo& info) {
		AX_CORE_ASSERT(linkVulkan(), "Failed to link Vulkan library!");
		AX_CORE_LOG_INFO("Initializing Vulkan Instance...");

		Vk::ApplicationInfo appInfo(
			info.appName.c_str(), 
			Vk::makeVersion(info.appVersionMajor, info.appVersionMinor, info.appVersionPatch),
			info.engineName.c_str(),
			Vk::makeVersion(info.engineVersionMajor, info.engineVersionMinor, info.engineVersionPatch),
			Vk::ApiVersion14
		);

		std::vector<const char*> extensions = getRequiredExtensions();
		std::vector<const char*> layers = getValidationLayers();

		Vk::InstanceCreateInfo createInfo({}, &appInfo, layers, extensions);
		Vk::ResultValue<Vk::Instance> instanceResult = Vk::createInstance(createInfo, nullptr, VULKAN_HPP_DEFAULT_DISPATCHER);
		AX_CORE_ASSERT(instanceResult.result == Vk::Result::eSuccess, "Failed to create Vulkan instance: {}", Vk::to_string(instanceResult.result));
		instance = instanceResult.value;
		VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

		AX_CORE_LOG_TRACE("INIT Queue submit adress {}", reinterpret_cast<void*>(VULKAN_HPP_DEFAULT_DISPATCHER.vkQueueSubmit));
		AX_CORE_LOG_TRACE("INIT ResetCmd address {}", reinterpret_cast<void*>(VULKAN_HPP_DEFAULT_DISPATCHER.vkResetCommandBuffer));

#ifdef AX_DEBUG
		Vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo(
			{},
			Vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | Vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | Vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
			Vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | Vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | Vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
			debugCallback
		);
		Vk::ResultValue<Vk::DebugUtilsMessengerEXT> debugMessengerResult = instance.createDebugUtilsMessengerEXT(debugCreateInfo);
		AX_CORE_ASSERT(debugMessengerResult.result == Vk::Result::eSuccess, "Failed to create Vulkan debug messenger: {}", Vk::to_string(debugMessengerResult.result));
		debugMessenger = debugMessengerResult.value;
#endif // AX_DEBUG
	}

	std::unique_ptr<Adapter> VulkanInstance::getAdapter() {
		std::vector<VulkanAdapter> adapters = VulkanAdapter::getAvailableAdapters(*this);
		for (const auto& adapter : adapters) {
			AX_CORE_LOG_DEBUG("Found Vulkan Adapter: {} (API Version: {}.{}.{} | Driver Version: {} | Vendor ID: {} | Device ID: {})", std::string_view(adapter.getProperties().deviceName),
							  Vk::versionMajor(adapter.getProperties().apiVersion),
							  Vk::versionMinor(adapter.getProperties().apiVersion),
							  Vk::versionPatch(adapter.getProperties().apiVersion),
							  adapter.getProperties().driverVersion,
							  adapter.getProperties().vendorID,
							  adapter.getProperties().deviceID);
			if (adapter.checkAvailableDeviceExtensions()) {
				AX_CORE_LOG_DEBUG("Selected Vulkan Adapter: {}", std::string_view(adapter.getProperties().deviceName));
				return std::make_unique<VulkanAdapter>(adapter);
			}
		}
		return nullptr;
	}

	Vk::SurfaceKHR VulkanInstance::createSurface(void* windowHandle) const  {
		Vk::SurfaceKHR surface = nullptr;
#if defined(AX_PLATFORM_WINDOWS)
		Win32Window* window = reinterpret_cast<Win32Window*>(windowHandle);
		Vk::Win32SurfaceCreateInfoKHR createInfo({}, reinterpret_cast<HINSTANCE>(window->getNativeDisplay()), reinterpret_cast<HWND>(window->getNativeWindow()));
		Vk::ResultValue<Vk::SurfaceKHR> surfaceResult = instance.createWin32SurfaceKHR(createInfo);

		AX_CORE_ASSERT(surfaceResult.result == Vk::Result::eSuccess, "Failed to create window surface!");
		surface = surfaceResult.value;
#elif defined(AX_PLATFORM_LINUX)
		XLibWindow* window = reinterpret_cast<XLibWindow*>(windowHandle);
		Vk::XlibSurfaceCreateInfoKHR createInfo({}, static_cast<Display*>(window->getNativeDisplay()), *static_cast<::Window*>(window->getNativeWindow()));
		Vk::ResultValue<Vk::SurfaceKHR> surfaceResult = instance.createXlibSurfaceKHR(createInfo);

		AX_CORE_ASSERT(surfaceResult.result == Vk::Result::eSuccess, "Failed to create window surface!");
		surface = surfaceResult.value;
#endif
		return surface;
	}

	void VulkanInstance::destroySurface(Vk::SurfaceKHR surface) const {
		instance.destroySurfaceKHR(surface);
	}

	std::vector<const char*> VulkanInstance::getRequiredExtensions() const {
		std::vector<const char*> extensions;
		extensions.push_back(Vk::KHRSurfaceExtensionName);
#ifdef AX_DEBUG
		extensions.push_back(Vk::EXTDebugUtilsExtensionName);
#endif // AX_DEBUG
#if defined(AX_PLATFORM_WINDOWS)
		extensions.push_back(Vk::KHRWin32SurfaceExtensionName);
#elif defined(AX_PLATFORM_LINUX)
		extensions.push_back(Vk::KHRXlibSurfaceExtensionName);
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
		Vk::ResultValue<std::vector<Vk::LayerProperties>> availableLayersResult = Vk::enumerateInstanceLayerProperties();

		AX_CORE_ASSERT(availableLayersResult.result == Vk::Result::eSuccess, "Failed to enumerate Vulkan instance layers: {}", Vk::to_string(availableLayersResult.result));
		std::vector<Vk::LayerProperties> availableLayers = availableLayersResult.value;

		for (uint32_t i = 0; i < layers.size(); i++) {
			bool layerFound = false;
			for (const auto& layerProperties : availableLayers) {
				if (std::string_view(layerProperties.layerName.data()) == layers[i]) {
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
