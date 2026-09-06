#pragma once

#include <SDKDDKVer.h>
#include <stdlib.h>
#include <malloc.h>
#include <tchar.h>

// std
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <string>
#include <memory>
#include <algorithm>

// dx11
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxguid.lib")

#ifdef _DEBUG
#pragma comment (lib, "DirectXTKd.lib")
#else
#pragma comment (lib, "DirectXTK.lib")
#endif

#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <d3d11.h>
#include <wrl.h>

// includes
#include "RenderTypes.h"
#include "RenderGlobal.h"

#pragma warning(disable : 4819)
