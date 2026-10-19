#include <jni.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>
#include <cmath>
#include <cstring>
#define VK_USE_PLATFORM_ANDROID_KHR
#include <vulkan/vulkan.h>
#include <algorithm>
#include <string>
#include <vector>

#define IXO_LOGI(...) __android_log_print(ANDROID_LOG_INFO, "IxoEngine", __VA_ARGS__)
#define IXO_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "IxoEngine", __VA_ARGS__)

namespace {
struct Renderer {
    ANativeWindow* window = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physical = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    uint32_t queueFamily = 0;
    VkQueue queue = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkExtent2D extent{};
    VkRenderPass renderPass = VK_NULL_HANDLE;
    std::vector<VkImageView> views;
    std::vector<VkFramebuffer> framebuffers;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commands;
    VkSemaphore acquired = VK_NULL_HANDLE;
    VkSemaphore rendered = VK_NULL_HANDLE;
    bool ready = false;
} g;

struct Vertex { float x,y,z,r,g,b; };
struct Mat4 { float v[16]{}; };
Mat4 identity(){Mat4 m{};m.v[0]=m.v[5]=m.v[10]=m.v[15]=1;return m;}
Mat4 mul(const Mat4&a,const Mat4&b){Mat4 o{};for(int c=0;c<4;c++)for(int r=0;r<4;r++)for(int k=0;k<4;k++)o.v[c*4+r]+=a.v[k*4+r]*b.v[c*4+k];return o;}
Mat4 proj(float f,float a,float n,float z){Mat4 m{};float t=1/std::tan(f/2);m.v[0]=t/a;m.v[5]=-t;m.v[10]=z/(n-z);m.v[11]=-1;m.v[14]=z*n/(n-z);return m;}
Mat4 view(){Mat4 m{};float ex=5,ey=4,ez=7;float fx=-5/9.0f,fy=-3.55f/9.0f,fz=-7/9.0f;float sx=-fz,sz=fx,sl=std::sqrt(sx*sx+sz*sz);sx/=sl;sz/=sl;float ux=-sz*fy,uy=sz*fx-sx*fz,uz=sx*fy;m.v[0]=sx;m.v[8]=sz;m.v[12]=-(sx*ex+sz*ez);m.v[1]=ux;m.v[5]=uy;m.v[9]=uz;m.v[13]=-(ux*ex+uy*ey+uz*ez);m.v[2]=-fx;m.v[6]=-fy;m.v[10]=-fz;m.v[14]=fx*ex+fy*ey+fz*ez;m.v[15]=1;return m;}
uint32_t memType(uint32_t bits,VkMemoryPropertyFlags flags){VkPhysicalDeviceMemoryProperties p{};vkGetPhysicalDeviceMemoryProperties(g.physical,&p);for(uint32_t i=0;i<p.memoryTypeCount;i++)if((bits&(1u<<i))&&(p.memoryTypes[i].propertyFlags&flags)==flags)return i;return UINT32_MAX;}
std::vector<uint32_t> loadShader(const char* path){if(!g.assets)return {};AAsset*a=AAssetManager_open(g.assets,path,AASSET_MODE_BUFFER);if(!a)return {};size_t n=AAsset_getLength(a);std::vector<uint32_t> v((n+3)/4);int got=AAsset_read(a,v.data(),n);AAsset_close(a);if(got!=(int)n)return {};return v;}
VkShaderModule shader(const char* path){auto code=loadShader(path);if(code.empty())return VK_NULL_HANDLE;VkShaderModuleCreateInfo i{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};i.codeSize=code.size()*4;i.pCode=code.data();VkShaderModule m{};if(vkCreateShaderModule(g.device,&i,nullptr,&m)!=VK_SUCCESS)return VK_NULL_HANDLE;return m;}
std::vector<Vertex> world(){std::vector<Vertex> v;auto line=[&](float x,float z,float X,float Z,float r,float gg,float b){float e=0.012f;Vertex a{x-e,0,z,r,gg,b},bb{x+e,0,z,r,gg,b},cc{X+e,0,Z,r,gg,b},d{X-e,0,Z,r,gg,b};v.insert(v.end(),{a,bb,cc,a,cc,d});};for(int i=-10;i<=10;i++){float t=(float)i;bool q=i%5==0;line(t,-10,t,10,q?.25f:.10f,q?.38f:.17f,q?.5f:.24f);line(-10,t,10,t,q?.25f:.10f,q?.38f:.17f,q?.5f:.24f);}Vertex p[8]={{-.7f,0,-.7f,.2f,.7f,.95f},{.7f,0,-.7f,.2f,.7f,.95f},{.7f,1.4f,-.7f,.2f,.7f,.95f},{-.7f,1.4f,-.7f,.2f,.7f,.95f},{-.7f,0,.7f,.2f,.7f,.95f},{.7f,0,.7f,.2f,.7f,.95f},{.7f,1.4f,.7f,.2f,.7f,.95f},{-.7f,1.4f,.7f,.2f,.7f,.95f}};int f[36]={0,1,2,0,2,3,5,4,7,5,7,6,4,0,3,4,3,7,1,5,6,1,6,2,3,2,6,3,6,7,4,5,1,4,1,0};for(int i:f)v.push_back(p[i]);return v;}
bool createPipeline(){auto vs=shader("ixo/shaders/mesh.vert.spv"),fs=shader("ixo/shaders/mesh.frag.spv");if(!vs||!fs){IXO_LOGE("Cannot load compiled shaders from APK assets");return false;}VkPipelineShaderStageCreateInfo st[2]{};st[0]={VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};st[0].stage=VK_SHADER_STAGE_VERTEX_BIT;st[0].module=vs;st[0].pName="main";st[1]={VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};st[1].stage=VK_SHADER_STAGE_FRAGMENT_BIT;st[1].module=fs;st[1].pName="main";VkVertexInputBindingDescription b{0,sizeof(Vertex),VK_VERTEX_INPUT_RATE_VERTEX};VkVertexInputAttributeDescription a[2]={{0,0,VK_FORMAT_R32G32B32_SFLOAT,0},{1,0,VK_FORMAT_R32G32B32_SFLOAT,12}};VkPipelineVertexInputStateCreateInfo vi{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};vi.vertexBindingDescriptionCount=1;vi.pVertexBindingDescriptions=&b;vi.vertexAttributeDescriptionCount=2;vi.pVertexAttributeDescriptions=a;VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};ia.topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;VkPipelineViewportStateCreateInfo vp{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};vp.viewportCount=1;vp.scissorCount=1;VkDynamicState dynStates[2]={VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};VkPipelineDynamicStateCreateInfo dyn{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};dyn.dynamicStateCount=2;dyn.pDynamicStates=dynStates;VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};rs.polygonMode=VK_POLYGON_MODE_FILL;rs.cullMode=VK_CULL_MODE_NONE;rs.lineWidth=1;VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};ms.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT;VkPipelineColorBlendAttachmentState ba{};ba.colorWriteMask=15;VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};cb.attachmentCount=1;cb.pAttachments=&ba;VkPushConstantRange pc{VK_SHADER_STAGE_VERTEX_BIT,0,128};VkPipelineLayoutCreateInfo li{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};li.pushConstantRangeCount=1;li.pPushConstantRanges=&pc;if(vkCreatePipelineLayout(g.device,&li,nullptr,&g.pipelineLayout)!=VK_SUCCESS)return false;VkGraphicsPipelineCreateInfo pi{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};pi.stageCount=2;pi.pStages=st;pi.pVertexInputState=&vi;pi.pInputAssemblyState=&ia;pi.pViewportState=&vp;pi.pRasterizationState=&rs;pi.pMultisampleState=&ms;pi.pColorBlendState=&cb;pi.pDynamicState=&dyn;pi.layout=g.pipelineLayout;pi.renderPass=g.renderPass;bool good=vkCreateGraphicsPipelines(g.device,VK_NULL_HANDLE,1,&pi,nullptr,&g.pipeline)==VK_SUCCESS;vkDestroyShaderModule(g.device,vs,nullptr);vkDestroyShaderModule(g.device,fs,nullptr);if(!good)return false;auto vertices=world();g.vertexCount=(uint32_t)vertices.size();VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};bi.size=vertices.size()*sizeof(Vertex);bi.usage=VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;bi.sharingMode=VK_SHARING_MODE_EXCLUSIVE;if(vkCreateBuffer(g.device,&bi,nullptr,&g.vertexBuffer)!=VK_SUCCESS)return false;VkMemoryRequirements mr{};vkGetBufferMemoryRequirements(g.device,g.vertexBuffer,&mr);uint32_t mt=memType(mr.memoryTypeBits,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);if(mt==UINT32_MAX)return false;VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};ai.allocationSize=mr.size;ai.memoryTypeIndex=mt;if(vkAllocateMemory(g.device,&ai,nullptr,&g.vertexMemory)!=VK_SUCCESS)return false;vkBindBufferMemory(g.device,g.vertexBuffer,g.vertexMemory,0);void* mapped=nullptr;vkMapMemory(g.device,g.vertexMemory,0,bi.size,0,&mapped);std::memcpy(mapped,vertices.data(),(size_t)bi.size);vkUnmapMemory(g.device,g.vertexMemory);return true;}


