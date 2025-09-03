#pragma once
#include "vulkan/vulkan.hpp"
namespace Render {

class Shader final {
public:
	static void Init(const std::string& vertexSource, const std::string& fragSource);
	static void Quit();

	static Shader& Instance(){
		return *instance;
	}
    ~Shader();
public:
    std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStages() const;

public:
	vk::ShaderModule fragmentModule;
	vk::ShaderModule vertexModule;
private:
	Shader(const std::string& vertexSource, const std::string& fragSource);
	void InitStage();
	
private:
	static std::unique_ptr<Shader> instance;
	std::vector<vk::PipelineShaderStageCreateInfo> stages;

};

}