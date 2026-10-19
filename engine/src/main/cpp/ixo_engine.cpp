#include <jni.h>
#include <android/log.h>
#include <vulkan/vulkan.h>
#include <string>

#define IXO_LOGI(...) __android_log_print(ANDROID_LOG_INFO, "IxoEngine", __VA_ARGS__)

extern "C" JNIEXPORT jstring JNICALL
Java_dev_ixo_engine_MainActivity_nativeRendererStatus(JNIEnv* env, jclass) {
    VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.pApplicationName = "Ixo Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "Ixo";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;
    VkInstance instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    std::string message;
    if (result == VK_SUCCESS) {
        uint32_t count = 0;
        VkResult devicesResult = vkEnumeratePhysicalDevices(instance, &count, nullptr);
        message = (devicesResult == VK_SUCCESS && count > 0)
            ? "Vulkan available • " + std::to_string(count) + " physical device(s)"
            : "Vulkan instance ready • no physical device reported";
        vkDestroyInstance(instance, nullptr);
        IXO_LOGI("%s", message.c_str());
    } else {
        message = "Vulkan unavailable (VkResult " + std::to_string(result) + ")";
        IXO_LOGI("%s", message.c_str());
    }
    return env->NewStringUTF(message.c_str());
}