bool ok(VkResult r, const char* what) {
    if (r == VK_SUCCESS) return true;
    IXO_LOGE("%s failed: VkResult %d", what, static_cast<int>(r));
    return false;
}

void destroySwapchainResources() {
    if (g.device == VK_NULL_HANDLE) return;
    vkDeviceWaitIdle(g.device);
    if(g.pipeline)vkDestroyPipeline(g.device,g.pipeline,nullptr); g.pipeline=VK_NULL_HANDLE;
    if(g.pipelineLayout)vkDestroyPipelineLayout(g.device,g.pipelineLayout,nullptr); g.pipelineLayout=VK_NULL_HANDLE;
    if(g.vertexBuffer)vkDestroyBuffer(g.device,g.vertexBuffer,nullptr); g.vertexBuffer=VK_NULL_HANDLE;
    if(g.vertexMemory)vkFreeMemory(g.device,g.vertexMemory,nullptr); g.vertexMemory=VK_NULL_HANDLE;
    if (g.acquired) vkDestroySemaphore(g.device, g.acquired, nullptr);
    if (g.rendered) vkDestroySemaphore(g.device, g.rendered, nullptr);
    g.acquired = g.rendered = VK_NULL_HANDLE;
    if (!g.commands.empty() && g.commandPool) {
        vkFreeCommandBuffers(g.device, g.commandPool,
            static_cast<uint32_t>(g.commands.size()), g.commands.data());
    }
    g.commands.clear();
    for (VkFramebuffer fb : g.framebuffers) vkDestroyFramebuffer(g.device, fb, nullptr);
    g.framebuffers.clear();
    if (g.renderPass) vkDestroyRenderPass(g.device, g.renderPass, nullptr);
    g.renderPass = VK_NULL_HANDLE;
    for (VkImageView view : g.views) vkDestroyImageView(g.device, view, nullptr);
    g.views.clear();
    if (g.swapchain) vkDestroySwapchainKHR(g.device, g.swapchain, nullptr);
    g.swapchain = VK_NULL_HANDLE;
    if (g.commandPool) vkDestroyCommandPool(g.device, g.commandPool, nullptr);
    g.commandPool = VK_NULL_HANDLE;
    g.ready = false;
}

