#include "ClientPch.h"
#include "ClientGlobal.h"
#include "EngineGlobal.h"

#include "ComponentRegistry.h"

#include "Dice.h"
#include "DiceCup.h"

bool GRunning = true;

void InitGame()
{
    GameEngine::InitEngine("./Bin/Resource/Shader/", wnd);

    // ComponentRegistry
    GameEngine::ComponentRegistry::Register<Dice>("Dice");
    GameEngine::ComponentRegistry::Register<DiceCup>("DiceCup");
}

void ReleaseGame()
{
    GameEngine::ReleaseEngine();
}
