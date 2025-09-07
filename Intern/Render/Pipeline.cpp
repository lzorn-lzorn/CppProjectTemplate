
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
				/* eLineList: 多个直线, 1,2,3,4 -> 1-2, 3-4 */
				/* eLineStrip: 连接成一条线, 1,2,3,4 -> 1-2-3-4 */
				/* ePointList: 仅仅是点, 1,2,3,4 -> 1, 2, 3, 4 */
				/* eTriangleList: 连接成三角形, 1,2,3,4 -> 1-2-3, 1-3-4 */
				/* eTriangleStrip: 连接成三角形带, 1,2,3,4 -> 1-2-3, 2-3-4 */
				/* eTriangleFan: 连接成三角形扇, 1,2,3,4 -> 1-2-3, 1-3-4 */
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
			/* 指定真正绘制到屏幕上的区域大小 */
	viewportState.setScissors(rect); // 全部绘制
	pipelineCreateInfo.setPViewportState(&viewportState);

	// 5. Rasterization
	vk::PipelineRasterizationStateCreateInfo rastInfo;
	rastInfo.setRasterizerDiscardEnable(false)
			/* 面剔除 eBack -> 背面剔除 */
			.setCullMode(vk::CullModeFlagBits::eBack)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1);
	pipelineCreateInfo.setPRasterizationState(&rastInfo);

	// 6. Multisampling
	vk::PipelineMultisampleStateCreateInfo multisample;
				/* 关闭超采样 */
	multisample.setSampleShadingEnable(false)
			   .setRasterizationSamples(vk::SampleCountFlagBits::e1);
	pipelineCreateInfo.setPMultisampleState(&multisample);

	// 7. Test stencil test, depth test


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
		/* 关闭颜色混合 */
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