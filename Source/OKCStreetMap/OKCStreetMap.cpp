// Copyright 2024 OKC Street Map Project. All Rights Reserved.

#include "OKCStreetMap.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogOKCStreetMap);

void FOKCStreetMapModule::StartupModule()
{
    UE_LOG(LogOKCStreetMap, Log, TEXT("OKC Street Map Module Started"));
}

void FOKCStreetMapModule::ShutdownModule()
{
    UE_LOG(LogOKCStreetMap, Log, TEXT("OKC Street Map Module Shutdown"));
}

IMPLEMENT_PRIMARY_GAME_MODULE(FOKCStreetMapModule, OKCStreetMap, "OKCStreetMap");
