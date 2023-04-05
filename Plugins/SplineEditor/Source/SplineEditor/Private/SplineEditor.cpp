// Copyright Epic Games, Inc. All Rights Reserved.

#include "SplineEditor.h"
#include "SplineEditorStyle.h"
#include "SplineEditorCommands.h"
#include "LevelEditor.h"
#include "SSplineEditorMenu.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName SplineEditorTabName("SplineEditor");

#define LOCTEXT_NAMESPACE "FSplineEditorModule"

void FSplineEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FSplineEditorStyle::Initialize();
	FSplineEditorStyle::ReloadTextures();

	FSplineEditorCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FSplineEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FSplineEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSplineEditorModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SplineEditorTabName,
	                                                  FOnSpawnTab::CreateRaw(
		                                                  this, &FSplineEditorModule::OnSpawnPluginTab))
	                        .SetDisplayName(LOCTEXT("FSplineEditorTabTitle", "SplineEditor"))
	                        .SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FSplineEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FSplineEditorStyle::Shutdown();

	FSplineEditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SplineEditorTabName);
}

TSharedRef<SDockTab> FSplineEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SSplineEditorMenu)
		];
}

void FSplineEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(SplineEditorTabName);
}

void FSplineEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FSplineEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(
					FToolMenuEntry::InitToolBarButton(FSplineEditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSplineEditorModule, SplineEditor)
