// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "SplineEditorStyle.h"

class FSplineEditorCommands : public TCommands<FSplineEditorCommands>
{
public:

	FSplineEditorCommands()
		: TCommands<FSplineEditorCommands>(TEXT("SplineEditor"), NSLOCTEXT("Contexts", "SplineEditor", "SplineEditor Plugin"), NAME_None, FSplineEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};