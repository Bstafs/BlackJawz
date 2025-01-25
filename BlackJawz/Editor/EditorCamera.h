#pragma once
#include "../pch.h"
#include "../Windows/Application.h"

namespace BlackJawz::EditorCamera
{
	class EditorCamera
	{
	public:
		EditorCamera(float fov, float aspectRatio, float nearPlane, float farPlane);

		XMFLOAT4X4 GetViewMatrix() const { return viewMatrix; }
		XMFLOAT4X4 GetProjectionMatrix() const { return projectionMatrix; }

		void SetPosition(XMFLOAT3 pos) { cameraPosition = pos; }
		void SetRotation(float pitch, float yaw) { cameraPitch = pitch, cameraYaw = yaw; }
		void SetAspectRatio(float ar) { cameraAspectRatio = ar; }

		XMFLOAT3 GetPosition() { return cameraPosition; }
		float GetPitch() { return cameraPitch; }
		float GetYaw() { return cameraYaw; }

		void UpdateViewMatrix();
		void UpdateProjectionMatrix();

	private:
		
		XMFLOAT3 cameraPosition;
		float cameraPitch;
		float cameraYaw;

		float cameraFOV;
		float cameraAspectRatio;
		float cameraNearPlane;
		float cameraFarPlane;

		XMFLOAT4X4 viewMatrix;
		XMFLOAT4X4 projectionMatrix;
	};
}