#include "Editor.h"

BlackJawz::Editor::Editor::Editor()
{
	editorCamera = std::make_unique<BlackJawz::EditorCamera::EditorCamera>(
		45.0f,
		BlackJawz::Application::Application::GetWindowWidth() / BlackJawz::Application::Application::GetWindowHeight(),
		0.001f, 10000.0f
	);

	cameraPitch = 0.0f;
	cameraYaw = 0.0f;
	cameraPosition = XMFLOAT3(0.0f, 0.0f, -5.0f);

	transformSystem = systemManager.RegisterSystem<BlackJawz::System::TransformSystem>(transformArray);
	appearanceSystem = systemManager.RegisterSystem<BlackJawz::System::AppearanceSystem>(appearanceArray);
}

BlackJawz::Editor::Editor::~Editor()
{

}

void BlackJawz::Editor::Editor::Render(Rendering::Render& renderer)
{
	// Begin ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Create a full-window dockable space
	static bool dockspaceOpen = true;
	static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

	// Create a parent window to hold the dockspace
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background for the parent window

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("DockSpace Root", &dockspaceOpen, windowFlags);

	// Create the dockspace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspaceID = ImGui::GetID("MainDockspace");
		ImGui::DockSpace(dockspaceID, ImVec2(0, 0), dockspaceFlags);
	}
	else
	{
		ImGui::Text("Docking is not enabled! Please enable it in ImGui configuration.");
	}
	ImGui::End();
	ImGui::PopStyleVar(); // Pop the style variable

	// ImGui demo window
	if (showImGuiDemo)
	{
		ImGui::ShowDemoWindow();
	}

	// Begin rendering the main content
	renderer.BeginFrame();

	editorCamera->SetPosition(cameraPosition);
	editorCamera->SetRotation(cameraPitch, cameraYaw);

	renderer.SetViewMatrix(editorCamera->GetViewMatrix());
	renderer.SetProjectionMatrix(editorCamera->GetProjectionMatrix());

	// Render editor components
	MenuBar(renderer);          // Menu at the top
	ContentMenu();      // Content browser (dockable)
	Hierarchy(renderer);        // Hierarchy window (dockable)
	ObjectProperties(); // Object properties (dockable)
	ViewPort(renderer); // Viewport (dockable)

	// End rendering frame
	renderer.EndFrame();
}

// Utility function to extract data from a ID3D11Buffer
std::vector<uint8_t> ExtractBufferData(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Buffer* buffer)
{
	// Get original buffer description.
	D3D11_BUFFER_DESC desc;
	buffer->GetDesc(&desc);

	// Create a staging buffer description.
	D3D11_BUFFER_DESC stagingDesc = desc;
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.BindFlags = 0;                // No bind flags for staging
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	// Create the staging buffer.
	ComPtr<ID3D11Buffer> stagingBuffer;
	HRESULT hr = device->CreateBuffer(&stagingDesc, nullptr, stagingBuffer.GetAddressOf());
	if (FAILED(hr)) 
	{
		// Handle error.
		return {};
	}

	// Copy the content of the original GPU buffer into the staging buffer.
	context->CopyResource(stagingBuffer.Get(), buffer);

	// Map the staging buffer for reading.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = context->Map(stagingBuffer.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
	if (FAILED(hr)) 
	{
		// Handle error.
		return {};
	}

	// Copy the data from the mapped staging resource.
	uint8_t* dataPtr = static_cast<uint8_t*>(mappedResource.pData);
	std::vector<uint8_t> data(dataPtr, dataPtr + desc.ByteWidth);

	context->Unmap(stagingBuffer.Get(), 0);
	return data;
}

