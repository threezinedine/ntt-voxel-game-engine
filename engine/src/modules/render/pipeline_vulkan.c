#if MEED_USE_VULKAN

#include "MEEDEngine/modules/render/pipeline.h"
#include "MEEDEngine/modules/render/shader.h"
#include "vulkan_common.h"

static void createLayout(struct MEEDPipeline* pPipeline);
static void createPipeline(struct MEEDPipeline* pPipeline);

static void freeInternalPipeline(void*);
static void deleteShaderResources(void*);

struct MEEDPipeline* meedPipelineCreate(const char* vertexShaderPath, const char* fragmentShaderPath)
{
	// Implementation of pipeline creation using Vulkan
	struct MEEDPipeline* pPipeline = MEED_MALLOC(struct MEEDPipeline);
	MEED_ASSERT(pPipeline != MEED_NULL);

	pPipeline->vertexShaderPath	  = vertexShaderPath;
	pPipeline->fragmentShaderPath = fragmentShaderPath;
	pPipeline->pReleaseStack	  = meedReleaseStackCreate();

	pPipeline->pInternal = MEED_MALLOC(struct VulkanPipeline);
	MEED_ASSERT(pPipeline->pInternal != MEED_NULL);

	meedReleaseStackPush(pPipeline->pReleaseStack, pPipeline, freeInternalPipeline);

	struct VulkanPipeline* pVulkanPipeline = (struct VulkanPipeline*)pPipeline->pInternal;
	meedPlatformMemorySet(pVulkanPipeline, 0, sizeof(struct VulkanPipeline));

	pVulkanPipeline->pVertexShader	 = meedShaderCreate(MEED_SHADER_TYPE_VERTEX, vertexShaderPath);
	pVulkanPipeline->pFragmentShader = meedShaderCreate(MEED_SHADER_TYPE_FRAGMENT, fragmentShaderPath);
	meedReleaseStackPush(pPipeline->pReleaseStack, pPipeline, deleteShaderResources);

	createLayout(pPipeline);
	createPipeline(pPipeline);

	return pPipeline;
}

static void freeInternalPipeline(void* pData)
{
	MEED_ASSERT(pData != MEED_NULL);
	struct MEEDPipeline* pPipeline = (struct MEEDPipeline*)pData;

	MEED_FREE(pPipeline->pInternal, struct VulkanPipeline);
}

static void deleteShaderResources(void* pData)
{
	MEED_ASSERT(pData != MEED_NULL);
	struct MEEDPipeline* pPipeline = (struct MEEDPipeline*)pData;

	struct VulkanPipeline* pVulkanPipeline = (struct VulkanPipeline*)pPipeline->pInternal;

	meedShaderDestroy(pVulkanPipeline->pVertexShader);
	meedShaderDestroy(pVulkanPipeline->pFragmentShader);
}

static void destroyLayout(void* pData);
static void createLayout(struct MEEDPipeline* pPipeline)
{
	MEED_ASSERT(pPipeline != MEED_NULL);
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->device != MEED_NULL);

	struct VulkanPipeline* pVulkanPipeline = (struct VulkanPipeline*)pPipeline->pInternal;

	VkPipelineLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCreateInfo.pPushConstantRanges		= MEED_NULL;
	layoutCreateInfo.pushConstantRangeCount		= 0;
	layoutCreateInfo.pSetLayouts				= MEED_NULL;
	layoutCreateInfo.setLayoutCount				= 0;

	VK_ASSERT(vkCreatePipelineLayout(g_vulkan->device, &layoutCreateInfo, MEED_NULL, &pVulkanPipeline->layout));
	meedReleaseStackPush(pPipeline->pReleaseStack, pPipeline, destroyLayout);
}

static void destroyLayout(void* pData)
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->device != MEED_NULL);
	MEED_ASSERT(pData != MEED_NULL);
	struct MEEDPipeline*   pPipeline	   = (struct MEEDPipeline*)pData;
	struct VulkanPipeline* pVulkanPipeline = (struct VulkanPipeline*)pPipeline->pInternal;

	vkDestroyPipelineLayout(g_vulkan->device, pVulkanPipeline->layout, MEED_NULL);
}

static void destroyPipeline(void* pData);
static void createPipeline(struct MEEDPipeline* pPipeline)
{
	MEED_ASSERT(pPipeline != MEED_NULL);
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->device != MEED_NULL);
	MEED_ASSERT(g_vulkan->renderPass != MEED_NULL);

	struct VulkanPipeline* pVulkanPipeline = (struct VulkanPipeline*)pPipeline->pInternal;

	MEED_ASSERT(pVulkanPipeline->pVertexShader != MEED_NULL);
	MEED_ASSERT(pVulkanPipeline->pFragmentShader != MEED_NULL);
	MEED_ASSERT(pVulkanPipeline->layout != MEED_NULL);

	struct VulkanShader* pVertexVulkanShader = (struct VulkanShader*)pVulkanPipeline->pVertexShader->pInternal;
	VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
	vertexShaderStageInfo.sType							  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage							  = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageInfo.pName							  = "main";
	vertexShaderStageInfo.module						  = pVertexVulkanShader->module;

	struct VulkanShader* pFragmentVulkanShader = (struct VulkanShader*)pVulkanPipeline->pFragmentShader->pInternal;
	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
	fragmentShaderStageInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageInfo.stage							= VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageInfo.pName							= "main";
	fragmentShaderStageInfo.module							= pFragmentVulkanShader->module;

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo, fragmentShaderStageInfo};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType								 = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount		 = 0;
	vertexInputInfo.pVertexBindingDescriptions			 = MEED_NULL;
	vertexInputInfo.vertexAttributeDescriptionCount		 = 0;
	vertexInputInfo.pVertexAttributeDescriptions		 = MEED_NULL;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType									 = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology								 = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable				 = VK_FALSE;

