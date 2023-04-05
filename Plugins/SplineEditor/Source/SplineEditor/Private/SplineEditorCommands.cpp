// Copyright Epic Games, Inc. All Rights Reserved.

#include "SplineEditorCommands.h"

#define LOCTEXT_NAMESPACE "FSplineEditorModule"

void FSplineEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "SplineEditor", "Bring up SplineEditor window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
