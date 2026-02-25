#include "UViewerApplication.hpp"
#include "UContext.hpp"
#include "UInput.hpp"
#include "../lib/glfw/deps/glad/gl.h"

#include <filesystem>
#include <imgui.h>
#include "ImGuizmo.h"
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

#include "IconsForkAwesome.h"
#include "IconsLucide.h"

static UContext* ResizeContext = nullptr;

void HandleFramebufferResize(GLFWwindow* window, int w, int h){
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	if(ResizeContext != nullptr){
		//todo
	}
}

UViewerApplication::UViewerApplication() {
	mWindow = nullptr;
	mContext = nullptr;
}

bool UViewerApplication::Setup() {
	// Initialize GLFW
	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
	glfwWindowHint(GLFW_SAMPLES, 4);

	mWindow = glfwCreateWindow(1280, 720, "LM Studio", nullptr, nullptr);
	if (mWindow == nullptr) {
		glfwTerminate();
		return false;
	}

	UInput::SetWindow(mWindow);

	glfwSetKeyCallback(mWindow, UInput::GLFWKeyCallback);
	glfwSetCursorPosCallback(mWindow, UInput::GLFWMousePositionCallback);
	glfwSetMouseButtonCallback(mWindow, UInput::GLFWMouseButtonCallback);
	glfwSetScrollCallback(mWindow, UInput::GLFWMouseScrollCallback);
	glfwSetFramebufferSizeCallback(mWindow, HandleFramebufferResize);

	glfwMakeContextCurrent(mWindow);
	gladLoadGL(glfwGetProcAddress);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glfwSwapInterval(1);

	//glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageCallback(DealWithGLErrors, nullptr);

	// Initialize imgui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
	ImGui_ImplOpenGL3_Init("#version 150");

	std::filesystem::path RES_BASE_PATH = std::filesystem::current_path() / "res";

	if(std::filesystem::exists((RES_BASE_PATH / "font" / "NotoSansJP-Regular.otf"))){
		io.Fonts->AddFontFromFileTTF((RES_BASE_PATH / "font" / "NotoSansJP-Regular.otf").string().c_str(), 16.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	}

	if(std::filesystem::exists((RES_BASE_PATH / "font" / "lucide.ttf"))){
		static const ImWchar icons_ranges[] = { ICON_MIN_LC, ICON_MAX_16_LC, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.GlyphOffset.y = 2.25f;
		icons_config.GlyphMinAdvanceX = 14.0f;
		io.Fonts->AddFontFromFileTTF((RES_BASE_PATH / "font" / "lucide.ttf").string().c_str(), icons_config.GlyphMinAdvanceX, &icons_config, icons_ranges );
	}

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TitleBg] = ImColor(0x22, 0x22, 0x22, 0xFF);
	style.Colors[ImGuiCol_TitleBgActive] = ImColor(0x22, 0x22, 0x22, 0xFF);
	style.Colors[ImGuiCol_WindowBg] = ImColor(0x22, 0x22, 0x22, 0xFF);

	style.Colors[ImGuiCol_MenuBarBg] = ImColor(0x28, 0x28, 0x28, 0xFF);

	style.Colors[ImGuiCol_Button] = ImColor(0x3F, 0x6D, 0x65, 0xFF);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(0x4A, 0x93, 0x84, 0xFF);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(0x5A, 0xA3, 0x94, 0xFF);

	style.Colors[ImGuiCol_Tab] = ImColor(0x3F, 0x6D, 0x65, 0xFF);
	style.Colors[ImGuiCol_TabActive] = ImColor(0x4A, 0x93, 0x84, 0xFF);
	style.Colors[ImGuiCol_TabHovered] = ImColor(0x5A, 0xA3, 0x94, 0xFF);

	style.Colors[ImGuiCol_TabUnfocused] = ImColor(0x3F, 0x6D, 0x65, 0xFF);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImColor(0x3F, 0x6D, 0x65, 0xFF);

	style.Colors[ImGuiCol_TabDimmed] = ImColor(0x3F, 0x6D, 0x65, 0xFF);
	style.Colors[ImGuiCol_TabDimmedSelected] = ImColor(0x4A, 0x93, 0x84, 0xFF);


	style.Colors[ImGuiCol_ResizeGripActive] = ImColor(0x4A, 0x93, 0x84, 0xFF);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImColor(0x5A, 0xA3, 0x94, 0xFF);
	style.Colors[ImGuiCol_ResizeGrip] = ImColor(0x3F, 0x6D, 0x65, 0xFF);

	style.Colors[ImGuiCol_SliderGrab] = ImColor(0x3F, 0x6D, 0x65, 0xFF);
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(0x4A, 0x93, 0x84, 0xFF);

	style.Colors[ImGuiCol_ChildBg] = ImColor(0x28, 0x28, 0x28, 0xFF);
	style.Colors[ImGuiCol_PopupBg] = ImColor(0x28, 0x28, 0x28, 0xFF);

	style.FrameBorderSize = 1.5f;
	style.Colors[ImGuiCol_FrameBg] = ImColor(0x28, 0x28, 0x28, 0xFF);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(0x28, 0x28, 0x28, 0xFF);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(0x32, 0x32, 0x32, 0xFF);;
	style.Colors[ImGuiCol_CheckMark] = ImColor(0x4A, 0x93, 0x84, 0xFF);


	style.Colors[ImGuiCol_HeaderHovered] = ImColor(0x4A, 0x93, 0x84, 0xFF);
	style.Colors[ImGuiCol_HeaderActive] = ImColor(0x4A, 0x93, 0x84, 0xFF);
	style.Colors[ImGuiCol_Header] = ImColor(0x4A, 0x93, 0x84, 0xFF);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImColor(0x18, 0x18, 0x18, 0xA0);

	style.FrameRounding = 2.f;
	style.FramePadding = ImVec2(5.0f,3.0f);

	// #ECECEC
	// #84934A
	// #656D3F
	// #492828

	glEnable(GL_MULTISAMPLE);

	// Create viewer context
	mContext = new UContext();
	ResizeContext = mContext;

	return true;
}

bool UViewerApplication::Teardown() {

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(mWindow);
	glfwTerminate();

	delete mContext;

	return true;
}

bool UViewerApplication::Execute(float deltaTime) {
	// Try to make sure we return an error if anything's fucky
	if (mContext == nullptr || mWindow == nullptr || glfwWindowShouldClose(mWindow))
		return false;

	// Update viewer context
	mContext->Update(deltaTime);

	// Begin actual rendering
	glfwMakeContextCurrent(mWindow);
	glfwPollEvents();

	UInput::UpdateInputState();

	// The context renders both the ImGui elements and the background elements.
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Update buffer size
	int width, height;
	glfwGetFramebufferSize(mWindow, &width, &height);
	glViewport(0, 0, width, height);

	glDepthMask(true);
	// Clear buffers
	glClearColor(0.100f, 0.261f, 0.402f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render viewer context
	mContext->Render(deltaTime);

	// Render imgui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Swap buffers
	glfwSwapBuffers(mWindow);

	return true;
}
