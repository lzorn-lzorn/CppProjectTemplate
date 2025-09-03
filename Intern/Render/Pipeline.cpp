
#include "Pipeline.h"
#include "Context.h"
#include "Shader.h"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace Render {

void RenderProcess::InitPipeline(int width, int height) {
	// Initialize the Vulkan pipeline
	vk::GraphicsPipelineCreateInfo pipelineCreateInfo;

	// 1. Vertext Input
	vk::PipelineVertexInputStateCreateInfo inputState;
	pipelineCreateInfo.setPVertexInputState(&inputState);

	// 2. Vertex Assembly
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.setPrimitiveRestartEnable(false)
				 .setTopology(vk::PrimitiveTopology::eTriangleList);
	pipelineCreateInfo.setPInputAssemblyState(&inputAssembly);

	// 3. Shader
	auto stages = Shader::Instance().GetShaderStages();
	pipelineCreateInfo.setStages(stages);

	// 4. viewport
	vk::PipelineViewportStateCreateInfo viewportState;
	vk::Viewport viewport(0, 0, width, height, 0, 1);
	viewportState.setViewports(viewport);
	vk::Rect2D rect(
		{0, 0},
		{static_cast<uint32_t>(width), static_cast<uint32_t>(height)}
	);
	viewportState.setScissors(rect);
	pipelineCreateInfo.setPViewportState(&viewportState);

	// 5. Rasterization
	vk::PipelineRasterizationStateCreateInfo rastInfo;
	rastInfo.setRasterizerDiscardEnable(false)
			.setCullMode(vk::CullModeFlagBits::eBack)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1);
	pipelineCreateInfo.setPRasterizationState(&rastInfo);

	// 6. Multisampling
	vk::PipelineMultisampleStateCreateInfo multisample;
	multisample.setSampleShadingEnable(false)
			   .setRasterizationSamples(vk::SampleCountFlagBits::e1);
	pipelineCreateInfo.setPMultisampleState(&multisample);

	// 7. Test


	// 8. Color Blending
	vk::PipelineColorBlendStateCreateInfo blend;
	vk::PipelineColorBlendAttachmentState attaches;
	attaches.setBlendEnable(false)
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eA |
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB
			);
	blend.setLogicOpEnable(false)
		 .setAttachments(attaches);
	pipelineCreateInfo.setPColorBlendState(&blend);

	auto result = VulkanContext::Instance().vkDevice.createGraphicsPipeline(nullptr, pipelineCreateInfo);
	if (result.result != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to create graphics pipeline");
	}
	vkPipeline = result.value;

}

void RenderProcess::DestroyPipeline() {
	// Destroy the Vulkan pipeline
	VulkanContext::Instance().vkDevice.destroyPipeline(vkPipeline);
}

}