// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Projekt_UE5 : ModuleRules
{
	public Projekt_UE5(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