#if 0
	VkPipelineTessellationStateCreateInfo tessellationState = {};
	tessellationState.sType									= VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	tessellationState.patchControlPoints					= 0;
#endif

	VkViewport viewport = {};
	viewport.x			= 0.0f;
	viewport.y			= 0.0f;
	viewport.width		= (f32)g_vulkan->extent.width;
	viewport.height		= (f32)g_vulkan->extent.height;
	viewport.minDepth	= 0.0f;
	viewport.maxDepth	= 1.0f;

	VkRect2D scissor = {};
	scissor.offset	 = (VkOffset2D){0, 0};
	scissor.extent	 = g_vulkan->extent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType								= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount						= 1;
	viewportState.pViewports						= &viewport;
	viewportState.scissorCount						= 1;
	viewportState.pScissors							= &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType								  = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable						  = VK_FALSE;
	rasterizer.rasterizerDiscardEnable				  = VK_FALSE;
	rasterizer.polygonMode							  = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth							  = 1.0f;
	rasterizer.cullMode								  = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace							  = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable						  = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType								   = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable				   = VK_FALSE;
	multisampling.rasterizationSamples				   = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading					   = 1.0f;
	multisampling.pSampleMask						   = MEED_NULL;
	multisampling.alphaToCoverageEnable				   = VK_FALSE;
	multisampling.alphaToOneEnable					   = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType								   = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable					   = VK_TRUE;
	depthStencil.depthWriteEnable					   = VK_TRUE;
	depthStencil.depthCompareOp						   = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable				   = VK_FALSE;
	depthStencil.stencilTestEnable					   = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType								  = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.attachmentCount					  = 1;
	colorBlending.pAttachments						  = &colorBlendAttachment;
	colorBlending.logicOpEnable						  = VK_FALSE;
	colorBlending.logicOp							  = VK_LOGIC_OP_COPY;
	colorBlending.blendConstants[0]					  = 0.0f;
	colorBlending.blendConstants[1]					  = 0.0f;
	colorBlending.blendConstants[2]					  = 0.0f;
	colorBlending.blendConstants[3]					  = 0.0f;

	VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType							  = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount				  = MEED_ARRAY_SIZE(dynamicStates);
	dynamicState.pDynamicStates					  = dynamicStates;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType						= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount					= MEED_ARRAY_SIZE(shaderStages);
	pipelineCreateInfo.pStages						= shaderStages;
	pipelineCreateInfo.pVertexInputState			= &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState			= &inputAssembly;
	pipelineCreateInfo.pTessellationState			= MEED_NULL;
	pipelineCreateInfo.pViewportState				= &viewportState;
	pipelineCreateInfo.pRasterizationState			= &rasterizer;
	pipelineCreateInfo.pMultisampleState			= &multisampling;
	pipelineCreateInfo.pDepthStencilState			= &depthStencil;
	pipelineCreateInfo.pColorBlendState				= &colorBlending;
	pipelineCreateInfo.pDynamicState				= &dynamicState;
	pipelineCreateInfo.layout						= pVulkanPipeline->layout;
	pipelineCreateInfo.renderPass					= g_vulkan->renderPass;
	pipelineCreateInfo.subpass						= 0;

	VK_ASSERT(vkCreateGraphicsPipelines(
		g_vulkan->device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, MEED_NULL, &pVulkanPipeline->pipeline));

	meedReleaseStackPush(pPipeline->pReleaseStack, pPipeline, destroyPipeline);
}

static void destroyPipeline(void* pData)
{
	MEED_ASSERT(g_vulkan != MEED_NULL);
	MEED_ASSERT(g_vulkan->device != MEED_NULL);
	MEED_ASSERT(pData != MEED_NULL);
	struct MEEDPipeline*   pPipeline	   = (struct MEEDPipeline*)pData;
	struct VulkanPipeline* pVulkanPipeline = (struct VulkanPipeline*)pPipeline->pInternal;

	vkDestroyPipeline(g_vulkan->device, pVulkanPipeline->pipeline, MEED_NULL);
}

void meedPipelineDestroy(struct MEEDPipeline* pPipeline)
{
	MEED_ASSERT(pPipeline != MEED_NULL);
	MEED_ASSERT(pPipeline->pReleaseStack != MEED_NULL);

	meedReleaseStackDestroy(pPipeline->pReleaseStack);
	MEED_FREE(pPipeline, struct MEEDPipeline);
}

#endif // MEED_USE_VULKAN