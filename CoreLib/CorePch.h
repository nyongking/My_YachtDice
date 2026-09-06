#pragma once
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#define NOMINMAX

#include <iostream>
#include <memory>
#include <Windows.h>
#include <functional>
#include <typeindex>

#pragma warning (disable : 4251)
#pragma warning (disable : 4101)

#include "Types.h"
#include "CoreMacro.h"
#include "CoreGlobal.h"
#include "Container.h"
#include "CoreTLS.h"

#include "Lock.h"
#include "Memory.h"
#include "ObjectPool.h"
#include "TypeCast.h"