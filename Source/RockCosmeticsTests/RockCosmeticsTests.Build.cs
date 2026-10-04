// Copyright Broken Rock Studios LLC. All Rights Reserved.

using UnrealBuildTool;

// Automation tests for RockCosmetics. Developer module: not built for Shipping, so CQTest never leaks into a shipped target.
public class RockCosmeticsTests : ModuleRules
{
	public RockCosmeticsTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"NetCore",
				"CQTest",
				"CustomizableObject",
				"RockCosmetics",
			}
		);
	}
}
