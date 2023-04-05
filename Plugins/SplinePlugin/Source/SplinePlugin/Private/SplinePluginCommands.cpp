// Copyright Epic Games, Inc. All Rights Reserved.

#include "SplinePluginCommands.h"

#define LOCTEXT_NAMESPACE "FSplinePluginModule"

void FSplinePluginCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "SplinePlugin", "Execute SplinePlugin action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
