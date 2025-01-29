#include "Editor.h"

BlackJawz::Editor::Editor::Editor()
{
	editorCamera = std::make_unique<BlackJawz::EditorCamera::EditorCamera>(
		60.0f,
		BlackJawz::Application::Application::GetWindowWidth() / BlackJawz::Application::Application::GetWindowHeight(),
		1.0f, 1000.0f
	);
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

	// Render editor components
	MenuBar();          // Menu at the top
	ContentMenu();      // Content browser (dockable)
	Hierarchy(renderer);        // Hierarchy window (dockable)
	ObjectProperties(); // Object properties (dockable)
	ViewPort(renderer); // Viewport (dockable)

	// End rendering frame
	renderer.EndFrame();
}

void BlackJawz::Editor::Editor::MenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Main Menu"))
		{
			if (ImGui::MenuItem("Create New Project"))
			{

			}

			if (ImGui::MenuItem("Open Project"))
			{

			}

			if (ImGui::MenuItem("Save.."))
			{

			}

			if (ImGui::MenuItem("Save as.."))
			{

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

					// Reset the selected object index
					selectedObject = -1;
				}

				//// Remove the object from the list
				//if (objects[i].type == "Cube") 
				//{
				//	//renderer.RenderCube(renderer.GetCubeCount() - 1); // Decrease cube count
				//}
				//else if (objects[i].type == "Sphere") 
				//{
				//	renderer.RenderSphere(renderer.GetSphereCount() - 1); // Decrease sphere count
				//}
				//else if (objects[i].type == "Plane") {
				//	renderer.RenderPlane(renderer.GetPlaneCount() - 1); // Decrease plane count
				//}

				//objects.erase(objects.begin() + i);

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
			//renderer.RenderCube(renderer.GetCubeCount() + 1);
			//objects.push_back({ "Cube" + std::to_string(renderer.GetCubeCount()), "Cube"});

			BlackJawz::Entity::Entity newEntity = entityManager.CreateEntity();
			entities.push_back(newEntity);

			entityNames[newEntity] = "Cube " + std::to_string(entities.size());

			BlackJawz::Component::Transform transform;
			transform.position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			transform.rotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			transform.scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
			transformArray.InsertData(newEntity, transform);

			BlackJawz::Component::Geometry cubeGeo = renderer.CreateCubeGeometry();
			BlackJawz::Component::Appearance appearance(cubeGeo);
			appearanceArray.InsertData(newEntity, appearance);

			std::bitset<32> signature;
			signature.set(0);  // Assume component 0 is Transform
			signature.set(1);  // Assume component 1 is Appearance
			entityManager.SetSignature(newEntity, signature);
		}
		if (ImGui::MenuItem("Add Sphere"))
		{
			//renderer.RenderSphere(renderer.GetSphereCount() + 1);
			//objects.push_back({ "Sphere" + std::to_string(renderer.GetSphereCount()), "Sphere" });

			BlackJawz::Entity::Entity newEntity = entityManager.CreateEntity();
			entities.push_back(newEntity);

			entityNames[newEntity] = "Sphere " + std::to_string(entities.size());

			BlackJawz::Component::Transform transform;
			transform.position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			transform.rotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			transform.scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
			transformArray.InsertData(newEntity, transform);

			BlackJawz::Component::Geometry sphereGeo = renderer.CreateSphereGeometry();
			BlackJawz::Component::Appearance appearance(sphereGeo);
			appearanceArray.InsertData(newEntity, appearance);

			std::bitset<32> signature;
			signature.set(0);  // Assume component 0 is Transform
			signature.set(1);  // Assume component 1 is Appearance
			entityManager.SetSignature(newEntity, signature);
		}
		if (ImGui::MenuItem("Add Plane"))
		{
			//renderer.RenderPlane(renderer.GetPlaneCount() + 1);
			//objects.push_back({ "Plane" + std::to_string(renderer.GetPlaneCount()), "Plane" });

			BlackJawz::Entity::Entity newEntity = entityManager.CreateEntity();
			entities.push_back(newEntity);

			entityNames[newEntity] = "Plane " + std::to_string(entities.size());

			BlackJawz::Component::Transform transform;
			transform.position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			transform.rotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			transform.scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
			transformArray.InsertData(newEntity, transform);

			BlackJawz::Component::Geometry planeGeo = renderer.CreatePlaneGeometry();
			BlackJawz::Component::Appearance appearance(planeGeo);
			appearanceArray.InsertData(newEntity, appearance);

			std::bitset<32> signature;
			signature.set(0);  // Assume component 0 is Transform
			signature.set(1);  // Assume component 1 is Appearance
			entityManager.SetSignature(newEntity, signature);
		}

		ImGui::EndPopup();
	}

	ImGui::End();
}

void BlackJawz::Editor::Editor::ObjectProperties()
{
	ImGui::Begin("Object Properties");

	XMFLOAT3 posX = editorCamera->GetPosition();
	float pitch = editorCamera->GetPitch();
	float yaw = editorCamera->GetYaw();

	ImGui::DragFloat3("Camera Position", &posX.x, 0.1f);
	ImGui::DragFloat("Camera Pitch", &pitch, 0.01f);
	ImGui::DragFloat("Camera Yaw", &yaw, 0.01f);

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
			ImGui::DragFloat3("Scale", &transform->scale.x, 0.1f);
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

	// Update matrices
	editorCamera->UpdateViewMatrix();
	editorCamera->UpdateProjectionMatrix();

	// Pass camera matrices to renderer
	renderer.SetViewMatrix(editorCamera->GetViewMatrix());
	renderer.SetProjectionMatrix(editorCamera->GetProjectionMatrix());

	renderer.RenderToTexture();

	ImGui::Image((ImTextureID)renderer.GetShaderResourceView(), viewportSize);

	ImGui::End(); // End the ImGui viewport window
	ImGui::PopStyleVar(); // Pop the style variable
}