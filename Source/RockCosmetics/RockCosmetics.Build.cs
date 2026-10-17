// Copyright Broken Rock Studios LLC. All Rights Reserved.
// See the LICENSE file for details.

using UnrealBuildTool;

public class RockCosmetics : ModuleRules
{
	public RockCosmetics(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"DeveloperSettings",
				"GameplayTags",
				"ModularGameplay",
				"NetCore",
				"CustomizableObject", 
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
			}
		);

		// Registers this module's fast arrays (character parts, Mutable layers) with Iris.
		SetupIrisSupport(Target);
	}
}
