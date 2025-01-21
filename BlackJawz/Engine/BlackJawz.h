#pragma once
#include "../pch.h"

#include "../Rendering/Rendering.h"
#include "../Editor/Editor.h"

namespace BlackJawz
{
	class Engine
	{
	public:
		Engine();
		~Engine();

		void Setup(HINSTANCE hInstance, int nCmdShow);
		void Run();
		void Cleanup();

	private:
		std::unique_ptr<Application::Application> mWindowsApp;
		std::unique_ptr<Rendering::Render> mRendering;
		std::unique_ptr<Editor::Editor> mEditor;
	};

}