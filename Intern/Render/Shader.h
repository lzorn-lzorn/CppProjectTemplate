#pragma once
#include "vulkan/vulkan.hpp"
namespace Render {

class Shader {
public:
	static void Init(const std::string& vertexSource, const std::string& fragSource);
	static void Quit();

	static Shader& Instance(){
		return *instance;
	}
    ~Shader();

private:
	Shader(const std::string& vertexSource, const std::string& fragSource);
    

private:
	static std::unique_ptr<Shader> instance;

    vk::ShaderModule fragmentModule;
	vk::ShaderModule vertexModule;
};

}