#pragma once
#include "../pch.h"

namespace BlackJawz::Editor
{
	class Editor
	{
	public:
		Editor();
		~Editor();

		void UpdateEditor(ID3D11ShaderResourceView* viewPortSRV);

		ImVec2 GetViewPortSize() { return viewPortsize; }
	private:
		void MenuBar();
		void ContentMenu();
		void Hierarchy();
		void ObjectProperties();
		void ViewPort(ID3D11ShaderResourceView* viewPortSRV);

	private:
		bool showImGuiDemo = false;
		ImVec2 viewPortsize;
	};
}