void shutdownRenderer() {
    destroySwapchainResources();
    if (g.device) vkDestroyDevice(g.device, nullptr);
    g.device = VK_NULL_HANDLE;
    if (g.surface && g.instance) vkDestroySurfaceKHR(g.instance, g.surface, nullptr);
    g.surface = VK_NULL_HANDLE;
    if (g.instance) vkDestroyInstance(g.instance, nullptr);
    g.instance = VK_NULL_HANDLE;
    if (g.window) ANativeWindow_release(g.window);
    g.window = nullptr;
    g.physical = VK_NULL_HANDLE;
    g.queue = VK_NULL_HANDLE;
}

bool createDevice() {
    uint32_t count = 0;
    if (!ok(vkEnumeratePhysicalDevices(g.instance, &count, nullptr), "enumerate devices") || count == 0) return false;
    std::vector<VkPhysicalDevice> devices(count);
    if (!ok(vkEnumeratePhysicalDevices(g.instance, &count, devices.data()), "read devices")) return false;
    for (VkPhysicalDevice candidate : devices) {
        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(candidate, &familyCount, nullptr);
        std::vector<VkQueueFamilyProperties> families(familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(candidate, &familyCount, families.data());
        for (uint32_t i = 0; i < familyCount; ++i) {
            VkBool32 present = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(candidate, i, g.surface, &present);
            if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present) {
                g.physical = candidate;
                g.queueFamily = i;
                break;
            }
        }
        if (g.physical) break;
    }
    if (!g.physical) { IXO_LOGE("No graphics+present queue found"); return false; }

    const float priority = 1.0f;
    VkDeviceQueueCreateInfo queueInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queueInfo.queueFamilyIndex = g.queueFamily;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &priority;
    const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkPhysicalDeviceFeatures features{};
    VkDeviceCreateInfo info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    info.queueCreateInfoCount = 1;
    info.pQueueCreateInfos = &queueInfo;
    info.enabledExtensionCount = 1;
    info.ppEnabledExtensionNames = extensions;
    info.pEnabledFeatures = &features;
    if (!ok(vkCreateDevice(g.physical, &info, nullptr, &g.device), "create device")) return false;
    vkGetDeviceQueue(g.device, g.queueFamily, 0, &g.queue);
    return g.queue != VK_NULL_HANDLE;
}

