// BlackJawz.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "BlackJawz.h"


BlackJawz::Engine::Engine()
{
	// Constructor implementation
	mWindowsApp = std::make_unique<Application::Application>();
}

BlackJawz::Engine::~Engine()
{
	// Deconstructor implementation

}

void BlackJawz::Engine::Setup(HINSTANCE hInstance, int nCmdShow)
{
	// Engine Setup implementation
	mWindowsApp->Initialise(hInstance, nCmdShow);
}

void BlackJawz::Engine::Run()
{
	// Engine Setup implementation

}

void BlackJawz::Engine::Cleanup()
{
	// Engine Setup implementation

}
