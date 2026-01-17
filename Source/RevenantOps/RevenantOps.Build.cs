// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RevenantOps : ModuleRules
{
	public RevenantOps(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"RevenantOps",
			"RevenantOps/Variant_Platforming",
			"RevenantOps/Variant_Platforming/Animation",
			"RevenantOps/Variant_Combat",
			"RevenantOps/Variant_Combat/AI",
			"RevenantOps/Variant_Combat/Animation",
			"RevenantOps/Variant_Combat/Gameplay",
			"RevenantOps/Variant_Combat/Interfaces",
			"RevenantOps/Variant_Combat/UI",
			"RevenantOps/Variant_SideScrolling",
			"RevenantOps/Variant_SideScrolling/AI",
			"RevenantOps/Variant_SideScrolling/Gameplay",
			"RevenantOps/Variant_SideScrolling/Interfaces",
			"RevenantOps/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