bool createSwapchain(uint32_t width, uint32_t height) {
    if (!g.device || !g.surface || width == 0 || height == 0) return false;
    destroySwapchainResources();
    VkSurfaceCapabilitiesKHR caps{};
    if (!ok(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g.physical, g.surface, &caps), "surface capabilities")) return false;
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(g.physical, g.surface, &formatCount, nullptr);
    if (formatCount == 0) return false;
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(g.physical, g.surface, &formatCount, formats.data());
    VkSurfaceFormatKHR chosen = formats[0];
    for (const auto& f : formats) {
        if (f.format == VK_FORMAT_R8G8B8A8_UNORM || f.format == VK_FORMAT_B8G8R8A8_UNORM) {
            chosen = f;
            break;
        }
    }
    g.format = chosen.format;
    if (caps.currentExtent.width != UINT32_MAX) {
        g.extent = caps.currentExtent;
    } else {
        g.extent.width = std::max(caps.minImageExtent.width, std::min(caps.maxImageExtent.width, width));
        g.extent.height = std::max(caps.minImageExtent.height, std::min(caps.maxImageExtent.height, height));
    }
    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) imageCount = caps.maxImageCount;
    VkCompositeAlphaFlagBitsKHR alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if (!(caps.supportedCompositeAlpha & alpha)) {
        for (VkCompositeAlphaFlagBitsKHR option : {VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
             VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR, VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR}) {
            if (caps.supportedCompositeAlpha & option) { alpha = option; break; }
        }
    }
    VkSwapchainCreateInfoKHR swap{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swap.surface = g.surface;
    swap.minImageCount = imageCount;
    swap.imageFormat = chosen.format;
    swap.imageColorSpace = chosen.colorSpace;
    swap.imageExtent = g.extent;
    swap.imageArrayLayers = 1;
    swap.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swap.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swap.preTransform = caps.currentTransform;
    swap.compositeAlpha = alpha;
    swap.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swap.clipped = VK_TRUE;
    if (!ok(vkCreateSwapchainKHR(g.device, &swap, nullptr, &g.swapchain), "create swapchain")) return false;

    uint32_t actualCount = 0;
    vkGetSwapchainImagesKHR(g.device, g.swapchain, &actualCount, nullptr);
    std::vector<VkImage> images(actualCount);
    vkGetSwapchainImagesKHR(g.device, g.swapchain, &actualCount, images.data());
    g.views.reserve(actualCount);
    for (VkImage image : images) {
        VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        view.image = image;
        view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view.format = g.format;
        view.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                          VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
        view.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        VkImageView handle = VK_NULL_HANDLE;
        if (!ok(vkCreateImageView(g.device, &view, nullptr, &handle), "create image view")) return false;
        g.views.push_back(handle);
    }
    VkAttachmentDescription color{};
    color.format = g.format;
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference ref{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &ref;
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    VkRenderPassCreateInfo rp{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    rp.attachmentCount = 1; rp.pAttachments = &color;
    rp.subpassCount = 1; rp.pSubpasses = &subpass;
    rp.dependencyCount = 1; rp.pDependencies = &dependency;
    if (!ok(vkCreateRenderPass(g.device, &rp, nullptr, &g.renderPass), "create render pass")) return false;

    for (VkImageView view : g.views) {
        VkFramebufferCreateInfo fb{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        fb.renderPass = g.renderPass; fb.attachmentCount = 1; fb.pAttachments = &view;
        fb.width = g.extent.width; fb.height = g.extent.height; fb.layers = 1;
        VkFramebuffer handle = VK_NULL_HANDLE;
        if (!ok(vkCreateFramebuffer(g.device, &fb, nullptr, &handle), "create framebuffer")) return false;
        g.framebuffers.push_back(handle);
    }
    VkCommandPoolCreateInfo pool{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool.queueFamilyIndex = g.queueFamily;
    if (!ok(vkCreateCommandPool(g.device, &pool, nullptr, &g.commandPool), "create command pool")) return false;
    g.commands.resize(actualCount);
    VkCommandBufferAllocateInfo alloc{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    alloc.commandPool = g.commandPool; alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc.commandBufferCount = actualCount;
    if (!ok(vkAllocateCommandBuffers(g.device, &alloc, g.commands.data()), "allocate command buffers")) return false;
    VkSemaphoreCreateInfo sem{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    if (!ok(vkCreateSemaphore(g.device, &sem, nullptr, &g.acquired), "create acquire semaphore")) return false;
    if (!ok(vkCreateSemaphore(g.device, &sem, nullptr, &g.rendered), "create render semaphore")) return false;
    if (!createPipeline()) { IXO_LOGE("Mesh graphics pipeline creation failed; shader assets missing or incompatible"); return false; }
    g.ready = true;
    return true;
}

void renderFrame() {
    if (!g.ready) return;
    uint32_t index = 0;
    VkResult acquired = vkAcquireNextImageKHR(g.device, g.swapchain, UINT64_MAX, g.acquired, VK_NULL_HANDLE, &index);
    if (acquired == VK_ERROR_OUT_OF_DATE_KHR || acquired == VK_SUBOPTIMAL_KHR) {
        IXO_LOGI("Swapchain needs recreation after surface change");
        return;
    }
    if (acquired != VK_SUCCESS) { IXO_LOGE("Acquire image failed: %d", acquired); return; }
    VkCommandBuffer cmd = g.commands[index];
    vkResetCommandBuffer(cmd, 0);
    VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    if (!ok(vkBeginCommandBuffer(cmd, &begin), "begin frame command buffer")) return;
    // Native Vulkan draw: procedural XZ grid plus colored cube mesh.
    VkClearValue clear{};
    clear.color = {{0.035f, 0.055f, 0.09f, 1.0f}};
    VkRenderPassBeginInfo pass{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    pass.renderPass = g.renderPass; pass.framebuffer = g.framebuffers[index];
    pass.renderArea.extent = g.extent; pass.clearValueCount = 1; pass.pClearValues = &clear;
    vkCmdBeginRenderPass(cmd, &pass, VK_SUBPASS_CONTENTS_INLINE);
    VkViewport viewport{0.0f,0.0f,(float)g.extent.width,(float)g.extent.height,0.0f,1.0f};
    VkRect2D scissor{{0,0},g.extent};
    vkCmdSetViewport(cmd,0,1,&viewport); vkCmdSetScissor(cmd,0,1,&scissor);
    vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,g.pipeline);
    VkDeviceSize offset=0; vkCmdBindVertexBuffers(cmd,0,1,&g.vertexBuffer,&offset);
    Mat4 camera=mul(proj(1.04719755f,(float)g.extent.width/(float)g.extent.height,0.1f,100.0f),view());
    Mat4 model=identity(); uint8_t push[128]; std::memcpy(push,camera.v,64);std::memcpy(push+64,model.v,64);
    vkCmdPushConstants(cmd,g.pipelineLayout,VK_SHADER_STAGE_VERTEX_BIT,0,128,push);
    vkCmdDraw(cmd,g.vertexCount,1,0,0);
    vkCmdEndRenderPass(cmd);
    if (!ok(vkEndCommandBuffer(cmd), "end frame command buffer")) return;
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.waitSemaphoreCount = 1; submit.pWaitSemaphores = &g.acquired;
    submit.pWaitDstStageMask = &waitStage;
    submit.commandBufferCount = 1; submit.pCommandBuffers = &cmd;
    submit.signalSemaphoreCount = 1; submit.pSignalSemaphores = &g.rendered;
    if (!ok(vkQueueSubmit(g.queue, 1, &submit, VK_NULL_HANDLE), "submit frame")) return;
    VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present.waitSemaphoreCount = 1; present.pWaitSemaphores = &g.rendered;
    present.swapchainCount = 1; present.pSwapchains = &g.swapchain; present.pImageIndices = &index;
    VkResult result = vkQueuePresentKHR(g.queue, &present);
    vkQueueWaitIdle(g.queue);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR && result != VK_ERROR_OUT_OF_DATE_KHR)
        IXO_LOGE("Present failed: %d", result);
}
} // namespace

extern "C" JNIEXPORT jstring JNICALL
Java_dev_ixo_engine_MainActivity_nativeRendererStatus(JNIEnv* env, jclass) {
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName = "Ixo Engine";
    app.applicationVersion = VK_MAKE_VERSION(0, 2, 0);
    app.pEngineName = "Ixo";
    app.engineVersion = VK_MAKE_VERSION(0, 2, 0);
    app.apiVersion = VK_API_VERSION_1_0;
    VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    info.pApplicationInfo = &app;
    VkInstance instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&info, nullptr, &instance);
    std::string message;
    if (result == VK_SUCCESS) {
        uint32_t count = 0;
        vkEnumeratePhysicalDevices(instance, &count, nullptr);
        message = count ? "Vulkan ready • " + std::to_string(count) + " device(s)"
                        : "Vulkan instance ready • no device";
        vkDestroyInstance(instance, nullptr);
    } else {
        message = "Vulkan unavailable (VkResult " + std::to_string(result) + ")";
    }
    return env->NewStringUTF(message.c_str());
}

extern "C" JNIEXPORT jboolean JNICALL
Java_dev_ixo_engine_MainActivity_nativeSurfaceCreated(JNIEnv* env, jclass, jobject surface, jobject assetManager) {
    g.assets = AAssetManager_fromJava(env, assetManager);
    shutdownRenderer();
    g.window = ANativeWindow_fromSurface(env, surface);
    if (!g.window) return JNI_FALSE;
    const char* extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME};
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName = "Ixo Engine";
    app.applicationVersion = VK_MAKE_VERSION(0, 2, 0);
    app.pEngineName = "Ixo Mobile Renderer";
    app.engineVersion = VK_MAKE_VERSION(0, 2, 0);
    app.apiVersion = VK_API_VERSION_1_0;
    VkInstanceCreateInfo instanceInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    instanceInfo.pApplicationInfo = &app;
    instanceInfo.enabledExtensionCount = 2;
    instanceInfo.ppEnabledExtensionNames = extensions;
    if (!ok(vkCreateInstance(&instanceInfo, nullptr, &g.instance), "create Vulkan instance")) { shutdownRenderer(); return JNI_FALSE; }
    VkAndroidSurfaceCreateInfoKHR surfaceInfo{VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR};
    surfaceInfo.window = g.window;
    if (!ok(vkCreateAndroidSurfaceKHR(g.instance, &surfaceInfo, nullptr, &g.surface), "create Android surface") ||
        !createDevice()) { shutdownRenderer(); return JNI_FALSE; }
    return JNI_TRUE;
}

extern "C" JNIEXPORT void JNICALL
Java_dev_ixo_engine_MainActivity_nativeSurfaceChanged(JNIEnv*, jclass, jint width, jint height) {
    if (createSwapchain(static_cast<uint32_t>(std::max(0, width)), static_cast<uint32_t>(std::max(0, height)))) {
        renderFrame();
        IXO_LOGI("Vulkan swapchain frame presented: %ux%u", g.extent.width, g.extent.height);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_dev_ixo_engine_MainActivity_nativeSurfaceDestroyed(JNIEnv*, jclass) {
    shutdownRenderer();
}
