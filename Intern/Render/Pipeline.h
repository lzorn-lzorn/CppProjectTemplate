#pragma once
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_handles.hpp>

namespace Render {

class RenderProcess final {
public:
	RenderProcess() = default;
	~RenderProcess();

public:
	vk::Pipeline vkPipeline;
	vk::PipelineLayout vkPipelineLayout;
	vk::RenderPass vkRenderPass;

	void InitLayout();
	void InitRenderPass();
	void InitPipeline(int width, int height);

};

}