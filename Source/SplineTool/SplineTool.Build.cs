// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SplineTool : ModuleRules
{
	public SplineTool(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		MinFilesUsingPrecompiledHeaderOverride = 1;
		bUseUnity = false;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"UnrealEd"
		});
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"InputCore",
			"SlateCore",
			"PropertyEditor",
			"LevelEditor"
		});

		PrivateIncludePaths.AddRange(new string[]
		{
			"SplineTool/Private"
		});
		PublicIncludePaths.AddRange(new string[]
		{
			"SplineTool/Public"
		});


		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}