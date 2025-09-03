
#include "Shader.h"
#include "Context.h"
#include <string>
#include <vulkan/vulkan_structs.hpp>

namespace Render {

std::unique_ptr<Shader> Shader::instance = nullptr;

void Shader::Init(const std::string& vertexSource, const std::string& fragSource) {
    instance.reset(new Shader(vertexSource, fragSource));
}
void Shader::Quit() {
	instance.reset();
}
 
void Shader::InitStage() {
    stages.resize(2);
    stages[0].setStage(vk::ShaderStageFlagBits::eVertex)
             .setModule(vertexModule)
             .setPName("main");
    stages[1].setStage(vk::ShaderStageFlagBits::eFragment)
             .setModule(fragmentModule)
             .setPName("main");
}

std::vector<vk::PipelineShaderStageCreateInfo> Shader::GetShaderStages() const {
    return stages;
}

Shader::~Shader(){
    auto& device = VulkanContext::Instance().vkDevice;
    device.destroyShaderModule(vertexModule);
    device.destroyShaderModule(fragmentModule);
}

Shader::Shader(const std::string& vertexSource, const std::string& fragSource) {
    vk::ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = vertexSource.size();
    createInfo.pCode = (uint32_t*)vertexSource.data();

    vertexModule = VulkanContext::Instance().vkDevice.createShaderModule(createInfo);

    createInfo.codeSize = fragSource.size();
    createInfo.pCode = (uint32_t*)fragSource.data();
    fragmentModule = VulkanContext::Instance().vkDevice.createShaderModule(createInfo);

    InitStage();
}

}