void BlackJawz::Editor::Editor::SaveScene(const std::string& filename, Rendering::Render& renderer)
{
	// Create a FlatBufferBuilder with an initial size (in bytes)
	flatbuffers::FlatBufferBuilder builder(1024);

	// This vector will hold the offsets to each serialized Entity.
	std::vector<flatbuffers::Offset<ECS::Entity>> entityOffsets;

	// Iterate through all entities in the scene.
	for (auto entity : entities) 
	{
		// Get the name from the map
		auto nameStr = entityNames[entity];
		auto nameOffset = builder.CreateString(nameStr);

		// --- Serialize the Transform component ---
		auto& transform = transformArray.GetData(entity);
		// Update the world matrix (if needed) before serializing.
		transform.UpdateWorldMatrix();

		// Create vectors for position, rotation, and scale.
		auto posVec = builder.CreateVector(std::vector<float>{
			transform.position.x, transform.position.y, transform.position.z});
		auto rotVec = builder.CreateVector(std::vector<float>{
			transform.rotation.x, transform.rotation.y, transform.rotation.z});
		auto scaleVec = builder.CreateVector(std::vector<float>{
			transform.scale.x, transform.scale.y, transform.scale.z});

		// Serialize the world matrix as a flat array of 16 floats.
		float* matrixPtr = reinterpret_cast<float*>(&transform.worldMatrix);
		auto worldMatrixVec = builder.CreateVector(std::vector<float>(
			matrixPtr, matrixPtr + 16));

		auto transformOffset = ECS::CreateTransform(builder,
			posVec, rotVec, scaleVec, worldMatrixVec);

		// --- Serialize the Appearance (and its Geometry) component ---
		auto& appearanceComp = appearanceArray.GetData(entity);
		auto geometry = appearanceComp.GetGeometry();

		// Extract the vertex and index buffer data
		auto vertexData = ExtractBufferData(renderer.GetDevice(), renderer.GetDeviceContext(), geometry.pVertexBuffer.Get());
		auto indexData = ExtractBufferData(renderer.GetDevice(), renderer.GetDeviceContext(), geometry.pIndexBuffer.Get());

		// Create FlatBuffers vectors for the vertex and index data
		auto vertexBufferVec = builder.CreateVector(vertexData);
		auto indexBufferVec = builder.CreateVector(indexData);

		auto geometryOffset = ECS::CreateGeometry(builder,
			geometry.IndicesCount,
			geometry.vertexBufferStride,
			geometry.vertexBufferOffset,
			vertexBufferVec,
			indexBufferVec
			);
		auto appearanceOffset = ECS::CreateAppearance(builder, geometryOffset);

		// --- Create the Entity ---
		// Here we assume that CreateEntity takes the following parameters:
		// (builder, id, name, transform, appearance)
		// Adjust the parameter order and types according to your generated code.
		auto entityOffset = ECS::CreateEntity(builder,
			static_cast<uint32_t>(entity),  // Assuming your entity ID is convertible to uint32_t
			nameOffset,
			transformOffset,
			appearanceOffset);

		// Add the created Entity offset to our vector.
		entityOffsets.push_back(entityOffset);
	}

	// Create a FlatBuffers vector from the vector of Entity offsets.
	auto entitiesVector = builder.CreateVector(entityOffsets);

	// Create the final Scene table with the entities vector.
	auto sceneOffset = ECS::CreateScene(builder, entitiesVector);

	// Finish the buffer.
	builder.Finish(sceneOffset);

	uint8_t* buf = builder.GetBufferPointer();
	int size = builder.GetSize();

	// Write the serialized buffer to a binary file.
	std::ofstream outFile(filename, std::ios::binary);
	if (!outFile) 
	{
		// Handle file open error.
		return;
	}
	outFile.write(reinterpret_cast<char*>(buf), size);
	outFile.close();
}

ComPtr<ID3D11Buffer> BlackJawz::Editor::Editor::CreateBuffer(ID3D11Device* device, const std::vector<uint8_t>& data, UINT stride)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = static_cast<UINT>(data.size());
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = data.data();

	ComPtr<ID3D11Buffer> buffer;
	HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, buffer.GetAddressOf());
	if (FAILED(hr)) 
	{
		// Handle buffer creation error
		return nullptr;
	}

	return buffer;
}

