#pragma warning(push,1)

#define NTDDI_VERSION NTDDI_WIN7
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <comdef.h>
#include <D3D11.h>
#include <D3DCompiler.inl>
#include <DirectXMath.h>

#include <wrl\client.h>
using Microsoft::WRL::ComPtr;

#include <cassert>
#include <cstdio>
#include <memory>
#include <unordered_map>
#include <string>
#include <array>

#include <Engine.h>
#include <RenderPrivate.h>

#pragma warning(pop)