// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Project_GM : ModuleRules
{
	public Project_GM(ReadOnlyTargetRules Target) : base(Target)
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
			"Project_GM",
			"Project_GM/Variant_Platforming",
			"Project_GM/Variant_Platforming/Animation",
			"Project_GM/Variant_Combat",
			"Project_GM/Variant_Combat/AI",
			"Project_GM/Variant_Combat/Animation",
			"Project_GM/Variant_Combat/Gameplay",
			"Project_GM/Variant_Combat/Interfaces",
			"Project_GM/Variant_Combat/UI",
			"Project_GM/Variant_SideScrolling",
			"Project_GM/Variant_SideScrolling/AI",
			"Project_GM/Variant_SideScrolling/Gameplay",
			"Project_GM/Variant_SideScrolling/Interfaces",
			"Project_GM/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
