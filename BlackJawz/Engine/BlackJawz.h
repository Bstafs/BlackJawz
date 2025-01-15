#pragma once
#include "../pch.h"

#include "../Rendering/Rendering.h"

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
	};

}