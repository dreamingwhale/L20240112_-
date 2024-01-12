// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ShootingCodeGame : ModuleRules
{
	public ShootingCodeGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "ItemPlugin", "OnlineSubsystem", "OnlineSubsystemUtils" });

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");

		PublicIncludePaths.AddRange(new string[] { "ShootingCodeGame" });
    }
}
