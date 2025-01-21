#include "Editor.h"

BlackJawz::Editor::Editor::Editor()
{

}

BlackJawz::Editor::Editor::~Editor()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void BlackJawz::Editor::Editor::Render(Rendering::Render& renderer)
{
	// Begin ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Create a full-window dockable space
	static bool dockspaceOpen = true;
	static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

	// Create a parent window to hold the dockspace
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background for the parent window

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("DockSpace Root", &dockspaceOpen, windowFlags);

	// Create the dockspace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) 
	{
		ImGuiID dockspaceID = ImGui::GetID("MainDockspace");
		ImGui::DockSpace(dockspaceID, ImVec2(0, 0), dockspaceFlags);
	}
	else 
	{
		ImGui::Text("Docking is not enabled! Please enable it in ImGui configuration.");
	}
	ImGui::End();
	ImGui::PopStyleVar(); // Pop the style variable

	// Optionally show ImGui demo window
	if (showImGuiDemo) {
		ImGui::ShowDemoWindow();
	}

	// Begin rendering the main content
	renderer.BeginFrame();

	// Render editor components
	MenuBar();          // Menu at the top
	ContentMenu();      // Content browser (dockable)
	Hierarchy();        // Hierarchy window (dockable)
	ObjectProperties(); // Object properties (dockable)
	ViewPort(renderer); // Viewport (dockable)

	// End rendering frame
	renderer.EndFrame();
}

void BlackJawz::Editor::Editor::MenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Main Menu"))
		{
			if (ImGui::MenuItem("New Project"))
			{

			}

			if (ImGui::MenuItem("Open Project"))
			{

			}

			if (ImGui::MenuItem("Save.."))
			{

			}

			if (ImGui::MenuItem("Save as.."))
			{

			}

			if (ImGui::MenuItem("Exit"))
			{
				PostQuitMessage(0);
			}

			ImGui::EndMenu();
		}

		ImGui::Separator();

		if (ImGui::BeginMenu("Debug"))
		{
			ImGui::MenuItem("Demo Window", "", &showImGuiDemo);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

}

void BlackJawz::Editor::Editor::ContentMenu()
{
	ImGui::Begin("Content Browser");

	ImGui::End();
}

void BlackJawz::Editor::Editor::Hierarchy()
{
	ImGui::Begin("Hierarchy");

	ImGui::End();
}

void BlackJawz::Editor::Editor::ObjectProperties()
{
	ImGui::Begin("ObjectProperties");

	ImGui::End();
}

void BlackJawz::Editor::Editor::ViewPort(Rendering::Render& renderer)
{
	// Push zero padding style for the viewport window
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	// Begin the ImGui viewport window
	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	static ImVec2 lastViewportSize = ImVec2(viewportSize.x, viewportSize.y); // Initial size
	if (viewportSize.x != lastViewportSize.x || viewportSize.y != lastViewportSize.y) {
		renderer.ResizeRenderTarget(static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));
		lastViewportSize = viewportSize;
	}

	renderer.RenderToTexture();

	ImGui::Image((ImTextureID)renderer.GetShaderResourceView(), viewportSize);

	ImGui::End(); // End the ImGui viewport window
	ImGui::PopStyleVar(); // Pop the style variable
}