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
	private:
		void MenuBar();
		void ContentMenu();
		void Hierarchy();
		void ObjectProperties();
		void ViewPort(Rendering::Render& renderer);
		void CreatePrimitiveObject(const std::string& objectType);
	private:
		bool showImGuiDemo = false;
		std::vector<std::string> objects;
	};
}