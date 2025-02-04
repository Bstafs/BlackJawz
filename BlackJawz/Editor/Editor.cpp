#include "Editor.h"

extern const std::filesystem::path filePath = std::filesystem::current_path();

BlackJawz::Editor::Editor::Editor() : currentPath(filePath)
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
	ContentMenu(renderer);      // Content browser (dockable)
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

std::vector<uint8_t> ExtractTextureData(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv)
{
	// Get the resource from the SRV.
	ComPtr<ID3D11Resource> resource;
	srv->GetResource(resource.GetAddressOf());

	// Ensure it's a 2D texture.
	ComPtr<ID3D11Texture2D> texture;
	HRESULT hr = resource.As(&texture);
	if (FAILED(hr))
		return {};

	// Use DirectXTex to capture the texture
	DirectX::ScratchImage image;
	hr = DirectX::CaptureTexture(device, context, texture.Get(), image);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to capture texture.\n");
		return {};
	}

	// Convert the image to a DDS memory buffer
	DirectX::Blob ddsBlob;
	hr = DirectX::SaveToDDSMemory(*image.GetImage(0, 0, 0), DirectX::DDS_FLAGS_NONE, ddsBlob);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to save DDS to memory.\n");
		return {};
	}

	// Return the DDS data as a vector
	std::vector<uint8_t> ddsData((uint8_t*)ddsBlob.GetBufferPointer(),
		(uint8_t*)ddsBlob.GetBufferPointer() + ddsBlob.GetBufferSize());
	return ddsData;
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
		// Update the world matrix before serializing.
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

		auto textureData = ExtractTextureData(renderer.GetDevice(), renderer.GetDeviceContext(), appearanceComp.GetTexture().Get());
		auto textureVec = builder.CreateVector(textureData);
		auto textureOffset = ECS::CreateTexture(builder, textureVec);

		auto appearanceOffset = ECS::CreateAppearance(builder, geometryOffset, textureOffset);

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