void BlackJawz::Editor::Editor::LoadScene(const std::string& filename, Rendering::Render& renderer)
{
	// Open the binary file
	std::ifstream inFile(filename, std::ios::binary);
	if (!inFile) 
	{
		// Handle file open error
		return;
	}

	// Read the file into a buffer
	std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
	inFile.close();

	// Get the FlatBuffer scene from the buffer
	auto scene = ECS::GetScene(buffer.data());

	if (!scene) 
	{
		// Handle invalid scene data
		return;
	}

	// Clear the current scene before loading a new one
	for (auto entity : entities) 
	{
		entityManager.DestroyEntity(entity);
		transformArray.RemoveData(entity);
		appearanceArray.RemoveData(entity);
	}
	entities.clear();
	entityNames.clear();

	// Load entities from the FlatBuffer
	auto entitiesVector = scene->entities();
	if (!entitiesVector) return;

	for (const auto* entityData : *entitiesVector)
	{
		if (!entityData) continue;

		// Create a new entity
		BlackJawz::Entity::Entity newEntity = entityManager.CreateEntity();
		entities.push_back(newEntity);

		// Load entity name
		if (entityData->name())
		{
			entityNames[newEntity] = entityData->name()->str();
		}

		// Load transform component
		if (entityData->transform())
		{
			auto transformData = entityData->transform();

			BlackJawz::Component::Transform transform;
			if (transformData->position()) {
				auto pos = transformData->position();
				transform.position = { pos->Get(0), pos->Get(1), pos->Get(2) };
			}
			if (transformData->rotation()) {
				auto rot = transformData->rotation();
				transform.rotation = { rot->Get(0), rot->Get(1), rot->Get(2) };
			}
			if (transformData->scale()) {
				auto scale = transformData->scale();
				transform.scale = { scale->Get(0), scale->Get(1), scale->Get(2) };
			}

			// Calculate the world matrix from position, rotation, and scale
			DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z)
				* DirectX::XMMatrixRotationRollPitchYaw(transform.rotation.x, transform.rotation.y, transform.rotation.z)
				* DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z);

			// Store the world matrix in the transform component
			XMStoreFloat4x4(&transform.worldMatrix, worldMatrix);

			transform.UpdateWorldMatrix();

			transformArray.InsertData(newEntity, transform);
		}

		// Load appearance component
		if (entityData->appearance() && entityData->appearance()->geometry())
		{
			auto geometryData = entityData->appearance()->geometry();

			// Create vertex buffer from FlatBuffer data
			auto vertexData = geometryData->vertex_buffer();
			std::vector<uint8_t> vertexBufferData(vertexData->begin(), vertexData->end());
			ComPtr<ID3D11Buffer> vertexBuffer = CreateBuffer(renderer.GetDevice(), vertexBufferData, geometryData->vertex_buffer_stride());

			// Create index buffer from FlatBuffer data
			auto indexData = geometryData->index_buffer();
			std::vector<uint8_t> indexBufferData(indexData->begin(), indexData->end());
			ComPtr<ID3D11Buffer> indexBuffer = CreateBuffer(renderer.GetDevice(), indexBufferData, sizeof(uint32_t)); // Assuming uint32_t indices

			//BlackJawz::Component::Geometry geometry = renderer.CreateCubeGeometry();
			BlackJawz::Component::Geometry geometry;
			geometry.IndicesCount = geometryData->indices_count();
			geometry.vertexBufferStride = geometryData->vertex_buffer_stride();
			geometry.vertexBufferOffset = geometryData->vertex_buffer_offset();
			geometry.pVertexBuffer = vertexBuffer;
			geometry.pIndexBuffer = indexBuffer;

			// Save geometry data and buffers in appearance
			BlackJawz::Component::Appearance appearance(geometry);
			appearanceArray.InsertData(newEntity, appearance);
		}

		// Set the ECS signature
		std::bitset<32> signature;
		signature.set(0); // Assume Transform is component 0
		signature.set(1); // Assume Appearance is component 1
		entityManager.SetSignature(newEntity, signature);

		transformSystem->AddEntity(newEntity);
		systemManager.SetSignature<BlackJawz::System::TransformSystem>(signature);

		appearanceSystem->AddEntity(newEntity);
		systemManager.SetSignature<BlackJawz::System::AppearanceSystem>(signature);
	}
}

