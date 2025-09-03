
#include "Shader.h"
#include "Context.h"
#include <string>

namespace Render {

std::unique_ptr<Shader> Shader::instance = nullptr;

void Shader::Init(const std::string& vertexSource, const std::string& fragSource) {
    instance.reset(new Shader(vertexSource, fragSource));
}
void Shader::Quit() {
	instance.reset();
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
}

}