// Function to load texture from DDS data into SRV
ComPtr<ID3D11ShaderResourceView> BlackJawz::Editor::Editor::LoadTextureFromDDSData(ID3D11Device* device, const std::vector<uint8_t>& textureData)
{
	ComPtr<ID3D11Resource> resource;
	ComPtr<ID3D11ShaderResourceView> srv;

	HRESULT hr = CreateDDSTextureFromMemory(
		device,
		textureData.data(),
		textureData.size(),
		resource.GetAddressOf(),  // Resource is optional but helps debugging
		srv.ReleaseAndGetAddressOf()
	);

	if (FAILED(hr))
	{
		char errorMsg[256];
		snprintf(errorMsg, sizeof(errorMsg), "CreateDDSTextureFromMemory failed! HRESULT: 0x%08X\n", hr);
		OutputDebugStringA(errorMsg);
		return nullptr;
	}

	return srv;
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

			BlackJawz::Component::Geometry geometry;
			geometry.IndicesCount = geometryData->indices_count();
			geometry.vertexBufferStride = geometryData->vertex_buffer_stride();
			geometry.vertexBufferOffset = geometryData->vertex_buffer_offset();
			geometry.pVertexBuffer = vertexBuffer;
			geometry.pIndexBuffer = indexBuffer;

			// Loading the texture SRV from the DDS data in the Appearance
			auto textureData = entityData->appearance()->texture();

			// Convert FlatBuffer vector to a standard vector
			std::vector<uint8_t> textureBufferData(textureData->dds_data()->begin(), textureData->dds_data()->end());

			ComPtr<ID3D11ShaderResourceView> textureSRV = LoadTextureFromDDSData(renderer.GetDevice(), textureBufferData);

			// Save geometry data and buffers in appearance
			BlackJawz::Component::Appearance appearance(geometry, textureSRV.Get());
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
	// Static buffer for the scene file name, scenes are default to save in Scenes/...
	static char sceneNameBuffer[256] = "ecs.bin";

	// Flags to indicate whether the save or manual load popup should be displayed.
	static bool openSavePopup = false;
	static bool openLoadPopup = false;

	// Main Menu Bar
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Create New Project.."))
			{
				// Create New Project functionality 
			}

			if (ImGui::MenuItem("Open Project.."))
			{
				// Open Project functionality 
			}

			if (ImGui::MenuItem("Save Project.."))
			{
				// Save Project functionality 
			}

			if (ImGui::MenuItem("Save Project as.."))
			{
				// Save Project as functionality 
			}

			if (ImGui::MenuItem("Save Scene.."))
			{
				openSavePopup = true;
				strcpy_s(sceneNameBuffer, "ecs"); // Reset to default.
			}

			// Load Scene submenu
			if (ImGui::BeginMenu("Load Scene"))
			{
				// Submenu: Load From List
				if (ImGui::BeginMenu("Load From Files"))
				{
					try
					{
						// Iterate over all files in the "Scenes" folder.
						for (const auto& entry : std::filesystem::directory_iterator("Scenes"))
						{
							// Get just the filename (not the full path).
							std::string fileName = entry.path().filename().string();
							if (ImGui::MenuItem(fileName.c_str()))
							{
								// Build the full path and load the scene.
								std::string fullPath = "Scenes/" + fileName;
								LoadScene(fullPath, renderer);
							}
						}
					}
					catch (const std::filesystem::filesystem_error& e)
					{
						// If the folder doesn't exist or there is an error, show an error item.
						ImGui::MenuItem("Error reading Scenes folder");
					}
					ImGui::EndMenu();
				}

				// Submenu: Load From Name
				if (ImGui::MenuItem("Load From Filename"))
				{
					openLoadPopup = true;
					strcpy_s(sceneNameBuffer, "ecs.bin"); // Reset to default
				}

				ImGui::EndMenu(); // End Load Scene submenu.
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

	// --- Save Scene Popup Modal ---
	if (openSavePopup)
	{
		ImGui::OpenPopup("Save Scene");
		openSavePopup = false;
	}
	if (ImGui::BeginPopupModal("Save Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Enter file name to save scene (only file name):");
		// Display "Scenes/" as a non-editable label.
		ImGui::Text("Scenes/");
		ImGui::SameLine();
		ImGui::InputText("##SaveFilename", sceneNameBuffer, IM_ARRAYSIZE(sceneNameBuffer));

		if (ImGui::Button("Save", ImVec2(120, 0)))
		{
			// Prepend "Scenes/" to the file name before saving.
			std::string fullPath = "Scenes/" + std::string(sceneNameBuffer) + ".bin";
			SaveScene(fullPath, renderer);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	// --- Load Scene Popup Modal for Manual Entry (Load From Name) ---
	if (openLoadPopup)
	{
		ImGui::OpenPopup("Load Scene by Name");
		openLoadPopup = false;
	}
	if (ImGui::BeginPopupModal("Load Scene by Name", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Enter file name to load scene (only file name):");
		ImGui::Text("Scenes/");
		ImGui::SameLine();
		ImGui::InputText("##LoadFilename", sceneNameBuffer, IM_ARRAYSIZE(sceneNameBuffer));

		if (ImGui::Button("Load", ImVec2(120, 0)))
		{
			std::string fullPath = "Scenes/" + std::string(sceneNameBuffer);
			LoadScene(fullPath, renderer);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void BlackJawz::Editor::Editor::ContentMenu(Rendering::Render& renderer)
{
	ImGui::Begin("Content Browser");

	if (!std::filesystem::equivalent(filePath, currentPath) && ImGui::Button("<--"))
	{
		currentPath = currentPath.parent_path();
	}

	ImGui::SameLine();
	ImGui::TextUnformatted(currentPath.string().c_str());

	constexpr float padding = 16.0f, thumbnailSize = 64.0f;
	float cellSize = thumbnailSize + padding;
	int columnCount = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / cellSize));

	ImGui::Columns(columnCount, nullptr, false);

	int i = 0;
	for (auto& entry : std::filesystem::directory_iterator(currentPath))
	{
		ImGui::PushID(i++);

		auto path = entry.path();
		auto relativePath = std::filesystem::relative(path, filePath);
		auto filename = relativePath.filename().string();
		auto icon = entry.is_directory() ? directoryIcon : fileIcon;

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::ImageButton("##icon", (ImTextureID)icon, { thumbnailSize, thumbnailSize }, { 0,1 }, { 1,0 });
		ImGui::PopStyleColor();

		if (ImGui::BeginDragDropSource())
		{
			const wchar_t* contentPath = relativePath.c_str();
			ImGui::SetDragDropPayload("ContentItem", contentPath, (wcslen(contentPath) + 1) * sizeof(wchar_t));
			ImGui::EndDragDropSource();
		}

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			if (entry.is_directory())
			{
				currentPath /= path.filename();
			}
			else if (entry.is_regular_file() && path.extension() == ".bin") {
				LoadScene(relativePath.string(), renderer);
			}
			std::cout << relativePath << std::endl;
		}

		ImGui::TextWrapped(filename.c_str());
		ImGui::NextColumn();
		ImGui::PopID();
	}

	ImGui::Columns(1);
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

			ID3D11ShaderResourceView* tex;
			CreateDDSTextureFromFile(renderer.GetDevice(), L"Textures\\bricks.dds", nullptr, &tex);
			BlackJawz::Component::Appearance appearance(cubeGeo, tex);
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

			ID3D11ShaderResourceView* tex;
			CreateDDSTextureFromFile(renderer.GetDevice(), L"Textures\\bricks.dds", nullptr, &tex);



			BlackJawz::Component::Appearance appearance(sphereGeo, tex);
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

			ID3D11ShaderResourceView* tex;
			CreateDDSTextureFromFile(renderer.GetDevice(), L"Textures\\tile.dds", nullptr, &tex);

			BlackJawz::Component::Appearance appearance(planeGeo, tex);
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
		if (ImGui::MenuItem("Add Light"))
		{
			BlackJawz::Entity::Entity newEntity = entityManager.CreateEntity();
			entities.push_back(newEntity);
			entityNames[newEntity] = "Light " + std::to_string(entities.size());
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