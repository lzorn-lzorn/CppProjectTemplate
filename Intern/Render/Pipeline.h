#pragma once
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_handles.hpp>

namespace Render {

class RenderProcess final {
public:
	RenderProcess() = default;
	~RenderProcess() = default;

public:
	vk::Pipeline vkPipeline;

	void InitPipeline(int width, int height);
	void DestroyPipeline();

};

}