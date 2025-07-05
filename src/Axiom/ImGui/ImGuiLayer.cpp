#include "axpch.h"
#include "ImGuiLayer.h"

namespace Axiom {
	ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {

	}

	ImGuiLayer::~ImGuiLayer() {

	}

	void ImGuiLayer::onAttach() {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui::StyleColorsDark();

		Application& app = Application::get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.getWindow().getNativeWindow());

		ImGui_ImplGlfw_InitForVulkan(window, true);
	}

	void ImGuiLayer::onDetach() {

	}
	
	void ImGuiLayer::onUpdate() {

	}

	void ImGuiLayer::onEvent(Event& event) {

	}

	void ImGuiLayer::begin() {
		
	}

	void ImGuiLayer::end() {

	}
}
