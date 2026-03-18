#include "UI.h"

namespace Axiom {
	UI* UI::instance = nullptr;

	UI::UI() {
		Buffer::CreateInfo vertexBufferCreateInfo = {
			.size = sizeof(UIVertex) * 4 * maxQuads,
			.usage = BufferUsage::Vertex,
			.memoryUsage = MemoryUsage::GPUandCPU
		};
		vertexBuffer = Renderer::createBuffer(vertexBufferCreateInfo);

		Buffer::CreateInfo indexBufferCreateInfo = {
			.size = sizeof(uint32_t) * 6 * maxQuads,
			.usage = BufferUsage::Index | BufferUsage::TransferDst,
			.memoryUsage = MemoryUsage::GPUOnly
		};
		indexBuffer = Renderer::createBuffer(indexBufferCreateInfo);

		Buffer::CreateInfo stagingBufferCreateInfo = {
			.size = sizeof(uint32_t) * 6 * maxQuads,
			.usage = BufferUsage::TransferSrc,
			.memoryUsage = MemoryUsage::GPUandCPU
		};
		std::unique_ptr<Buffer> stagingBuffer = Renderer::createBuffer(stagingBufferCreateInfo);

		std::vector<uint32_t> indices(maxQuads * 6);
		uint32_t offset = 0;
		for (size_t i = 0; i < indices.size(); i += 6) {
			indices[i + 0] = offset + 0;
			indices[i + 1] = offset + 1;
			indices[i + 2] = offset + 2;
			indices[i + 3] = offset + 0;
			indices[i + 4] = offset + 2;
			indices[i + 5] = offset + 3;
			offset += 4;
		}
		stagingBuffer->setData<uint32_t>(indices);
		auto commandBuffer = Renderer::beginSingleTimeCommands();
		commandBuffer->copyBuffer(stagingBuffer.get(), indexBuffer.get(), stagingBufferCreateInfo.size);
		Renderer::endSingleTimeCommands(commandBuffer.get());

		RenderAttachment colorAttachment{};
		colorAttachment.loadOp = LoadOp::Load;
		colorAttachment.storeOp = StoreOp::Store;
		colorAttachment.clearColor = Math::Vec4(0.0f);
		renderPass.colorAttachments[0] = colorAttachment;
		renderPass.colorAttachmentCount = 1;

		std::vector<VertexBindingDescription> vertexBindings = {
			{ .binding = 0, .stride = sizeof(UIVertex), .inputRate = VertexInputRate::Vertex }
		};
		std::vector<VertexAttributeDescription> vertexAttributes = {
			{ .location = 0, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(UIVertex, position) },
			{ .location = 1, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(UIVertex, uv) },
			{ .location = 2, .binding = 0, .format = Format::R32G32B32A32Sfloat, .offset = offsetof(UIVertex, color) },
		};

		Pipeline::CreateInfo pipelineCreateInfo = {
			.vertexShaderPath = "Assets/Shaders/BuiltIn.UI.vert",
			.fragmentShaderPath = "Assets/Shaders/BuiltIn.UI.frag",
			.vertexBindings = vertexBindings,
			.vertexAttributes = vertexAttributes,
			.topology = PrimitiveTopology::TriangleList,
			.polygonMode = PolygonMode::Fill,
			.cullMode = CullMode::None,
			.frontFaceClockwise = true,
			.enableBlending = true,
			.enableDepthTest = false,
			.enableDepthWrite = false,
			.colorAttachmentFormats = { Renderer::getCurrentRenderTarget()->getFormat() },
			.depthAttachmentFormat = Format::Undefined,
			.resourceLayouts = {}
		};
		pipeline = Renderer::createPipeline(pipelineCreateInfo);
	}

	void UI::init() {
		AX_CORE_LOG_INFO("Initializing UI system...");
		if (instance) {
			AX_CORE_LOG_WARN("UI system already initialized!");
			return;
		}
		instance = new UI();
	}

	void UI::shutdown() {
		AX_CORE_LOG_INFO("Shutting down UI system...");
		if (instance == nullptr) {
			AX_CORE_LOG_WARN("UI system not initialized!");
			return;
		}
		delete instance;
		instance = nullptr;
	}

