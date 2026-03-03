#pragma once

namespace Axiom {
	class ResourceSet {
	public:

		virtual void updateBuffer() = 0; // TODO: add parameters
		virtual void updateTexture() = 0; // TODO: add parameters
	};
}
