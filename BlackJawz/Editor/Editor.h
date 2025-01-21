#pragma once
#include "../pch.h"
#include "../Rendering/Rendering.h"

namespace BlackJawz::Editor
{
	class Editor
	{
	public:
		Editor();
		~Editor();

		void Render(Rendering::Render& renderer);

		ImVec2 GetViewPortSize() { return viewPortsize; }
	private:
		void MenuBar();
		void ContentMenu();
		void Hierarchy();
		void ObjectProperties();
		void ViewPort(Rendering::Render& renderer);

	private:
		bool showImGuiDemo = false;
		ImVec2 viewPortsize;
	};
}