#pragma once

namespace Axiom {
	enum class CommandBufferState {
		READY,
		RECORDING,
		IN_RENDER_PASS,
		RECORDING_ENDED,
		SUBMITTED,
		NOT_ALLOCATED
	};

	class CommandBuffer {
	public:
		virtual ~CommandBuffer() = default;

		virtual void begin(bool isSingleUse, bool isRenderPassCont, bool isSimultaneous) = 0;
		virtual void end() = 0;
		virtual void reset() = 0;

		void setState(CommandBufferState newState) { state = newState; }
		
	protected:
		CommandBufferState state = CommandBufferState::NOT_ALLOCATED;
	};
}