	void UI::beginFrame() {
		instance->hotItem = 0;
		instance->vertices.clear();
	}

	void UI::endFrame() {
		if (!instance->isMouseButtonOneDown) {
			instance->activeItem = 0;
		}
		instance->shouldConsumeMouse = (instance->hotItem != 0 || instance->activeItem != 0);
	}

	void UI::render(CommandBuffer* commandBuffer) {
		if (instance->vertices.empty()) {
			return;
		}
		instance->vertexBuffer->setData<UIVertex>(instance->vertices);

		instance->renderPass.colorAttachments[0].texture = Renderer::getCurrentRenderTarget();
		Math::Vec2 renderTargetSize = Renderer::getCurrentRenderTargetSize();
		instance->renderPass.width = static_cast<uint32_t>(renderTargetSize.x());
		instance->renderPass.height = static_cast<uint32_t>(renderTargetSize.y());

		Math::Mat4 projection = Math::Mat4::orthographic(0.0f, renderTargetSize.x(), 0.0f, renderTargetSize.y(), -1.0f, 1.0f);

		commandBuffer->beginRendering(instance->renderPass);
		commandBuffer->bindPipeline(instance->pipeline.get());
		commandBuffer->bindPushConstants(&projection, sizeof(projection));
		commandBuffer->bindVertexBuffers({ instance->vertexBuffer.get() });
		commandBuffer->bindIndexBuffer(instance->indexBuffer.get());
		commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());
		commandBuffer->setScissor(0, 0, renderTargetSize.x(), renderTargetSize.y());
		commandBuffer->drawIndexed(static_cast<uint32_t>(instance->vertices.size() / 4) * 6, 1, 0, 0, 0);
		commandBuffer->endRendering();
	}

	void UI::setMousePosition(Math::Vec2 pos) {
		instance->mousePosition = pos;
	}

	void UI::setMouseButtonState(KeyCode button, bool pressed) {
		switch (button) {
			case KeyCode::LeftButton:
			instance->isMouseButtonOneDown = pressed;
			break;
			case KeyCode::RightButton:
			instance->isMouseButtonTwoDown = pressed;
			default:
				break;
		}
	}

	bool UI::shouldConsumeMouseEvents() {
		return instance->shouldConsumeMouse;
	}

	bool UI::button(const std::string& label, Math::Vec2 pos, Math::Vec2 size) {
		if (instance->vertices.size() / 4 >= maxQuads) {
			AX_CORE_LOG_WARN("UI Quad limit reached! Cannot draw button: {0}", label);
			return false;
		}

		uint32_t id = std::hash<std::string>{}(label);
		bool isClicked = false;

		bool isHovering = instance->mousePosition.x() >= pos.x() && instance->mousePosition.x() <= pos.x() + size.x() &&
			instance->mousePosition.y() >= pos.y() && instance->mousePosition.y() <= pos.y() + size.y();
		if (isHovering) {
			instance->hotItem = id;
			if (instance->activeItem == 0 && instance->isMouseButtonOneDown) {
				instance->activeItem = id;
			}
		}

		Math::Vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }; // Default: Dark Gray

		if (instance->hotItem == id) {
			if (instance->activeItem == id) {
				color = { 1.0f, 0.0f, 0.0f, 1.0f }; // Active: Darker Gray
			} else {
				color = { 0.0f, 1.0f, 0.0f, 1.0f }; // Hover: Lighter Gray
			}
		}

		if (instance->activeItem == id && instance->hotItem == id && !instance->isMouseButtonOneDown) {
			isClicked = true;
		}

		// Top-Left
		instance->vertices.push_back({ {pos.x(), pos.y()}, {0.0f, 0.0f}, color });
		// Bottom-Left					 
		instance->vertices.push_back({ {pos.x(), pos.y() + size.y()}, {0.0f, 1.0f}, color });
		// Bottom-Right
		instance->vertices.push_back({ {pos.x() + size.x(), pos.y() + size.y()}, {1.0f, 1.0f}, color });
		// Top-Right					 
		instance->vertices.push_back({ {pos.x() + size.x(), pos.y()}, {1.0f, 0.0f}, color });

		return isClicked;
	}
}
