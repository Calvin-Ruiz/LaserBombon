#ifndef VULKAN_MGR_HPP_
#define VULKAN_MGR_HPP_

#include <vulkan/vulkan.hpp>
#include "EntityCore/SubMemory.hpp"
#include <string>
#include <fstream>

class SDL_Window;
class MemoryManager;

#ifndef MAX_FRAMES_IN_FLIGHT
#define MAX_FRAMES_IN_FLIGHT 2
#endif

// pattern : SETUP_PFN(PFN_#, ptr_#, "#");
#define SETUP_PFN(pfn, ptr, str) ptr = reinterpret_cast<pfn>(vkGetInstanceProcAddr(instance, str))

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

enum class LogType {
    INFO,
    DEBUG,
    LAYER,
    WARNING,
    ERROR
};

struct QueueFamily {
    uint32_t id;
    unsigned char capacity;
    unsigned char size;
    bool graphic;
    bool compute;
    bool transfer;
    bool present;
    unsigned char dedicatedGraphicCount;
    unsigned char dedicatedComputeCount;
    unsigned char dedicatedGraphicAndComputeCount;
    unsigned char dedicatedTransferCount;
};

struct QueueRequirement {
    unsigned char transfer;
    unsigned char dedicatedGraphic;
    unsigned char dedicatedCompute;
    unsigned char dedicatedGraphicAndCompute; // If not available, querry pair of graphic and compute queues
    unsigned char dedicatedTransfer;
};

/*
* Core class, manage global ressources
* Used as base to create any vulkan ressource
*/
class VulkanMgr {
public:
    VulkanMgr(const char *AppName = nullptr, uint32_t appVersion = 1, SDL_Window *window = nullptr, int width = 600, int height = 600, const QueueRequirement &queueRequest = {1, 1, 0, 0, 0}, int chunkSize = 64, bool enableDebugLayers = true, bool drawLogs = true, bool saveLogs = false, std::string _cachePath = "\0");
    ~VulkanMgr();
    bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, SubMemory& bufferMemory, VkMemoryPropertyFlags preferedProperties = 0);
    //! for malloc
    MemoryManager *getMemoryManager() {return memoryManager;}
    void free(SubMemory& bufferMemory);
    void mapMemory(SubMemory& bufferMemory, void **data);
    void unmapMemory(SubMemory& bufferMemory);
    void waitIdle();
    VkPipelineViewportStateCreateInfo &getViewportState() {return viewportState;}
    VkSwapchainKHR &getSwapchain() {return swapChain;}
    std::vector<VkImageView> &getSwapchainView() {return swapChainImageViews;}
    VkExtent2D &getSwapChainExtent() {return swapChainExtent;}
    const VkPhysicalDeviceFeatures &getDeviceFeatures() {return deviceFeatures;}
    VkPipelineCache &getPipelineCache() {return pipelineCache;}
    //! Only MemoryManager should use this method
    VkPhysicalDevice getPhysicalDevice() {return physicalDevice;}
    void setObjectName(void *handle, VkObjectType type, const std::string &name);
    //! Called by MemoryManager when getting low of memory,
    void releaseUnusedMemory();

    // Load a dedicated graphic, compute, graphic_compute or transfer queue in the queue argument
    // Return the queue family from which the queue was created, or nullptr in case of failure
    // The number of dedicated queues can be lower than expected if they are not available
    // The number of dedicated queues can be higher than expected to fulfill the requirement of queue ability
    // e.g. a request for 1 graphic and 1 compute queue with 1 dedicated graphic may result to 1 dedicated graphic and 1 dedicated compute if there is no graphic and compute queue available.
    enum class QueueType {
        GRAPHIC,
        COMPUTE,
        GRAPHIC_COMPUTE,
        TRANSFER
    };
    const QueueFamily *previewQueueFamily(QueueType type);
    const QueueFamily *acquireQueue(VkQueue &queue, QueueType type, const std::string &name = "\0");

    //! Others
    void putLog(const std::string &str, LogType type = LogType::INFO);

    //! getDevice();
    const VkDevice &refDevice;
    static VulkanMgr *instance;
private:
    const bool drawLogs;
    const bool saveLogs;
    std::ofstream logs;
    std::string cachePath;
    MemoryManager *memoryManager;
    VkPipelineViewportStateCreateInfo viewportState{};
    VkViewport viewport{};
    VkRect2D scissor{};
    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    VkPhysicalDeviceFeatures deviceFeatures{};
    VkPipelineCache pipelineCache;

    void initVulkan(const char *AppName, uint32_t appVersion, SDL_Window *window, bool _hasLayer = false);
    vk::UniqueInstance vkinstance;
    vk::PhysicalDevice physicalDevice;

    void initWindow(SDL_Window *window);
    VkSurfaceKHR surface;

    void initQueues(const QueueRequirement &queueRequest);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::vector<QueueFamily> queues;

    void initDevice();
    VkDevice device;

    void initSwapchain(int width, int height);
    VkSwapchainKHR swapChain;
    uint32_t finalImageCount;
    std::vector<VkImage> swapChainImages;
    VkExtent2D swapChainExtent;
    VkFormat swapChainImageFormat;

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
    void createImageViews();
    std::vector<VkImageView> swapChainImageViews;

    // Debug
    void initDebug(vk::InstanceCreateInfo *instanceCreateInfo);
    void startDebug();
    void destroyDebug();
    // Note : identifier ascii code must be included between 0x40 and 0x7e both included
    void setDebugFunction(char identifier, void (*func)(void *self, std::ostringstream &ss)) {
        debugFunc[identifier & 0x3f] = func;
    }
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void * /*pUserData*/);
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    VkDebugUtilsMessengerEXT callback;
    void displayPhysicalDeviceInfo(VkPhysicalDeviceProperties &prop);
    void displayEnabledFeaturesInfo();
    PFN_vkSetDebugUtilsObjectNameEXT ptr_vkSetDebugUtilsObjectNameEXT = nullptr;

    bool checkDeviceExtensionSupport(VkPhysicalDevice pDevice);
    bool isDeviceSuitable(VkPhysicalDevice pDevice);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    std::vector<const char *> instanceExtension = {"VK_KHR_surface", VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME};
    std::vector<const char *> deviceExtension = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    bool hasLayer;
    static bool isAlive;
    bool isReady = false;
    bool presenting;
    void (*debugFunc[63])(void *self, std::ostringstream &ss) {};
};

#endif
