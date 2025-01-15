#include "Editor.h"

BlackJawz::Editor::Editor::Editor()
{

}

BlackJawz::Editor::Editor::~Editor()
{

}

void BlackJawz::Editor::Editor::UpdateEditor()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (showImGuiDemo)
	{
		ImGui::ShowDemoWindow();
	}

	MenuBar();
	ContentMenu();
	Hierarchy();
	ObjectProperties();
	ViewPort();
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

void BlackJawz::Editor::Editor::ViewPort()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	// Check if the viewport size has changed
	static ImVec2 previousSize = ImVec2(0, 0);
	if (viewportSize.x != previousSize.x || viewportSize.y != previousSize.y)
	{
		previousSize = viewportSize;

		// Resize your render target (DirectX 11 Texture2D and Render Target View)
		//ResizeRenderTarget((UINT)viewportSize.x, (UINT)viewportSize.y);
	}

	//RenderGameSceneToTexture();

	 // Get the ImGui texture ID from the DirectX render target
	//ID3D11ShaderResourceView* srv = GetRenderTargetShaderResourceView();
	//ImTextureID textureID = (ImTextureID)srv;

	// Display the render target in ImGui
	//ImGui::Image(textureID, viewportSize);

	ImGui::End();
	ImGui::PopStyleVar();
}