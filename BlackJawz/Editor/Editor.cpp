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
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (showImGuiDemo)
	{
		ImGui::ShowDemoWindow();
	}

	renderer.BeginFrame();

	MenuBar();
	ContentMenu();
	Hierarchy();
	ObjectProperties();
	ViewPort(renderer);

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
	renderer.RenderToTexture();

	// Push zero padding style for the viewport window
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	// Begin the ImGui viewport window
	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImGui::Image((ImTextureID)renderer.GetShaderResourceView(), ImVec2(800, 600));

	ImGui::End(); // End the ImGui viewport window
	ImGui::PopStyleVar(); // Pop the style variable

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}