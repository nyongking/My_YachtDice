#pragma once

#include "CorePch.h"
#include "RenderPch.h"
#include "ServerCorePch.h"
#include "GameEnginePch.h"

#include <SDKDDKVer.h>
#include <stdlib.h>
#include <malloc.h>
#include <tchar.h>


#pragma warning(push)
#pragma warning(disable: 4251 4244 4267)
#include "YachtDice.pb.h"
#pragma warning(pop)

#include "PacketId.h"
#include "PacketHelper.h"

#include "ClientGlobal.h"
#include "CoreGlobal.h"