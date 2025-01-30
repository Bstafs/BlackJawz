#include "EditorCamera.h"

BlackJawz::EditorCamera::EditorCamera::EditorCamera(float fov, float aspectRation, float nearPlane, float farPlane)
	: cameraPosition({ 0.0f, 0.0f, -5.0f }), 
	  cameraPitch(0.0f), 
	  cameraYaw(0.0f), 
	  cameraFOV(fov), 
	  cameraAspectRatio(aspectRation),
	  cameraNearPlane(nearPlane),
	  cameraFarPlane(farPlane)
{  
	UpdateViewMatrix();
	UpdateProjectionMatrix();
}


void BlackJawz::EditorCamera::EditorCamera::UpdateViewMatrix()
{
	XMVECTOR position = XMLoadFloat3(&cameraPosition);
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(cameraPitch, cameraYaw, 0.0f);

	// Forward direction
	XMVECTOR forward = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), rotation);
	XMVECTOR lookAt = position + forward;

	// Up direction
	XMVECTOR up = XMVector3TransformCoord(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotation);

	XMStoreFloat4x4(&viewMatrix, XMMatrixLookAtLH(position, lookAt, up));
}

void BlackJawz::EditorCamera::EditorCamera::UpdateProjectionMatrix()
{
	XMMATRIX projection = XMMatrixPerspectiveFovLH(
		XM_PIDIV2,
		cameraAspectRatio,
		cameraNearPlane,
		cameraFarPlane
	);
	XMStoreFloat4x4(&projectionMatrix, projection);
}