#pragma once
#include "../pch.h"
#include "../Rendering/Rendering.h"

#include "../Editor/EditorCamera.h"

namespace BlackJawz::Editor
{
	struct Object
	{
		std::string name;
		std::string type;
	};

	class Editor
	{
	public:
		Editor();
		~Editor();

		void Render(Rendering::Render& renderer);
	private:
		void MenuBar();
		void ContentMenu();
		void Hierarchy(Rendering::Render& renderer);
		void ObjectProperties();
		void ViewPort(Rendering::Render& renderer);
	private:
		bool showImGuiDemo = false;
		std::vector<Object> objects;
		std::unique_ptr<BlackJawz::EditorCamera::EditorCamera> editorCamera;	
	};
}