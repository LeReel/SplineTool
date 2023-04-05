// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "SplinePluginStyle.h"

class FSplinePluginCommands : public TCommands<FSplinePluginCommands>
{
public:

	FSplinePluginCommands()
		: TCommands<FSplinePluginCommands>(TEXT("SplinePlugin"), NSLOCTEXT("Contexts", "SplinePlugin", "SplinePlugin Plugin"), NAME_None, FSplinePluginStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
