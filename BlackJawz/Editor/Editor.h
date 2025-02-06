#pragma once
#include "../pch.h"
#include "../Rendering/Rendering.h"
#include "../Editor/EditorCamera.h"

#include "../ECS/EntityManager.h"
#include "../ECS/Components.h"
#include "../ECS/Systems.h"
#include "../ECS/ComponentArray.h"
#include "../ECS/SystemManager.h"

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

		void Initialise(Rendering::Render& renderer);
		void Render(Rendering::Render& renderer);
	private:
		void MenuBar(Rendering::Render& renderer);
		void ContentMenu(Rendering::Render& renderer);
		void Hierarchy(Rendering::Render& renderer);
		void ObjectProperties();
		void ViewPort(Rendering::Render& renderer);

		void SaveScene(const std::string& filename, Rendering::Render& renderer);

		ComPtr<ID3D11Buffer> CreateBuffer(ID3D11Device* device, const std::vector<uint8_t>& data, UINT stride);
		void LoadScene(const std::string& filename, Rendering::Render& renderer);
		ComPtr<ID3D11ShaderResourceView> LoadTextureFromDDSData(ID3D11Device* device, const std::vector<uint8_t>& textureData);
	private:
		bool showImGuiDemo = false;
		std::vector<Object> objects;
		std::unique_ptr<BlackJawz::EditorCamera::EditorCamera> editorCamera;

		 std::vector<BlackJawz::Entity::Entity> entities;
		 std::unordered_map<BlackJawz::Entity::Entity, std::string> entityNames;
		 int selectedObject = -1;

		BlackJawz::Entity::EntityManager entityManager;
		BlackJawz::Component::ComponentArray<BlackJawz::Component::Transform> transformArray;
		BlackJawz::Component::ComponentArray<BlackJawz::Component::Appearance> appearanceArray;
		BlackJawz::Component::ComponentArray<BlackJawz::Component::Light> lightArray;

		BlackJawz::System::SystemManager systemManager;
		std::shared_ptr<BlackJawz::System::TransformSystem> transformSystem;
		std::shared_ptr<BlackJawz::System::AppearanceSystem> appearanceSystem;
		std::shared_ptr<BlackJawz::System::LightSystem> lightSystem;

		XMFLOAT3 cameraPosition;
		float cameraYaw;
		float cameraPitch;

		UINT directoryIcon = 0;
		UINT fileIcon = 0;

		std::filesystem::path currentPath;
	};
}