void BlackJawz::Editor::Editor::MenuBar(Rendering::Render& renderer)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Main Menu"))
		{
			if (ImGui::MenuItem("Create New Project.."))
			{

			}

			if (ImGui::MenuItem("Open Project.."))
			{

			}

			if (ImGui::MenuItem("Save Project.."))
			{

			}

			if (ImGui::MenuItem("Save Project as.."))
			{

			}

			if (ImGui::MenuItem("Save Scene.."))
			{
				SaveScene("Scenes/ecs.bin", renderer);
			}


			if (ImGui::MenuItem("Load Scene.."))
			{
				LoadScene("Scenes/ecs.bin", renderer);
			}

			if (ImGui::MenuItem("Exit"))
			{
				PostQuitMessage(0);
			}

			ImGui::EndMenu();
		}

		ImGui::Separator();

		if (ImGui::BeginMenu("Debug"))
		{
			ImGui::MenuItem("Demo Window", "", &showImGuiDemo);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

}

void BlackJawz::Editor::Editor::ContentMenu()
{
	ImGui::Begin("Content Browser");

	ImGui::End();
}

void BlackJawz::Editor::Editor::Hierarchy(Rendering::Render& renderer)
{
	ImGui::Begin("Hierarchy");

	// Track the selected object
	static bool isRenaming = false; // Flag to track  renaming mode
	static char renameBuffer[128] = ""; // Buffer to store the name while renaming
	bool objectContextMenuOpened = false; // Tracks if an object-specific context menu is opened

	// Loop through all objects
	for (size_t i = 0; i < entities.size(); i++)
	{
		// Push a unique ID for each object
		ImGui::PushID(static_cast<int>(i));

		// Display each object in the hierarchy
		bool isSelected = selectedObject == static_cast<int>(i);
		if (ImGui::Selectable(entityNames[entities[i]].c_str(), isSelected))
		{
			selectedObject = static_cast<int>(i);
		}

		// Handle double-click to start renaming
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !isRenaming)
		{
			// Enter renaming mode
			isRenaming = true;
			strncpy_s(renameBuffer, sizeof(renameBuffer), entityNames[entities[i]].c_str(), _TRUNCATE); // Copy current name
			selectedObject = static_cast<int>(i); // Keep track of which object is being renamed
		}

		// Handle renaming input inline
		if (isRenaming && selectedObject == static_cast<int>(i))
		{
			ImGui::SetKeyboardFocusHere(); // Focus on the rename input

			// Create an input field for the object name to be edited inline
			if (ImGui::InputText("##Rename", renameBuffer, sizeof(renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
			{
				// When Enter is pressed, update the name and exit renaming mode
				entityNames[entities[i]] = renameBuffer;
				isRenaming = false; // Exit renaming mode
			}

			// Optional: If Escape is pressed, cancel renaming and revert name
			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				isRenaming = false; // Cancel renaming and keep the old name
			}
		}

		// Check if the object is right-clicked and open its context menu
		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("ObjectContextMenu");
			objectContextMenuOpened = true; // Mark that an object-specific menu is opened
		}

		// Object-specific context menu
		if (ImGui::BeginPopup("ObjectContextMenu"))
		{
			if (ImGui::MenuItem("Rename"))
			{
				// Set renaming mode if the "Rename" option is selected
				strncpy_s(renameBuffer, sizeof(renameBuffer), entityNames[entities[i]].c_str(), _TRUNCATE); // Copy current name
				isRenaming = true; // Set renaming mode
				selectedObject = static_cast<int>(i); // Track the selected object
				ImGui::CloseCurrentPopup();
				objectContextMenuOpened = false; // Ensure no lingering state
			}

			if (ImGui::MenuItem("Delete"))
			{
				if (selectedObject >= 0 && selectedObject < entities.size())
				{
					// Store the entity to be destroyed
					auto entity = entities[selectedObject];

					// Remove from the entities list first
					entities.erase(entities.begin() + selectedObject);

					// Destroy the entity in ECS (after removing it from the list)
					entityManager.DestroyEntity(entity);

					transformSystem->RemoveEntity(entity);
					appearanceSystem->RemoveEntity(entity);

					// Reset the selected object index
					selectedObject = -1;
				}

				// Adjust the selected object index
				if (selectedObject == static_cast<int>(i))
					selectedObject = -1; // Deselect if the deleted object was selected
				else if (selectedObject > static_cast<int>(i))
					selectedObject--; // Shift selection if needed

				ImGui::CloseCurrentPopup();
				objectContextMenuOpened = false; // Ensure no lingering state
			}

			ImGui::EndPopup();
		}

		// Pop the unique ID
		ImGui::PopID();
	}

	// Prevent the "Add Object" menu from appearing when right-clicking an object
	if (!objectContextMenuOpened && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		ImGui::OpenPopup("HierarchyContextMenu"); // Open the Add Object menu
	}

	int cubeCounter = 0;
	int sphereCounter = 0;

	// Add a right-click context menu for adding new objects
	if (ImGui::BeginPopup("HierarchyContextMenu"))
	{
		if (ImGui::MenuItem("Add Cube"))
		{
			BlackJawz::Entity::Entity newEntity = entityManager.CreateEntity();
			entities.push_back(newEntity);

			entityNames[newEntity] = "Cube " + std::to_string(entities.size());

			BlackJawz::Component::Transform transform;
			transformArray.InsertData(newEntity, transform);

			BlackJawz::Component::Geometry cubeGeo = renderer.CreateCubeGeometry();
			BlackJawz::Component::Appearance appearance(cubeGeo);
			appearanceArray.InsertData(newEntity, appearance);

			std::bitset<32> signature;
			signature.set(0);  // Assume component 0 is Transform
			signature.set(1);  // Assume component 1 is Appearance
			entityManager.SetSignature(newEntity, signature);

			transformSystem->AddEntity(newEntity);
			systemManager.SetSignature<BlackJawz::System::TransformSystem>(signature);

			appearanceSystem->AddEntity(newEntity);
			systemManager.SetSignature<BlackJawz::System::AppearanceSystem>(signature);

		}
		if (ImGui::MenuItem("Add Sphere"))
		{
			BlackJawz::Entity::Entity newEntity = entityManager.CreateEntity();
			entities.push_back(newEntity);

			entityNames[newEntity] = "Sphere " + std::to_string(entities.size());

			BlackJawz::Component::Transform transform;
			transformArray.InsertData(newEntity, transform);

			BlackJawz::Component::Geometry sphereGeo = renderer.CreateSphereGeometry();
			BlackJawz::Component::Appearance appearance(sphereGeo);
			appearanceArray.InsertData(newEntity, appearance);

			std::bitset<32> signature;
			signature.set(0);  // Assume component 0 is Transform
			signature.set(1);  // Assume component 1 is Appearance
			entityManager.SetSignature(newEntity, signature);

			transformSystem->AddEntity(newEntity);
			systemManager.SetSignature<BlackJawz::System::TransformSystem>(signature);

			appearanceSystem->AddEntity(newEntity);
			systemManager.SetSignature<BlackJawz::System::AppearanceSystem>(signature);

		}
		if (ImGui::MenuItem("Add Plane"))
		{
			BlackJawz::Entity::Entity newEntity = entityManager.CreateEntity();
			entities.push_back(newEntity);

			entityNames[newEntity] = "Plane " + std::to_string(entities.size());

			BlackJawz::Component::Transform transform;
			transformArray.InsertData(newEntity, transform);

			BlackJawz::Component::Geometry planeGeo = renderer.CreatePlaneGeometry();
			BlackJawz::Component::Appearance appearance(planeGeo);
			appearanceArray.InsertData(newEntity, appearance);

			std::bitset<32> signature;
			signature.set(0);  // component 0 is Transform
			signature.set(1);  // component 1 is Appearance
			entityManager.SetSignature(newEntity, signature);

			transformSystem->AddEntity(newEntity);
			systemManager.SetSignature<BlackJawz::System::TransformSystem>(signature);

			appearanceSystem->AddEntity(newEntity);
			systemManager.SetSignature<BlackJawz::System::AppearanceSystem>(signature);
		}

		ImGui::EndPopup();
	}

	ImGui::End();
}

