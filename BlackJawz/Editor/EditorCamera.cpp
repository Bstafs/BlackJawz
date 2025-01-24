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
    // Calculate the camera's direction vector
    XMVECTOR forward = XMVector3Normalize(
        XMVectorSet(
            cosf(cameraYaw) * cosf(cameraPitch),
            sinf(cameraPitch),
            sinf(cameraYaw) * cosf(cameraPitch),
            0.0f
        ));

    // Calculate right and up vectors
    XMVECTOR right = XMVector3Normalize(XMVector3Cross(forward, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
    XMVECTOR up = XMVector3Cross(right, forward);

    // Calculate the view matrix
    XMVECTOR position = XMLoadFloat3(&cameraPosition);
    XMMATRIX view = XMMatrixLookToLH(position, forward, up);
    XMStoreFloat4x4(&viewMatrix, view);
}

void BlackJawz::EditorCamera::EditorCamera::UpdateProjectionMatrix()
{
    XMMATRIX projection = XMMatrixPerspectiveFovLH(cameraFOV, cameraAspectRatio, cameraNearPlane, cameraFarPlane);
    XMStoreFloat4x4(&projectionMatrix, projection);
}