// BlackJawz.cpp : Defines the functions for the static library.
//
#include "BlackJawz.h"

BlackJawz::Engine::Engine()
{
	// Constructor implementation
	mWindowsApp = std::make_unique<Application::Application>();
	mRendering = std::make_unique<Rendering::Render>();
	mEditor = std::make_unique<Editor::Editor>();
}

BlackJawz::Engine::~Engine()
{
	// Deconstructor implementation

}

void BlackJawz::Engine::Setup(HINSTANCE hInstance, int nCmdShow)
{
	// Engine Setup implementation
	mWindowsApp->Initialise(hInstance, nCmdShow);

	mRendering->Initialise();
}

void BlackJawz::Engine::Run()
{
	// Engine Setup implementation
	mEditor->Render(*mRendering);
	mRendering->Update();
}

void BlackJawz::Engine::Cleanup()
{
	// Engine Setup implementation

}
