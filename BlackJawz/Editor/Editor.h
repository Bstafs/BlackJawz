#pragma once
#include "../pch.h"

namespace BlackJawz::Editor
{
	class Editor
	{
	public:
		Editor();
		~Editor();

		void UpdateEditor();
	private:
		void MenuBar();
		void ContentMenu();
		void Hierarchy();
		void ObjectProperties();
		void ViewPort();


	private:
		bool showImGuiDemo = false;
	};
}