void BlackJawz::Editor::Editor::ObjectProperties()
{
	ImGui::Begin("Object Properties");

	ImGui::DragFloat3("Camera Position", &cameraPosition.x, 0.1f);
	ImGui::DragFloat("Camera Pitch", &cameraPitch, 0.01f);
	ImGui::DragFloat("Camera Yaw", &cameraYaw, 0.01f);

	if (selectedObject != -1)
	{
		// Get the selected entity
		BlackJawz::Entity::Entity entity = entities[selectedObject];

		// Get the Transform component (you can add more components later)
		BlackJawz::Component::Transform* transform = &transformArray.GetData(entity);
		BlackJawz::Component::Appearance* appearance = &appearanceArray.GetData(entity);

		if (transform)
		{
			ImGui::SeparatorText("Transform");
			// Display the properties of the Transform component
			ImGui::DragFloat3("Position", &transform->position.x, 0.1f);
			ImGui::DragFloat3("Rotation", &transform->rotation.x, 0.1f);
			ImGui::DragFloat3("Scale", &transform->scale.x, 0.1f, 0.0f, 100000.0f);
		}

		if (appearance)
		{
			ImGui::SeparatorText("Appearance");
			int indicesCount = static_cast<int>(appearance->objectGeometry.IndicesCount);
			int stride = static_cast<int>(appearance->objectGeometry.vertexBufferStride);
			int offset = static_cast<int>(appearance->objectGeometry.vertexBufferOffset);
			ImGui::DragInt("Indices", &indicesCount, 0.1f);
			ImGui::DragInt("Stride", &stride, 0.1f);
			ImGui::DragInt("Offset", &offset, 0.1f);
		}
	}

	ImGui::End();
}

void BlackJawz::Editor::Editor::ViewPort(Rendering::Render& renderer)
{
	// Push zero padding style for the viewport window
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	// Begin the ImGui viewport window
	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImVec2 viewportSize = ImGui::GetContentRegionAvail();

	static ImVec2 lastViewportSize = ImVec2(viewportSize.x, viewportSize.y); // Initial size
	if (viewportSize.x != lastViewportSize.x || viewportSize.y != lastViewportSize.y)
	{
		renderer.ResizeRenderTarget(static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));
		editorCamera->SetAspectRatio(viewportSize.x / viewportSize.y);
		lastViewportSize = viewportSize;
	}

	// Pass camera matrices to renderer
	renderer.RenderToTexture(*transformSystem, *appearanceSystem);

	ImGui::Image((ImTextureID)renderer.GetShaderResourceView(), viewportSize);

	// Update matrices
	editorCamera->UpdateViewMatrix();
	editorCamera->UpdateProjectionMatrix();

	ImGui::End(); // End the ImGui viewport window
	ImGui::PopStyleVar(); // Pop the style variable
}