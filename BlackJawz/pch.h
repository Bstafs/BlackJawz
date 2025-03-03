// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.
#pragma comment(lib, "d3d11.lib")

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#define WIN32_LEAN_AND_MEAN 

#ifndef PCH_H
#define PCH_H

// C++
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <math.h>
#include <queue>
#include <bitset>
#include <array>
#include <unordered_map>
#include <set>
#include <typeindex>
#include <sstream> 
#include <filesystem>
#include <cstdint>
#include <cstring>

// Windows
#include <windows.h>
#include <wrl/client.h>

// DirectX
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <DirectXMath.h>
#include <directxcolors.h>
#include "Util/ID3D11Functions.h"
#include "Util/DDSTextureLoader11.h"
#include "../DirectXTex/DirectXTex.h"

// ImGui
#include "../BlackJawz/ImGui/imgui.h"
#include "../BlackJawz/ImGui/imgui_impl_dx11.h"
#include "../BlackJawz/ImGui/imgui_impl_win32.h"

// FlatBuffers
#undef min
#undef max
#include <flatbuffers/flatbuffers.h>
#include "ecs_generated.h"

#endif //PCH_H

using Microsoft::WRL::ComPtr;
using namespace DirectX;

